/* Utility functions for regutil
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
 * $Header: /netlabs.cvs/osfree/src/REXX/libs/rexxutil/regutil.c,v 1.3 2004/08/21 14:48:41 prokushev Exp $
 */
#ifdef _WIN32
# include <windows.h>

static void gettimeofday(struct timeval * tv, struct timezone * tz)
{
   SYSTEMTIME st;

   GetSystemTime(&st);

   /* this is not precisely correct, but good enough to seed a random number
    * generator. */
   tv->tv_sec = (st.wYear-1970)*31536000+
    st.wMonth*2592000+
    st.wDay*86400+
    st.wHour*3600+
    st.wMinute*60+
    st.wSecond;
   tv->tv_usec = st.wMilliseconds * 1000;
}


#else
# include <sys/time.h>
#endif
#include "regutil.h"

/* ******************************************************************** */
/* ************************ RegUtil functions ************************* */
/* ******************************************************************** */

struct {
    char * name;
    APIRET (APIENTRY*funcptr)(PUCHAR fname, ULONG argc, PRXSTRING argv, PSZ pSomething, PRXSTRING result);
} funclist[] = {
    {"SYSADDREXXMACRO", sysaddrexxmacro},
    {"SYSCLEARREXXMACROSPACE", sysclearrexxmacrospace},
    {"SYSDROPREXXMACRO", sysdroprexxmacro},
    {"SYSLOADREXXMACROSPACE", sysloadrexxmacrospace},
    {"SYSQUERYREXXMACRO", sysqueryrexxmacro},
    {"SYSREORDERREXXMACRO", sysreorderrexxmacro},
    {"SYSSAVEREXXMACROSPACE", syssaverexxmacrospace},
    {"SYSCLS", syscls},
    {"SYSCURPOS", syscurpos},
    {"SYSCURSTATE", syscurstate},
    {"SYSGETKEY", sysgetkey},
    {"SYSTEXTSCREENREAD", systextscreenread},
    {"SYSTEXTSCREENSIZE", systextscreensize},
    {"SYSCLOSEEVENTSEM", syscloseeventsem},
    {"SYSCLOSEMUTEXSEM", sysclosemutexsem},
    {"SYSCREATEEVENTSEM", syscreateeventsem},
    {"SYSCREATEMUTEXSEM", syscreatemutexsem},
    {"SYSOPENEVENTSEM", sysopeneventsem},
    {"SYSOPENMUTEXSEM", sysopenmutexsem},
    {"SYSPOSTEVENTSEM", sysposteventsem},
    {"SYSPULSEEVENTSEM", syspulseeventsem},
    {"SYSRELEASEMUTEXSEM", sysreleasemutexsem},
    {"SYSREQUESTMUTEXSEM", sysrequestmutexsem},
    {"SYSRESETEVENTSEM", sysreseteventsem},
    {"SYSWAITEVENTSEM", syswaiteventsem},
    {"SYSINI", sysini},
    {"SYSBOOTDRIVE", sysbootdrive},
    {"SYSWINVER", syswinver},
    {"SYSOS2VER", syswinver},
    {"SYSLINVER", syswinver},
    {"SYSVERSION", syswinver},
    {"SYSUTILVERSION", sysutilversion},
    {"SYSDRIVEINFO", sysdriveinfo},
    {"SYSDRIVEMAP", sysdrivemap},
    {"SYSSETFILEDATETIME", syssetfiledatetime},
    {"SYSGETFILEDATETIME", sysgetfiledatetime},
    {"SYSDUMPVARIABLES", sysdumpvariables},
    {"SYSSTEMSORT", sysstemsort},
    {"SYSSTEMCOPY", sysstemcopy},
    {"SYSSTEMDELETE", sysstemdelete},
    {"SYSSTEMINSERT", syssteminsert},
    {"SYSSETPRIORITY", syssetpriority},
    {"SYSSLEEP", syssleep},
    {"SYSSWITCHSESSION", sysswitchsession},
    {"SYSSYSTEMDIRECTORY", syssystemdirectory},
    {"SYSVOLUMELABEL", sysvolumelabel},
    {"SYSWAITNAMEDPIPE", syswaitnamedpipe},
    {"SYSFILESEARCH", sysfilesearch},
    {"SYSFILEDELETE", sysfiledelete},
    {"SYSFILETREE", sysfiletree},
    {"SYSFILESEARCH", sysfilesearch},
    {"SYSMKDIR", sysmkdir},
    {"SYSRMDIR", sysrmdir},
    {"SYSSEARCHPATH", syssearchpath},
    {"SYSTEMPFILENAME", systempfilename},
    {"SYSFILESYSTEMTYPE", sysfilesystemtype},
    {"SYSFILETREE", sysfiletree},
    {"SYSCOPYOBJECT", syscopyobject},
    {"SYSMOVEOBJECT", sysmoveobject},
    {"SYSCREATESHADOW", syscreateshadow},
    {"REGSTEMDOOVER", regstemdoover},
    {"REGSTEMREAD", regstemread},
    {"REGSTEMWRITE", regstemwrite},
    {"REGSTEMSEARCH", regstemsearch},
    {"SYSQUERYPROCESS", sysqueryprocess},
    {"RXMESSAGEBOX", rxmessagebox},
    {"RXWINEXEC", rxwinexec},
    {"SYSGETERRORTEXT", sysgeterrortext},
    {"SYSTOUNICODE", systounicode},
    {"SYSFROMUNICODE", sysfromunicode},
    {"SYSWINENCRYPTFILE", syswinencryptfile},
    {"SYSWINDECRYPTFILE", syswindecryptfile},

    {"SYSLOADFUNCS", sysloadfuncs},
    {"SYSDROPFUNCS", sysdropfuncs}
};


void init_random(void)
{
   static rxbool done = false;
   if (!done) {
      struct timeval tv;

      gettimeofday(&tv, NULL);
      srandom(tv.tv_sec|tv.tv_usec);

      done = true;
   }
}


#ifdef DYNAMIC_STATIC
/* for static loading -- see the Regina distribution for details */
extern void *getRexxUtilFunctionAddress( char *name )
{
   register int i;
   void * fn = NULL;

   /* search from the bottom to make calling sysloadfuncs faster */
   for (i = DIM(funclist)-1; i >= 0; i--) {
      if (!strcasecmp(name, funclist[i].name)) {
         fn = (void *)funclist[i].funcptr;
      }
   }

   return fn;
}
#endif

/* sysloadfuncs() */
rxfunc(sysloadfuncs)
{
    register int i;

    checkparam(0,0);

    for (i = 0; i < DIM(funclist); i++) {
	RexxRegisterFunctionExe(funclist[i].name, funclist[i].funcptr);
    }

    result_zero();

    return 0;
}


/* sysdropfuncs() */
rxfunc(sysdropfuncs)
{
    register int i;
    checkparam(0,0);

    for (i = 0; i < DIM(funclist); i++) {
	RexxDeregisterFunction(funclist[i].name);
    }

    result_zero();
    return 0;
}
