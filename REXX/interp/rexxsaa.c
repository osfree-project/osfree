#ifndef lint
static char *RCSid = "$Id: rexxsaa.c,v 1.2 2003/12/11 04:43:17 prokushev Exp $";
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

/*
 * This file implements the client part of the SAA API when Regina
 * is linked into a program using SAA API. There is one routine for
 * each of the functions in SAA API, and the functionality is partly
 * implemented here, and partly by calling subroutines in Regina.
 * Note that the interface to Regina is as simple as possible, so that
 * a multitude of different transport mechanisms can be used (although
 * normal linking is probably the most common.
 *
 * The following SAA API functions is defined in this source file:
 *
 *    RexxStart()               --- execute Rexx code
 *    RexxRegisterSubcomExe()   --- register subcommand handler
 *    RexxRegisterSubcomDll()   --- ditto (not yet implemented)
 *    RexxQuerySubcom()         --- query subcommand handler
 *    RexxDeregisterSubcom()    --- deregister subcommand handler
 *    RexxVariablePool()        --- handle Rexx variable manipulation
 *    RexxRegisterExitExe()     --- register exit handler
 *    RexxRegisterExitDll()     --- ditto (not yet implemented)
 *    RexxDeregisterExit()      --- deregister exit handler
 *    RexxQueryExit()           --- query exit handler
 *    RexxRegisterFunctionExe() --- register external function handler
 *    RexxRegisterFunctionDll() --- ditto (from dynamic library)
 *    RexxQueryFunction()       --- query external function
 *    RexxDeregisterFunction()  --- deregister external function
 *    RexxSetHalt()             --- set Halt and Trace
 *    RexxCreateQueue()         --- create named queued
 *    RexxDeleteQueue()         --- delete named queued
 *    RexxQueryQueue()          --- query named queued
 *    RexxAddQueue()            --- add line to named queued
 *    RexxPullQueue()           --- pull line from named queued
 *    RexxAddMacro()            --- add a macro to macrospace
 *    RexxClearMacroSpace()     --- remove all macros from macrospace
 *    RexxDropMacro()           --- remove macro from macrospace
 *    RexxLoadMacroSpace()      --- load macrospace macros from file
 *    RexxQueryMacro()          --- find a macro's search order
 *    RexxReorderMacro()        --- change the search order for a macro
 *    RexxSaveMacroSpace()      --- save macrospace to file
 *
 * These functions are Regina extensions
 *    RexxFreeMemory()          --- free memory allocated by Rexx API
 *    RexxAllocateMemory()      --- allocate memory to be freed by Rexx API
 *    RexxCallBack()            --- execute an internal procedure within the running script
 *
 * These functions are all defined in the doc for SAA API. In addition,
 * a number of calls in Regina are called, as well as a number of calls
 * are defined for use by Regina. These all start with the prefix Ifc.
 * First the one defined in rexxsaa.c, which can be called from other
 * parts of Regina:
 *
 *    IfcSubCmd()               --- invoke a subcommand
 *    IfcDoExit()               --- invoke a system exit handler
 *    IfcExecFunc()             --- invoke an external function handler
 *    IfcExecFuncDll()          --- invoke an external function handler in a DLL
 *
 * Then the functions which are defined elsewhere, which can be called
 * by this source code:
 *
 *    IfcExecScript()           --- start to execute Rexx code
 *    IfcExecCallBack()         --- start to execute Rexx procedure
 *    IfcVarPool()              --- handle a variable manipulation request
 *    IfcRegFunc()              --- register an external function name
 *    IfcDelFunc()              --- deregister an external function name
 *    IfcQueryFunc()            --- queries an external function name
 *    IfcAllocateMemory()       --- allocate memory for API user
 *    IfcFreeMemory()           --- free memory from API user
 *    IfcDeleteQueue()          --- delete queue
 *    IfcAddQueue()             --- add a line to the queue
 *    IfcPullQueue()            --- pull a line off the queue
 *
 * All these routines are properly defined in the documentation for
 * Regina. Other than the functions listed, the code in this file has
 * been isolated as far as possible, and no functions specific to
 * Regina is used, not even for memory allocation.
 */

/*
 * We need to define these symbols in order to get the proper macros,
 * datatypes, and declaration when including rexxsaa.h.
 */
#define INCL_RXSHV
#define INCL_RXSUBCOM
#define INCL_RXFUNC
#define INCL_RXSYSEXIT
#define INCL_RXARI
#define INCL_RXQUEUE

#include "rexx_charset.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
/*
 * The rexxsaa.h header file defines the interface between this file and
 * the client program which uses SAA API. The rxiface.h header file
 * defines the interface between this file and Regina.
 */
#include "configur.h"
/*
 * The following #define __REGINA_INTERNAL stops an error with MingW32
 *
 */
#define __REGINA_INTERNAL
#include "rexxsaa.h"
#include "defs.h"
#define DONT_TYPEDEF_PFN
#define RXLIB
#include "rexx.h"
#if defined(DYNAMIC) && defined(HAVE_GCI)
# include "gci/gci.h"
#endif
#include "rxiface.h"
#include "extstack.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>

#if defined(__EPOC32__) || defined(__WINS__)
# ifdef APIRET
#  undef APIRET
# endif
# define APIRET unsigned long
# ifdef APIENTRY
#  undef APIENTRY
# endif
#else
# define EXPORT_C
#endif

typedef union {
   RXFNCCAL_PARM fnccal ;
   RXCMDHST_PARM cmdhst ;
   RXMSQPLL_PARM msqpll ;
   RXMSQPSH_PARM msqpsh ;
   RXMSQSIZ_PARM msqsiz ;
   RXMSQNAM_PARM msqnam ;
   RXSIOSAY_PARM siosay ;
   RXSIOTRC_PARM siotrc ;
   RXSIOTRD_PARM siotrd ;
   RXSIODTR_PARM siodtr ;
   RXHLTTST_PARM hlttst ;
   RXTRCTST_PARM trctst ;
   RXENVGET_PARM envget ;
   RXENVSET_PARM envset ;
   RXCWDGET_PARM cwdget ;
   RXCWDSET_PARM cwdset ;
} EXIT ;

struct funcbox2 {
   struct funcbox2 *next, *prev ;
   PSZ name ;
   RexxFunctionHandler *entry ;
   unsigned hash ;
};

/* The following value allows called programs to call "free" to the return
 * parameters without destroying our stack.
 */
#define ILLEGAL_USE_SIZE (8 * sizeof(void *))

typedef struct { /* rex_tsd: static variables of this module (thread-safe) */
   struct funcbox2 *    saafuncs[133];
   struct ExitHandlers *CurrentHandlers ;
   struct EnvBox *      FirstEnv ;
   struct EnvBox *      FirstExit ;
} rex_tsd_t; /* thread-specific but only needed by this module. see
              * init_rexxsaa
              */

#define EXT_FUNCS_COUNT (sizeof(rt->saafuncs) / sizeof(rt->saafuncs[0]))

/*
 * The struct EnvBox datatype holds the definition of any subcommand
 * handler (i.e. an 'environment'). It is intended as an double-linked
 * list of entries, though performence concerns may force a change in
 * this structure later. It contains the name of the environment, and
 * an eight byte dataarea of user defined data. The same datastructure
 * is also used for holding the symbol table of external functions.
 *
 * This may prove a problem in the future, since the number of external
 * functions are generally much larger than the number of subcommand
 * handlers. Thus, different datastructures may be necessary to acheive
 * maximum performance.
 */
struct EnvBox
{
   struct EnvBox *prev, *next ;    /* double linked list pointers */
   char *EnvName ;                 /* environment/function name */
   unsigned hash ;                 /* hash value of the EnvName */
   unsigned char UserData[8] ;     /* user defined data area */
   union {
     PFN EntryPnt ;                /* external function entry point */
     RexxSubcomHandler *SubCom ;   /* subcommand handler entry point */
   } u ;
} ;

struct ExitHandlers
{
   RexxExitHandler *(Handlers[RXNOOFEXITS]) ; /* for RexxRegisterExitExe */
   struct ExitHandlers *prev ;
} ;



/*
 * The following MAP_TYPE() macro maps from the SAA API macros holding
 * the type of an invocation (function, subroutine or command), to its
 * equivalent value in the internal interface of Regina (as defined in
 * the file rxiface.h
 */
#define MAP_TYPE(a) ((a)==RXCOMMAND ? RX_TYPE_COMMAND : \
              (a)==RXFUNCTION ? RX_TYPE_FUNCTION : RX_TYPE_SUBROUTINE)


/* init_rexxsaa initializes the module.
 * Currently, we set up the thread specific data.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_rexxsaa( tsd_t *TSD )
{
   rex_tsd_t *rt;

   if (TSD->rex_tsd != NULL)
      return(1);

   if ((rt = TSD->rex_tsd = MallocTSD(sizeof(rex_tsd_t))) == NULL)
      return(0);
   memset(rt,0,sizeof(rex_tsd_t));  /* correct for all values */
   return(1);
}

/* deinit_rexxsaa deinitializes the module and frees used memory blocks not
 * allocated by the Malloc-Interface. There isn't anything to do currently.
 */
