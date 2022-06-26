
/*
 *@@sourcefile xsecapi.c:
 *
 *@@header "shared\xsecapi.h"
 */

/*
 *      Copyright (C) 2001-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSQUEUES
#define INCL_DOSERRORS
#include <os2.h>

// C library headers
#include <stdio.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines

// XWorkplace implementation headers
#include "helpers\xwpsecty.h"           // XWorkplace Security

// other SOM headers
#pragma hdrstop

/* ******************************************************************
 *
 *   Helpers
 *
 ********************************************************************/

/*
 *@@ XWPSHELLCOMMAND:
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 */

typedef struct _XWPSHELLCOMMAND
{
    PXWPSHELLQUEUEDATA  pShared;
    PID                 pidXWPShell;
    HQUEUE              hqXWPShell;
} XWPSHELLCOMMAND, *PXWPSHELLCOMMAND;

VOID FreeXWPShellCommand(PXWPSHELLCOMMAND pCommand);

/*
 *@@ CreateXWPShellCommand:
 *      creates a command for XWPShell to process.
 *
 *      See XWPSHELLQUEUEDATA for a description of
 *      what's going on here.
 *
 *      ulCommand must be one of QUECMD_* values.
 *
 *      If this returns NO_ERROR, the caller must
 *      then fill in the shared data according to
 *      what the command requires and use
 *      SendXWPShellCommand then.
 *
 *      Among others, this can return:
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  ERROR_QUE_NAME_NOT_EXIST (343): XWPShell queue
 *          not found, XWPShell is probably not running.
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 */

APIRET CreateXWPShellCommand(ULONG ulCommand,               // in: command
                             PXWPSHELLCOMMAND *ppCommand)   // out: cmd structure if NO_ERROR is returned
{
    APIRET      arc = NO_ERROR;

    PXWPSHELLCOMMAND pCommand;
    if (!(pCommand = NEW(XWPSHELLCOMMAND)))
        arc = ERROR_NOT_ENOUGH_MEMORY;
    else
    {
        ZERO(pCommand);

        // check if XWPShell is running; if so the queue must exist
        if (    (!(arc = DosOpenQueue(&pCommand->pidXWPShell,
                                      &pCommand->hqXWPShell,
                                      QUEUE_XWPSHELL)))
             && (!(arc = DosAllocSharedMem((PVOID*)&pCommand->pShared,
                                           NULL,     // unnamed
                                           sizeof(XWPSHELLQUEUEDATA),
                                           PAG_COMMIT | OBJ_GIVEABLE | PAG_READ | PAG_WRITE)))
           )
        {
            PXWPSHELLQUEUEDATA pShared = pCommand->pShared;

            if (    (!(arc = DosGiveSharedMem(pShared,
                                              pCommand->pidXWPShell,
                                              PAG_READ | PAG_WRITE)))
                 && (!(arc = DosCreateEventSem(NULL,
                                               &pShared->hevData,
                                               DC_SEM_SHARED,
                                               FALSE)))      // reset
               )
            {
                pShared->ulCommand = ulCommand;
                pShared->cbStruct = sizeof(XWPSHELLQUEUEDATA);
            }
        }

        if (!arc)
            *ppCommand = pCommand;
        else
            FreeXWPShellCommand(pCommand);
    }

    return arc;
}

/*
 *@@ SendXWPShellCommand:
 *      sends a command to XWPShell and waits for
 *      the command to be processed.
 *
 *      Be warned, this blocks the calling thread.
 *      Even though XWPShell should be following
 *      the PM 0.1 seconds rule in most cases,
 *      some calls such as "create user" can be
 *      expensive and you might want to start a
 *      second thread for this.
 *
 *      Returns:
 *
 *      --  NO_ERROR: command written, and XWPShell
 *          responded correctly.
 *
 *      --  ERROR_TIMEOUT (640): XWPShell did not
 *          respond within five seconds.
 *
 *      --  XWPSEC_QUEUE_INVALID_CMD: XWPShell did
 *          not recognize the given command code.
 *
 *      --  XWPSEC_STRUCT_MISMATCH: sizeof XWPSHELLQUEUEDATA
 *          does not match; probably a wrong version of
 *          XWPShell is installed.
 *
 *      --  XWPSEC_INTEGRITY: internal error in XWPShell.
 *          Some data structure was corrupt, and this is
 *          really a bug and should be fixed.
 *
 *      plus the many command-specific XWPSEC_*
 *      error codes.
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 */

