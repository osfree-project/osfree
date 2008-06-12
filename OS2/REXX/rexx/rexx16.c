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

#include <os21x\os2.h>
#include <rexxsaa.h>
//#include "rexx16.h"
#include <stdio.h>

#pragma pack(2)

USHORT _Far16 _Pascal RXTRACERESET(LONG pid, LONG tid);

USHORT _Far16 _Pascal RXTRACERESET(
         LONG pid,                        /* Process Id                  */
         LONG tid)                        /* Thread Id                   */
{
  return RexxResetTrace(pid, tid);
}

typedef char _Far16 * PCH16;
typedef char _Far16 * PSZ16;

typedef struct {
   ULONG           strlength;          /*   length of string         */
   PCH16           strptr;             /*   far pointer to string    */
   } RXSTRING16;

typedef RXSTRING16 _Far16 *PRXSTRING16;       /* pointer to a RXSTRING      */

typedef struct shvnode {
    struct shvnode _Far16 *shvnext;      /* pointer to the next block   */
    RXSTRING16         shvname;       /* Pointer to the name buffer  */
    RXSTRING16         shvvalue;      /* Pointer to the value buffer */
    ULONG              shvnamelen;    /* Length of the name value    */
    ULONG              shvvaluelen;   /* Length of the fetch value   */
    UCHAR              shvcode;       /* Function code for this block*/
    UCHAR              shvret;        /* Individual Return Code Flags*/
    } SHVBLOCK16;

typedef SHVBLOCK16 _Far16 *PSHVBLOCK16;

USHORT _Far16 _Pascal RXVAR(PSHVBLOCK16 _Far16 PSHV);

USHORT _Far16 _Pascal RXVAR(
         PSHVBLOCK16 _Far16 PSHV16)              /* Pointer to list of SHVBLOCKs*/
{
  PSHVBLOCK PSHV;

  // Convert 16-bit structure to 32-bit
//  return RexxVariablePool(PSHV);
  // Free allocated memory
  printf("RXVAR\n");
  return 0;
}

typedef struct {
   PSZ16 sysexit_name;                 /* subcom enviro for sysexit  */
   SHORT sysexit_code;                 /* sysexit function code      */
   } RXSYSEXIT16;

typedef RXSYSEXIT16 _Far16 *PRXSYSEXIT16;     /* pointer to a RXSYSEXIT     */

#pragma pack()

SHORT _Far16 _Pascal REXXSAA(
         SHORT argc,                        /* Num of args passed to rexx */
         PRXSTRING16 argv,                  /* Array of args passed to rex*/
         PSZ16  path,                       /* [d:][path] filename[.ext]  */
         PRXSTRING16 buf,                   /* Loc of rexx proc in memory */
         PSZ16 env,                         /* ASCIIZ initial environment.*/
         SHORT type,                        /* type (command,subrtn,funct)*/
         PRXSYSEXIT16 sysexit,              /* SysExit env. names &  codes*/
         PSHORT _Far16 retc,                /* Ret code from if numeric   */
         PRXSTRING16 retv );                /* Retvalue from the rexx proc*/

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
  LONG rc;
  SHORT           rexxrc = 0;
  RXSTRING        rexxresult;

  rexxresult.strlength=0;
  rc=RexxStart(
         0,//argc,
         NULL,//&argv32,
         "test",
         NULL,//Instore,
         "CMD",//env32,
         RXCOMMAND, //type,
         NULL,//sysexit32,
         &rexxrc,
         &rexxresult); //&retv32

  return rc;
}

USHORT _Far16 _Pascal RXHALTSET(
         LONG PID,                         /* Process Id                  */
         LONG TID);                        /* Thread Id                   */

USHORT _Far16 _Pascal RXHALTSET(
         LONG PID,                         /* Process Id                  */
         LONG TID)                        /* Thread Id                   */
{
  return RexxSetHalt(PID, TID);
}

USHORT _Far16 _Pascal RXTRACESET(
         LONG PID,                         /* Process Id                  */
         LONG TID);                        /* Thread Id                   */

USHORT _Far16 _Pascal RXTRACESET(
         LONG PID,                         /* Process Id                  */
         LONG TID)                        /* Thread Id                   */
{
  return RexxSetTrace(PID, TID);
}


// This function not found in any of OS/2 Toolkits
USHORT _Far16 _Pascal RXBREAKCLEANUP(VOID);

USHORT _Far16 _Pascal RXBREAKCLEANUP(VOID)
{
  printf("RXBREAKCLEANUP\n");
  return 0;
}