void deinit_rexxsaa( tsd_t *TSD )
{
}


/* StartupInterface initializes the Rexx system once per thread. Values like
 * like __regina_get_tsd()->systeminfo are set. The true purpose of this
 * function is to create an environment which allows the run of the
 * interpreter. This is exactly the case when systeminfo exists. The last
 * systeminfo is hopefully never deleted but that won't do any harm if we
 * reinitialize the Rexx environment.
 *
 * There is a three-stage step to let a Rexx program run:
 * 1) Initialize the runtime system and programming environment. This is
 *    done by __regina_get_tsd() done in GLOBAL_ENTRY_POINT().
 *    After this call you may use Malloc() and friends: basic things.
 * 2) Initialize the Rexx system. This is done by
 *    __regina_faked_main() here or directly by any other caller of
 *    __regina_faked_main() or main():
 *    You are then allowed to access variables from the variable pool, load
 *    a Rexx program or just call a Rexx API function.
 *    Hint: The detection of this step is done as follows:.
 *    Rexx_system_is_running = (__regina_get_tsd()->systeminfo != NULL);
 *    or a similar compare.
 * 3) Load the Rexx program in memory.
 *    Although you don't know, what program is currently loaded, you can
 *    check if a program is loaded (and running) by checking:
 *    Program_running = (__regina_get_tsd()->systeminfo->tree.root != NULL);
 *    This step is done in RexxStart or by main()/__regina_faked_main() when
 *    called as a program.
 * Never use __regina_get_tsd() when it is not needed, of course.
 * This function should be called as GLOBAL_ENTRY_POINT() at the very
 * start of the interpreter but after GLOBAL_ENTRY_POINT.
 */
static void StartupInterface(tsd_t *TSD)
{
   /* The following value won't change and is const */
   static char *args[] = { "regina", "-Ci,foo", NULL } ;

   if (TSD->systeminfo != NULL)
      return;

   __regina_faked_main( 2, args ) ;
}


/*
 * FillReq prepares a String with a given Length to be exported to an
 * external application.
 * The FillReq() function takes as parameter a pointer to a VarPool()
 * request structure variable, and the definition of a string, and
 * fill the content of the string into the request block. Note that the
 * third parameter is gobbled up, so it can not be used or released by
 * the calling function afterwards. Also, there are two macros defined,
 * which gives a better access to the contents of the function
 */
#define FillReqName(a,b,c) FillReq(a,b,c,1)
#define FillReqValue(a,b,c) FillReq(a,b,c,0)

static void FillReq( PSHVBLOCK Req, ULONG Length, const char *String, int name )
{
   RXSTRING *string;
   const ULONG *strlen;

   string = name ? &Req->shvname : &Req->shvvalue;
   strlen = name ? &Req->shvnamelen : &Req->shvvaluelen;

   /*
    * If the string is undefined, set ->strptr to NULL. It is not required
    * that the lengths parameters are set to zero, but I'll do this as
    * nice gest to the users; someone is probably going to believe that
    * this is how the data is returned.
    * shvnamelen and shvvaluelen are read-only values describing the maximum
    * size of the destination buffer.
    */
   if ( Length == RX_NO_STRING )
   {
      MAKERXSTRING( *string, NULL, 0 );
      return;
   }

   /*
    * If a string was supplied, use it, else allocate sufficient space.
    * The then part of the if will just copy the data to the user-supplied
    * return string data area, noting a truncation is one occurred.
    */
   if ( RXSTRPTR( *string ) )
   {
      /*
       * We need a terminator, therefore we need one byte more for allocation.
       * We may come to the funny situation indicating a truncation but have
       * copied all bytes from the string.
       */
      if ( *strlen <= Length )
      {
          Req->shvret |= RXSHV_TRUNC;
          Length = *strlen;
      }
      else
      {
         string->strptr[Length] = '\0';
      }
      memcpy(string->strptr, String, Length );
      string->strlength = Length;
   }
   else
   {
      /*
       * The else part of the if will allocate new space for the data, and
       * fills in the data, or return a memory fault if data could not
       * properly be allocated.
       *
       * We have to ASCII0-terminate the string silently
       */
      string->strptr = (char *)IfcAllocateMemory( Length + 1 );
      if ( string->strptr )
      {
         if ( Length )
            memcpy( string->strptr, String, Length );
         string->strptr[Length] = '\0';
         string->strlength = Length;
      }
      else
         Req->shvret |= RXSHV_MEMFL;
   }
}

/* ========================================================================
 * Here starts the section for maintaining the list of environments
 * supported by this environment. There are several routines using
 * the functions in this section, the routines defined here are:
 *
 *  FindEnv()  --- retrieves a pointer to a environment box.
 *
 * Actually, it used to be more, one to insert and one to delete.
 * However, in order to save code, these was moved into the routines
 * where they were called (they were used only once). The functions
 * into which the code was moved are RexxRegisterSubcomExe(), and
 * RexxDeregisterSubcom(). To improve modularization, and to
 * ease the introduction of a new datastructure, the code should
 * probably be extracted and inserted in this section.
 */


#define BOX_IS_ENVIR 0
#define BOX_IS_EXIT 1

/*
 * Find a particular environment, and return a pointer to a struct
 * containing information about that environment. If it is not found,
 * a pointer to NULL is returned.
 */
#define FindEnvir(a,b) FindBox(TSD,a,b,BOX_IS_ENVIR)
#define FindExit(a,b) FindBox(TSD,a,b,BOX_IS_EXIT)

static struct EnvBox *FindBox( const tsd_t *TSD, const char *Env, int EnvLen, int type )
{
   struct EnvBox *bptr;
   rex_tsd_t *rt;
   unsigned h;

   rt = TSD->rex_tsd;
   bptr = ((type==BOX_IS_ENVIR) ? rt->FirstEnv : rt->FirstExit) ;
   assert( Env ) ;
   h = hashvalue(Env, EnvLen);
   for (; bptr; bptr=bptr->next)
   {
      if (h != bptr->hash)
         continue;
      if (memcmp(bptr->EnvName,Env,EnvLen) == 0)
         return bptr ;
   }

   return NULL ;
}

#define AddEnvir(a,b,c,d) AddBox(TSD,a,b,c,d,BOX_IS_ENVIR)
#define AddExit(a,b,c,d) AddBox(TSD,a,b,c,d,BOX_IS_EXIT)

static struct EnvBox *AddBox( const tsd_t *TSD, const char *EnvName, int EnvLen,
                              const void *UserArea, PFN EntryPoint, int type )
{
   struct EnvBox *NewBox ;
   struct EnvBox **first ;
   rex_tsd_t *rt;

   rt = TSD->rex_tsd;
   first = (type==BOX_IS_ENVIR) ? &rt->FirstEnv : &rt->FirstExit ;
   NewBox = MallocTSD( sizeof( struct EnvBox ) ) ; /* This is not exported */
   if (!NewBox)
      return NULL ;

   NewBox->EnvName = MallocTSD( EnvLen+1 ) ; /* This is not exported */
   if (!NewBox->EnvName)
   {
      FreeTSD( NewBox ) ;
      return NULL ;
   }

   NewBox->prev = NULL ;
   NewBox->next = (*first) ;
   if (*first)
      (*first)->prev = NewBox ;
   (*first) = NewBox ;

   memcpy( NewBox->EnvName, EnvName , EnvLen ) ;
   NewBox->EnvName[EnvLen] = '\0';
   NewBox->hash = hashvalue(EnvName, EnvLen);
   NewBox->u.EntryPnt = EntryPoint ;
   if (UserArea)
      memcpy( NewBox->UserData, UserArea, 8 ) ;
   else
      memset( NewBox->UserData, 0x00, 8 ) ;

   return NewBox ;
 }


#define RemoveExit(a,b) RemoveBox(TSD,a,b,BOX_IS_EXIT)
#define RemoveEnvir(a,b) RemoveBox(TSD,a,b,BOX_IS_ENVIR)

static int RemoveBox( const tsd_t *TSD, const char *EnvName, int EnvLen, int type )
{
   struct EnvBox *OldBox ;
   struct EnvBox **First ;
   rex_tsd_t *rt;

   rt = TSD->rex_tsd;
   OldBox = FindBox( TSD, EnvName, EnvLen, type ) ;
   if (OldBox)
   {
      First = (type==BOX_IS_ENVIR) ? &rt->FirstEnv : &rt->FirstExit ;
      if (OldBox->prev)
         OldBox->prev->next = OldBox->next ;
      if (OldBox->next)
         OldBox->next->prev = OldBox->prev ;
      if ((*First)==OldBox)
         (*First) = OldBox->prev ;

      FreeTSD( OldBox->EnvName ) ;
      FreeTSD( OldBox ) ;
      return 0 ;
   }
   return 1 ;
}

/* RetLen and RetStr should point to {0,NULL}. They will be filled with
 * freshly allocated values. A return value will always exist.
 */
