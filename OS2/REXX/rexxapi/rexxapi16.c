/*!
   $Id: rexxapi.c,v 1.2 2004/08/16 04:51:08 prokushev Exp $

   @file rexx16.c

   @brief REXX 16-bit function - 16->32 wrappers for old REXX functions

   (c) osFree Project 2004, <http://www.osFree.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   @author Yuri Prokushev (prokushev@freemail.ru)

*/

#include <stdio.h>
#include <string.h>

#define  INCL_OS2DEF

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif
#include "configur.h"

#include <stdio.h>
#include <stdlib.h>

#define INCL_REXXSAA

#if defined(OS2)
# define INCL_DOSMODULEMGR
# define INCL_DOSMISC
# define INCL_DOSERRORS
# undef INCL_REXXSAA
# include <os2.h>
# define INCL_REXXSAA
# define DONT_TYPEDEF_PFN
# define DYNAMIC_OS2
#endif

#include "rexxapi.h"
#include "stk.h"

/*
APIRET APIENTRY RexxLoadMacroSpace( ULONG FuncCount,
                                    PSZ * FuncNames,
                                    PSZ MacroLibFile);
 */

#define RXFUNC_DYNALINK       1        /* Function Available in DLL  */
#define RXFUNC_CALLENTRY      2        /* Registered as mem entry pt.*/

#define SUBCOM_COUNT 16

void LoadInterpreter( void );

// installed handlers array
PVOID sub[SUBCOM_COUNT + 1];

extern int Trace;
extern int InterpreterIdx;

void null(char *fmt, ...) {};

USHORT _Far16 _Pascal RXMACROLOAD (
         USHORT argc,                      /* Argument count (0==save all)*/
         PSZ16  * _Seg16 argv,             /* List of funct names to save */
         PSZ16 file )                      /* File to save functions in   */
{
  PSZ *args;
  PSZ macrofile;
  char  * _Seg16 * _Seg16 p;
  int i;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  args = (PSZ *)malloc((argc + 1) * sizeof(char * _Seg16));

  for (i = 0, p = argv; i < argc; i++, p++)
      args[i] = (PSZ)(*p);

  args[argc] = NULL;

  macrofile = (PSZ)file;
  rc = RexxLoadMacroSpace(argc, args, macrofile);
  debug("rc=%lx\n", rc);

  free(args);

  stkoff();

  return rc;
}

USHORT _Far16 _Pascal RXMACROSAVE (
         USHORT argc,                      /* Argument count (0==save all)*/
         PSZ16  * _Seg16 argv,             /* List of funct names to save */
         PSZ16 file )                      /* File to save functions in   */
{
  PSZ *args;
  PSZ macrofile;
  char  * _Seg16 * _Seg16 p;
  int i;
  APIRET rc;
  
  stkon();

  debug("%s\n", __FUNCTION__);
  args = (PSZ *)malloc((argc + 1) * sizeof(char * _Seg16));

  for (i = 0, p = argv; i < argc; i++, p++)
      args[i] = (PSZ)(*p);

  args[argc] = NULL;

  macrofile = (PSZ)file;
  rc = RexxSaveMacroSpace(argc, args, macrofile);
  debug("rc=%lx\n", rc);
  free(args);

  stkoff();
  
  return rc;
}

#if 0

USHORT _Far16 _Pascal RXSUBCOMLOAD(
         PSZ16 env,                         /* Name of the Environment    */
         PSZ16 dll)                         /* DLL Module Name            */
{
  PSZ envp;
  PSZ dllp;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  envp = (PSZ)env;
  dllp = (PSZ)dll;
  rc = RexxLoadSubcom(envp, dllp);
  debug("rc=%lx\n", rc);

  stkoff();
  
  return rc;
}

#endif

USHORT _Far16 _Pascal RXMACRODROP (
         PSZ16 fn )                        /* Name of function to remove */
{
  PSZ func;
  APIRET rc;
  
  stkon();

  debug("%s\n", __FUNCTION__);
  func = (PSZ)fn;
  rc = RexxDropMacro(func);
  debug("rc=%lx\n", rc);

  stkoff();

  return rc;
}