APIRET SendXWPShellCommand(PXWPSHELLCOMMAND pCommand)
{
    APIRET arc = NO_ERROR;

    if (!pCommand)
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        if (    (!(arc = DosWriteQueue(pCommand->hqXWPShell,
                                       (ULONG)pCommand->pShared,   // request data
                                       0,
                                       NULL,
                                       0)))              // priority
                // wait 5 seconds for XWPShell to write the data
             && (!(arc = DosWaitEventSem(pCommand->pShared->hevData,
                                         5*1000)))
           )
        {
            // return what XWPShell returns
            arc = pCommand->pShared->arc;
        }
    }

    return arc;
}

/*
 *@@ FreeXWPShellCommand:
 *      cleans up.
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 */

VOID FreeXWPShellCommand(PXWPSHELLCOMMAND pCommand)
{
    if (pCommand)
    {
        if (pCommand->pShared)
        {
            if (pCommand->pShared->hevData)
                DosCloseEventSem(pCommand->pShared->hevData);

            DosFreeMem(pCommand->pShared);
        }

        if (pCommand->hqXWPShell)
            DosCloseQueue(pCommand->hqXWPShell);

        free(pCommand);
    }
}

/* ******************************************************************
 *
 *   XWPShell security APIs
 *
 ********************************************************************/

/*
 *@@ xsecQueryStatus:
 *      tests whether XWorkplace security is working and
 *      returns ring-0 statistics.
 *
 +      If this returns NO_ERROR, XWPShell is running
 *      correctly. Check XWPSECSTATUS.fLocalSecurity to
 *      learn whether the ring-0 driver is active also.
 *
 *      Required authority: None.
 *
 *      Command-specific error codes in addition to those
 *      returned by CreateXWPShellCommand and
 *      SendXWPShellCommand:
 *
 *      --  none.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

APIRET xsecQueryStatus(PXWPSECSTATUS pStatus)        // out: ring-0 status (ptr can be null)
{
    APIRET              arc = NO_ERROR;
    PXWPSHELLCOMMAND    pCommand;

    if (!(arc = CreateXWPShellCommand(QUECMD_QUERYSTATUS,
                                      &pCommand)))
    {
        if (!(arc = SendXWPShellCommand(pCommand)))
            if (pStatus)
                memcpy(pStatus,
                       &pCommand->pShared->Data.Status,
                       sizeof(pCommand->pShared->Data.Status));

        FreeXWPShellCommand(pCommand);
    }

    return arc;
}

/*
 *@@ xsecQueryLocalUser:
 *      returns info for the user who's currently
 *      logged on locally, that is, the user who
 *      owns the shell.
 *
 *      Required authority: None.
 *
 *      Command-specific error codes in addition to those
 *      returned by CreateXWPShellCommand and
 *      SendXWPShellCommand:
 *
 *      --  XWPSEC_NO_LOCAL_USER: no user is currently
 *          logged on locally (XWPShell is probably
 *          displaying logon dialog).
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 */

APIRET xsecQueryLocalUser(PXWPUSERDBENTRY *ppLocalUser)     // out: currently logged on user
{
    APIRET              arc = NO_ERROR;
    PXWPSHELLCOMMAND    pCommand;

    if (!(arc = CreateXWPShellCommand(QUECMD_QUERYLOCALUSER,
                                      &pCommand)))
    {
        if (!(arc = SendXWPShellCommand(pCommand)))
        {
            // alright:
            PXWPUSERDBENTRY pLocal = pCommand->pShared->Data.pLocalUser;
            if (!(*ppLocalUser = (PXWPUSERDBENTRY)malloc(pLocal->cbStruct)))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
                memcpy(*ppLocalUser,
                       pLocal,
                       pLocal->cbStruct);
        }

        FreeXWPShellCommand(pCommand);
    }

    return arc;
}