int IfcSubCmd( tsd_t *TSD, int EnvLen, const char *EnvStr,
               int CmdLen, const char *CmdStr,
               int *RetLen, char **RetStr )
{
   RXSTRING Cmd, Ret ;
   char *OldResult= NULL ;
   USHORT Flags=0 ;
   char *Command ;
   char *EnvNam;
   struct EnvBox *Envir=NULL ;
   int rvalue=0, RCode=0, rc=RXEXIT_NOT_HANDLED ;
   char subcmd_result[ILLEGAL_USE_SIZE+RXAUTOBUFLEN] ;
   RXCMDHST_PARM cmdhst;
   PUCHAR parm=NULL;
   rex_tsd_t *rt;

   rt = TSD->rex_tsd;

   Command = MallocTSD( CmdLen + 1);
   memcpy(Command,CmdStr,CmdLen);
   Command[CmdLen] = '\0';
   memset( subcmd_result, 0, sizeof( subcmd_result ) ) ;
   MAKERXSTRING( Cmd, Command, CmdLen ) ;
   MAKERXSTRING( Ret, subcmd_result + ILLEGAL_USE_SIZE, RXAUTOBUFLEN) ;
   OldResult = subcmd_result + ILLEGAL_USE_SIZE;
   /*
    * Terminate the command string with nul character
    */
   if ( rt->CurrentHandlers && rt->CurrentHandlers->Handlers[RXCMD] )
   {
      EnvNam = MallocTSD( EnvLen + 1 ) ;
      memcpy(EnvNam, EnvStr, EnvLen ) ;
      EnvNam[EnvLen] = '\0';
      cmdhst.rxcmd_flags.rxfcfail = 0;
      cmdhst.rxcmd_flags.rxfcerr = 0;
      cmdhst.rxcmd_command = Cmd ;
      cmdhst.rxcmd_address = (unsigned char *)EnvNam ;
      cmdhst.rxcmd_addressl = (USHORT) EnvLen ;
      cmdhst.rxcmd_retc = Ret;
      cmdhst.rxcmd_dll = NULL;
      cmdhst.rxcmd_dll_len = 0;
      parm = (PUCHAR)&cmdhst;
      rc = (*(rt->CurrentHandlers->Handlers[RXCMD]))(RXCMD, RXCMDHST, parm);
      TSD->var_indicator = 0;
      assert( rc==RXEXIT_HANDLED || rc==RXEXIT_NOT_HANDLED ||
              rc==RXEXIT_RAISE_ERROR ) ;
      if (cmdhst.rxcmd_flags.rxfcerr)
         RCode = RXFLAG_ERROR ;
      else if (cmdhst.rxcmd_flags.rxfcfail)
         RCode = RXFLAG_FAILURE ;
      else
         RCode = RXFLAG_OK;
      Ret = cmdhst.rxcmd_retc;
      FreeTSD( EnvNam ) ;
   }
   if (rc == RXEXIT_NOT_HANDLED)
   {
      Envir = FindEnvir( EnvStr , EnvLen ) ;
      if (Envir)
      {
         MAKERXSTRING( Cmd, Command, CmdLen ) ;
         if (Ret.strlength && OldResult != Ret.strptr) /* Ignore return values*/
            IfcFreeMemory( Ret.strptr ) ;
         MAKERXSTRING( Ret, subcmd_result + ILLEGAL_USE_SIZE, RXAUTOBUFLEN) ;
         OldResult = subcmd_result + ILLEGAL_USE_SIZE ;
         rvalue = (*(Envir->u.SubCom))( &Cmd, &Flags, &Ret ) ;
         TSD->var_indicator = 0;
         if (Flags==RXSUBCOM_OK)
            RCode = RXFLAG_OK ;
         else if (Flags==RXSUBCOM_ERROR)
            RCode = RXFLAG_ERROR ;
         else if (Flags==RXSUBCOM_FAILURE)
            RCode = RXFLAG_FAILURE ;
         else
            exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
      }
      else
      {
         RCode = RXFLAG_NOTREG ;
         Ret.strlength = 0 ;
      }
   }

   if (Ret.strlength)
   {
      *RetLen = Ret.strlength ;
      *RetStr = MallocTSD( Ret.strlength ) ;
      memcpy( *RetStr, Ret.strptr, Ret.strlength ) ;
   }
   else
   {
      *RetLen = 1 ;
      *RetStr = MallocTSD( 1 ) ;
      (*RetStr)[0] = '0' ;
   }

   if (Ret.strlength && OldResult != Ret.strptr)
      IfcFreeMemory( Ret.strptr ) ;

   FreeTSD(Command);
   return RCode ;
}

/* IfcDoExit calls an exit handler with one of the following codes set in Code.
 * The arguments may either be input or output or nothing but not both.
 * Parameter      INIT TERMIN PULL TRCIN STDOUT STDERR GETENV PUTENV
 * ---------------------------------------------------------------------------------------
 * InputLength    NULL NULL   set  set   NULL   NULL   set    NULL
 * InputString    NULL NULL   set  set   NULL   NULL   set    NULL
 * OutputLength1  0    0      0    0     set    set    set    set
 * OutputString1  NULL NULL   NULL NULL  set    set    set    set
 * OutputLength2  0    0      0    0     0      0      0      set
 * OutputString2  NULL NULL   NULL NULL  NULL   NULL   NULL   set
 *
 * Notes:
 * 1) An output string should always be a fresh copy. Although it is not
 *    allowed the user program may destroy the contents.
 *    For this reason OutputString is not declared as const.
 *    OutputString should be 0-terminated (0 not counted in OutputLength).
 * 2) An input string is normally NOT required. Just provide a position
 *    where to place the input to. Example:
 *    char *in = NULL;
 *    int inlen = 0;
 *    IfcDoExit(?,?,0,NULL,0,NULL,&inlen,&in);
 *    If the caller of this function provides a valid input string it is
 *    ignored on exit. This function always returns back a freshly allocated
 *    string in InputString (an empty string in case of errors).
 * 3) The user may change or overwrite the outcome of an exit like the return
 *    values to functions. A user-allocated string will be freed.
 */
