/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysGetKey
*      SysIni
*     *SysCls
*     *SysCurPos
*     *SysCurState
*     *SysSleep
*     *SysGetMessage
*      SysWaitNamedPipe
*      SysSwitchSession
*      SysQuerySwitchList
*      SysElapsedTime
*      SysShutDownSystem
*      SysWaitForShell
*
*      * rewritten, highly modified - based on toolkit
*        example and/or oorexx source
*
*  Michael Greene, January 2008
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

#define INCL_WINWORKPLACE // need for VAC
#define INCL_WINWINDOWMGR
#define INCL_WINSWITCHLIST
#define INCL_WINSHELLDATA
#define INCL_DOSMEMMGR
#define INCL_DOSNMPIPES
#define INCL_DOSPROFILE
#define INCL_DOSMISC
#define INCL_VIO

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <alloca.h>
#include <dos.h>
#include <conio.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

#ifdef __WATCOMC__
// The following from OS/2 wpobject.h toolkit header.
// the Watcom lib will link correctly

/* Allow applications to wait for the Shell to be available */
#define WWFS_DESKTOPCREATED   1
#define WWFS_DESKTOPOPENED    2
#define WWFS_DESKTOPPOPULATED 3

#define WWFS_QUERY            0x80000000

bool APIENTRY WinWaitForShell( unsigned long ulEvent );
#endif

// used by SysElapsedTime
#define TIMER_ELAPSE  0
#define TIMER_RESET   1

unsigned long TimeFreq  = 0;
double        TimeStart = 0;
// end SysElapsedTime

int           ExtendedFlag = 0;   /* extended character saved   */
unsigned char ExtendedChar;       /* saved extended character   */

RexxFunctionHandler SysGetKey;
RexxFunctionHandler SysIni;
RexxFunctionHandler SysCls;
RexxFunctionHandler SysCurPos;
RexxFunctionHandler SysCurState;
RexxFunctionHandler SysSleep;
RexxFunctionHandler SysGetMessage;
RexxFunctionHandler SysWaitNamedPipe;
RexxFunctionHandler SysSwitchSession;
RexxFunctionHandler SysWaitForShell;
RexxFunctionHandler SysQuerySwitchList;
RexxFunctionHandler SysElapsedTime;


/*************************************************************************
* Function:  SysGetKey                                                   *
*                                                                        *
* Syntax:    call SysGetKey [echo]                                       *
*                                                                        *
* Params:    echo - Either of the following:                             *
*                    'ECHO'   - Echo the inputted key (default).         *
*                    'NOECHO' - Do not echo the inputted key.            *
*                                                                        *
* Return:    The key striked.                                            *
*************************************************************************/

