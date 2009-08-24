/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005 Rexx Language Association. All rights reserved.         */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/******************************************************************************/
/* REXX Library                                                 oryxlib.h     */
/* Function Prototypes and Type Definitions                                   */
/*                                                                            */
/******************************************************************************/

#ifndef ORYXLIB_H
#define ORYXLIB_H

/******************************************************************************/
/* Dummy class definitions required for "look-a-head" declarations            */
/******************************************************************************/

  class ActivityTable;
  class RexxActivation;
  class RexxActivity;
  class RexxActivityClass;
  class RexxArray;
  class RexxBehaviour;
  class RexxBuffer;
  class RexxClass;
  class RexxClause;
  class RexxCode;
  class RexxCompoundVariable;
  class RexxDirectory;
  class RexxDoBlock;
  class RexxEnvelope;
  class RexxExpressionMessage;
  class RexxExpressionStack;
  class RexxHashTable;
  class RexxHashTableCollection;
  class RexxInteger;
  class RexxIntegerClass;
  class RexxInstruction;
  class RexxInstructionCallBase;
  class RexxInstructionDo;
  class RexxInstructionEnd;
  class RexxInstructionEndIf;
  class RexxInstructionForward;
  class RexxInstructionIf;
  class RexxInstructionOtherWise;
  class RexxInstructionRaise;
  class RexxInternalObject;
  class RexxList;
  class RexxListClass;
  class RexxListTable;
  class RexxMemory;
  class RexxMessage;
  class RexxMethod;
  class RexxMethodClass;
  class RexxNativeActivation;
  class RexxNativeCode;
  class RexxNativeCodeClass;
  class RexxNumberString;
  class RexxNumberStringClass;
  class RexxObject;
  class RexxObjectTable;
  class RexxParseVariable;
  class RexxQueue;
  class RexxRelation;
  class RexxStack;
  class RexxSaveStack;
  class RexxStem;
  class RexxStemVariable;
  class RexxStringClass;
  class RexxString;
  class RexxSOMCode;
  class RexxSOMProxy;
  class RexxSOMProxyClass;
  class RexxSource;
  class RexxSupplier;
  class RexxSmartBuffer;
  class RexxTable;
  class RexxTarget;
  class RexxToken;
  class RexxTrigger;
  class RexxVariable;
  class RexxVariableBase;
  class RexxVariableDictionary;
  class MemorySegmentPool;
#ifdef SOM
  class OrxSOMMethodInformation;
  class OrxSOMArgumentList;
#endif

typedef RexxObject *OREF;              /* reference to a REXX object        */
typedef OREF *POREF;                   /* Pointer to a REXX object reference*/

#define OREF_NULL NULL                 /* definition of a NULL REXX object  */

typedef double *PDBL;                  /* pointer to double                 */

#ifndef _REXXDATETIME                  /* prevent muliple decls             */
#define _REXXDATETIME
typedef struct _REXXDATETIME {         /* REXX time stamp format            */
  unsigned short hours;                /* hour of the day (24-hour)         */
  unsigned short minutes;              /* minute of the hour                */
  unsigned short seconds;              /* second of the minute              */
  unsigned short hundredths;           /* hundredths of a second            */
  unsigned short day;                  /* day of the month                  */
  unsigned short month;                /* month of the year                 */
  unsigned short year;                 /* current year                      */
  unsigned short weekday;              /* day of the week                   */
  unsigned long  microseconds;         /* microseconds                      */
  unsigned long  yearday;              /* day number within the year        */
  unsigned short valid;                /* valid time stamp marker           */
} REXXDATETIME;
#endif /* _REXXDATETIME */

#include "RexxPlatformDefinitions.h"

/* min and max macros */
#ifndef max
#define max(a,b)        (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)        (((a) < (b)) ? (a) : (b))
#endif

/*** following macros are older names for the above ***/

/* non-ANSI string functions */
#ifdef NEED_NON_ANSI
char *strdup (const char *str);
char *strupr (char *str);
char *strlwr (char *str);
int stricmp (const char *s1, const char *s2);
int memicmp (void *m1, void *m2, unsigned int count);
#endif

#ifdef SHARED
/* Support for shared semaphores */
void OryxPostEventSem (SEV *psem);
void OryxResetEventSem (SEV *psem);
void OryxRequestWaitMutexSem (SMTX *psem);
void OryxReleaseMutexSem (SMTX *psem);
void OryxWaitEventSem (SEV *psem);
#endif /*SHARED*/

#endif /*ORYXLIB_H*/