int IfcDoExit( tsd_t *TSD, int Code,
               int OutputLength1, char *OutputString1,
               int OutputLength2, char *OutputString2,
               int *InputLength, char **InputString )
{
   int rc=0;
   LONG SubCode=0, MainCode=0 ;
   ULONG retlen=0;
   char *retstr=NULL;
   char *mustFree;
   RXSIOSAY_PARM siosay;
   RXSIOTRD_PARM siotrd;
   RXSIODTR_PARM siodtr;
   RXENVSET_PARM envset;
   RXENVGET_PARM envget;
   RXCWDSET_PARM cwdset;
   RXCWDGET_PARM cwdget;
   PEXIT parm=NULL;
   rex_tsd_t *rt;

   rt = TSD->rex_tsd;

   MAKERXSTRING( siodtr.rxsiodtr_retc, NULL, 0) ; /* Make compiler happy */
   MAKERXSTRING( siotrd.rxsiotrd_retc, NULL, 0) ; /* Make compiler happy */
   MAKERXSTRING( envget.rxenv_value, NULL, 0) ; /* Make compiler happy */
   MAKERXSTRING( cwdget.rxcwd_value, NULL, 0) ; /* Make compiler happy */

   switch (Code)
   {
      case RX_EXIT_STDERR:
      case RX_EXIT_STDOUT:
         assert(InputLength == NULL &&
                InputString == NULL &&
                OutputLength2 == 0 &&
                OutputString2 == NULL &&
                OutputLength2 == 0 &&
                OutputString2 == NULL);
         siosay.rxsio_string.strptr = OutputString1 ;
         siosay.rxsio_string.strlength = OutputLength1 ;
         parm = (PEXIT)&siosay;
         SubCode = (Code==RX_EXIT_STDOUT) ? RXSIOSAY : RXSIOTRC ;
         MainCode = RXSIO ;

         break ;

      case RX_EXIT_TRCIN:
         assert(OutputLength1 == 0 &&
                OutputString1 == NULL &&
                InputLength != NULL &&
                InputString != NULL &&
                OutputLength2 == 0 &&
                OutputString2 == NULL);
         siodtr.rxsiodtr_retc.strlength = *InputLength ;
         siodtr.rxsiodtr_retc.strptr = *InputString ;
         parm = (PEXIT)&siodtr;
         SubCode = RXSIODTR ;
         MainCode = RXSIO ;
         break ;

      case RX_EXIT_PULL:
         assert(OutputLength1 == 0 &&
                OutputString1 == NULL &&
                InputLength != NULL &&
                InputString != NULL &&
                OutputLength2 == 0 &&
                OutputString2 == NULL);
         siotrd.rxsiotrd_retc.strlength = *InputLength ;
         siotrd.rxsiotrd_retc.strptr = *InputString ;
         parm = (PEXIT)&siotrd;
         SubCode = RXSIOTRD ;
         MainCode = RXSIO ;
         break ;

      case RX_EXIT_INIT:
         assert(OutputLength1 == 0 &&
                OutputString1 == NULL &&
                InputLength == NULL &&
                InputString == NULL &&
                OutputLength2 == 0 &&
                OutputString2 == NULL);
         MainCode = RXINI ;
         SubCode = RXINIEXT ;
         break ;

      case RX_EXIT_TERMIN:
         assert(OutputLength1 == 0 &&
                OutputString1 == NULL &&
                InputLength == NULL &&
                InputString == NULL &&
                OutputLength2 == 0 &&
                OutputString2 == NULL);
         MainCode = RXTER ;
         SubCode = RXTEREXT ;
         break ;

      case RX_EXIT_SETENV:
         assert(InputLength == NULL &&
                InputString == NULL &&
                OutputLength1 != 0 &&
                OutputString1 != NULL &&
                OutputLength2 != 0 &&
                OutputString2 != NULL);
         envset.rxenv_name.strptr = OutputString1 ;
         envset.rxenv_name.strlength = OutputLength1 ;
         envset.rxenv_value.strptr = OutputString2 ;
         envset.rxenv_value.strlength = OutputLength2 ;
         parm = (PEXIT)&envset;
         MainCode = RXENV ;
         SubCode = RXENVSET ;
         break ;

      case RX_EXIT_GETENV:
         assert(OutputLength1 != 0 &&
                OutputString1 != NULL &&
                InputLength != NULL &&
                InputString != NULL &&
                OutputLength2 == 0 &&
                OutputString2 == NULL);
         envget.rxenv_value.strlength = *InputLength ;
         envget.rxenv_value.strptr = *InputString ;
         envget.rxenv_name.strptr = OutputString1 ;
         envget.rxenv_name.strlength = OutputLength1 ;
         parm = (PEXIT)&envget;
         SubCode = RXENVGET ;
         MainCode = RXENV ;
         break ;

      case RX_EXIT_SETCWD:
         assert(InputLength == NULL &&
                InputString == NULL &&
                OutputLength1 != 0 &&
                OutputString1 != NULL);
         cwdset.rxcwd_value.strptr = OutputString1 ;
         cwdset.rxcwd_value.strlength = OutputLength1 ;
         parm = (PEXIT)&cwdset;
         MainCode = RXENV ;
         SubCode = RXCWDSET ;
         break ;

      case RX_EXIT_GETCWD:
         assert(OutputLength1 == 0 &&
                OutputString1 == NULL &&
                InputLength != NULL &&
                InputString != NULL &&
                OutputLength2 == 0 &&
                OutputString2 == NULL);
         cwdget.rxcwd_value.strlength = *InputLength ;
         cwdget.rxcwd_value.strptr = *InputString ;
         parm = (PEXIT)&cwdget;
         SubCode = RXCWDGET ;
         MainCode = RXENV ;
         break ;

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
         break;
   }

   assert( rt->CurrentHandlers->Handlers[MainCode] ) ;

   rc = (*(rt->CurrentHandlers->Handlers[MainCode]))(MainCode, SubCode, parm);
   TSD->var_indicator = 0;
   assert( rc==RXEXIT_HANDLED || rc==RXEXIT_NOT_HANDLED ||
           rc==RXEXIT_RAISE_ERROR ) ;

   mustFree = NULL;
   switch (Code)
   {
      case RX_EXIT_STDERR:
      case RX_EXIT_STDOUT:
      case RX_EXIT_INIT:
      case RX_EXIT_TERMIN:
      case RX_EXIT_SETENV:
      case RX_EXIT_SETCWD:
         break ;

     case RX_EXIT_TRCIN:
         retlen = siodtr.rxsiodtr_retc.strlength ;
         retstr = siodtr.rxsiodtr_retc.strptr ;
         mustFree = ( retstr != *InputString ) ? retstr : NULL;
         break ;

     case RX_EXIT_PULL:
         retlen = siotrd.rxsiotrd_retc.strlength ;
         retstr = siotrd.rxsiotrd_retc.strptr ;
         mustFree = ( retstr != *InputString ) ? retstr : NULL;
         break ;

     case RX_EXIT_GETENV:
         retlen = envget.rxenv_value.strlength ;
         retstr = envget.rxenv_value.strptr ;
         mustFree = ( retstr != *InputString ) ? retstr : NULL;
         break ;

     case RX_EXIT_GETCWD:
         retlen = cwdget.rxcwd_value.strlength ;
         retstr = cwdget.rxcwd_value.strptr ;
         mustFree = ( retstr != *InputString ) ? retstr : NULL;
         break ;

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
   }

   if (rc==RXEXIT_HANDLED)
      rc = RX_HOOK_NOPE ;
   else if (rc==RXEXIT_NOT_HANDLED)
      rc = RX_HOOK_GO_ON ;
   else if (rc==RXEXIT_RAISE_ERROR)
      rc = RX_HOOK_ERROR ;

   if (InputLength != NULL) /* retlen and retstr forms a return string. */
   {
      if ((retlen == 0) || (retstr == NULL))
      {
         retlen = 0;
         retstr = "";
      }

      /* Make a fresh copy, the user may change the value very fast. */
      *InputString = MallocTSD( (retlen < 1) ? 1 : retlen );
      memcpy(*InputString, retstr, retlen);
      *InputLength = retlen;
   }

   if ( mustFree )
      IfcFreeMemory( mustFree );
   return rc ;
}

/* ================================================================ */
/* ================ general purpose API functions ================= */

/* You are not allowed to use TSD or __regina_get_tsd() here! */
EXPORT_C APIRET APIENTRY RexxFreeMemory(PVOID MemoryBlock )
{
   if (!MemoryBlock)
      return(RXFUNC_BADTYPE);

   return IfcFreeMemory( MemoryBlock );
}

/* You are not allowed to use TSD or __regina_get_tsd() here! */
EXPORT_C PVOID APIENTRY RexxAllocateMemory(ULONG size )
{
   return IfcAllocateMemory( size );
}

/* ================================================================ */
/* ================ in order to start Rexx scripts ================ */

