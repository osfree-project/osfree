/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysSetIcon
*      SysRegisterObjectClass
*      SysDeregisterObjectClass
*      SysCreateObject
*      SysQueryClassList
*      SysDestroyObject
*      SysSetObjectData
*      SysSaveObject
*      SysOpenObject
*      SysMoveObject
*      SysCopyObject
*      SysCreateShadow
*      SysReplaceObject   // new
*
*  Michael Greene, January 2008
*
*  13 Jan 08 - First full version
*  14 Jan 08 - Add SysReplaceObject
*  15 Jan 08 - WININIT/WINTERM macros added based on advice from
*              some helpful people
*
------------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/******************************************************************************/

#define INCL_WINWORKPLACE
#define INCL_WINPOINTERS

#define  INCL_BASE
#define  INCL_PMWP
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <alloca.h>
#include <io.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

// From toolkit wpobject.h : Module Name: WPTYPES
/*  Views for the wpOpen(), wpFilterMenu and wpModifyMenu methods */
#define OPEN_UNKNOWN      -1
#define OPEN_DEFAULT       0
#define OPEN_CONTENTS      1
#define OPEN_SETTINGS      2
#define OPEN_HELP          3
#define OPEN_RUNNING       4
#define OPEN_PROMPTDLG     5
#define OPEN_PALETTE       121
#define CLOSED_ICON        122
#define OPEN_USER          0x6500
// end // From toolkit wpobject.h

RexxFunctionHandler SysSetIcon;
RexxFunctionHandler SysRegisterObjectClass;
RexxFunctionHandler SysDeregisterObjectClass;
RexxFunctionHandler SysCreateObject;
RexxFunctionHandler SysQueryClassList;
RexxFunctionHandler SysDestroyObject;
RexxFunctionHandler SysSetObjectData;
RexxFunctionHandler SysSaveObject;
RexxFunctionHandler SysOpenObject;
RexxFunctionHandler SysMoveObject;
RexxFunctionHandler SysCopyObject;
RexxFunctionHandler SysCreateShadow;
RexxFunctionHandler SysReplaceObject;


/*************************************************************************
* Function:  SysSetIcon                                                  *
*                                                                        *
* Syntax:    call SysSetIcon filename, iconfilename                      *
*                                                                        *
* Params:    filename     - name of the target file whose icon is        *
*                           to be set                                    *
*            iconfilename - name of the OS/2 icon file (.ICO) which      *
*                           contains the icon data to be used            *
*                                                                        *
* Return:    WinSetFileIcon API return code. Possible values are 1       *
*            if the icon was set successfully, or 0 otherwise            *
*************************************************************************/

unsigned long SysSetIcon(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc;

    ICONINFO icon;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 2 || !RXVALIDSTRING(args[0]) ||
           !RXVALIDSTRING(args[1])) return INVALID_ROUTINE;

    WININIT(WinIntial,AnchBlk)

    if(access(args[0].strptr, F_OK) !=0) {
        WINTERM(WinIntial,WinIntial)
        RETVAL(0)
    }

    if(access(args[1].strptr, F_OK) !=0) {
        WINTERM(WinIntial,WinIntial)
        RETVAL(0)
    }

    icon.fFormat     = ICON_FILE;
    icon.pszFileName = args[1].strptr;
    icon.cb          = sizeof(ICONINFO);

    rc = WinSetFileIcon(args[0].strptr, &icon);

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysRegisterObjectClass                                      *
*                                                                        *
* Syntax:    call SysRegisterObjectClass classname, module               *
*                                                                        *
* Params:    classname - The name of the new object class, as defined    *
*                        within the specified module.                    *
*            module - The name of the DLL containing the object class    *
*                     definition. If this DLL is not located in the      *
*                     system LIBPATH, it should be specified as a        *
*                     fully-qualified filename.                          *
*                                                                        *
* Return:    WinRegisterObjectClass  API return code. Possible values    *
*            are 1 if the icon was set successfully, or 0 otherwise      *
*************************************************************************/

unsigned long SysRegisterObjectClass(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 2 || !RXVALIDSTRING(args[0]) ||
           !RXVALIDSTRING(args[1])) return INVALID_ROUTINE;

    WININIT(WinIntial,AnchBlk)

    rc = WinRegisterObjectClass(args[0].strptr, args[1].strptr);

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysCreateObject                                             *
*                                                                        *
* Syntax:    call SysCreateObject classname,title,location,              *
*                                                [setup],[option]        *
*                                                                        *
* Params:    classname- name of the Workplace Shell class                *
*            title    - title of the object being created                *
*            location - location in which the new object will be created *
*            setup    - Workplace Shell setup string                     *
*            option   - action to take if the object already exists      *
*                                                                        *
* Return:    1 for successful object creation, or 0 for failure          *
*                                                                        *
*************************************************************************/

