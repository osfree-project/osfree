/* NT semaphore functions for regutil
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is regutil.
 *
 * The Initial Developer of the Original Code is Patrick TJ McPhee.
 * Portions created by Patrick McPhee are Copyright © 1998, 2001
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /opt/cvs/Regina/regutil/regsem.c,v 1.1 2009/10/07 07:51:59 mark Exp $
 */
#include "regutil.h"
#include <windows.h>

/* ******************************************************************** */
/* **************************** Semaphores **************************** */
/* ******************************************************************** */
static const char notimp[] = "not implemented";
#define what() memcpy(result->strptr, notimp, sizeof(notimp)-1), result->strlength = sizeof(notimp)-1

/* syscloseeventsem(handle) */
rxfunc(syscloseeventsem)
{
   HANDLE semid;

   checkparam(1,1);

   if (argv[0].strlength != sizeof(semid))
      return BADGENERAL;

   memcpy(&semid, argv[0].strptr, sizeof(semid));

   CloseHandle(semid);
   result->strlength = sprintf(result->strptr, "%d", GetLastError());

   return 0;
}

/* sysclosemutexsem(handle) */
rxfunc(sysclosemutexsem)
{
   return syscloseeventsem(fname, argc, argv, pSomething, result);
}

/* syscreateeventsem([name],[manual_reset]) */
rxfunc(syscreateeventsem)
{
   char *name, *manual;
   HANDLE event;
   BOOL bManual = FALSE;

   checkparam(0,2);

   if (argc > 0 && argv[0].strlength) {
      rxstrdup(name, argv[0]);
   }
   else
      name = NULL;

   if (argc > 1 && argv[1].strlength)
      rxstrdup(manual, argv[1]);
   else
      manual = NULL;

   /* docs suggest the fact that this argument exists means the semaphore
    * should be manual-reset */
   if (manual && !stricmp(manual, "yes"))
      bManual = TRUE;
   else
      bManual = FALSE;

   event = CreateEvent(NULL, bManual, FALSE, name);

   if (event) {
      result->strlength = sizeof(event);
      memcpy(result->strptr, &event, sizeof(event));
   }

   else {
      result->strlength = 0;
   }

   return 0;
}

/* syscreatemutexsem([name]) */
rxfunc(syscreatemutexsem)
{
   HANDLE mutex;
   char * name;

   if (argc > 0 && argv[0].strlength) {
      rxstrdup(name, argv[0]);
   }
   else
      name = NULL;

   mutex = CreateMutex(NULL, FALSE, name);

   if (mutex) {
      result->strlength = sizeof(mutex);
      memcpy(result->strptr, &mutex, sizeof(mutex));
   }
   else {
      result->strlength = 0;
   }

   return 0;
}


/* sysopeneventsem(name) */
rxfunc(sysopeneventsem)
{
   HANDLE event;
   char * name;

   checkparam(1,1);

   rxstrdup(name, argv[0]);

   event = OpenEvent(EVENT_ALL_ACCESS, FALSE, name);

   if (event) {
      result->strlength = sizeof(event);
      memcpy(result->strptr, &event, sizeof(event));
   }
   else {
      result_zero();
   }

   return 0;
}


/* sysopenmutexsem(name) */
rxfunc(sysopenmutexsem)
{
   HANDLE mutex;
   char * name;

   checkparam(1,1);

   rxstrdup(name, argv[0]);

   mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, name);

   if (mutex) {
      result->strlength = sizeof(mutex);
      memcpy(result->strptr, &mutex, sizeof(mutex));
   }
   else {
      result_zero();
   }

   return 0;
}

/* sysposteventsem(handle) */
rxfunc(sysposteventsem)
{
   HANDLE event;

   checkparam(1,1);

   memcpy(&event, argv[0].strptr, sizeof(event));

   SetEvent(event);

   result->strlength = sprintf(result->strptr, "%d", GetLastError());

   return 0;
}

/* syspulseeventsem(handle) */
rxfunc(syspulseeventsem)
{
   HANDLE event;

   checkparam(1,1);

   memcpy(&event, argv[0].strptr, sizeof(event));

   PulseEvent(event);

   result->strlength = sprintf(result->strptr, "%d", GetLastError());

   return 0;
}


/* sysreleasemutexsem(handle) */
rxfunc(sysreleasemutexsem)
{
   HANDLE mutex;

   checkparam(1,1);

   memcpy(&mutex, argv[0].strptr, sizeof(mutex));

   ReleaseMutex(mutex);

   result->strlength = sprintf(result->strptr, "%d", GetLastError());

   return 0;
}

/* sysrequestmutexsem(handle, [timeout]) */
rxfunc(sysrequestmutexsem)
{
   HANDLE mutex;
   char * timeout;
   DWORD mtimeout;
   int rc;

   checkparam(1,2);

   memcpy(&mutex, argv[0].strptr, sizeof(mutex));

   if (argc > 1) {
      rxstrdup(timeout, argv[1]);
      mtimeout = atoi(timeout);
   }
   else {
      mtimeout = INFINITE;
   }

   rc = WaitForSingleObject(mutex, mtimeout);

   result->strlength = sprintf(result->strptr, "%d", rc);

   return 0;
}

/* sysreseteventsem(handle) */
rxfunc(sysreseteventsem)
{
   HANDLE event;

   checkparam(1,1);

   memcpy(&event, argv[0].strptr, sizeof(event));

   ResetEvent(event);

   result->strlength = sprintf(result->strptr, "%d", GetLastError());

   return 0;
}

/* syswaiteventsem(handle, [timeout]) */
rxfunc(syswaiteventsem)
{
   HANDLE event;
   int rc, mtimeout;
   char * timeout;

   checkparam(1,2);

   memcpy(&event, argv[0].strptr, sizeof(event));

   if (argc > 1) {
      rxstrdup(timeout, argv[1]);
      mtimeout = atoi(timeout);
   }
   else {
      mtimeout = INFINITE;
   }

   rc = WaitForSingleObject(event, mtimeout);

   result->strlength = sprintf(result->strptr, "%d", rc);

   return 0;
}
