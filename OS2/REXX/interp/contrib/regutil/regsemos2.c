/* OS/2 semaphore functions for regutil
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
 * $Header: /opt/cvs/Regina/regutil/regsemos2.c,v 1.1 2009/10/28 02:33:15 mark Exp $
 */
#include "regutil.h"
#include <os2.h>

/* ******************************************************************** */
/* **************************** Semaphores **************************** */
/* ******************************************************************** */
static const char notimp[] = "not implemented";
#define what() memcpy(result->strptr, notimp, sizeof(notimp)-1), result->strlength = sizeof(notimp)-1

/* syscloseeventsem(handle) */
rxfunc(syscloseeventsem)
{
   what();
   return 0;
}

/* sysclosemutexsem(handle) */
rxfunc(sysclosemutexsem)
{
   what();
   return 0;
}

/* syscreateeventsem([name],[manual_reset]) */
rxfunc(syscreateeventsem)
{
   what();
   return 0;
}

/* syscreatemutexsem([name]) */
rxfunc(syscreatemutexsem)
{
   what();
   return 0;
}


/* sysopeneventsem(name) */
rxfunc(sysopeneventsem)
{
   what();
   return 0;
}


/* sysopenmutexsem(name) */
rxfunc(sysopenmutexsem)
{
   what();
   return 0;
}

/* sysposteventsem(handle) */
rxfunc(sysposteventsem)
{
   what();
   return 0;
}

/* syspulseeventsem(handle) */
rxfunc(syspulseeventsem)
{
   what();
   return 0;
}


/* sysreleasemutexsem(handle) */
rxfunc(sysreleasemutexsem)
{
   what();
   return 0;
}

/* sysrequestmutexsem(handle, [timeout]) */
rxfunc(sysrequestmutexsem)
{
   what();
   return 0;
}

/* sysreseteventsem(handle) */
rxfunc(sysreseteventsem)
{
   what();
   return 0;
}

/* syswaiteventsem(handle, [timeout]) */
rxfunc(syswaiteventsem)
{
   what();
   return 0;
}