unsigned long SysGetKey(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    int  tmp;
    bool echo = TRUE;

    if (numargs > 1) return INVALID_ROUTINE;

    if (numargs == 1) {
        if (!stricmp(args[0].strptr, "NOECHO")) echo = FALSE;
        else if (stricmp(args[0].strptr, "ECHO")) return INVALID_ROUTINE;
    }

    if (ExtendedFlag) {
       tmp = ExtendedChar;
       ExtendedFlag = FALSE;
    } else {
        tmp = getch( );

        /* If a function key or arrow */
        if ((tmp == 0x00) || (tmp == 0xe0)) {
            ExtendedChar = getch();
            ExtendedFlag = TRUE;
        } else ExtendedFlag = FALSE;
    }

    if (echo) putch(tmp);

    retstr->strlength = sprintf(retstr->strptr, "%c", tmp);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysIni                                                      *
*                                                                        *
* Syntax:    call SysIni [inifile], app [,key/stem] [,val/stem]          *
*                                                                        *
* Params:    inifile - INI file from which to query or write info.  The  *
*                       default is the current user INI file.            *
*            app     - Application title to work with.  May be either    *
*                       of the following:                                *
*                        'ALL:' - All app titles will be returned in the *
*                                  stem variable specified by the next   *
*                                  parameter.                            *
*                        other  - Specific app to work with.             *
*            key     - Key to work with.  May be any of the following:   *
*                        'ALL:'    - All key titles will be returned in  *
*                                     the stem variable specified by the *
*                                     next parameter.                    *
*                        'DELETE:' - All keys associated with the app    *
*                                     will be deleted.                   *
*                        other     - Specific key to work with.          *
*            val     - Key to work with. May be either of the following: *
*                        'DELETE:' - Delete app/key pair.                *
*                        other     - Set app/key pair info to data spec- *
*                                     ified.                             *
*            stem    - Name of stem variable in which to store results.  *
*                      Stem.0 = Number found (n).                        *
*                      Stem.1 - Stem.n = Entries found.                  *
*                                                                        *
* Return:    other          - Info queried from specific app/key pair.   *
*            ''             - Info queried and placed in stem or data    *
*                              deleted successfully.                     *
*            ERROR_NOMEM    - Out of memory.                             *
*            ERROR_RETSTR   - Error opening INI or querying/writing info.*
*************************************************************************/

unsigned long SysIni(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
  HAB         hab;                     /* Anchor block               */
  HINI        hini;                    /* Ini file handle            */
  ULONG       x;                       /* Temp counter               */
  ULONG       len;                     /* Len var used when creating */
                                       /* stem                       */
  ULONG       lSize;                   /* Size of queried info buffer*/
                                       /* area                       */
  PSZ         IniFile;                 /* Ini file (USER, SYSTEM,    */
                                       /* BOTH, file)                */
  PSZ         App;                     /* Application field          */
  PSZ         Key;                     /* Key field                  */
  CHAR       *Val;                     /* Ptr to data associated w/  */
                                       /* App->Key                   */
  CHAR        Temp[256];               /* Temp string var            */
  CHAR        UserName[256];           /* Filename of User INI       */
                                       /* profile                    */
  CHAR        SysName[256];            /* Filename of System INI     */
                                       /* profile                    */
  PRFPROFILE  PrfInfo;                 /* Profile name structure     */
  LONG        Error = FALSE;           /* Set to true if error       */
                                       /* encountered                */
  BOOL        WildCard = FALSE;        /* Set to true if a wildcard  */
                                       /* operation                  */
  BOOL        QueryApps;               /* Set to true if a query     */
                                       /* operation                  */
  BOOL        terminate = TRUE;        /* perform WinTerminate call  */
  RXSTEMDATA  ldp;                     /* local data                 */

    IniFile = "";
    Key = "";

    /* validate arguments         */
    if (numargs < 2 || numargs > 4 || !RXVALIDSTRING(args[1]))
        return INVALID_ROUTINE;

    /* get pointers to args       */
    IniFile = args[0].strptr;
    App = args[1].strptr;

    if (numargs >= 3 && args[2].strptr) Key = args[2].strptr;

    if (numargs == 4) Val = args[3].strptr;

    /* Check KEY and APP values for "WildCard"             */
    if (!stricmp(App, "ALL:")) {
        App = "";
        QueryApps = TRUE;
        WildCard = TRUE;


        if (numargs != 3) return INVALID_ROUTINE; /* Error - Not enough args    */
        else x = 2;                               /* Arg number of STEM variable*/
    } else if (!stricmp(Key, "ALL:")) {
        Key = "";
        Val = "";
        QueryApps = FALSE;
        WildCard = TRUE;

        if (numargs != 4) return INVALID_ROUTINE; /* Error - Not enough args    */
        else x = 3;                               /* Arg number of STEM variable*/
    }

    /*
     * If this is a "WildCard search, then allocate mem
     * for stem struct and get the stem name
     */
    if (WildCard == TRUE) {
        ldp.count = 0;                       /* get the stem variable name */
        strcpy(ldp.varname, args[x].strptr);
        ldp.stemlen = args[x].strlength;
        strupr(ldp.varname);                 /* uppercase the name         */

        if (ldp.varname[ldp.stemlen-1] != '.') ldp.varname[ldp.stemlen++] = '.';
    }

    hab = WinInitialize((USHORT)0);          /* create anchor block        */

    if (!hab) {                              /* already done? get desktop anchor */
        hab = WinQueryAnchorBlock(HWND_DESKTOP);
        terminate = FALSE;                   /* don't terminate            */
    }

      /**************************************************************
      * The following section of code gets the INI file handle      *
      * given the INI file spec (IniFile).                          *
      *                                                             *
      * Possible Ini file specs:                                    *
      *                                                             *
      *   NULL     - Same as USERPROFILE   ( OS2.INI )              *
      *   "BOTH"   - Same as PROFILE       ( OS2.INI & OS2SYS.INI ) *
      *   "USER"   - Same as USERPROFILE   ( OS2.INI )              *
      *   "SYSTEM" - Same as SYSTEMPROFILE ( OS2SYS.INI)            *
      *   other    - Filespec of INI file.                          *
      **************************************************************/

    hini = NULLHANDLE;

    if (!IniFile) hini = HINI_USERPROFILE;
    else if (!stricmp(IniFile, "BOTH")) hini = HINI_PROFILE;
    else if (!stricmp(IniFile, "USER")) hini = HINI_USERPROFILE;
    else if (!stricmp(IniFile, "SYSTEM")) hini = HINI_SYSTEMPROFILE;

      /***********************************************************
      * If Ini file spec is 'other' then make sure it does not   *
      * specify the current USER or SYSTEM profiles.             *
      *                                                          *
      * Trying to open the current USER or SYSTEM ini file via   *
      * PrfOpenProfile() will fail.  Therefore, use the function *
      * PrfQueryProfile() to query the current USER and SYSTEM   *
      * ini file specs. If the IniFile string matches either     *
      * the current USER or SYSTEM file specs, then use either   *
      * HINI_USERPROFILE or HINI_SYSTEMPROFILE as appropriate.   *
      *                                                          *
      * If IniFile does not specify the current USER or SYSTEM   *
      * ini file then use PrfOpenProfile() to get the ini file   *
      * handle.                                                  *
      ***********************************************************/

    else {
        PrfInfo.pszUserName = UserName;
        PrfInfo.cchUserName = sizeof(UserName);
        PrfInfo.pszSysName = SysName;
        PrfInfo.cchSysName = sizeof(SysName);

        if (PrfQueryProfile(hab, &PrfInfo)) {
            if (!stricmp(IniFile, PrfInfo.pszUserName)) hini = HINI_USERPROFILE;
            else if (!stricmp(IniFile, PrfInfo.pszSysName)) hini = HINI_SYSTEMPROFILE;
            else hini = PrfOpenProfile(hab, IniFile);
        } else hini = PrfOpenProfile(hab, IniFile);

      /**************************************************
      * Exit with INI FILE error if the ini file handle *
      * is NULL at this point (and if IniFile != BOTH,  *
      * as that would make the handle NULL also).       *
      **************************************************/

        if (hini == NULLHANDLE && stricmp(IniFile, "BOTH")) {
            BUILDRXSTRING(retstr, ERROR_RETSTR);
            if (terminate) WinTerminate(hab);

            return VALID_ROUTINE;

        }
    }
                                       /* get value if is a query    */
    if ((numargs == 3 && stricmp(Key, "DELETE:")) || WildCard == TRUE) {
        lSize = 0x0000ffffL;
                                       /* Allocate a large buffer    */
        if (DosAllocMem((PPVOID)&Val, lSize, AllocFlag)) {
            PrfCloseProfile(hini);     /* close the INI file         */
            if (terminate) WinTerminate(hab);

            BUILDRXSTRING(retstr, ERROR_NOMEM);

            return VALID_ROUTINE;
        }

        if (WildCard && QueryApps) Error = !PrfQueryProfileData(hini, NULL, NULL, Val, &lSize);
        else if (WildCard && !QueryApps) Error = !PrfQueryProfileData(hini, App, NULL, Val, &lSize);
        else Error = !PrfQueryProfileData(hini, App, Key, Val, &lSize);

        if (Error) {
            BUILDRXSTRING(retstr, ERROR_RETSTR);
        } else if (WildCard == FALSE) {
            if (lSize > retstr->strlength) if (DosAllocMem((PPVOID)&retstr->strptr, lSize, AllocFlag)) {
                DosFreeMem(Val);             /* release buffer             */
                PrfCloseProfile(hini);       /* close the INI file         */
                if (terminate) WinTerminate(hab);
                BUILDRXSTRING(retstr, ERROR_NOMEM);

                return VALID_ROUTINE;
            }

            memcpy(retstr->strptr, Val, lSize);
            retstr->strlength = lSize;
            DosFreeMem(Val);                 /* release buffer             */
        }
    } else {                                 /* set the var to new value   */
        if (!stricmp(Key, "DELETE:") || !RXVALIDSTRING(args[2]))
            Error = !PrfWriteProfileData(hini, App, NULL, NULL, 0L);
        else if (!stricmp(Val, "DELETE:") || !RXVALIDSTRING(args[3]))
            Error = !PrfWriteProfileData(hini, App, Key, NULL, 0L);
        else {
            lSize = args[3].strlength;
            Error = !PrfWriteProfileData(hini, App, Key, Val, lSize);
        }

        if (Error) {
            BUILDRXSTRING(retstr, ERROR_RETSTR);
        }
    }

    PrfCloseProfile(hini);               /* close the INI file         */

    if (terminate) WinTerminate(hab);    /* destroy anchor block       */


      /******************************************
      * If this was a wildcard search, change   *
      * the Val variable from one long string   *
      * of values to a REXX stem variable.      *
      ******************************************/

    if (WildCard == TRUE) {              /* fill stem variable         */
        if (Error == FALSE) {
            x = 0;
            ldp.count = 0;
            do {

                /* Copy string terminated by \0 to Temp.  Last string will end     */
                /* in \0\0 and thus have a length of 0.                            */
                len = 0;

                while (Val[x+len] != '\0') Temp[len++] = Val[x+len];
                Temp[len] = '\0';
                                       /* if non-zero length, then   */
                                       /* set the stem element       */
                if (len != 0) {
                    x += (len+1);      /* Increment pointer past the */
                                       /* new string                 */
                    memcpy(ldp.ibuf, Temp, len);
                    ldp.vlen = len;
                    ldp.count++;
                    sprintf(ldp.varname+ldp.stemlen, "%d", ldp.count);

                    if (ldp.ibuf[ldp.vlen-1] == '\n') ldp.vlen--;
                    ldp.shvb.shvnext = NULL;
                    ldp.shvb.shvname.strptr = ldp.varname;
                    ldp.shvb.shvname.strlength = strlen(ldp.varname);
                    ldp.shvb.shvvalue.strptr = ldp.ibuf;
                    ldp.shvb.shvvalue.strlength = ldp.vlen;
                    ldp.shvb.shvnamelen = ldp.shvb.shvname.strlength;
                    ldp.shvb.shvvaluelen = ldp.vlen;
                    ldp.shvb.shvcode = RXSHV_SET;
                    ldp.shvb.shvret = 0;
                    if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN) return INVALID_ROUTINE;
                }
            }

            while (Val[x] != '\0');
        } else ldp.count = 0;
                                       /* set number returned        */
        sprintf(ldp.ibuf, "%d", ldp.count);
        ldp.varname[ldp.stemlen] = '0';
        ldp.varname[ldp.stemlen+1] = 0;
        ldp.shvb.shvnext = NULL;
        ldp.shvb.shvname.strptr = ldp.varname;
        ldp.shvb.shvname.strlength = ldp.stemlen+1;
        ldp.shvb.shvnamelen = ldp.stemlen+1;
        ldp.shvb.shvvalue.strptr = ldp.ibuf;
        ldp.shvb.shvvalue.strlength = strlen(ldp.ibuf);
        ldp.shvb.shvvaluelen = ldp.shvb.shvvalue.strlength;
        ldp.shvb.shvcode = RXSHV_SET;
        ldp.shvb.shvret = 0;
        if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN) return INVALID_ROUTINE;

    }                                    /* * End - IF (Wildcard ... * */

    return VALID_ROUTINE;                /* no error on call           */
}


