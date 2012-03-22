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

#include "rexxapi.h"
#include "stk.h"

/*
APIRET APIENTRY RexxLoadMacroSpace( ULONG FuncCount,
                                    PSZ * FuncNames,
                                    PSZ MacroLibFile);
 */

#define SUBCOM_COUNT 16

// installed handlers array
PVOID sub[SUBCOM_COUNT + 1];

void null(char *fmt, ...) {};

USHORT _Far16 _Pascal RXMACROLOAD (
         USHORT argc,                      /* Argument count (0==save all)*/
         PSZ16 _Far16 *argv,                   /* List of funct names to save */
         PSZ16 file )                       /* File to save functions in   */
{
  debug("RXMACROLOAD\n");
  return 0;
}


USHORT _Far16 _Pascal RXMACROSAVE (
         USHORT argc,                      /* Argument count (0==save all)*/
         PSZ16 _Far16 *argv,                   /* List of funct names to save */
         PSZ16 file )                       /* File to save functions in   */
{
  debug("RXMACROSAVE\n");
  return 0;
}


USHORT _Far16 _Pascal RXSUBCOMLOAD(
         PSZ16 env,                          /* Name of the Environment    */
         PSZ16 dll)                         /* DLL Module Name            */
{
  debug("RXSUBCOMLOAD\n");
  return 0;
}


USHORT _Far16 _Pascal RXMACRODROP (
         PSZ16 fn )                        /* Name of function to remove */
{
  debug("RXMACRODROP\n");
  return 0;
}


USHORT _Far16 _Pascal RXEXITQUERY (
         PSZ16 fn,                          /* Exit name                  */
         PSZ16 dll,                          /* DLL Module name.           */
         PUSHORT _Far16 flag,                      /* Existance flag.            */
         double _Far16 *data )               /* User data.                 */
{
  debug("RXEXITQUERY\n");
  return 0;
}


USHORT _Far16 _Pascal RXMACROERASE(
         VOID )                      /* No Arguments.               */
{
  debug("RXMACROERASE\n");
  return 0;
}


USHORT _Far16 _Pascal RXSUBCOMDROP(
         PSZ16 env,                          /* Name of the Environment    */
         PSZ16 dll)                         /* DLL Module Name            */
{
  debug("RXSUBCOMDROP\n");
  return 0;
}


USHORT _Far16 _Pascal RXMACROQUERY (
         PSZ16 fn,                         /* Function to search for      */
         PUSHORT _Far16 flag)                   /* Ptr for position flag return*/
{
  debug("RXMACROQUERY\n");
  return 0;
}


USHORT _Far16 _Pascal RXMACROCHANGE (
         PSZ16 fn,                         /* Function to add/change      */
         PSZ16 file,                         /* Name of file to get function*/
         USHORT flag)                    /* Flag indicating search pos  */
{
  debug("RXMACROCHANGE\n");
  return 0;
}


USHORT _Far16 _Pascal RXSUBCOMQUERY(
         PSZ16 env,                          /* Name of the Environment    */
         PSZ16 dll,                          /* DLL Module Name            */
         PUSHORT _Far16 codestor,                      /* Stor for existance code    */
         double _Far16 *userstor)                /* Stor for user word         */
{
  debug("RXSUBCOMQUERY\n");
  return 0;
}


USHORT _Far16 _Pascal RXFUNCTIONCALL (
        PSZ16 fn,                           /* Name of function to call   */
        USHORT argc,                        /* Number of arguments        */
        PRXSTRING16 argv,                     /* Array of argument strings  */
        PUSHORT _Far16 rc,                       /* RC from function called    */
        PRXSTRING16 stor,                     /* Storage for returned data  */
        PSZ16 data)                         /* Name of active data queue  */
{
  debug("RXFUNCTIONCALL\n");
  return 0;
}


USHORT _Far16 _Pascal RXEXITREGISTER (
         PSCBLOCK16 scb )                   /* Ptr to SCBLOCK to register */
{
  debug("RXEXITREGISTER\n");
  return 0;
}