USHORT _Far16 _Pascal RXEXITQUERY (
         PSZ16 fn,                           /* Exit name                  */
         PSZ16 dll,                          /* DLL Module name.           */
         USHORT _Far16 *flag,                /* Existance flag.            */
         //PUSHORT _Far16 flag,              /* Existance flag.            */
         UCHAR _Far16 *data )                /* User data.                 */
{
  PSZ func;
  PSZ module;
  PUSHORT Flag;
  PUCHAR Data;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  func = (PSZ)fn;
  module = (PSZ)dll;
  Flag = (PUSHORT)flag;
  Data = (PUCHAR)data;
  rc = RexxQueryExit(func, module, Flag, Data);
  debug("rc=%lx\n", rc);

  stkoff();

  return rc;
}


USHORT _Far16 _Pascal RXMACROERASE( VOID )   /* No Arguments.               */
{
  APIRET rc;
  
  stkon();

  debug("%s\n", __FUNCTION__);
  rc = RexxClearMacroSpace();
  debug("rc=%lx\n", rc);

  stkoff();
  
  return rc;
}


USHORT _Far16 _Pascal RXSUBCOMDROP(
         PSZ16 env,                          /* Name of the Environment    */
         PSZ16 dll)                          /* DLL Module Name            */
{
  PSZ Env;
  PSZ Dll;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  Env = (PSZ)env;
  Dll = (PSZ)dll;
  rc = RexxDeregisterSubcom(Env, Dll);
  debug("rc=%lx\n", rc);

  stkoff();

  return rc;
}


USHORT _Far16 _Pascal RXMACROQUERY (
         PSZ16 fn,                         /* Function to search for      */
         USHORT _Far16 *flag)              /* Ptr for position flag return*/
{
  PSZ func;
  PUSHORT pos;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  func = (PSZ)fn;
  pos = (PUSHORT)flag;
  rc = RexxQueryMacro(func, pos);
  debug("rc=%lx\n", rc);

  stkoff();

  return rc;
}

USHORT _Far16 _Pascal RXMACROCHANGE (
         PSZ16 fn,                         /* Function to add/change      */
         PSZ16 file,                       /* Name of file to get function*/
         USHORT pos)                       /* Flag indicating search pos  */
{
  PSZ func;
  PSZ source;
  APIRET rc;
  
  stkon();

  debug("%s\n", __FUNCTION__);
  func = (PSZ)fn;
  source = (PSZ)file;
  rc = RexxAddMacro(func, source, pos);
  debug("rc=%lx\n", rc);

  stkoff();

  return rc;
}


USHORT _Far16 _Pascal RXSUBCOMQUERY(
         PSZ16 env,                          /* Name of the Environment    */
         PSZ16 dll,                          /* DLL Module Name            */
         USHORT _Far16 *codestor,            /* Stor for existance code    */
         UCHAR _Far16 *userstor)             /* Stor for user word         */
{
  PSZ Env;
  PSZ Module;
  PUSHORT Flag;
  PUCHAR UserArea;
  APIRET rc;
  
  stkon();

  debug("%s\n", __FUNCTION__);
  Env = (PSZ)env;
  Module = (PSZ)dll;
  Flag = (PUSHORT)codestor;
  UserArea = (PUCHAR)userstor;
  rc = RexxQuerySubcom(Env, Module, Flag, UserArea);
  debug("rc=%lx\n", rc);

  stkoff();

  return rc;
}

#if 0

USHORT _Far16 _Pascal RXFUNCTIONCALL (
        PSZ16 fn,                           /* Name of function to call   */
        USHORT argc,                        /* Number of arguments        */
        PRXSTRING16 argv,                   /* Array of argument strings  */
        PUSHORT _Far16 ret,                 /* RC from function called    */
        PRXSTRING16 stor,                   /* Storage for returned data  */
        PSZ16 data)                         /* Name of active data queue  */
{
  APIRET rc;
  
  stkon();

  debug("%s\n", __FUNCTION__);
  rc = RexxCallFunction(...);
  debug("rc=%lx\n", rc);

  stkoff();

  return rc;
}