/*************************************************************************
* Function:  SysCls                                                      *
*                                                                        *
* Syntax:    call SysCls                                                 *
*                                                                        *
* Return:    Return 0                                                    *
*************************************************************************/
/* MKG: Based on toolkit and oorexx sample, re-written */

unsigned long SysCls(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned char Cell[2] = {0x20,0x07};

    if (numargs) return INVALID_ROUTINE;

    VioScrollDn( 0, 0,
                (unsigned short)0xFFFF,
                (unsigned short)0XFFFF,
                (unsigned short)0xFFFF,
                Cell,
                (SHANDLE) 0);

    VioSetCurPos(0, 0, (SHANDLE) 0);

    RETVAL(0)
}


/*************************************************************************
* Function:  SysCurPos - positions cursor in OS/2 session                *
*                                                                        *
* Syntax:    call SysCurPos [row, col]                                   *
*                                                                        *
* Params:    row   - row to place cursor on                              *
*            col   - column to place cursor on                           *
*                                                                        *
* Return:    row, col                                                    *
*************************************************************************/

unsigned long SysCurPos(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned short start_row;
    unsigned short start_col;
    long new_row;
    long new_col;

    // 0 args return position, 2 args set position
    if ((numargs != 0 && numargs != 2))  return INVALID_ROUTINE;

    // get current position
    VioGetCurPos(&start_row, &start_col, (SHANDLE)0);


    retstr->strlength = sprintf(retstr->strptr, "%d %d",
                                   (int)start_row, (int)start_col);

    // set position if required
    if (numargs != 0) {
        if (!RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[1]))
                return INVALID_ROUTINE;

        if (!string2long(args[0].strptr, &new_row) || new_row < 0)
                return INVALID_ROUTINE;

        if (!string2long(args[1].strptr, &new_col) || new_col < 0)
                return INVALID_ROUTINE;

        /* Set the cursor position, using the input values. */
        VioSetCurPos((unsigned short)new_row,
                     (unsigned short)new_col,
                     (SHANDLE) 0);
    }

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysCurState                                                 *
*                                                                        *
* Syntax:    call SysCurState state                                      *
*                                                                        *
* Params:    state - Either 'ON' or 'OFF'.                               *
*                                                                        *
* Return:    Returns 0                                                   *
*************************************************************************/
/* MKG: Based on toolkit and oorexx sample, re-written */