/*
 *@@ xsecQueryUsers:
 *      returns an array of XWPUSERDBENTRY structs
 *      with all the users currently in the userdb.
 *
 *      Warning: The array items are variable in
 *      size depending on group memberships, so
 *      always use the cbStruct member of each
 *      array item to climb to the next.
 *
 *      Required authority: XWPPERM_QUERYUSERINFO.
 *
 *      Command-specific error codes in addition to those
 *      returned by CreateXWPShellCommand and
 *      SendXWPShellCommand:
 *
 *      --  XWPSEC_INSUFFICIENT_AUTHORITY
 *
 *      plus the error codes from sudbQueryUsers.
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

APIRET xsecQueryUsers(PULONG pcUsers,               // ou: array item count
                      PXWPUSERDBENTRY *ppaUsers)    // out: array of users (to be freed)
{
    APIRET              arc = NO_ERROR;
    PXWPSHELLCOMMAND    pCommand;

    if (!(arc = CreateXWPShellCommand(QUECMD_QUERYUSERS,
                                      &pCommand)))
    {
        if (!(arc = SendXWPShellCommand(pCommand)))
        {
            // alright:
            // the array items are variable in size, so check
            ULONG   ul,
                    cbTotal = 0;
            PBYTE   pbUserThis = (PBYTE)pCommand->pShared->Data.QueryUsers.paUsers;
            PXWPUSERDBENTRY paUsers;
            for (ul = 0;
                 ul < pCommand->pShared->Data.QueryUsers.cUsers;
                 ++ul)
            {
                ULONG cbThis = ((PXWPUSERDBENTRY)pbUserThis)->cbStruct;
                cbTotal += cbThis;
                pbUserThis += cbThis;
            }

            if (!(paUsers = malloc(cbTotal)))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                memcpy(paUsers,
                       pCommand->pShared->Data.QueryUsers.paUsers,
                       cbTotal);
                *pcUsers = pCommand->pShared->Data.QueryUsers.cUsers;
                *ppaUsers = paUsers;
            }

            // free shared mem given to us by XWPShell
            DosFreeMem(pCommand->pShared->Data.QueryUsers.paUsers);
        }

        FreeXWPShellCommand(pCommand);
    }

    return arc;
}

/*
 *@@ xsecQueryGroups:
 *      returns an array of XWPGROUPDBENTRY structs
 *      with all the groups currently in the userdb.
 *
 *      Required authority: XWPPERM_QUERYUSERINFO.
 *
 *      Command-specific error codes in addition to those
 *      returned by CreateXWPShellCommand and
 *      SendXWPShellCommand:
 *
 *      --  XWPSEC_INSUFFICIENT_AUTHORITY
 *
 *      plus the error codes from sudbQueryGroups.
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

APIRET xsecQueryGroups(PULONG pcGroups,
                       PXWPGROUPDBENTRY *ppaGroups)    // out: array of users (to be freed)
{
    APIRET              arc = NO_ERROR;
    PXWPSHELLCOMMAND    pCommand;

    if (!(arc = CreateXWPShellCommand(QUECMD_QUERYGROUPS,
                                      &pCommand)))
    {
        if (!(arc = SendXWPShellCommand(pCommand)))
        {
            // alright:
            PXWPGROUPDBENTRY paGroups;
            ULONG cb =   pCommand->pShared->Data.QueryGroups.cGroups
                       * sizeof(XWPGROUPDBENTRY);

            if (!(paGroups = malloc(cb)))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                memcpy(paGroups,
                       pCommand->pShared->Data.QueryGroups.paGroups,
                       cb);
                *pcGroups = pCommand->pShared->Data.QueryGroups.cGroups;
                *ppaGroups = paGroups;
            }

            // free shared mem given to us by XWPShell
            DosFreeMem(pCommand->pShared->Data.QueryGroups.paGroups);
        }

        FreeXWPShellCommand(pCommand);
    }

    return arc;
}

/*
 *@@ xsecQueryProcessOwner:
 *      returns the UID on whose behalf the given
 *      process is running.
 *
 *      Required authority: XWPPERM_QUERYUSERINFO.
 *
 *      Command-specific error codes in addition to those
 *      returned by CreateXWPShellCommand and
 *      SendXWPShellCommand:
 *
 *      --  XWPSEC_INSUFFICIENT_AUTHORITY
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

APIRET xsecQueryProcessOwner(ULONG ulPID,           // in: process ID
                             XWPSECID *puid)        // out: user ID
{
    APIRET              arc = NO_ERROR;
    PXWPSHELLCOMMAND    pCommand;

    if (!(arc = CreateXWPShellCommand(QUECMD_QUERYPROCESSOWNER,
                                      &pCommand)))
    {
        if (!(arc = SendXWPShellCommand(pCommand)))
        {
            // alright:
            *puid = pCommand->pShared->Data.QueryProcessOwner.uid;
        }

        FreeXWPShellCommand(pCommand);
    }

    return arc;
}

/*
 *@@ xsecCreateUser:
 *      creates a new user in the user database.
 *
 *      Required authority: XWPPERM_CREATEUSER.
 *
 *      Command-specific error codes in addition to those
 *      returned by CreateXWPShellCommand and
 *      SendXWPShellCommand:
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_FILENAME_EXCED_RANGE: one of the given
 *          strings is too long.
 *
 *      --  XWPSEC_INSUFFICIENT_AUTHORITY
 *
 *      --  XWPSEC_USER_EXISTS
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

APIRET xsecCreateUser(PCSZ pcszUserName,        // in: user name
                      PCSZ pcszFullName,        // in: user's descriptive name
                      PCSZ pcszPassword,        // in: user's password
                      XWPSECID gid,             // in: group of the new user
                      XWPSECID *puid)           // out: user ID that was created
{
    APIRET              arc = NO_ERROR;
    PXWPSHELLCOMMAND    pCommand;

    if (!(arc = CreateXWPShellCommand(QUECMD_CREATEUSER,
                                      &pCommand)))
    {
        PQUEUEUNION pUnion = &pCommand->pShared->Data;

        if (    (!(arc = strhCopyBuf(pUnion->CreateUser.szUserName,
                                     pcszUserName,
                                     sizeof(pUnion->CreateUser.szUserName))))
             && (!(arc = strhCopyBuf(pUnion->CreateUser.szFullName,
                                     pcszFullName,
                                     sizeof(pUnion->CreateUser.szFullName))))
             && (!(arc = strhCopyBuf(pUnion->CreateUser.szPassword,
                                     pcszPassword,
                                     sizeof(pUnion->CreateUser.szPassword))))
             && (!(arc = SendXWPShellCommand(pCommand)))
           )
        {
            // alright:
            *puid = pUnion->CreateUser.uidCreated;
        }

        FreeXWPShellCommand(pCommand);
    }

    return arc;
}

/*
 *@@ xsecSetUserData:
 *      updates the user data for the given UID.
 *
 *      Required authority: XWPPERM_CHANGEUSER.
 *
 *      Command-specific error codes in addition to those
 *      returned by CreateXWPShellCommand and
 *      SendXWPShellCommand:
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_FILENAME_EXCED_RANGE: one of the given
 *          strings is too long.
 *
 *      --  XWPSEC_INSUFFICIENT_AUTHORITY
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

APIRET xsecSetUserData(XWPSECID uid,
                       PCSZ pcszUserName,
                       PCSZ pcszFullName)
{
    APIRET              arc = NO_ERROR;
    PXWPSHELLCOMMAND    pCommand;

    if (!(arc = CreateXWPShellCommand(QUECMD_SETUSERDATA,
                                      &pCommand)))
    {
        PQUEUEUNION pUnion = &pCommand->pShared->Data;

        pUnion->SetUserData.uid = uid;

        if (    (!(arc = strhCopyBuf(pUnion->SetUserData.szUserName,
                                     pcszUserName,
                                     sizeof(pUnion->SetUserData.szUserName))))
             && (!(arc = strhCopyBuf(pUnion->SetUserData.szFullName,
                                     pcszFullName,
                                     sizeof(pUnion->SetUserData.szFullName))))
             && (!(arc = SendXWPShellCommand(pCommand)))
           )
        {
        }

        FreeXWPShellCommand(pCommand);
    }

    return arc;
}

/*
 *@@ xsecDeleteUser:
 *      deletes the user account for the given UID.
 *
 *      Required authority: XWPPERM_DELETEUSER.
 *
 *      Command-specific error codes in addition to those
 *      returned by CreateXWPShellCommand and
 *      SendXWPShellCommand:
 *
 *      --  XWPSEC_INSUFFICIENT_AUTHORITY
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

APIRET xsecDeleteUser(XWPSECID uid)
{
    APIRET              arc = NO_ERROR;
    PXWPSHELLCOMMAND    pCommand;

    if (!(arc = CreateXWPShellCommand(QUECMD_DELETEUSER,
                                      &pCommand)))
    {
        PQUEUEUNION pUnion = &pCommand->pShared->Data;

        pUnion->uidDelete = uid;

        arc = SendXWPShellCommand(pCommand);

        FreeXWPShellCommand(pCommand);
    }

    return arc;
}

/*
 *@@ xsecQueryPermissions:
 *      returns the permissions that the user on whose
 *      behalf the calling process runs has on the
 *      given resource. *pflAccess receives the ORed
 *      XWPACCESS_* flags that represent these
 *      permissions.
 *
 *      This performs a full authorization on the given
 *      resource according to the general authorization
 *      rules.
 *
 *      Required authority: none.
 *
 *      Command-specific error codes in addition to those
 *      returned by CreateXWPShellCommand and
 *      SendXWPShellCommand:
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_FILENAME_EXCED_RANGE
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

APIRET xsecQueryPermissions(PCSZ pcszFilename,
                            PULONG pflAccess)
{
    APIRET              arc = NO_ERROR;
    PXWPSHELLCOMMAND    pCommand;

    if (!(arc = CreateXWPShellCommand(QUECMD_QUERYPERMISSIONS,
                                      &pCommand)))
    {
        PQUEUEUNION pUnion = &pCommand->pShared->Data;

        if (    (!(arc = strhCopyBuf(pUnion->QueryPermissions.szResource,
                                     pcszFilename,
                                     sizeof(pUnion->QueryPermissions.szResource))))
             && (!(arc = SendXWPShellCommand(pCommand)))
           )
        {
            *pflAccess = pUnion->QueryPermissions.flAccess;
        }

        FreeXWPShellCommand(pCommand);
    }

    return arc;
}


