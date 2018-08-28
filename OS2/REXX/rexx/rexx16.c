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

// This is an attempt to provide 16-bit wrappers around Regina REXX 32-bit API
// Because most of 16-bit programs has small stack and heap we will have
// traps in most cases (Regina REXX requires bigger stack). So we execute
// another process (rexx16.exe) and communicate with it via pipes.

#if defined(__OSFREE__)
#define OS2
#define HAVE_STDLIB_H
#undef  HAVE_CONFIG_H
#include <ctype.h>
#endif

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>

#if defined(HAVE_STDLIB_H)
# include <stdlib.h>
#endif

#undef __USE_BSD
#if defined(HAVE_STRING_H)
# include <string.h>
#endif

#include "rexx.h"
#include "stk.h"

void LoadInterpreter( void );

extern int Trace;
extern int InterpreterIdx;

void null(char *fmt, ...) {};

USHORT _Far16 _Pascal RXTRACERESET(
         LONG pid,                        /* Process Id                  */
         LONG tid)                        /* Thread Id                   */
{
  APIRET rc;

  stkon();

  log("%s\n", __FUNCTION__);
  rc = RexxResetTrace(pid, tid);
  log("rc=%lx\n", rc);

  stkoff();

  return rc;
}

USHORT _Far16 _Pascal RXVAR(
         SHVBLOCK16  * _Seg16 pshv16)              /* Pointer to list of SHVBLOCKs*/
{
  PSHVBLOCK pshv;
  PSHVBLOCK p;
  APIRET rc;

  stkon();

  log("%s\n", __FUNCTION__);
  p = (PSHVBLOCK)pshv16;
  pshv = (PSHVBLOCK)malloc(sizeof(SHVBLOCK));
  
  pshv->shvname.strlength = p->shvname.strlength;
  pshv->shvname.strptr = (PSZ)p->shvname.strptr;

  pshv->shvvalue.strlength = p->shvvalue.strlength;
  pshv->shvvalue.strptr = (PSZ)p->shvvalue.strptr;

  pshv->shvnamelen = p->shvnamelen;
  pshv->shvvaluelen = p->shvvaluelen;
  pshv->shvcode = p->shvcode;
  pshv->shvret = p->shvret;

  rc = RexxVariablePool(pshv);

  free(pshv);
  log("rc=%lx\n", rc);

  stkoff();

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
         PSHORT16     retc,                 /* Ret code from if numeric   */
         PRXSTRING16  retv )                /* Retvalue from the rexx proc*/
{
  PRXSTRING    a          = NULL;
  PRXSTRING    r;
  RXSTRING     bf[2];
  PRXSTRING    b          = NULL;
  PRXSYSEXIT   p          = NULL;
  PSZ          e          = (PSZ)env;
  PRXSYSEXIT   q;
  int          ulSysexNum = 0;
  PRXSTRING    arg   = (PRXSTRING)argv;
  PSZ          pth;
  PRXSYSEXIT   sysex = (PRXSYSEXIT)sysexit;
  PRXSYSEXIT   s     = sysex;
  SHORT        ret;
  RXSTRING     retvar;
  char         retbuf[250];
  APIRET       rc;
  int          i;

  stkon();

  log("%s\n", __FUNCTION__);
  log("argc=%u\n", argc);

  if (arg)
    a = (PRXSTRING) malloc (argc * sizeof(RXSTRING));

  for (i = 0, r = (PRXSTRING)argv; i < argc; r++, i++)
  {
    if (a[i].strlength = r->strlength)
    {
      a[i].strptr = (PSZ)((PRXSTRING16)r)->strptr;
      log("argv[%lu]=%s\n", i, a[i].strptr);
    }
    else
    {
      a[i].strptr = NULL;
      log("argv[%lu]=(NULL)\n", i);
    }
  }

  if (pth = (PSZ)path)
    log("path=%s\n", pth);
  else
    log("path=(NULL)\n");

  if (buf != NULL)
  {
    b = (PRXSTRING)buf;

    b->strptr = (PSZ)buf->strptr;
    bf[0].strlength = buf->strlength;
    bf[0].strptr    = buf->strptr;
    b++;
    b->strptr = (PSZ)buf->strptr;
    bf[1].strlength = b->strlength;
    bf[1].strptr    = b->strptr;
  }

  if (!pth)
    b = bf;
  else
    b = NULL;

  log("buf=%lx\n", b);

  if (e)
    log("env=%s\n", e);
  else
    log("env=(NULL)\n");

  log("type=%x\n", type);
  log("sysex=%lx\n", sysex);

  // count the number of elements in sysexit array
  // not sure if it is correct
  for (; s; s++, ulSysexNum++) ;

  log("ulSysexNum=%lu\n", ulSysexNum);

  // create an array of 32-bit RXSYSEXIT structures
  if (sysex)
    p = (PRXSYSEXIT) malloc (ulSysexNum * sizeof(RXSYSEXIT));

  // convert sysexits array to 32 bits
  for (i = 0, q = (PRXSYSEXIT)sysexit; i < ulSysexNum; q++, i++)
  {
    p[i].sysexit_name = (PSZ)((PRXSYSEXIT16)q)->sysexit_name;
    p[i].sysexit_code = q->sysexit_code;

    if (p[i].sysexit_name)
      log("sysexit[%lu].sysexit_name=%s\n",  i, p[i].sysexit_name);
    else
      log("sysexit[%lu].sysexit_name=(NULL)\n",  i);

    log("sysexit[%lu].sysexit_code=%lx\n", i, p[i].sysexit_code);
  }

  log("calling RexxStart\n");

  MAKERXSTRING(retvar, retbuf, sizeof(retbuf));

  rc = RexxStart(argc,
                 a,
                 pth,
                 b,
                 e,
                 type,
                 p,
                 &ret,
                 &retvar);

  log("RexxStart returned\n");

  *retc = ret;
  retv->strlength = retvar.strlength;
  retv->strptr    = (PCH16)retvar.strptr;

  log("rc=%lx\n", rc);

  if (a) free(a);
  if (p) free(p);

  stkoff();

  return rc;
}


USHORT _Far16 _Pascal RXHALTSET(
         LONG pid,                        /* Process Id                  */
         LONG tid)                        /* Thread Id                   */
{
  APIRET rc;

  stkon();

  log("%s\n", __FUNCTION__);
  rc = RexxSetHalt(pid, tid);
  log("rc=%lx\n", rc);

  stkoff();

  return rc;
}


USHORT _Far16 _Pascal RXTRACESET(
         LONG pid,                        /* Process Id                  */
         LONG tid)                        /* Thread Id                   */
{
  APIRET rc;

  stkon();

  log("%s\n", __FUNCTION__);
  rc = RexxSetTrace(pid, tid);
  log("rc=%lx\n", rc);

  stkoff();

  return rc;
}

#if 1

USHORT _Far16 _Pascal RXBREAKCLEANUP(void)
{
  APIRET rc;

  stkon();

  log("%s\n", __FUNCTION__);
  rc = RexxBreakCleanup();
  log("rc=%lx\n", rc);

  stkoff();

  return rc;
}

#endif