unsigned long SysCurState(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned short state;                /* State                      */
    VIOCURSORINFO vioci;                 /* Cursor info struct         */

    if (numargs != 1) return INVALID_ROUTINE;

    if (stricmp(args[0].strptr, "ON") == 0) state = (unsigned short) 0;
    else if (stricmp(args[0].strptr, "OFF") == 0) state = (unsigned short) -1;
    else return INVALID_ROUTINE;         /* Invalid state              */

    VioGetCurType(&vioci, (HVIO) 0);     /* Get current state          */
    vioci.attr= state;                   /* Set state info             */
    VioSetCurType(&vioci, (HVIO) 0);     /* Set new state              */

    RETVAL(0)
}


/*************************************************************************
* Function:  SysSleep                                                    *
*                                                                        *
* Syntax:    call SysSleep secs                                          *
*                                                                        *
* Params:    secs - Number of seconds to sleep.                          *
*                                                                        *
* Return:    Return 0                                                    *
*************************************************************************/
/* MKG: Based on toolkit and oorexx sample, re-written */

unsigned long SysSleep(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    long secs;                                 /* Time to sleep in secs      */

    if (numargs != 1) return INVALID_ROUTINE;  /* Must have one argument     */

    /* raise error if bad  */
    if (!string2long(args[0].strptr, &secs) || secs < 0) return INVALID_ROUTINE;

    sleep(secs);

    RETVAL(0)
}