USHORT _Far16 _Pascal RXMACROREORDER(
         PSZ16 fn,                         /* Name of funct change order  */
         USHORT pos)                    /* New position for function   */
{
  debug("RXMACROREORDER\n");
  return 0;
}


USHORT _Far16 _Pascal RXSUBCOMEXECUTE(
         PSZ16 env,                          /* Name of Subcommand Environ */
         PSZ16 dll,                          /* Module name of its' DLL    */
         PRXSTRING16 cmd,                    /* Command string to be passed*/
         PUSHORT _Far16 flag,                      /* Stor for error flag notice */
         PUSHORT _Far16 rc,                      /* Stor for rc from handler   */
         PRXSTRING16 stor )                  /* Stor for returned string   */
{
  debug("RXSUBCOMEXECUTE\n");
  return 0;
}


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

// 32-bit handler wrappers pool
SUBCOM_HANDLER(0)
SUBCOM_HANDLER(1)
SUBCOM_HANDLER(2)
SUBCOM_HANDLER(3)
SUBCOM_HANDLER(4)
SUBCOM_HANDLER(5)
SUBCOM_HANDLER(6)
SUBCOM_HANDLER(7)
SUBCOM_HANDLER(8)
SUBCOM_HANDLER(9)
SUBCOM_HANDLER(10)
SUBCOM_HANDLER(11)
SUBCOM_HANDLER(12)
SUBCOM_HANDLER(13)
SUBCOM_HANDLER(14)
SUBCOM_HANDLER(16)

PVOID sethand(PSUBCOM hand)
{
  int  i  = 1;
  void *p = NULL;

  // find first free array element
  while (i < SUBCOM_COUNT && sub[i]) i++;
  
  if (i >= SUBCOM_COUNT)
    return NULL;

  sub[i] = hand;

  p = (PVOID)(((ULONG)&Subcom_Handler2 - (ULONG)&Subcom_Handler1) * (i - 1) + 
              (ULONG)&Subcom_Handler1);

  return p;
}

USHORT _Far16 _Pascal RXSUBCOMREGISTER(PSCBLOCK16 PSCB)
{
  PSZ        subcom_name;
  PVOID      subcom_addr;
  PVOID      subcom_user;
  PVOID      p;
  APIRET     rc;

  stkon();
  subcom_name = (PSZ)PSCB->scbname;
  subcom_addr = (PVOID)PSCB->scbaddr;
  subcom_user = (PVOID)PSCB->scbuser;

  debug("RXSUBCOMREGISTER\n");
  debug("name=%s\n", subcom_name);
  debug("addr=%lx\n", subcom_addr);
  debug("user=%lx\n", subcom_user);

  // get a 32-bit wrapper instead of 16-bit handler itself
  p = sethand(subcom_addr);

  if (p == NULL)
    return RXSUBCOM_NOTREG;

  rc = RexxRegisterSubcomExe(subcom_name,    // environment name
                             p,              // subcommand handler proc
                             subcom_user);   // user area
  debug("rc=%lu\n", rc);  
  stkoff();

  return rc;
}


USHORT _Far16 _Pascal RXFUNCTIONQUERY( PSZ16 fn)
{
  debug("RXFUNCTIONQUERY\n");
  return 0;
}


USHORT _Far16 _Pascal RXFUNCTIONREGISTER(
        PSZ16 fn,                           /* Name of function to add    */
        PSZ16 dll,                           /* Dll file name (if in dll)  */
        PSZ16 entry,                           /* Entry in dll OR mem address*/
        USHORT flag)                       /* RX_DYNALINK || RX_CALLENTRY*/
{
  debug("RXFUNCTIONREGISTER\n");
  return 0;
}


USHORT _Far16 _Pascal RXEXITDROP (
         PSZ16 fn,                          /* Exit name                  */
         PSZ16 dll )                        /* DLL module name            */
{
  debug("RXEXITDROP\n");
  return 0;
}


USHORT _Far16 _Pascal RXFUNCTIONDEREGISTER (PSZ16 fn)
{
  debug("RXFUNCTIONDEREGISTER\n");
  return 0;
}