EXPORT_C APIRET APIENTRY RexxStart(LONG       ArgCount,
                          PRXSTRING  ArgList,
                          PCSZ       ProgName,
                          PRXSTRING  Instore,
                          PCSZ       EnvName,
                          LONG       CallType,
                          PRXSYSEXIT Exits,
                          PSHORT     ReturnCode,
                          PRXSTRING  Result )
{
   int cnt=0, RLength=0 ;
   char *RString=NULL ;
   int ParLengths[MAX_ARGS_TO_REXXSTART] ;
   const char *ParStrings[MAX_ARGS_TO_REXXSTART] ;
   int ExitFlags=0 ;
   int EnvNamLen=0 ;
   const char *EnvNamStr ;
   int WhereCode=0, rc=0 ;
   const char *SourcePtr ;
   const void *TinPtr ;
   unsigned long SourceLen, TinLen ;
   struct ExitHandlers *Handlers=NULL ;
   RexxExitHandler *handler=NULL ;
   struct EnvBox *EnvPtr=NULL ;
   LONG ResValue=0L ;
   unsigned long instore_length = 0;
   void *instore_buf = NULL;
   PCSZ ProgramName=ProgName;
   tsd_t *TSD;
   rex_tsd_t *rt;
   int restricted = 0;

   TSD = GLOBAL_ENTRY_POINT();
   rt = TSD->rex_tsd;
   StartupInterface(TSD);

   if ((ArgCount < 0) || ((ArgCount > 0) && (ArgList == NULL)))
      return(RXFUNC_BADTYPE);
   if (!ProgName)
      return(RXFUNC_BADTYPE);
   /*
    * Check if running in restricted mode first.
    */
   if ( CallType & RXRESTRICTED )
   {
      restricted = 1;
      CallType -= RXRESTRICTED;
   }
   if ((CallType != RXCOMMAND) &&
       (CallType != RXSUBROUTINE) &&
       (CallType != RXFUNCTION))
      return(RXFUNC_BADTYPE);
   if ((CallType == RXCOMMAND) && (ArgCount > 1))
      return(RX_START_TOOMANYP);
   if (ArgCount > sizeof(ParLengths) / sizeof(ParLengths[0]) )
      return(RX_START_TOOMANYP);

   if (Instore)
   {
/* MH 2602     if (Instore[1].strptr && Instore[1].strlength < sizeof(int)) */
      if (Instore[1].strptr && Instore[1].strlength < 1)
         return RX_START_BADP ;
   }

   if (ArgCount > sizeof(ParLengths) / sizeof(ParLengths[0]) )
      ArgCount = sizeof(ParLengths) / sizeof(ParLengths[0]) ;
   for (cnt=0; cnt<ArgCount; cnt++)
   {
      ParLengths[cnt] = ArgList[cnt].strlength ;
      ParStrings[cnt] = ArgList[cnt].strptr ;
      if (ParStrings[cnt]==NULL)
         ParLengths[cnt] = RX_NO_STRING ;
   }

   Handlers = TSD->MTMalloc( TSD, sizeof( struct ExitHandlers )) ;
   Handlers->prev = rt->CurrentHandlers ;
   rt->CurrentHandlers = Handlers ;
   for (cnt=0; cnt<RXNOOFEXITS; cnt++)
      rt->CurrentHandlers->Handlers[cnt] = NULL ;

   ExitFlags = 0x00000000 ;
   for (cnt=0; Exits && Exits->sysexit_code!=RXENDLST; Exits++ )
   {
      if ( Exits->sysexit_name == NULL
      ||   strlen( Exits->sysexit_name ) == 0 )
         return(RX_START_BADP);

      EnvPtr = FindExit( Exits->sysexit_name , strlen(Exits->sysexit_name) ) ;
      if (!EnvPtr)
         continue ;

      /* Sigh ... Definition requires some strange casting */
      handler = (RexxExitHandler*)(EnvPtr->u.EntryPnt) ;
      switch (Exits->sysexit_code)
      {
         case RXSIO:
            ExitFlags |= (1<<RX_EXIT_STDOUT) | (1<<RX_EXIT_STDERR) |
                         (1<<RX_EXIT_TRCIN) | (1<<RX_EXIT_PULL) ;
            rt->CurrentHandlers->Handlers[RXSIO] = handler ;
            break ;

         case RXINI:
            ExitFlags |= (1<<RX_EXIT_INIT) ;
            rt->CurrentHandlers->Handlers[RXINI] = handler ;
            break ;

         case RXTER:
            ExitFlags |= (1<<RX_EXIT_TERMIN) ;
            rt->CurrentHandlers->Handlers[RXTER] = handler ;
            break ;

         case RXCMD:
            ExitFlags |= (1<<RX_EXIT_SUBCOM) ;
            rt->CurrentHandlers->Handlers[RXCMD] = handler ;
            break ;

         case RXFNC:
            ExitFlags |= (1<<RX_EXIT_FUNC) ;
            rt->CurrentHandlers->Handlers[RXFNC] = handler ;
            break ;

         case RXENV:
            ExitFlags |= (1<<RX_EXIT_GETENV) | (1<<RX_EXIT_SETENV) |
                         (1<<RX_EXIT_GETCWD) | (1<<RX_EXIT_SETCWD) ;
            rt->CurrentHandlers->Handlers[RXENV] = handler ;
            break ;

         default:
            return(RX_START_BADP);
      }
   }

   if (EnvName)
   {
      EnvNamLen = strlen(EnvName) ;
      EnvNamStr = EnvName ;
   }
   else
   {
      EnvNamLen = RX_NO_STRING ;
      EnvNamStr = NULL ;
   }

   SourcePtr = NULL ;
   SourceLen = 0 ;
   TinPtr = NULL ;
   TinLen = 0 ;
   if (Instore && Instore[1].strptr)
   {
      WhereCode = RX_TYPE_INSTORE ;
      TinPtr = Instore[1].strptr ;
      TinLen = Instore[1].strlength ;
      SourcePtr = Instore[0].strptr ;
      SourceLen = Instore[0].strlength ;
   }
   else if (Instore && Instore[0].strptr)
   {
      WhereCode = RX_TYPE_SOURCE ;
      SourcePtr = Instore[0].strptr ;
      SourceLen = Instore[0].strlength ;
   }
   else if (Instore)
      WhereCode = RX_TYPE_MACRO ;
   else
      WhereCode = RX_TYPE_EXTERNAL ;
   starttrace(TSD) ;

   rc = IfcExecScript( TSD, strlen(ProgramName), ProgramName,
          ArgCount, ParLengths, (const char **) ParStrings, MAP_TYPE(CallType),
          ExitFlags, EnvNamLen, EnvNamStr,  WhereCode, restricted,
          SourcePtr, SourceLen, TinPtr, TinLen,
          &RLength, &RString, &instore_buf, &instore_length) ;
   Handlers = rt->CurrentHandlers ;
   rt->CurrentHandlers = Handlers->prev ;
   TSD->MTFree( TSD, Handlers ) ;

   if (WhereCode == RX_TYPE_SOURCE)
   {
      Instore[1].strptr = instore_buf;
      Instore[1].strlength = instore_length;
   }

   if (RLength!=RX_NO_STRING)
      ResValue = atoi( RString ) ;
   else
      ResValue = 0 ;

   if (ReturnCode)
      *ReturnCode = (SHORT) ResValue ;   /* FGC */

   if (Result)
   {
      if (!Result->strptr || (int)Result->strlength>=RLength+1)
      {
         Result->strlength = RLength ;
         Result->strptr = RString ;
      }
      else
      {
         Result->strlength = RLength ;
         memcpy( Result->strptr, RString, RLength+1 ) ;
         IfcFreeMemory( RString ) ;
      }
   }
   else if (RString)
      IfcFreeMemory( RString ) ;

   /*
    * Close all open files.
    */
   CloseOpenFiles( TSD ) ;

   return rc ;
}

EXPORT_C APIRET APIENTRY RexxCallBack( PCSZ       ProcedureName,
                                       LONG       ArgCount,
                                       PRXSTRING  ArgList,
                                       PSHORT     ReturnCode,
                                       PRXSTRING  Result )
{
   int rc, cnt, RLength;
   char *RString;
   int ParLengths[MAX_ARGS_TO_REXXSTART];
   const char *ParStrings[MAX_ARGS_TO_REXXSTART];
   LONG ResValue;
   tsd_t *TSD;

   /*
    * This can only be called with an active Rexx session running
    */
   TSD = __regina_get_tsd();

   if ( TSD->systeminfo == NULL )
      return RX_CB_NOTSTARTED;

   if ( ( ArgCount < 0 ) || ( ( ArgCount > 0 ) && ( ArgList == NULL ) ) )
      return RX_CB_BADP;
   if ( !ProcedureName )
      return RX_CB_BADP;

   if (ArgCount > sizeof( ParLengths ) / sizeof( ParLengths[0] ) )
      return RX_CB_TOOMANYP;

   for ( cnt = 0; cnt < ArgCount; cnt++ )
   {
      ParLengths[cnt] = ArgList[cnt].strlength;
      ParStrings[cnt] = ArgList[cnt].strptr;
      if ( ParStrings[cnt] == NULL )
         ParLengths[cnt] = RX_NO_STRING;
   }

   rc = IfcExecCallBack( TSD, strlen(ProcedureName), ProcedureName,
          ArgCount, ParLengths, (const char **) ParStrings,
          &RLength, &RString );
   if ( rc == RX_CODE_NOSUCH )
      rc = RX_CB_BADN;

   /*
    * Determine numeric return code and pass it back
    */
   if ( RLength != RX_NO_STRING )
      ResValue = atoi( RString );
   else
      ResValue = 0;

   if ( ReturnCode )
      *ReturnCode = (SHORT) ResValue;   /* FGC */

   /*
    * Determine text return code and pass it back
    */
   if ( rc == RX_CB_OK )
   {
      if ( Result )
      {
         if ( RLength == RX_NO_STRING )
         {
            /*
             * Always overwrite any value. There is no return value at all.
             */
            Result->strlength = 0;
            Result->strptr = NULL;
         }
         else if ( Result->strptr && (int)Result->strlength >= RLength + 1 )
         {
            /*
             * Use the supplied users's buffer.
             */
            Result->strlength = RLength;
            memcpy( Result->strptr, RString, RLength+1 );
            IfcFreeMemory( RString );
         }
         else
         {
            /*
             * User's buffer invalid or too small: Use IfcExecCallBack's one.
             */
            Result->strlength = RLength;
            Result->strptr = RString;
         }
      }
      else if ( RString )
         IfcFreeMemory( RString );
   }

   return rc;
}



/* ============================================================= */
/* subcom handler subsystem */

EXPORT_C APIRET APIENTRY RexxRegisterSubcomExe(PCSZ EnvName,
                                      PFN EntryPoint,
                                      PUCHAR UserArea )
{
   int EnvLen=0 ;
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   /*
    * Perform sanity check on the parameters; UserArea may be NULL
    */
   if (!EnvName || !EntryPoint)
      return RXSUBCOM_BADTYPE ;

   EnvLen = strlen( EnvName ) ;
   if (EnvLen>MAXENVNAMELEN)
      return RXSUBCOM_NOTREG ;

   if (FindEnvir( EnvName, EnvLen ))
      return RXSUBCOM_NOTREG ;

   if (!AddEnvir( EnvName, strlen(EnvName) , UserArea, EntryPoint ))
      return RXSUBCOM_NOEMEM ;

   return RXSUBCOM_OK ;
}


EXPORT_C APIRET APIENTRY RexxRegisterSubcomDll(PCSZ EnvName,
                                      PCSZ ModuleName,
                                      PCSZ ProcedureName,
                                      PUCHAR UserArea,
                                      ULONG DropAuth )
{
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   if (!EnvName || !ModuleName || !ProcedureName)
      return(RXSUBCOM_BADTYPE);

   /* not yet functional */
   return RXSUBCOM_NOTREG ;
}


EXPORT_C APIRET APIENTRY RexxQuerySubcom(PCSZ EnvName,
                                PCSZ ModuleName,
                                PUSHORT Flag, /* Who knows what this is used for ... */
                                PUCHAR UserWord )
{
   int ret=0 ;
   struct EnvBox *eptr=NULL ;
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   if (!EnvName || !Flag)
      return(RXFUNC_BADTYPE);

   /* ModuleName is not yet functional */

   eptr = FindEnvir( EnvName , strlen(EnvName) ) ;
   if (eptr)
   {
      *Flag = RXSUBCOM_ISREG;
      ret = RXSUBCOM_OK ;
      if (UserWord)
         memcpy( UserWord, eptr->UserData, 8 ) ;
   }
   else
   {
      *Flag = 0;
      ret = RXSUBCOM_NOTREG ;
   }

   return ret ;
}