unsigned long SysCreateObject(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    unsigned long flags = CO_FAILIFEXISTS;  // default option
    char *SetupString   = NULL;             // default setup string

    if (numargs < 3 || numargs > 5 || !RXVALIDSTRING(args[0]) ||
          !RXVALIDSTRING(args[1]) || !RXVALIDSTRING(args[2]))
           return INVALID_ROUTINE;

    if(numargs >= 4 && RXVALIDSTRING(args[3])) {
        SetupString = args[3].strptr;
    }

    if(numargs == 5) {
        strupr( args[4].strptr );

        switch (args[4].strptr[0]) {

        case 'F':
            flags = CO_FAILIFEXISTS;
            break;

        case 'R':
            flags = CO_REPLACEIFEXISTS;
            break;

        case 'U':
            flags = CO_UPDATEIFEXISTS;
            break;

        default:
            return INVALID_ROUTINE;
        }
    }

    WININIT(WinIntial,AnchBlk)

    rc = WinCreateObject(args[0].strptr, args[1].strptr, SetupString,
                           args[2].strptr, flags);

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysDeregisterObjectClass                                    *
*                                                                        *
* Syntax:    call SysDeregisterObjectClass classname                     *
*                                                                        *
* Params:    classname - The object class name to deregister             *
*                                                                        *
* Return:    WinDeregisterObjectClass API return code. Possible values   *
*            are 1 if the icon was set successfully, or 0 otherwise      *
*************************************************************************/

unsigned long SysDeregisterObjectClass(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 1 || !RXVALIDSTRING(args[0])) return INVALID_ROUTINE;

    WININIT(WinIntial,AnchBlk)

    rc = WinDeregisterObjectClass(args[0].strptr);

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysDestroyObject                                            *
*                                                                        *
* Syntax:    call SysDestroyObject object                                *
*                                                                        *
* Params:    object - name of the object to destroy                      *
*                                                                        *
* Return:    0 if fail, 1 if succeed.                                    *
*************************************************************************/

unsigned long SysDestroyObject(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc = FALSE;

    HOBJECT  orgObject;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 1) return INVALID_ROUTINE;

    WININIT(WinIntial,AnchBlk)

    orgObject = WinQueryObject(args[0].strptr);

    if (orgObject) rc = WinDestroyObject(orgObject);

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysQueryClassList                                           *
*                                                                        *
* Syntax:    call SysQueryClassList stem                                 *
*                                                                        *
* Params:    stem  -  name of a stem variable in which the list of       *
*                     classes will be saved                              *
*                                                                        *
* Return:    WinSetFileIcon API return code. Possible values are 1       *
*            if the icon was set successfully, or 0 otherwise            *
*************************************************************************/

unsigned long SysQueryClassList(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    int  countClass = 0;

    char *szStemName = NULL;
    char *pszStemIdx = NULL;
    char *szValue    = NULL;

    unsigned long rc;
    unsigned long objSize = 0;

    OBJCLASS *ClassBuffer;
    OBJCLASS *ptrBuffer;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    /* Only one argument accepted */
    if (numargs != 1 || !RXVALIDSTRING(args[0]) ||
            args[0].strlength > 255) return INVALID_ROUTINE;

    /* remember stem name */
    szStemName = alloca(256);
    if(szStemName == NULL) RETVAL(0)

    memset(szStemName, 0, 256);
    strcpy(szStemName, args[0].strptr);
    strupr(szStemName);

    // check for '.' and if not there make it so
    if (szStemName[args[0].strlength-1] != '.')
        szStemName[args[0].strlength] = '.';

    // pointer to the index part of stem
    pszStemIdx = &(szStemName[strlen(szStemName)]);

    WININIT(WinIntial,AnchBlk)

    // get size of buffer required
    rc = WinEnumObjectClasses(NULL, &objSize);
    if(!rc) {
        WINTERM(WinIntial,WinIntial)
        RETVAL(0)
    }

    // make buffers
    ClassBuffer = alloca(objSize);
    if(ClassBuffer == NULL) {
        WINTERM(WinIntial,WinIntial)
        RETVAL(0)
    }

    // get classes
    rc = WinEnumObjectClasses(ClassBuffer, &objSize);
    if(!rc) {
        WINTERM(WinIntial,WinIntial)
        RETVAL(0)
    }

    WINTERM(WinIntial,WinIntial)

    // setup and clear
    szValue = alloca(TMPBUFF);
    if(szValue == NULL) RETVAL(0)
    memset(szValue, 0, TMPBUFF);

    ptrBuffer = ClassBuffer;

    while(ptrBuffer) {

        itoa(++countClass, pszStemIdx, 10);

        sprintf(szValue, "%s %s",
            ptrBuffer->pszClassName, ptrBuffer->pszModName);

        rc = SetRexxVariable(szStemName, szValue);

        if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV)) RETVAL(0)

        ptrBuffer = ptrBuffer->pNext;
    }

    // setup the 0 index with number of classes
    strcpy(pszStemIdx, "0");                    // index
    sprintf(szValue, "%ld", countClass);        // value

    rc = SetRexxVariable(szStemName, szValue);

    if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV)) RETVAL(0)
    else RETVAL(1)
}