/*************************************************************************
* Function:  SysGetMessage                                               *
*                                                                        *
* Syntax:    call SysGetMessage [file], msgnum [,str1] ... [,str9]       *
*                                                                        *
* Params:    file           - Name of message file to get message from.  *
*                              Default is OSO001.MSG.                    *
*            msgnum         - Number of message being queried.           *
*            str1 ... str9  - Insertion strings.  For messages which     *
*                              contain %1, %2, etc, the str1, str2, etc  *
*                              strings will be inserted (if given).      *
*                                                                        *
* Return:    The message with the inserted strings (if given).           *
*************************************************************************/

unsigned long SysGetMessage(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    char    *ivtable[9];
    char    dataarea[500];
    char    *msgfile;

    unsigned long   msglen;
    unsigned long   datalen = 500;
    unsigned long   ivcount = 3;

    long    msgnum;
    long    x;


    if (numargs < 1 || numargs > 11 || !RXVALIDSTRING(args[0]))
            return INVALID_ROUTINE;

    if (!string2long(args[0].strptr, &msgnum) || msgnum < 0)
            return INVALID_ROUTINE;

    if (numargs >= 2 && RXVALIDSTRING(args[1])) msgfile = args[1].strptr;
    else  msgfile = "OSO001.MSG";

    for (x = 2; x < numargs; x++) {
        if (RXNULLSTRING(args[x])) return INVALID_ROUTINE;
        ivtable[x-2] = args[x].strptr;
    }

    if (numargs >= 2) ivcount = numargs-2;
    else ivcount = 0;

    DosGetMessage(ivtable, ivcount, dataarea,
                      datalen, msgnum, msgfile, &msglen);

    if (msglen > retstr->strlength)
        if (DosAllocMem((PPVOID)&retstr->strptr, msglen, AllocFlag))
                         RETVAL(3)

    memcpy(retstr->strptr, dataarea, msglen);
    retstr->strlength = msglen;

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysWaitNamedPipe                                            *
*                                                                        *
* Syntax:    result = SysWaitNamedPipe(name, timeout)                    *
*                                                                        *
* Params:    name - name of the pipe                                     *
*            timeout - amount of time to wait.                           *
*                                                                        *
* Return:    Return code from DosWaitNPipe                               *
*************************************************************************/
/* MKG: Based on toolkit and oorexx sample, re-written */

unsigned long SysWaitNamedPipe(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    long timeout;

    if (numargs < 1 || numargs > 2 ||
        !RXVALIDSTRING(args[0])) return INVALID_ROUTINE;

    if (numargs == 2) {                  /* have a timeout value?      */
        if (!string2long(args[1].strptr, &timeout) ||
            (timeout < 0 && timeout != -1)) return INVALID_ROUTINE;
    }

    RETVAL(DosWaitNPipe(args[0].strptr, timeout));
}


/*************************************************************************
* Function:  SysSwitchSession                                            *
*                                                                        *
* Syntax:    result = SysSwitchSession(name)                             *
*                                                                        *
* Params:    name   - name of target session                             *
*                                                                        *
* Return:    OS/2 error return code                                      *
*************************************************************************/

unsigned long SysSwitchSession(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc = FALSE;

    HWND    hwndFrame;
    HSWITCH hwnd;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs != 1) return INVALID_ROUTINE;

    string2ulong(args[0].strptr, &hwndFrame);

    WININIT(WinIntial,AnchBlk)

    hwnd = WinQuerySwitchHandle(hwndFrame, 0);

    if (hwnd) rc = WinSwitchToProgram(hwnd);

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)

}