EXPORT_C APIRET APIENTRY RexxDeregisterSubcom(PCSZ EnvName,
                                     PCSZ ModuleName )
{
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   if (!EnvName)
      return(RXSUBCOM_BADTYPE);

   /* ModuleName is not yet functional */

   if (RemoveEnvir( EnvName , strlen(EnvName) ))
      return RXSUBCOM_NOTREG ;

   return RXSUBCOM_OK ;
}



/* ============================================================ */
/* Variable subsystem */
/* JH 20-10-99 */  /* To make Direct setting of stems Direct and not Symbolic. */
/****************************************************************************
 *
 *  JH 13/12/1999 (Original code changes on 20/10/1999)
 *
 *  BUG022            To make Direct setting of stems Direct and not Symbolic.
 *   - Added checks for the direct variable functions RX_GETVAR and RX_SETVAR.
 *     In the switch that determines what to do, based on the value passed in
 *     shvcode, symbolics still fall through to the code that is under the
 *     direct labels, but it sets a variable to denote that symbolic processing
 *     is to take place.  The direct section only sets this variable if it has
 *     not been set before.
 *   - Added new variable IVPcode (IfcVariablePool) that will contain the code
 *     used to call IfcVariablePool(), instead of hard coding the parameter,
 *
 *  NB that this routine lumps the Drop's and Set's together, before calling
 *    IfcVarPool().  At some point it might be better to pass the shvcode
 *    value, rather than translating it and later performing additional
 *    checks to split it back out.
 *
 ****************************************************************************/
EXPORT_C APIRET APIENTRY RexxVariablePool(PSHVBLOCK RequestBlockList )
{
   int Code=0, RetCode=0, IVPcode;
   int Lengths[2] ;
   int rc=0, allocated ;
   char *Strings[2] ;
   PSHVBLOCK Req=RequestBlockList ;
   tsd_t *TSD;
   rex_tsd_t *rt;

   TSD = GLOBAL_ENTRY_POINT();
   rt = TSD->rex_tsd;
   StartupInterface(TSD);

   if (!RequestBlockList) /* FGC: I assume we must have at least one param */
      return(RXFUNC_BADTYPE);

   if (TSD->systeminfo->tree.root==NULL) /* Doesn't the interpreter run? */
      return RXSHV_NOAVL ;

   RetCode = 0 ;

   for (;Req;Req=Req->shvnext)
   {
      IVPcode = 0; /* Needed for a correct IVPcode on a second request */
      allocated = 0;
      switch (Req->shvcode)
      {
         case RXSHV_SYDRO:
         case RXSHV_SYSET:
           IVPcode = RX_SETSVAR;                        /* JH 20-10-99 */
         case RXSHV_DROPV:                              /* MH 26-12-95 */
         case RXSHV_SET:                                /* MH 26-12-95 */
         {
            IVPcode = IVPcode ? IVPcode : RX_SETVAR;    /* JH 20-10-99 */
            Lengths[0] = Req->shvname.strlength ;
            Strings[0] = Req->shvname.strptr ;
            if (Req->shvcode==RXSHV_SYSET               /* MH 26-12-95 */
            ||  Req->shvcode==RXSHV_SET)                /* MH 26-12-95 */
            {
               Lengths[1] = Req->shvvalue.strlength ;
               Strings[1] = Req->shvvalue.strptr ;
            }
            else
               Lengths[1] = RX_NO_STRING ;

            Code = IfcVarPool( TSD, IVPcode, Lengths, Strings, &allocated );

            Req->shvret = RXSHV_OK ;
            if (Code==RX_CODE_NOVALUE)
               Req->shvret |= RXSHV_NEWV ;
            else if (Code==RX_CODE_INVNAME)
               Req->shvret |= RXSHV_BADN ;
            else if (Code!=RXSHV_OK)
               exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
            TSD->var_indicator=0;
            break ;
         }
         case RXSHV_SYFET:
           IVPcode = RX_GETSVAR;                        /* JH 20-10-99 */
         case RXSHV_FETCH:                              /* MH 26-12-95 */
         {
            IVPcode = IVPcode ? IVPcode : RX_GETVAR;    /* JH 20-10-99 */
            Lengths[0] = Req->shvname.strlength ;
            Strings[0] = Req->shvname.strptr ;
            Code = IfcVarPool( TSD, IVPcode, Lengths, Strings, &allocated );

            Req->shvret = RXSHV_OK ;
            if (Code==RX_CODE_NOVALUE)
               Req->shvret |= RXSHV_NEWV ;
            else if (Code==RX_CODE_INVNAME)
               Req->shvret |= RXSHV_BADN ;
            else if (Code!=RXSHV_OK)
               exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
            FillReqValue( Req, Lengths[1], Strings[1] ) ;
            TSD->var_indicator=0;
            break ;
         }

         case RXSHV_PRIV:
         {
            Req->shvret = RXSHV_OK ;
            if (Req->shvname.strlength==4 && Req->shvname.strptr &&
                !strncmp(Req->shvname.strptr, "PARM", 4 ))
            {
               rc = IfcVarPool( TSD, RX_CODE_PARAMS, Lengths, Strings, &allocated );
               FillReqValue( Req, Lengths[0], Strings[0] ) ;
            }

            else if (Req->shvname.strlength>=5 && Req->shvname.strptr &&
                !strncmp(Req->shvname.strptr, "PARM.", 5 ))
            {
               Lengths[0] = Req->shvname.strlength - 5 ;
               Strings[0] = Req->shvname.strptr + 5 ;

               rc = IfcVarPool( TSD, RX_CODE_PARAM, Lengths, Strings, &allocated );
               if (rc == RX_CODE_OK)
                  FillReqValue( Req, Lengths[1], Strings[1] ) ;
               else
                  Req->shvret |= RXSHV_BADN ;
            }

            else
            {
               int Code=0 ;
               if (Req->shvname.strptr)
               {
                  if (Req->shvname.strlength==7 &&
                         !memcmp(Req->shvname.strptr, "QUENAME", 7))
                  {
                     Code = RX_CODE_QUEUE ;
                  }
                  else if (Req->shvname.strlength==7 &&
                         !memcmp(Req->shvname.strptr, "VERSION", 7))
                  {
                     Code = RX_CODE_VERSION ;
                  }
                  else if (Req->shvname.strlength==6 &&
                         !memcmp(Req->shvname.strptr, "SOURCE", 6))
                  {
                     Code = RX_CODE_SOURCE ;
                  }
                  else
                     Req->shvret |= RXSHV_BADN ;

                  if (!Req->shvret | RXSHV_BADN)
                  {
                     rc=IfcVarPool( TSD, Code, Lengths, Strings, &allocated );
                     FillReqValue( Req, Lengths[0], Strings[0] ) ;
                  }
               }
               else
                  Req->shvret |= RXSHV_BADN ;
            }
            break ;
         }

         case RXSHV_NEXTV:
         {
            int Items ;

            Req->shvret = RXSHV_OK ;
            Items = IfcVarPool( TSD, RX_NEXTVAR, Lengths, Strings, &allocated );
            assert( Items==0 || Items==2 ) ;

            if (Items==2)
            {
               FillReqValue( Req, Lengths[1], Strings[1] ) ;
               FillReqName( Req, Lengths[0], Strings[0] ) ;
            }
            else
               Req->shvret |= RXSHV_LVAR ;

            break ;
         }

         default:
            Req->shvret = RXSHV_BADF ;
      }
      if (allocated & 1) /* fixes bug 596686 */
         FreeTSD( Strings[0] );
      if (allocated & 2)
         FreeTSD( Strings[1] );
      RetCode |= ( Req->shvret & 0x007f ) ;
   }

   return RetCode ;
}



/* ================================================================ */
/* system exit handler subsystem */

EXPORT_C APIRET APIENTRY RexxRegisterExitExe(PCSZ EnvName,
                                    PFN EntryPoint,
                                    PUCHAR UserArea )
{
   int EnvLen=0 ;
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   if (!EnvName || !EntryPoint)
      return RXEXIT_BADTYPE ;

   EnvLen = strlen( EnvName ) ;
   if (EnvLen>MAXENVNAMELEN)
      return RXEXIT_NOTREG ;

   if (FindExit( EnvName, EnvLen ))
      return RXEXIT_NOTREG ;

   if (!AddExit( EnvName, EnvLen, UserArea, EntryPoint ))
      return RXEXIT_NOEMEM ;

   return RXEXIT_OK ;
}

EXPORT_C APIRET APIENTRY RexxRegisterExitDll(PCSZ EnvName,
                                    PCSZ ModuleName,
                                    PCSZ ProcedureName,
                                    PUCHAR UserArea,
                                    ULONG DropAuth )
{
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   if (!EnvName || !ModuleName || !ProcedureName)
      return(RXFUNC_BADTYPE);
   /* not yet functional */
   return RXEXIT_NOTREG ;
}