/*************************************************************************
* Function:  SysSetObjectData                                            *
*                                                                        *
* Syntax:    call SysSetObjectData object, setup                         *
*                                                                        *
* Params:    orgObject - name of the object (either id or fully          *
*            setup - Workplace Shell (WPS) setup string                  *
*                                                                        *
* Return:    0 if fail, 1 if succeed.                                    *
*************************************************************************/

unsigned long SysSetObjectData(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc = FALSE;

    HOBJECT  orgObject;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 2 || !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[1]))
            return INVALID_ROUTINE;

    WININIT(WinIntial,AnchBlk)

    orgObject = WinQueryObject(args[0].strptr);

    if(!orgObject) {
        WINTERM(WinIntial,WinIntial)
        RETVAL(0)
    }

    rc = WinSetObjectData(orgObject, args[1].strptr);

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysSaveObject                                               *
*                                                                        *
* Syntax:    call SysSaveObject orgObject, sync                          *
*                                                                        *
* Params:    orgObject - name of the object to save, which must exist    *
*            sync - whether or not to save the object asynchronously     *
*                                                                        *
* Return:    0 if fail, 1 if succeed.                                    *
*************************************************************************/

unsigned long SysSaveObject(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc = FALSE;
    bool sync;

    HOBJECT  Object;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 2 || !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[1]))
            return INVALID_ROUTINE;

    // rexx tips & tricks says can pass 0 or 1 too
    if(!stricmp(args[1].strptr, "FALSE") || !stricmp(args[1].strptr, "0")) {
        sync = FALSE;
    } else if(!stricmp(args[1].strptr, "TRUE") || !stricmp(args[1].strptr, "1")) {
        sync = TRUE;
    } else return INVALID_ROUTINE;

    WININIT(WinIntial,AnchBlk)

    Object = WinQueryObject(args[0].strptr);

    if(!Object) {
        WINTERM(WinIntial,WinIntial)
        RETVAL(0)
    }

    rc = WinSaveObject(Object, sync);

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysOpenObject                                               *
*                                                                        *
* Syntax:    call SysOpenObject object, view, flag                       *
*                                                                        *
* Params:    object - name of the object to open, which must exist       *
*            view   - see below                                          *
*            flag - Either open a view of this object (wpOpen) or surface*
*                   an existing view (wpViewObject)                      *
*                                                                        *
* Return:    0 if fail, 1 if succeed.                                    *
*************************************************************************/

//                * view *
// OPEN_DEFAULT   DEFAULT   Open the object's default view.
// OPEN_CONTENTS  ICON      Open the object in icon view (folders only).
// OPEN_SETTINGS  SETTINGS  Open the object's settings dialog.
// OPEN_HELP      HELP      Open the object's default help panel.
// OPEN_RUNNING   RUNNING   Execute a program object or executable.
// OPEN_PROMPTDLG PROMPTDLG Open a prompt dialog object.
// OPEN_PALETTE   PALETTE   Open a palette object.