/*************************************************************************
* Function:  SysWaitForShell                                             *
*                                                                        *
* Syntax:    call SysWaitForShell event, queryflag                       *
*                                                                        *
* Params:    see WinWaitForShell                                         *
*            Wait for a specific initialization event of the             *
*            Workplace Shell.                                            *
*                                                                        *
* Return:    1 (true) if the event has occured or 0 (false) otherwise    *
*                                                                        *
*************************************************************************/

unsigned long SysWaitForShell(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc = FALSE;

    unsigned long event = 0;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs < 1 || numargs > 2) return INVALID_ROUTINE;

    if (stricmp(args[0].strptr, "DESKTOPCREATED") == 0) {
        event = WWFS_DESKTOPCREATED;
    } else if (stricmp(args[0].strptr, "DESKTOPOPENED") == 0) {
        event = WWFS_DESKTOPOPENED;
    } else if (stricmp(args[0].strptr, "DESKTOPPOPULATED") == 0) {
        event = WWFS_DESKTOPPOPULATED;
    } else return INVALID_ROUTINE;

    if (numargs ==2) {
        if (stricmp(args[1].strptr, "QUERY") == 0)  event |= WWFS_QUERY;
        else return INVALID_ROUTINE;
    }

    WININIT(WinIntial,AnchBlk)

    rc = WinWaitForShell( event );

    WINTERM(WinIntial,WinIntial)

    RETVAL(rc?1:0)
}