EXPORT_C APIRET APIENTRY RexxDeregisterExit(PCSZ EnvName,
                                   PCSZ ModuleName )
{
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   if (!EnvName)
      return(RXFUNC_BADTYPE);

   /* Ignore ModuleName, it may be NULL in case of an Exe-handler */

   if (RemoveExit(EnvName, strlen(EnvName) ))
      return RXEXIT_NOTREG ;

   return RXEXIT_OK ;
}

EXPORT_C APIRET APIENTRY RexxQueryExit(PCSZ EnvName,
                              PCSZ ModuleName,
                              PUSHORT Flag,
                              PUCHAR UserArea)
{
   tsd_t *TSD;
   struct EnvBox *EnvPtr=NULL ;
   ULONG rc=0;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   if (!EnvName || !Flag)
      return(RXEXIT_BADTYPE);

   EnvPtr = FindExit( EnvName, strlen( EnvName ) ) ;
   if ( EnvPtr == NULL )
   {
      *Flag = RXEXIT_NOTREG;
      rc = RXEXIT_NOTREG;
   }
   else
   {
      *Flag = RXEXIT_OK;
      rc = RXEXIT_OK;
      if ( UserArea )
         memcpy( UserArea, EnvPtr->UserData, 8 ) ;
   }
   return( rc );
}

/* =================================================================== */

/*
 * This section contains the support for the external functions
   */

static struct funcbox2 *findfunc( const tsd_t *TSD, const char *name )
{
   struct funcbox2 *fptr=NULL ;
   int hashbox ;
   rex_tsd_t *rt;
   unsigned hash;

   rt = TSD->rex_tsd;
   hash = hashvalue( name, -1 ) ;
   hashbox = hash % EXT_FUNCS_COUNT ;
   for (fptr=rt->saafuncs[hashbox]; fptr; fptr=fptr->prev)
      if (fptr->hash == hash)
         if (!strcmp(name, fptr->name))
            return fptr ;

   return NULL ;
}

static int delfunc2( const tsd_t *TSD, const char *name )
{
   struct funcbox2 *old=NULL ;
   int hashbox ;
   rex_tsd_t *rt;

   rt = TSD->rex_tsd;
   old = findfunc( TSD, name ) ;
   if (!old)
      return RXFUNC_NOTREG ;

   hashbox = hashvalue( name, -1 ) % EXT_FUNCS_COUNT ;
   FreeTSD( old->name ) ;
   if (old==rt->saafuncs[hashbox])
      rt->saafuncs[hashbox] = old->prev ;
   else
      old->next->prev = old->prev ;

   if (old->prev)
      old->prev->next = old->next ;

   FreeTSD( old ) ;
   return RXFUNC_OK ;
}

static int addfunc2( const tsd_t *TSD, const char *name, RexxFunctionHandler *EntryPoint )
{
   struct funcbox2 *new=NULL ;
   int hashbox, hash ;
   rex_tsd_t *rt;

   rt = TSD->rex_tsd;

   if (findfunc( TSD, name ))
      return RXFUNC_DEFINED ;

   new = MallocTSD( sizeof(struct funcbox2) ) ;
   if (!new)
      return RXFUNC_NOMEM ;

   new->name = MallocTSD( strlen( name )+1 ) ;
   if (!new->name)
   {
      FreeTSD( new ) ;
      return RXFUNC_NOMEM ;
   }

   strcpy( new->name, name ) ;
   hash = hashvalue( new->name, -1 ) ;
   hashbox = hash % EXT_FUNCS_COUNT ;

   new->entry = EntryPoint ;
   new->next = NULL ;
   new->hash = hash ;
   new->prev = rt->saafuncs[hashbox] ;
   if (rt->saafuncs[hashbox])
      rt->saafuncs[hashbox]->next = new ;
   rt->saafuncs[hashbox] = new ;

   return RXFUNC_OK ;
}


EXPORT_C APIRET APIENTRY RexxRegisterFunctionExe( PCSZ Name,
                                         PFN EntryPoint )
{
   int code;
   char *upper_name;
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   if (!Name || !EntryPoint)
      return(RXFUNC_BADTYPE);
   assert( Name );
   if ((upper_name = MallocTSD( strlen(Name)+1) ) == NULL)
      return(RXFUNC_NOMEM);
   strcpy(upper_name, Name);
   mem_upper( upper_name, strlen( upper_name ) );
   code = addfunc2( TSD, upper_name, (RexxFunctionHandler*)EntryPoint ) ;
   FreeTSD( upper_name );
   if (code)
      return code ;

   code = IfcRegFunc( TSD, Name ) ;
   switch (code) {
      case RX_CODE_OK:    code = RXFUNC_OK;    break;
      case RX_CODE_NOMEM: code = RXFUNC_NOMEM; break;
      default: code = -1;
   }
   assert( code==RXFUNC_OK ) ; /* A simple "if (code!=RXFUNC_OK)delfunc2()"
                                * might be better!
                                */

   return RXFUNC_OK ;
}

EXPORT_C APIRET APIENTRY RexxRegisterFunctionDll( PCSZ ExternalName,
                                         PCSZ LibraryName,
                                         PCSZ InternalName )
{
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   if (!ExternalName || !LibraryName || !InternalName)
      return(RXFUNC_BADTYPE);
   return ((ULONG)IfcRegDllFunc(TSD,ExternalName,LibraryName,InternalName));
}

EXPORT_C APIRET APIENTRY RexxQueryFunction( PCSZ Name )
{
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   if (!Name)
      return(RXFUNC_BADTYPE);
   return ( IfcQueryFunc( TSD, Name ) ) ? RXFUNC_NOTREG : RXFUNC_OK ;
}


EXPORT_C APIRET APIENTRY RexxDeregisterFunction( PCSZ Name )
{
   int rc;
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);
   if (!Name)
      return(RXFUNC_BADTYPE);
   if ((rc = delfunc2(TSD, Name)) == RXFUNC_OK)
      return rc;
   return (IfcDelFunc(TSD, Name)) ? RXFUNC_NOTREG : RXFUNC_OK ;
}

/* The caller of IfcFunctionExit should call this function with fresh copies of
 * Name and params to be bullet-proof. The called function MAY
 * change the values although this is illegal.
 */
static int IfcFunctionExit( tsd_t *TSD,
                            PSZ Name,
                            int Params,
                            RXSTRING *params,
                            PCSZ queuename,
                            int queuelen,
                            PRXSTRING Retstr,
                            int *RCode,
                            char called )
{
   int rc=0 ;
   RXFNCCAL_PARM fnccal;
   PUCHAR parm=NULL;
   rex_tsd_t *rt;

   rt = TSD->rex_tsd;

   if ( rt->CurrentHandlers && rt->CurrentHandlers->Handlers[RXFNC] )
   {
      fnccal.rxfnc_flags.rxfferr = 0;
      fnccal.rxfnc_flags.rxffnfnd = 0;
      fnccal.rxfnc_flags.rxffsub = (called) ? 1 : 0;
      fnccal.rxfnc_name = (unsigned char *)Name;
      fnccal.rxfnc_namel = (USHORT) strlen(Name);
      fnccal.rxfnc_que = (unsigned char *)queuename;
      fnccal.rxfnc_quel = (USHORT) queuelen;
      fnccal.rxfnc_argc = (USHORT) Params;
      fnccal.rxfnc_argv = params;
      fnccal.rxfnc_retc = *Retstr;
      parm = (PUCHAR)&fnccal;
      rc = (*(rt->CurrentHandlers->Handlers[RXFNC]))(RXFNC, RXFNCCAL, parm);
      TSD->var_indicator = 0;
      assert( rc==RXEXIT_HANDLED || rc==RXEXIT_NOT_HANDLED ||
              rc==RXEXIT_RAISE_ERROR ) ;
      if (rc == RXEXIT_HANDLED)
      {
         if (fnccal.rxfnc_flags.rxfferr)
            *RCode = RXFLAG_ERROR ;
         else if (fnccal.rxfnc_flags.rxffnfnd)
            *RCode = RXFLAG_FAILURE ;
         else
            *RCode = RXFLAG_OK;
      }
      *Retstr = fnccal.rxfnc_retc;
      return(rc);
   }
   else
   {
      return (RXEXIT_NOT_HANDLED);
   }
}

/* The caller of IfcExecFunc should call this function with fresh copies of
 * Name, Length and Strings to be bullet-proof. The called function MAY
 * change the values although this is illegal.
 * RetLength and RetString should point to {0,NULL}. They will be filled with
 * freshly allocated values if there are some.
 */