#endif

USHORT _Far16 _Pascal RXMACROREORDER(
         PSZ16 fn,                         /* Name of funct change order  */
         USHORT pos)                       /* New position for function   */
{
  PSZ func;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  func = (PSZ)fn;
  rc = RexxReorderMacro(func, pos);
  debug("rc=%lx\n", rc);

  stkoff();
  
  return rc;
}

#if 0

USHORT _Far16 _Pascal RXSUBCOMEXECUTE(
         PSZ16 env,                          /* Name of Subcommand Environ */
         PSZ16 dll,                          /* Module name of its' DLL    */
         PRXSTRING16 cmd,                    /* Command string to be passed*/
         PUSHORT _Far16 flag,                /* Stor for error flag notice */
         PUSHORT _Far16 ret,                 /* Stor for rc from handler   */
         PRXSTRING16 stor )                  /* Stor for returned string   */
{
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  rc = RexxCallSubcom(...);
  debug("rc=%lx\n", rc);

  stkoff();
  
  return rc;
}

#endif

APIRET APIENTRY Subcom_wrapper32(
  PRXSTRING cmd,
  PUSHORT   flags,
  PRXSTRING retstr,
  PSUBCOM   handler
)
{
  PRXSTRING16 cmd16;
  PUSHORT16   flags16;
  PRXSTRING16 retstr16;
  PSUBCOM16   handler16;
  APIRET rc;

  cmd16 = (void _Far16 *)cmd;
  cmd16->strptr = (void _Far16 *)(cmd->strptr);

  flags16 = (void _Far16 *)flags;

  retstr16 = (void _Far16 *)retstr;
  retstr16->strptr = (void _Far16 *)(retstr->strptr);

  rc = (APIRET)(*((PSUBCOM16)handler))((PRXSTRING)cmd16,
                                       (PUSHORT)flags16,
                                       (PRXSTRING)retstr16);

  retstr = (void *)retstr16;
  retstr->strptr = (void *)(retstr16->strptr);

  flags = (void *)flags16;

  cmd = (void *)cmd16;
  cmd->strptr = (void *)(cmd16->strptr);

  return rc;  
}

// wrapper template
#define SUBCOM_HANDLER(N) \ 
  APIRET APIENTRY Subcom_Handler##N (PRXSTRING cmd, PUSHORT flags, PRXSTRING retstr) \
  { \
    return Subcom_wrapper32(cmd, flags, retstr, sub[N]); \
  }

#include <wrappers.h>

PVOID sethand(PSUBCOM hand)
{
  int  i  = 0;
  void *p = NULL;

  // find first free array element
  while (i < SUBCOM_COUNT && sub[i]) i++;
  
  if (i >= SUBCOM_COUNT)
    return NULL;

  sub[i] = hand;

  p = (PVOID)(((ULONG)&Subcom_Handler1 - (ULONG)&Subcom_Handler0) * i + 
              (ULONG)&Subcom_Handler0);

  return p;
}

USHORT _Far16 _Pascal RXSUBCOMREGISTER(PSCBLOCK16 scb)
{
  PSZ    subcom_name;
  PVOID  subcom_addr;
  PVOID  subcom_user;
  PSZ dll;
  PSZ proc;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  subcom_name = (PSZ)scb->scbname;
  subcom_addr = (PVOID)scb->scbaddr;
  subcom_user = (PVOID)scb->scbuser;
  dll = (PSZ)scb->scbdll_name;
  proc = (PSZ)scb->scbdll_proc;

  debug("name=%s\n", subcom_name);
  debug("addr=%lx\n", subcom_addr);
  debug("user=%lx\n", subcom_user);
  debug("dll=%s\n", dll);
  debug("proc=%s\n", proc);

  if (*dll)
  {
      // dll
      debug("dll0\n");
      rc = RexxRegisterSubcomDll(subcom_name, dll, proc,
                                 subcom_user, scb->scbdrop_auth);
      debug("dll1\n");
  }
  else
  {
      // exe
      PVOID  p;

      // get a 32-bit wrapper instead of 16-bit handler itself
      debug("exe0\n");
      p = sethand(subcom_addr);
      debug("exe1\n");

      if (p == NULL)
      {
          debug("exe2\n");
          debug("RXSUBCOM_NOTREG\n");  
          stkoff();
          return RXSUBCOM_NOTREG;
      }
      
      debug("exe3\n");
      rc = RexxRegisterSubcomExe(subcom_name,    // environment name
                                 p,              // subcommand handler proc
                                 subcom_user);   // user area
      debug("exe4\n");
  }
    
  debug("rc=%lu\n", rc);  

  stkoff();

  debug("exit\n");
  return rc;
}