/*************************************************************************
* Function:  SysQuerySwitchList                                          *
*                                                                        *
* Syntax:    call SysSwitchSession name [,flags]                         *
*                                                                        *
* Params:    name   - The name of a stem variable in which the list of   *
*                     entries will be be saved.                          *
*            flages - too much to list here, see the docs                *
*                                                                        *
* Return:    Return the SetRexxVariable function result                  *
*************************************************************************/

unsigned long SysQuerySwitchList(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool rc = FALSE;

    int  cnt;

    char *szStemName = NULL;
    char *pszStemIdx = NULL;
    char *ptrOption  = NULL;
    char *retbuffer  = NULL;

    void *buffer;

    unsigned long numstem    = 0;
    unsigned long numentries = 0;
    unsigned long BufferSize = 0;

    // option defaults
    bool ShowDetail = FALSE;
    long ShowVis    = SWL_VISIBLE;
    long JumpVis    = SWL_JUMPABLE;

    SWBLOCK    *SwitchEntry;

    // initialize PM
    HAB  AnchBlk   = NULLHANDLE;
    bool WinIntial = TRUE;

    if (numargs < 1 || numargs > 2 || !RXVALIDSTRING(args[0]))
        return INVALID_ROUTINE;

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

    // handle option if passed
    if (numargs == 2) {

        strupr(args[1].strptr);
        ptrOption = args[1].strptr;

        while (*ptrOption) {
            switch(toupper(*ptrOption)) {

            case 'I':
                ShowVis |= SWL_INVISIBLE;
                break;

            case 'G':
                ShowVis |= SWL_GRAYED;
                break;

            case 'N':
                JumpVis |= SWL_NOTJUMPABLE;
                break;

            case 'D':
                ShowDetail = TRUE;
                break;

            default:
                return INVALID_ROUTINE;
            }

            ptrOption++;
        }
    }

    WININIT(WinIntial,AnchBlk)

    numentries = WinQuerySwitchList(AnchBlk, NULL, 0);
    BufferSize = (numentries * sizeof(SWENTRY)) + sizeof(HSWITCH);

    buffer = alloca(BufferSize);
    if(buffer == NULL) RETVAL(0)

    numentries = WinQuerySwitchList(AnchBlk, buffer, BufferSize);

    WINTERM(WinIntial,WinIntial)

    if(!numentries) RETVAL(0)

    SwitchEntry = (SWBLOCK *)buffer;

    retbuffer = (char *)alloca(512);

    for (cnt = 0; cnt < numentries; cnt++) {
        if(((SwitchEntry->aswentry[cnt].swctl.uchVisibility & ShowVis) != 0) &&
          ((SwitchEntry->aswentry[cnt].swctl.fbJump & JumpVis) != 0) ) {

            if (ShowDetail) {
                sprintf(retbuffer, "%8ld %8ld %1ld %1ld %3ld %s",
                        SwitchEntry->aswentry[cnt].swctl.idProcess,
                        SwitchEntry->aswentry[cnt].swctl.idSession,
                        SwitchEntry->aswentry[cnt].swctl.uchVisibility & 0x07,
                        SwitchEntry->aswentry[cnt].swctl.fbJump & 0x03,
                        SwitchEntry->aswentry[cnt].swctl.bProgType,
                        SwitchEntry->aswentry[cnt].swctl.szSwtitle);
            } else {
                sprintf(retbuffer, "%s",
                        SwitchEntry->aswentry[cnt].swctl.szSwtitle);
            }

            itoa(++numstem, pszStemIdx, 10);

            rc = SetRexxVariable(szStemName, retbuffer);
            if (rc == RXSHV_BADN) RETVAL(0)
        }
    }

    // setup the 0 index with number of entries
    strcpy(pszStemIdx, "0");                 // index
    sprintf(retbuffer, "%ld", numstem);        // value

    rc = SetRexxVariable(szStemName, retbuffer);

    RETVAL(rc)
}


