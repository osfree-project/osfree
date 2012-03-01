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
//#include "rexx16.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"

typedef unsigned short WORD;            // w

typedef WORD FAR *PWORD;                // pw

// To extract offset or selector from any FAR (16:16) pointer
#define OFFSETOF16(p)   (((PWORD)&(p))[0])
#define SEGMENTOF16(p)  (((PWORD)&(p))[1])

// To convert a tiled 16:16 address to a 0:32 address
#define MAKEFLATP(fp)   ((PVOID)((SEGMENTOF16(fp)&~7)<<13 | OFFSETOF16(fp)))


#pragma pack(2)

USHORT _Far16 _Pascal RXTRACERESET(LONG pid, LONG tid);

USHORT _Far16 _Pascal RXTRACERESET(
         LONG pid,                        /* Process Id                  */
         LONG tid)                        /* Thread Id                   */
{
  APIRET rc;

  printf("RXTRACESET\n");
  rc = RexxResetTrace(pid, tid);
  printf("rc=%lx\n", rc);

  return rc;
}

typedef char _Far16 * PCH16;
typedef char _pascal _Far16 * PSZ16;

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

LONG   APIENTRY ReginaRexxStart(LONG ,                        /* Num of args passed to rexx */
         PRXSTRING,                    /* Array of args passed to rex */
         PSZ,                          /* [d:][path] filename[.ext]  */
         PRXSTRING,                    /* Loc of rexx proc in memory */
         PSZ,                          /* ASCIIZ initial environment.*/
         LONG ,                        /* type (command,subrtn,funct) */
         PRXSYSEXIT,                   /* SysExit env. names &  codes */
         PSHORT,                       /* Ret code from if numeric   */
         PRXSTRING );                  /* Retvalue from the rexx proc */


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
  UCHAR       LoadError[255];
  RESULTCODES ChildRC;
  APIRET      rc;  /* Return code */
  CHAR prg[CCHMAXPATH];

  printf("REXXSAA\n");

  memset(prg, 0, sizeof(prg));
  strcpy(prg, "rexx.exe\0");
  sprintf(prg + 9, "%s", MAKEFLATP(path));

//  rc = DosExecPgm(LoadError,           /* Object name buffer           */
//                  sizeof(LoadError),   /* Length of object name buffer */
//                  EXEC_SYNC,           /* Asynchronous/Trace flags     */
//                  (PSZ) &prg,          /* Argument string              */
//                  NULL, //Envs,        /* Environment string           */
//                  &ChildRC,            /* Termination codes            */
//                  "rexx.exe");             /* Program file name            */

  if (rc != NO_ERROR) {
     printf("DosExecPgm error: return code = %u\n", rc);
     return 0;
  } else {
     printf("DosExecPgm complete.  Termination Code: %u  Return Code: %u\n",
             ChildRC.codeTerminate,
             ChildRC.codeResult);  /* This is explicitly set by other pgm */
  } /* endif */

  return 0;
}

USHORT _Far16 _Pascal RXHALTSET(
         LONG PID,                         /* Process Id                  */
         LONG TID);                        /* Thread Id                   */

USHORT _Far16 _Pascal RXHALTSET(
         LONG PID,                         /* Process Id                  */
         LONG TID)                        /* Thread Id                   */
{
  APIRET rc;

  printf("REXXSETHALT\n");
  rc = RexxSetHalt(PID, TID);
  printf("rc=%lx\n", rc);

  return rc;
}

USHORT _Far16 _Pascal RXTRACESET(
         LONG PID,                         /* Process Id                  */
         LONG TID);                        /* Thread Id                   */

USHORT _Far16 _Pascal RXTRACESET(
         LONG PID,                         /* Process Id                  */
         LONG TID)                        /* Thread Id                   */
{
  APIRET rc;

  printf("REXXSETTRACE\n");
  rc = RexxSetTrace(PID, TID);
  printf("rc=%lx\n", rc);

  return rc;
}


// This function not found in any of OS/2 Toolkits
USHORT _Far16 _Pascal RXBREAKCLEANUP(VOID);

USHORT _Far16 _Pascal RXBREAKCLEANUP(VOID)
{
  printf("RXBREAKCLEANUP\n");
  return 0;
}