USHORT _Far16 _Pascal RXEXITREGISTER (
         PSCBLOCK16 scb )                   /* Ptr to SCBLOCK to register */
{
  PSZ    subcom_name;
  PVOID  subcom_addr;
  PVOID  subcom_user;
  PSZ dll;
  PSZ proc;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);

  subcom_name = (PSZ)scb->scbname;
  subcom_addr = (PVOID)scb->scbaddr;
  subcom_user = (PVOID)scb->scbuser;
  dll = (PSZ)scb->scbdll_name;
  proc = (PSZ)scb->scbdll_proc;

  debug("name=%s\n", subcom_name);
  debug("addr=%lx\n", subcom_addr);
  debug("user=%lx\n", subcom_user);
  debug("dll=%s\n", dll);
  debug("proc=%s\n", proc);
  
  if (*dll)
  {
      // dll
      rc = RexxRegisterExitDll(subcom_name, dll, proc,
                               subcom_user, scb->scbdrop_auth);
  }
  else
  {
      // exe
      PVOID  p;

      // get a 32-bit wrapper instead of 16-bit handler itself
      p = sethand(subcom_addr);

      if (p == NULL)
      {
          stkoff();
          return RXSUBCOM_NOTREG;
      }
      
      rc = RexxRegisterExitExe(subcom_name,    // environment name
                               p,              // subcommand handler proc
                               subcom_user);   // user area
  }
  debug("rc=%lu\n", rc);  

  stkoff();

  return rc;
}

USHORT _Far16 _Pascal RXFUNCTIONQUERY( PSZ16 fn)
{
  PSZ func;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  func = (PSZ)fn;
  rc = RexxQueryFunction(func);
  debug("rc=%lx\n", rc);

  stkoff();
  
  return rc;
}


USHORT _Far16 _Pascal RXFUNCTIONREGISTER(
        PSZ16 fn,                           /* Name of function to add    */
        PSZ16 dll,                          /* Dll file name (if in dll)  */
        PSZ16 entry,                        /* Entry in dll OR mem address*/
        USHORT flag)                        /* RX_DYNALINK || RX_CALLENTRY*/
{
  PSZ func;
  PSZ module;
  PSZ ent;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  func = (PSZ)fn;
  module = (PSZ)dll;
  ent = (PSZ)entry;

  switch (flag)
  {
  case RXFUNC_CALLENTRY:
      rc = RexxRegisterFunctionExe(module, (RexxFunctionHandler *)ent);
      break;

  case RXFUNC_DYNALINK:
      rc = RexxRegisterFunctionDll(func, module, ent);
      break;

  default:
      rc = ERROR_INVALID_FUNCTION;
  }

  debug("rc=%lx\n", rc);

  stkoff();

  return rc;
}


USHORT _Far16 _Pascal RXEXITDROP (
         PSZ16 fn,                          /* Exit name                  */
         PSZ16 dll )                        /* DLL module name            */
{
  PSZ func;
  PSZ module;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  func = (PSZ)fn;
  module = (PSZ)dll;
  rc = RexxDeregisterExit(func, module);
  debug("rc=%lx\n", rc);

  stkoff();

  return rc;
}


USHORT _Far16 _Pascal RXFUNCTIONDEREGISTER (PSZ16 fn)
{
  PSZ func;
  APIRET rc;

  stkon();

  debug("%s\n", __FUNCTION__);
  func = (PSZ)fn;
  rc = RexxDeregisterFunction(func);
  debug("rc=%lx\n", rc);

  stkoff();

  return rc;
}