/*************************************************************************
* Function:  SysElapsedTime                                              *
*                                                                        *
* Syntax:    call SysElapsedTime option                                  *
*                                                                        *
* Params:    option - Controls the behaviour of the call to              *
*                     SysElapsedTime  (Elapsed [default] / Reset         *
*                                                                        *
* Return:    SysElapsedTime returns a time stamp in the following        *
*            format:                                                     *
*                                                                        *
*            sssssssss.uuuuuu                                            *
*                                                                        *
*            where 'sssssssss' is one to nine digits representing        *
*            seconds (no leading zeros or spaces), and 'uuuuuu' is       *
*            always six digits representing microseconds.                *
*************************************************************************/

unsigned long SysElapsedTime(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    int  flags = TIMER_ELAPSE;   // default

    double TimeCurrent;
    double TimeDiff;

    QWORD Ticks;

    // requires 0 or 1 args
    if (numargs > 1) return INVALID_ROUTINE;

    if (numargs == 1) {
        // handle flags passed first
        switch (toupper(args[0].strptr[0])) {

        case 'E':
            flags = TIMER_ELAPSE;
            break;

        case 'R':
            flags = TIMER_RESET;
            break;

        default:
            return INVALID_ROUTINE;
        }
    }

    // if this is the first call to SysElapsedTime
    // get the frequency of the IRQ0 high resolution
    // timer and set TimeStart
    if(!TimeFreq) {
        DosTmrQueryFreq(&TimeFreq);
        DosTmrQueryTime(&Ticks);
        TimeStart = ((long long int)Ticks.ulHi <<32) + Ticks.ulLo;
        TimeDiff  = 0;
    } else {
        DosTmrQueryTime(&Ticks);
        TimeCurrent = ((long long int)Ticks.ulHi <<32) + Ticks.ulLo;
        TimeDiff = (TimeCurrent - TimeStart)/TimeFreq;

        // reset
        if(flags == TIMER_RESET) TimeStart = TimeCurrent;
    }

    retstr->strlength = sprintf(retstr->strptr, "%.6f", TimeDiff);
    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysShutDownSystem                                           *
*                                                                        *
* Syntax:    result = SysShutDownSystem                                  *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    SysShutDownSystem returns 1 on successful shutdown, or      *
*            0 on failure.                                               *
*                                                                        *
* Note: I just threw this together from API docs and stuff I googled     *
*************************************************************************/

unsigned long SysShutDownSystem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool  fSuccess = FALSE;        /* Win API success indicator */

    HAB   hab      = NULLHANDLE;   /* Window handle */
    HMQ   hmq      = NULLHANDLE;   /* Message queue handle */


    if(numargs) return INVALID_ROUTINE;

    hab = WinInitialize( 0 );
    hmq = WinCreateMsgQueue( hab, 0 );

    /* Prevent our program from hanging the shutdown.  If this call is
       omitted, the system will wait for us to do a WinDestroyMsgQueue. */
    WinCancelShutdown( hmq, TRUE );

    /* Shutdown the system! */
    fSuccess = WinShutdownSystem( hab, hmq );

    if (!fSuccess) fSuccess = (DosShutdown(0)?FALSE:TRUE);

    RETVAL(fSuccess?1:0)
}