int IfcExecFunc( tsd_t *TSD, PFN Func, char *Name, int Params,
                 int *Lengths, char **Strings,
                 int queue_name_len, char *queue_name,
                 int *RetLength, char **RetString,
                 int *RC, char exitonly, char called, void *gci_info )
{
   struct funcbox2 *fptr=NULL ;
   int i=0, length=0, rc=0, RCode=0 ;
   RXSTRING *params, retstr ;
   rex_tsd_t *rt;
   char execfunc_result[ILLEGAL_USE_SIZE+RXAUTOBUFLEN] ;

   rt = TSD->rex_tsd;
   assert( Name ) ;
   assert( Params >= 0 ) ;

   params = MallocTSD( sizeof(RXSTRING)*Params ) ;
   for (i=0; i<Params; i++)
   {
      length = Lengths[i] ;
      if (length==RX_NO_STRING)
      {
         params[i].strptr = NULL ;
         params[i].strlength = 0 ;
      }
      else
      {
         params[i].strptr = Strings[i] ;
         params[i].strlength = length ;
      }
   }

   memset( execfunc_result, 0, sizeof( execfunc_result ) ) ;
   retstr.strptr = execfunc_result + ILLEGAL_USE_SIZE ;
   retstr.strlength = RXAUTOBUFLEN;        /* MH 26-12-95 */

   rc = IfcFunctionExit( TSD, Name, Params, params, queue_name, queue_name_len,
                         &retstr, &RCode, called );
   switch(rc)
   {
      case RXEXIT_NOT_HANDLED:
         if (exitonly)
         {
           *RC = ERR_ROUTINE_NOT_FOUND;
         }
         else
         {
            /* Func or fptr->entry will inherit a possible return value in
             * retstr. This might be a problem, expect suspicious results
             * if the called functions are not error free.
             */
            if (Func == NULL)
            {
               if ((fptr=findfunc( TSD, Name )) == NULL)
               {
                  FreeTSD( params );
                  return RX_CODE_NOSUCH ;
               }
               rc = (*(fptr->entry))( Name, Params, params, queue_name, &retstr ) ;
            }
#if defined(DYNAMIC) && defined(HAVE_GCI)
            else if ( gci_info != NULL )
               rc = GCI_Dispatcher( TSD, Func, gci_info, Params, params, &retstr );
#endif
            else
               rc = (*(Func))( Name, Params, params, queue_name, &retstr ) ;

            if (rc)
               *RC = ERR_INCORRECT_CALL;
            else
               *RC = 0;
            TSD->var_indicator = 0;
         }
         break;
      case RXEXIT_HANDLED:
         if (RCode == RXFLAG_ERROR)
            *RC = ERR_INCORRECT_CALL;
         else if (RCode == RXFLAG_FAILURE)
            *RC = ERR_ROUTINE_NOT_FOUND;
         else
            *RC = 0;
         break;
      case RXEXIT_RAISE_ERROR:
         *RC = ERR_SYSTEM_FAILURE;
         break;
   }

   FreeTSD( params ) ;

   if (!(*RC) && retstr.strptr)
   {
      *RetString = MallocTSD( (retstr.strlength < 1) ? 1 : retstr.strlength ) ;
      memcpy( *RetString, retstr.strptr, retstr.strlength ) ;
      *RetLength = retstr.strlength ;
   }
   else
      *RetLength = RX_NO_STRING ;

   if (retstr.strptr && retstr.strptr != execfunc_result + ILLEGAL_USE_SIZE)
      IfcFreeMemory( retstr.strptr ) ;

   return RX_CODE_OK ;
}

int IfcHaveFunctionExit(const tsd_t *TSD)
{
   rex_tsd_t *rt;

   rt = TSD->rex_tsd;
   if ( rt->CurrentHandlers && rt->CurrentHandlers->Handlers[RXFNC] )
      return 1;
   else
      return 0;
}

/* ============================================================= */
/* Asynchronous Rexx API interface */

EXPORT_C APIRET APIENTRY RexxSetHalt(LONG dummyProcess,
                            LONG dummyThread )
{
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);
   /*
    * Perform sanity check on the parameters; is process id me ?
    */
   set_rexx_halt( TSD );
   return RXARI_OK ;
}

/* ============================================================= */
/* Named queue interface */

EXPORT_C APIRET APIENTRY RexxCreateQueue( PSZ Buffer,
                                 ULONG BuffLen,
                                 PSZ RequestedName,
                                 ULONG* DupFlag)
{
   int code;
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   TSD->called_from_saa = 1;
   code = IfcCreateQueue( TSD, RequestedName, (RequestedName) ? strlen( RequestedName): 0, Buffer, DupFlag, BuffLen );
   TSD->called_from_saa = 0;
   return code;
}

EXPORT_C APIRET APIENTRY RexxDeleteQueue( PSZ QueueName )
{
   int code;
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   TSD->called_from_saa = 1;
   if (!QueueName || !strlen(QueueName))
      code = RXQUEUE_BADQNAME;
   else
      code = IfcDeleteQueue( TSD, QueueName, strlen( QueueName ) );
   TSD->called_from_saa = 0;
   return code;
}

EXPORT_C APIRET APIENTRY RexxQueryQueue( PSZ QueueName,
                                ULONG* Count)
{
   int code;
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   TSD->called_from_saa = 1;
   if (!QueueName || !strlen(QueueName))
      code = RXQUEUE_BADQNAME;
   else
      code = IfcQueryQueue( TSD, QueueName, strlen( QueueName ), Count );
   TSD->called_from_saa = 0;
   return code;
}

EXPORT_C APIRET APIENTRY RexxAddQueue( PSZ QueueName,
                              PRXSTRING EntryData,
                              ULONG AddFlag)
{
   int code;
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   TSD->called_from_saa = 1;
   if (!QueueName || !strlen(QueueName))
      code = RXQUEUE_BADQNAME;
   else
      code = IfcAddQueue( TSD, QueueName, strlen( QueueName), EntryData->strptr, EntryData->strlength, AddFlag==RXQUEUE_LIFO );
   TSD->called_from_saa = 0;
   return code;
}

EXPORT_C APIRET APIENTRY RexxPullQueue( PSZ QueueName,
                               PRXSTRING DataBuf,
                               PDATETIME TimeStamp,
                               ULONG WaitFlag)
{
   int code;
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface( TSD );

   if ( WaitFlag != RXQUEUE_WAIT && WaitFlag != RXQUEUE_NOWAIT )
      return RXQUEUE_BADWAITFLAG;

   if ( DataBuf == NULL )
      return RXQUEUE_MEMFAIL;

   TSD->called_from_saa = 1;
   if ( !QueueName || !strlen( QueueName ) )
      code = RXQUEUE_BADQNAME;
   else
   {
      code = IfcPullQueue( TSD,
                           QueueName, strlen( QueueName ),
                           &DataBuf->strptr, &DataBuf->strlength,
                           WaitFlag==RXQUEUE_WAIT );
      if ( code == 0 )
      {
         if ( TimeStamp )
            TimeStamp->valid = 0;
      }
   }
   TSD->called_from_saa = 0;
   return code;
}

/* ============================================================= */
/* MacroSpace Rexx API interface */

EXPORT_C APIRET APIENTRY RexxAddMacro( PSZ FuncName,
                              PSZ SourceFile,
                              ULONG Position )
{
   return 0;
}

EXPORT_C APIRET APIENTRY RexxDropMacro( PSZ FuncName)
{
   return 0;
}

EXPORT_C APIRET APIENTRY RexxSaveMacroSpace( ULONG FuncCount,
                                    PSZ * FuncNames,
                                    PSZ MacroLibFile)
{
   return 0;
}

EXPORT_C APIRET APIENTRY RexxLoadMacroSpace( ULONG FuncCount,
                                    PSZ * FuncNames,
                                    PSZ MacroLibFile)
{
   return 0;
}

EXPORT_C APIRET APIENTRY RexxQueryMacro( PSZ FuncName,
                                PUSHORT Position )
{
   return 0;
}

EXPORT_C APIRET APIENTRY RexxReorderMacro( PSZ FuncName,
                                  ULONG Position )
{
   return 0;
}

EXPORT_C APIRET APIENTRY RexxClearMacroSpace( VOID )
{
   return 0;
}

/* ============================================================= */
/* Regina extensions */
/* see rexxsaa.h for a description */
EXPORT_C APIRET APIENTRY ReginaVersion( PRXSTRING VersionString )
{
   char low[3];
   unsigned len;
   tsd_t *TSD;

   TSD = GLOBAL_ENTRY_POINT();
   StartupInterface(TSD);

   if (!VersionString)
      goto fastexit;

   if (VersionString->strlength == 0)
   {
      if ((VersionString->strptr =
                      IfcAllocateMemory(sizeof(PARSE_VERSION_STRING))) == NULL)
         goto fastexit;
       VersionString->strlength = sizeof(PARSE_VERSION_STRING);
   }

   if ((len = VersionString->strlength) > sizeof(PARSE_VERSION_STRING))
      len = sizeof(PARSE_VERSION_STRING);
   memcpy(VersionString->strptr,PARSE_VERSION_STRING,len);

   /* sizeof includes the terminating 0. Subtract it if we should. */
   if (len > sizeof(PARSE_VERSION_STRING) - 1)
      len = sizeof(PARSE_VERSION_STRING) - 1;
   VersionString->strlength = len;

fastexit:
   low[0] = REGINA_VERSION_MINOR[0];
   if (low[0] == '0') /* atoi may have problems with leading zeros (octal) */
   {
      low[0] = REGINA_VERSION_MINOR[1];
      low[1] = '\0';
   }
   else
      low[1] = REGINA_VERSION_MINOR[1];
   low[2] = '\0';
   return( (atoi(REGINA_VERSION_MAJOR) << 8) | atoi(low) ) ;
}