unsigned long SysOpenObject(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc = FALSE;

    unsigned long view;
    bool flags;
    HOBJECT  object;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 3) return INVALID_ROUTINE;

    if(!stricmp(args[1].strptr, "DEFAULT")) {
        view = OPEN_DEFAULT;
    } else if (!stricmp(args[1].strptr, "ICON")) {
        view = OPEN_CONTENTS;
    } else if (!stricmp(args[1].strptr, "SETTINGS")) {
        view = OPEN_SETTINGS;
    } else if (!stricmp(args[1].strptr, "HELP")) {
        view = OPEN_HELP;
    } else if (!stricmp(args[1].strptr, "RUNNING")) {
        view = OPEN_RUNNING;
    } else if (!stricmp(args[1].strptr, "PROMPTDLG")) {
        view = OPEN_PROMPTDLG;
    } else if (!stricmp(args[1].strptr, "PALETTE")) {
        view = OPEN_PALETTE;
    } else {
        int i = 0;

        // in case something new comes along check for
        // a number and convert
        while( i < strlen(args[1].strptr) ) {
            if(!isdigit(args[1].strptr[i])) return INVALID_ROUTINE;
            ++i;
        }
        string2ulong(args[1].strptr, &view);
    }

    if (!stricmp(args[2].strptr, "FALSE")) flags = FALSE;
    else if(!stricmp(args[2].strptr, "TRUE")) flags = TRUE;
    else return INVALID_ROUTINE;

    WININIT(WinIntial,AnchBlk)

    object = WinQueryObject(args[0].strptr);

    if(!object) {
        WINTERM(WinIntial,WinIntial)
        RETVAL(0)
    }

    rc = WinOpenObject(object, view, flags);

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysMoveObject                                               *
*                                                                        *
* Syntax:    call SysMoveObject orgObject , newObject                    *
*                                                                        *
* Params:    orgObject - name of the object to move, which must exist    *
*            newObject - name of the target folder to which the          *
*                        specified object will be moved                  *
*                                                                        *
* Return:    0 if fail, 1 if succeed.                                    *
*************************************************************************/

unsigned long SysMoveObject(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc = FALSE;

    HOBJECT  orgObject;
    HOBJECT  newObject;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 2) return INVALID_ROUTINE;

    WININIT(WinIntial,AnchBlk)

    orgObject = WinQueryObject(args[0].strptr);
    newObject = WinQueryObject(args[1].strptr);

    if (orgObject && newObject) {
        rc = WinMoveObject(orgObject, newObject, 0);
    }

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysCopyObject                                               *
*                                                                        *
* Syntax:    call SysCopyObject orgObject , newObject                    *
*                                                                        *
* Params:    orgObject - name of the object to copy                      *
*            newObject - location in which the copy will be created      *
*                                                                        *
* Return:    0 if fail, 1 if succeed.                                    *
*************************************************************************/

unsigned long SysCopyObject(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc = FALSE;

    HOBJECT  orgObject;
    HOBJECT  newObject;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 2) return INVALID_ROUTINE;

    WININIT(WinIntial,AnchBlk)

    orgObject = WinQueryObject(args[0].strptr);
    newObject = WinQueryObject(args[1].strptr);

    if (orgObject && newObject) {
        rc = WinCopyObject(orgObject, newObject, 0);
    }

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysCreateShadow                                             *
*                                                                        *
* Syntax:    SysCreateShadow orgObject , newObject                       *
*                                                                        *
* Params:    orgObject - name of the object to shadow                    *
*            newObject - location in which the shadow will be created    *
*                                                                        *
* Return:    0 if fail, 1 if succeed.                                    *
*************************************************************************/

unsigned long SysCreateShadow(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc = FALSE;

    HOBJECT  orgObject;
    HOBJECT  newObject;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 2 || !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[1]))
            return INVALID_ROUTINE;

    WININIT(WinIntial,AnchBlk)

    orgObject = WinQueryObject(args[0].strptr);
    newObject = WinQueryObject(args[1].strptr);

    if (orgObject && newObject) {
        rc = WinCreateShadow(orgObject, newObject, 0);
    }

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysReplaceObject                                            *
*                                                                        *
* Syntax:    call SysReplaceObject orgClass , newClass, flag             *
*                                                                        *
* Params:    orgClass  - class being replaced                            *
*            newClass  - new class name                                  *
*            flag - TRUE  - Replace the function of orgObject with       *
*                           function of newObject                        *
*                   FALSE - Undo the replacement of orgObject  with      *
*                           newObject                                    *
*                                                                        *
* Return:    0 if fail, 1 if succeed.                                    *
*                                                                        *
* Note: I added this because of a suggestion by Rick Walsh               *
*************************************************************************/

unsigned long SysReplaceObject(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc = FALSE;
    bool replace;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 3 || !RXVALIDSTRING(args[0]) ||
            !RXVALIDSTRING(args[1])) return INVALID_ROUTINE;

    if(!stricmp(args[2].strptr,"FALSE") || !stricmp(args[2].strptr,"0")) {
        replace = FALSE;
    } else if(!stricmp(args[2].strptr,"TRUE") || !stricmp(args[2].strptr,"1")) {
        replace = TRUE;
    } else return INVALID_ROUTINE;

    WININIT(WinIntial,AnchBlk)

    rc = WinReplaceObjectClass(args[0].strptr, args[1].strptr, replace);

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


