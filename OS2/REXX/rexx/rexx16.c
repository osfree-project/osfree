/*!
   $Id: rexx16.c,v 1.1 2004/08/16 06:26:35 prokushev Exp $

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

// This is attempt to provide 16-bit wrappers around Regina REXX 32-bit API
// Because most of 16-bit programs has small stack and heap we will have
// traps in most cases (Regina REXX requires bigger stack). So we execute
// another process (rexx16.exe) and communicate with it via pipes.

#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#include <os2.h>
#include <rexxsaa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"

#include "rexx.h"

USHORT _Far16 _Pascal RXTRACERESET(
         LONG pid,                        /* Process Id                  */
         LONG tid)                        /* Thread Id                   */
{
  APIRET rc;

  debug("RXTRACESET\n");
  rc = RexxResetTrace(pid, tid);
  debug("rc=%lx\n", rc);

  return rc;
}

USHORT _Far16 _Pascal RXVAR(
         PSHVBLOCK16 _Far16 PSHV16)              /* Pointer to list of SHVBLOCKs*/
{
  PSHVBLOCK PSHV;
  APIRET rc;

  debug("RXVAR\n");
  rc = RexxVariablePool(PSHV);
  debug("rc=%lx\n", rc);
  return rc;
}


SHORT _Far16 _Pascal REXXSAA(
         SHORT argc,                        /* Num of args passed to rexx */
         PRXSTRING16 argv,                  /* Array of args passed to rex*/
         PSZ16 path,                        /* [d:][path] filename[.ext]  */
         PRXSTRING16 buf,                   /* Loc of rexx proc in memory */
         PSZ16 env,                         /* ASCIIZ initial environment.*/
         SHORT type,                        /* type (command,subrtn,funct)*/
         PRXSYSEXIT16 sysexit,              /* SysExit env. names &  codes*/
         PSHORT _Far16 retc,                /* Ret code from if numeric   */
         PRXSTRING16 retv )                 /* Retvalue from the rexx proc*/
{
  PRXSTRING    a;
  PRXSTRING16  r;
  int          ulArgNum   = 0;
  RXSTRING     bf[2];
  PRXSYSEXIT   p;
  PRXSYSEXIT16 q;
  int          ulSysexNum = 0;
  PRXSTRING    retvar;
  PRXSTRING    arg   = (RXSTRING *)argv;
  PRXSTRING    sysex = (RXSTRING *)sysexit;
  APIRET       rc;
  int          i;

  debug("REXXSAA\n");

  for (; !RXZEROLENSTRING(*arg); arg++, ulArgNum++) ;

  a = (PRXSTRING) malloc (ulArgNum * sizeof(RXSTRING));

  for (i = 0, r = argv; i < ulArgNum; r++, i++)
  {
    a[i].strlength  = r->strlength;
    a[i].strptr     = r->strptr;
  }

  bf[0].strlength = buf[0].strlength;
  bf[0].strptr    = buf[0].strptr;
  bf[1].strlength = buf[1].strlength;
  bf[1].strptr    = buf[1].strptr;

  // count the number of elements in sysexit array
  for (; sysex != RXENDLST; sysex++, ulSysexNum++) ;

  // create an array of 32-bit RXSYSEXIT structures
  p = (PRXSYSEXIT) malloc (ulSysexNum * sizeof(RXSYSEXIT));

  // convert sysexits array to 32 bits
  for (i = 0, q = sysexit; i < ulSysexNum; q++, i++)
  {
    p[i].sysexit_name = q->sysexit_name;
    p[i].sysexit_code = q->sysexit_code;
  }

  rc = RexxStart(argc,
                 a,
                 path,
                 bf,
                 env,
                 type,
                 p,
                 retc,
                 retvar);

  debug("RexxStart returned\n");
  
  retv->strlength = retvar->strlength;
  strcpy(retv->strptr, retvar->strptr);

  debug("rc=%lx\n", rc);

  free(retvar);
  free(a);
  free(p);

  debug("memory released\n");

  return rc;
}


USHORT _Far16 _Pascal RXHALTSET(
         LONG PID,                         /* Process Id                  */
         LONG TID)                        /* Thread Id                   */
{
  APIRET rc;

  debug("REXXSETHALT\n");
  rc = RexxSetHalt(PID, TID);
  debug("rc=%lx\n", rc);

  return rc;
}


USHORT _Far16 _Pascal RXTRACESET(
         LONG PID,                         /* Process Id                  */
         LONG TID)                        /* Thread Id                   */
{
  APIRET rc;

  debug("REXXSETTRACE\n");
  rc = RexxSetTrace(PID, TID);
  debug("rc=%lx\n", rc);

  return rc;
}


USHORT _Far16 _Pascal RXBREAKCLEANUP(VOID)
{
  APIRET rc;

  debug("RXBREAKCLEANUP\n");
  rc = RexxBreakCleanup();
  debug("rc=%lx\n", rc);

  return rc;
}
