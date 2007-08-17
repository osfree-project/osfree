/* Prototypes for regutil
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
 * $Header: /netlabs.cvs/osfree/src/REXX/libs/rexxutil/regutil.h,v 1.3 2004/08/21 14:48:41 prokushev Exp $
 */


#include "rxproto.h"

typedef enum {
   false,
   true
} rxbool;

void init_random(void);

rxfunc(sysaddrexxmacro);
rxfunc(sysclearrexxmacrospace);
rxfunc(sysdroprexxmacro);
rxfunc(sysloadrexxmacrospace);
rxfunc(sysqueryrexxmacro);
rxfunc(sysreorderrexxmacro);
rxfunc(syssaverexxmacrospace);
rxfunc(syscls);
rxfunc(syscurpos);
rxfunc(syscopyobject);
rxfunc(sysmoveobject);
rxfunc(syscreateshadow);
rxfunc(syscurstate);
rxfunc(sysgetkey);
rxfunc(systextscreenread);
rxfunc(systextscreensize);
rxfunc(syscloseeventsem);
rxfunc(sysclosemutexsem);
rxfunc(syscreateeventsem);
rxfunc(syscreatemutexsem);
rxfunc(sysopeneventsem);
rxfunc(sysopenmutexsem);
rxfunc(sysposteventsem);
rxfunc(syspulseeventsem);
rxfunc(sysreleasemutexsem);
rxfunc(sysrequestmutexsem);
rxfunc(sysreseteventsem);
rxfunc(syswaiteventsem);
rxfunc(sysbootdrive);
rxfunc(syswinver);
rxfunc(sysos2ver);
rxfunc(sysversion);
rxfunc(sysutilversion);
rxfunc(syslinver);
rxfunc(sysdriveinfo);
rxfunc(sysdrivemap);
rxfunc(syssetpriority);
rxfunc(syssleep);
rxfunc(sysswitchsession);
rxfunc(syssystemdirectory);
rxfunc(sysvolumelabel);
rxfunc(sysloadfuncs);
rxfunc(sysdropfuncs);
rxfunc(syswaitnamedpipe);

rxfunc(sysfiledelete);
rxfunc(sysfiletree);
rxfunc(sysfilesearch);
rxfunc(sysmkdir);
rxfunc(sysrmdir);
rxfunc(syssearchpath);
rxfunc(sysfilesystemtype);
rxfunc(systempfilename);
rxfunc(syssetfiledatetime);
rxfunc(sysgetfiledatetime);
rxfunc(sysdumpvariables);
rxfunc(sysqueryprocess);
rxfunc(sysstemcopy);
rxfunc(sysstemdelete);
rxfunc(syssteminsert);
rxfunc(sysstemsort);
rxfunc(sysutilversion);
rxfunc(sysversion);

rxfunc(sysgeterrortext);
rxfunc(systounicode);
rxfunc(sysfromunicode);
rxfunc(syswindecryptfile);
rxfunc(syswinencryptfile);

rxfunc(rxmessagebox);
rxfunc(rxwinexec);

/* functions specific to this package */
rxfunc(regstemdoover);
rxfunc(regstemread);
rxfunc(regstemwrite);
rxfunc(regstemsearch);


/* map to a registry function */
rxfunc(sysini);
rxfunc(sysos2ver);

/* classic rexx functions which don't have an analog here */
rxfunc(SysCreateObject);
rxfunc(SysDeregisterObjectClass);
rxfunc(SysDestroyObject);
rxfunc(SysGetEA);
rxfunc(SysGetMessage);
rxfunc(SysOpenObject);
rxfunc(SysPutEA);
rxfunc(SysQueryClassList);
rxfunc(SysRegisterObjectClass);
rxfunc(SysSaveObject);
rxfunc(SysSetIcon);
rxfunc(SysSetObjectData);

