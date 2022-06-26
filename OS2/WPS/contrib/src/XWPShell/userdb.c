
/*
 *@@sourcefile userdb.c:
 *      implements the user database for XWorkplace Security
 *      (XWPSec), which contains all users and groups which
 *      have been defined for the system together with their
 *      passwords and authority flags.
 *
 *      As explained in xwpshell.c, the user database is
 *      a black box. This has the following consequences:
 *
 *      -- The implementation can be completely replaced as
 *         long as the implementation is capable of creating
 *         XWPGROUPDBENTRY and XWPUSERDBENTRY entries,
 *         which represent entries in the database.
 *         For example, this implementation could be replaced
 *         with a version which parses a server database (e.g.
 *         using the Net* API's).
 *
 *      -- The XWPSec kernel knows nothing about passwords.
 *         It is up to the UserDB implementation to store
 *         passwords as well as encrypt and decrypt them.
 *         These are private to the UserDB and not seen
 *         externally. If password encryption is desired,
 *         it must be implemented here.
 *
 *         As a result, only the UserDB can authenticate
 *         users (match login name against a password).
 *         See sudbAuthenticateUser.
 *
 *      The following definitions must be met by this
 *      implementation:
 *
 *      -- A unique user ID (uid) must be assigned to each user
 *         name. For speed, XWPSec operates on user IDs instead
 *         of user names to identify users. So user IDs must be
 *         persistent between reboots. It is the responsibility
 *         of the UserDB to associate user names with user IDs.
 *
 *      -- Same for group names and group IDs (gid's).
 *
 *      -- Each user can belong to no group, one group, or
 *         several groups.
 *
 *      -- User ID 0 (zero) is special and reserved for the
 *         the superuser, who is granted full access. The
 *         user name for the superuser can be freely assigned
 *         (i.e. doesn't have to be "root"). XWPSec completely
 *         disables access control for uid 0.
 *
 *      -- Group ID 0 (zero) is special and reserved for
 *         the super user as well. No other user can be
 *         a member of this group.
 *
 *      An XWP Security Database (UserDB) must implement the
 *      following functions:
 *
 *      --  sudbInit
 *
 *      --  sudbAuthenticateUser
 *
 *      --  sudbQueryName
 *
 *      --  sudbQueryID
 *
 *      --  sudbCreateUser
 *
 *      --  sudbDeleteUser
 *
 *      --  sudbCreateGroup
 *
 *      --  sudbDeleteGroup
 *
 *      These functions get called by the rest of XWPSec for
 *      user management.
 *
 *      This implementation stores users and groups in an XML
 *      file called xwpusers.xml.
 *
 *@@added V0.9.5 [umoeller]
 *@@header "security\xwpshell.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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

#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>

#include "setup.h"

#include "expat\expat.h"                // must come before xml.h

#include "helpers\dosh.h"
#include "helpers\linklist.h"
#include "helpers\stringh.h"
#include "helpers\tree.h"
#include "helpers\xstring.h"
#include "helpers\xml.h"

#include "helpers\xwpsecty.h"
#include "security\xwpshell.h"

/* ******************************************************************
 *
 *   Declarations
 *
 ********************************************************************/

/*
 *@@ XWPUSERDB:
 *      the user database.
 *      This contains two linked lists with user
 *      and group definitions.
 */

typedef struct _XWPUSERDB
{
    LINKLIST    llGroups;       // list of XWPGROUPDBENTRY items
    LINKLIST    llUsers;        // list of XWPUSERDBENTRY items
} XWPUSERDB, *PXWPUSERDB;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

HMTX        G_hmtxUserDB = NULLHANDLE;
    // mutex semaphore protecting global data

XWPUSERDB   G_Database;

/* ******************************************************************
 *
 *   Private helpers
 *
 ********************************************************************/

/*
 *@@ LockUserDB:
 *      locks the global security data by requesting
 *      its mutex.
 *
 *      Always call UnlockUserDB() when you're done.
 */

BOOL LockUserDB(VOID)
{
    return !DosRequestMutexSem(G_hmtxUserDB,
                               SEM_INDEFINITE_WAIT);
}

/*
 *@@ UnlockUserDB:
 *      unlocks the global security data.
 */

VOID UnlockUserDB(VOID)
{
    DosReleaseMutexSem(G_hmtxUserDB);
}

/*
 *@@ FindUserFromID:
 *
 *
 *      Private function.
 *
 *      Preconditions:
 *
 *      --  Caller must hold the db mutex.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

const XWPUSERDBENTRY* FindUserFromID(PLINKLIST pllUsers,
                                     XWPSECID uid)
{
    PLISTNODE pNode = lstQueryFirstNode(pllUsers);
    while (pNode)
    {
        const XWPUSERDBENTRY *pUserThis = (const XWPUSERDBENTRY *)pNode->pItemData;
        if (pUserThis->User.uid == uid)
            return pUserThis;

        pNode = pNode->pNext;
    }

    return NULL;
}

/*
 *@@ FindUserFromName:
 *
 *
 *      Private function.
 *
 *      Preconditions:
 *
 *      --  Caller must hold the db mutex.
 */

const XWPUSERDBENTRY* FindUserFromName(PLINKLIST pllUsers,
                                       PCSZ pcszUserName)
{
    PLISTNODE pNode = lstQueryFirstNode(pllUsers);
    while (pNode)
    {
        const XWPUSERDBENTRY *pUserThis = (const XWPUSERDBENTRY *)pNode->pItemData;
        if (!strcmp(pUserThis->User.szUserName, pcszUserName))
            return pUserThis;

        pNode = pNode->pNext;
    }

    return NULL;
}

/*
 *@@ FindGroupFromID:
 *
 *
 *      Private function.
 *
 *      Preconditions:
 *
 *      --  Caller must hold the db mutex.
 */

const XWPGROUPDBENTRY* FindGroupFromID(PLINKLIST pllGroups,
                                       XWPSECID gid)
{
    PLISTNODE pNode = lstQueryFirstNode(pllGroups);
    while (pNode)
    {
        const XWPGROUPDBENTRY *pGroupThis = (const XWPGROUPDBENTRY *)pNode->pItemData;
        if (pGroupThis->gid == gid)
            // found:
            return pGroupThis;

        pNode = pNode->pNext;
    }

    return NULL;
}

/*
 *@@ CreateGroup:
 *      creates a new XWPGROUPDBENTRY from the
 *      specified XML DOM node and appends it to
 *      the database.
 *
 *      Preconditions:
 *
 *      --  Caller must hold the db mutex.
 *
 *      Returns:
 *
 *      --  NO_ERROR: group was added to pDB->llGroups.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  XWPSEC_DB_GROUP_SYNTAX: syntax error in
 *          group's XML element.
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 */

APIRET CreateGroup(PXWPUSERDB pDB,
                   PDOMNODE pGroupElementThis)
{
    APIRET arc = NO_ERROR;

    PXWPGROUPDBENTRY pNewGroup;
    if (!(pNewGroup = malloc(sizeof(XWPGROUPDBENTRY))))
        arc = ERROR_NOT_ENOUGH_MEMORY;
    else
    {
        const XSTRING *pstrGroupID;
        if (!(pstrGroupID = xmlGetAttribute(pGroupElementThis,
                                            "GROUPID")))
            arc = XWPSEC_DB_GROUP_SYNTAX;
        else
        {
            PDOMNODE pCDATA;

            pNewGroup->gid = atoi(pstrGroupID->psz);

            // get group name (GROUP element character data)
            if (!(pCDATA = xmlGetFirstText(pGroupElementThis)))
                arc = XWPSEC_DB_GROUP_SYNTAX;
            else
            {
                strhncpy0(pNewGroup->szGroupName,
                          pCDATA->pstrNodeValue->psz,
                          sizeof(pNewGroup->szGroupName));

                _PmpfF(("created group \"%s\"", pNewGroup->szGroupName));

                // add to database
                lstAppendItem(&pDB->llGroups, pNewGroup);
            }
        }

        if (arc)
            free(pNewGroup);
    }

    _PmpfF(("returning %d", arc));

    return arc;
}

/*
 *@@ CreateUser:
 *      creates a new XWPUSERDBENTRY from the
 *      specified XML DOM node and appends it to
 *      the database.
 *
 *      Preconditions:
 *
 *      --  The groups must already have been loaded.
 *
 *      --  Caller must hold the db mutex.
 *
 *      Returns:
 *
 *      --  NO_ERROR: user was added to pDB->llUsers.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  XWPSEC_DB_USER_SYNTAX: syntax error in
 *          user's XML element.
 *
 *      --  XWPSEC_INVALID_ID: cannot find group
 *          for the user's group ID.
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 */

APIRET CreateUser(PXWPUSERDB pDB,
                  PDOMNODE pUserElementThis)
{
    APIRET arc = NO_ERROR;

    const XSTRING   *pstrUserID,
                    *pstrGroupID,
                    *pstrName,
                    *pstrPass;
    PDOMNODE        pCDATA;

    if (    (!(pstrGroupID = xmlGetAttribute(pUserElementThis,
                                             "GROUPID")))
         || (!(pstrUserID  = xmlGetAttribute(pUserElementThis,
                                             "USERID")))
         || (!(pstrName    = xmlGetAttribute(pUserElementThis,
                                             "NAME")))
         || (!(pstrPass    = xmlGetAttribute(pUserElementThis,
                                             "PASS")))
            // get long user name (USER element character data)
         || (!(pCDATA = xmlGetFirstText(pUserElementThis)))
       )
        arc = XWPSEC_DB_USER_SYNTAX;
    else
    {
        // parse the groups strings first so we know how much
        // to allocate for the user db entry
        LINKLIST llGroups;
        PSZ     pszGroups = strdup(pstrGroupID->psz);
        PSZ     pszToken = strtok(pszGroups, ",");

        const XSTRING *pstrUserShell;
        XSTRING *pstrFree = NULL;

        lstInit(&llGroups, FALSE);
        while (pszToken)
        {
            // append group ID
            const XWPGROUPDBENTRY *pGroupEntry;
            XWPSECID gid = atoi(pszToken);
            if (!(pGroupEntry = FindGroupFromID(&pDB->llGroups,
                                                gid)))
            {
                // cannot find group for this user:
                arc = XWPSEC_INVALID_ID;
                break;
            }

            lstAppendItem(&llGroups, (PVOID)gid);
            pszToken = strtok(NULL, ",");
        };

        // check USERSHELL; if not present, use PMSHELL.EXE
        if (!(pstrUserShell = xmlGetAttribute(pUserElementThis,
                                              "USERSHELL")))
        {
            PCSZ pcszShell;
            if (!(pcszShell = getenv("USERSHELL")))
                pcszShell = "PMSHELL.EXE";

            pstrUserShell = pstrFree = xstrCreate(0);
            xstrcpy(pstrFree, pcszShell, 0);
        }

        if (!arc)
        {
            ULONG   cGroups,
                    cbStruct;
            PXWPUSERDBENTRY pNewUser;

            cGroups = lstCountItems(&llGroups);
            cbStruct =   sizeof(XWPUSERDBENTRY)     // has room for one group already
                       + ((cGroups)
                            ? (cGroups - 1) * sizeof(XWPSECID)
                            : 0);

            if (!(pNewUser = malloc(cbStruct)))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                PLISTNODE pNode;

                pNewUser->cbStruct = cbStruct;

                // store user
                pNewUser->User.uid = atoi(pstrUserID->psz);

                strlcpy(pNewUser->User.szUserName,
                        pstrName->psz,
                        sizeof(pNewUser->User.szUserName));
                strlcpy(pNewUser->User.szFullName,
                        pCDATA->pstrNodeValue->psz,
                        sizeof(pNewUser->User.szFullName));
                strlcpy(pNewUser->User.szUserShell,
                        pstrUserShell->psz,
                        sizeof(pNewUser->User.szUserShell));
                strlcpy(pNewUser->szPassword,
                        pstrPass->psz,
                        sizeof(pNewUser->szPassword));

                // store group IDs
                pNewUser->Membership.cGroups = 0;
                for (pNode = lstQueryFirstNode(&llGroups);
                     pNode;
                     pNode = pNode->pNext)
                {
                    pNewUser->Membership.aGIDs[pNewUser->Membership.cGroups++]
                            = (XWPSECID)pNode->pItemData;
                }


                _PmpfF(("created user \"%s\", pass \"%s\"",
                            pNewUser->User.szUserName,
                            pNewUser->szPassword));

                // add to database
                lstAppendItem(&pDB->llUsers, pNewUser);
            }
        }

        if (pstrFree)
            xstrFree(&pstrFree);
    }

    _PmpfF(("returning %d", arc));

    return arc;
}

/*
 *@@ LoadDB:
 *      this function must load the XWorkplace users
 *      database.
 *
 *      This must fill the specified LINKLIST's with
 *      XWPUSERDBENTRY's for each user in the database
 *      and XWPGROUPDBENTRY's for each group in the
 *      database.
 *
 *      This implementation parses ?:\os2\xwpusers.xml,
 *      which has a special XML-compliant format.
 *
 *      This function gets called every single time
 *      something is needed from the user database.
 *
 *      Preconditions: Caller must hold the UserDB
 *      mutex.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  XWPSEC_DB_USER_SYNTAX: syntax error in
 *          user fields.
 *
 *      --  XWPSEC_DB_GROUP_SYNTAX: syntax error in
 *          group fields.
 *
 *      plus the many error codes from doshLoadTextFile,
 *      xmlCreateDOM, xmlParse,
 */

APIRET LoadDB(VOID)
{
    APIRET      arc = NO_ERROR;

    CHAR    szUserDB[CCHMAXPATH];
    PSZ     pszUserDB = NULL,
            pszDBPath = NULL;
    CHAR    szDBPath[CCHMAXPATH];

    _PmpfF(("entering"));

    lstInit(&G_Database.llGroups, TRUE);      // auto-free
    lstInit(&G_Database.llUsers, TRUE);       // auto-free

    if (!(pszDBPath = getenv("ETC")))
    {
        // ETC not specified:
        // default to "?:\os2" on boot drive
        sprintf(szDBPath, "%c:\\OS2", doshQueryBootDrive());
        pszDBPath = szDBPath;
    }
    sprintf(szUserDB, "%s\\xwpusers.xml", pszDBPath);

    if (!(arc = doshLoadTextFile(szUserDB,
                                 &pszUserDB,
                                 NULL)))
    {
        // text file loaded:

        // create the DOM
        PXMLDOM pDom = NULL;
        if (!(arc = xmlCreateDOM(0,             // no validation
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &pDom)))
        {
            if (!(arc = xmlParse(pDom,
                                 pszUserDB,
                                 strlen(pszUserDB),
                                 TRUE)))    // last chunk (we only have one)
            {
                // OK, now we got all the data in the DOMNODEs:

                // 1) get the root element
                PDOMNODE pRootElement;
                if (pRootElement = xmlGetRootElement(pDom))
                {
                    // 2) get all GROUP elements
                    PLINKLIST pllGroups;

                    _PmpfF(("loading groups"));

                    if (pllGroups = xmlGetElementsByTagName(pRootElement,
                                                            "GROUP"))
                    {
                        // copy all groups to our private data
                        PLISTNODE pGroupNode;

                        _PmpfF(("got %d groups", lstCountItems(pllGroups)));

                        for (pGroupNode = lstQueryFirstNode(pllGroups);
                             (pGroupNode) && (!arc);
                             pGroupNode = pGroupNode->pNext)
                        {
                            PDOMNODE pGroupElementThis = (PDOMNODE)pGroupNode->pItemData;
                            arc = CreateGroup(&G_Database,
                                              pGroupElementThis);
                        }

                        lstFree(&pllGroups);

                        _PmpfF(("loading users"));

                        if (!arc)
                        {
                            // 3) get all USER elements
                            PLINKLIST pllUsers;
                            if (pllUsers = xmlGetElementsByTagName(pRootElement,
                                                                   "USER"))
                            {
                                // copy all users to our private data
                                PLISTNODE pUserNode;

                                _PmpfF(("got %d users", lstCountItems(pllUsers)));

                                for (pUserNode = lstQueryFirstNode(pllUsers);
                                     (pUserNode) && (!arc);
                                     pUserNode = pUserNode->pNext)
                                {
                                    PDOMNODE pUserElementThis = (PDOMNODE)pUserNode->pItemData;
                                    arc = CreateUser(&G_Database,
                                                     pUserElementThis);
                                }

                                lstFree(&pllUsers);
                            }
                            else
                                arc = XWPSEC_DB_USER_SYNTAX;
                        }
                    }
                    else
                        arc = XWPSEC_DB_USER_SYNTAX;
                }
                else
                    arc = XWPSEC_DB_USER_SYNTAX;
            }

            xmlFreeDOM(pDom);
        }

        free(pszUserDB);
    }

    _PmpfF(("returning %d", arc));

    return arc;
}

/* ******************************************************************
 *
 *   Initialization
 *
 ********************************************************************/

/*
 *@@ sudbInit:
 *      initializes XWPSecurity.
 */

APIRET sudbInit(VOID)
{
    APIRET arc = NO_ERROR;

    if (G_hmtxUserDB == NULLHANDLE)
    {
        // first call:
        if (!(arc = DosCreateMutexSem(NULL,       // unnamed
                                      &G_hmtxUserDB,
                                      0,          // unshared
                                      TRUE)))       // request now
        {
            arc = LoadDB();

            DosReleaseMutexSem(G_hmtxUserDB);
        }
    }
    else
        arc = XWPSEC_INSUFFICIENT_AUTHORITY;

    return arc;
}

/* ******************************************************************
 *
 *   Public Interfaces
 *
 ********************************************************************/

/*
 *@@ sudbAuthenticateUser:
 *      authenticates the user specified by pUserInfo.
 *      This gets called during logon.
 *
 *      On input, specify XWPUSERDBENTRY.szUserName and
 *      XWPUSERDBENTRY.szPassword, as entered by the
 *      user. All other fields are ignored.
 *
 *      If XWPUSERDBENTRY.szUserName exists in the database
 *      and  XWPUSERDBENTRY.szPassword is correct, this
 *      returns NO_ERROR. In that case, the XWPUSERDBENTRY
 *      and XWPGROUPDBENTRY structures are updated to
 *      contain the user and group info, which the caller
 *      can then use to create the subject handles.
 *
 *      Otherwise this returns:
 *
 *      --  XWPSEC_CANNOT_GET_MUTEX:
 *
 *      --  XWPSEC_NOT_AUTHENTICATED: pUserInfo->szUserName
 *          is unknown, or pUserInfo->szPassword doesn't
 *          match the entry in the userdb.
 *
 *      --  XWPSEC_DB_INVALID_GROUPID: userdb error, group
 *          ID is invalid.
 */

APIRET sudbAuthenticateUser(PXWPUSERINFO pUserInfo,     // in/out: user info
                            PCSZ pcszPassword)          // in: password
{
    APIRET arc = NO_ERROR;
    BOOL fLocked;

    _PmpfF(("entering"));

    if (!(fLocked = LockUserDB()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        const XWPUSERDBENTRY *pUserFound;
        if (!(pUserFound = FindUserFromName(&G_Database.llUsers,
                                            pUserInfo->szUserName)))
            arc = XWPSEC_NOT_AUTHENTICATED;
        else
        {
            // user exists:
            if (strcmp(pcszPassword,
                       pUserFound->szPassword))
                arc = XWPSEC_NOT_AUTHENTICATED;
            else
            {
                // password correct also:
                // store user's uid
                memcpy(pUserInfo,
                       &pUserFound->User,
                       sizeof(XWPUSERINFO));
            }
        }
    }

    if (fLocked)
        UnlockUserDB();

    _PmpfF(("leaving"));

    return arc;
}

/*
 *@@ sudbCreateUser:
 *      creates a new user in the user database.
 *
 *      On input, XWPUSERDBENTRY.UserID is ignored.
 *      However, you must specify all other fields.
 *
 *      This does not create a subject handle for the
 *      user. However, to create a subject handle, the
 *      user must be in the database.
 *
 *      Required authority: Process must be running
 *      on behalf of a user of "admin" group (1).
 *
 *      Returns:
 *
 *      --  XWPSEC_CANNOT_GET_MUTEX
 *
 *      --  XWPSEC_USER_EXISTS: a user with the given
 *          (short) user name already exists.
 */

APIRET sudbCreateUser(PXWPUSERDBENTRY pUserInfo)
{
    APIRET arc = NO_ERROR;

    BOOL fLocked;
    if (!(fLocked = LockUserDB()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        const XWPUSERDBENTRY *pUserFound;
        if (pUserFound = FindUserFromName(&G_Database.llUsers,
                                          pUserInfo->User.szUserName))
            // user exists already: fail
            arc = XWPSEC_USER_EXISTS;
        else
        {
            // @@todo
            arc = XWPSEC_INTEGRITY;
        }
    }

    if (fLocked)
        UnlockUserDB();

    return arc;
}

/*
 *@@ sudbQueryUsers:
 *      returns a shared memory block containing all user
 *      definitions, including group memberships. This
 *      consists of XWPUSERDBENTRY structures, but is not
 *      strictly an array as every item can be variable in
 *      size.
 *
 *      The memory is allocated as shared giveable to be
 *      useful for the queue APIs. Call DosFreeMem to free it.
 *
 *      Returns:
 *
 *      --  XWPSEC_CANNOT_GET_MUTEX
 *
 *      --  XWPSEC_NO_USERS: no users in database.
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      plus the many error codes from LoadDB.
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

APIRET sudbQueryUsers(PULONG pcUsers,               // out: user count
                      PXWPUSERDBENTRY *ppaUsers)    // out: array of users (shared memory)
{
    APIRET arc = NO_ERROR;
    BOOL fLocked;

    if (!pcUsers || !ppaUsers)
        return ERROR_INVALID_PARAMETER;

    _PmpfF(("entering"));

    if (!(fLocked = LockUserDB()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        if (!(*pcUsers = lstCountItems(&G_Database.llUsers)))
            arc = XWPSEC_NO_USERS;
        else
        {
            // run through the array and count how much mem we need
            PLISTNODE   pNode;
            ULONG       cbTotal = 0;
            PXWPUSERDBENTRY paUsers;
            FOR_ALL_NODES(&G_Database.llUsers, pNode)
            {
                const XWPUSERDBENTRY *pUserThis = (const XWPUSERDBENTRY *)pNode->pItemData;
                cbTotal += pUserThis->cbStruct;
            }

            if (!(arc = DosAllocSharedMem((PVOID*)&paUsers,
                                          NULL,
                                          cbTotal,
                                          PAG_COMMIT | OBJ_GIVEABLE | PAG_READ | PAG_WRITE)))
            {
                PBYTE pbTarget = (PBYTE)paUsers;
                pNode = lstQueryFirstNode(&G_Database.llUsers);
                while (pNode)
                {
                    const XWPUSERDBENTRY *pSourceThis = (const XWPUSERDBENTRY *)pNode->pItemData;

                    memcpy(pbTarget,
                           pSourceThis,
                           pSourceThis->cbStruct);
                    // never give out the passwords
                    memset(((PXWPUSERDBENTRY)pbTarget)->szPassword,
                           0,
                           sizeof((PXWPUSERDBENTRY)pbTarget)->szPassword);

                    pNode = pNode->pNext;
                    pbTarget += pSourceThis->cbStruct;
                }

                *ppaUsers = paUsers;
            }
        }
    }

    if (fLocked)
        UnlockUserDB();

    _PmpfF(("leaving"));

    return arc;
}

/*
 *@@ sudbQueryGroups:
 *      returns a shared memory block containing all group
 *      definitions as an array of XWPGROUPDBENTRY structs.
 *
 *      The memory is allocated as shared giveable to be
 *      useful for the queue APIs. Call DosFreeMem to free it.
 *
 *      Returns:
 *
 *      --  XWPSEC_CANNOT_GET_MUTEX
 *
 *      --  XWPSEC_NO_GROUPS: no groups in database.
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      plus the many error codes from LoadDB.
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

APIRET sudbQueryGroups(PULONG pcGroups,               // out: user count
                       PXWPGROUPDBENTRY *ppaGroups)    // out: array of users (shared memory)
{
    APIRET arc = NO_ERROR;
    BOOL fLocked;

    if (!pcGroups || !ppaGroups)
        return ERROR_INVALID_PARAMETER;

    _PmpfF(("entering"));

    if (!(fLocked = LockUserDB()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        if (!(*pcGroups = lstCountItems(&G_Database.llGroups)))
            arc = XWPSEC_NO_GROUPS;
        else
        {
            PXWPGROUPDBENTRY paGroups;
            if (!(arc = DosAllocSharedMem((PVOID*)&paGroups,
                                          NULL,
                                          sizeof(XWPGROUPDBENTRY) * *pcGroups,
                                          PAG_COMMIT | OBJ_GIVEABLE | PAG_READ | PAG_WRITE)))
            {
                PLISTNODE pNode = lstQueryFirstNode(&G_Database.llGroups);
                PXWPGROUPDBENTRY pTargetThis = paGroups;
                while (pNode)
                {
                    const XWPGROUPDBENTRY *pSourceThis = (const XWPGROUPDBENTRY *)pNode->pItemData;

                    pTargetThis->gid = pSourceThis->gid;
                    memcpy(pTargetThis->szGroupName,
                           pSourceThis->szGroupName,
                           sizeof(pTargetThis->szGroupName));

                    pNode = pNode->pNext;
                    pTargetThis++;
                }

                *ppaGroups = paGroups;
            }
        }
    }

    if (fLocked)
        UnlockUserDB();

    _PmpfF(("leaving"));

    return arc;
}

/*
 *@@ sudbQueryUser:
 *      returns a single XWPUSERDBENTRY struct listing
 *      user info including the groups that the given user
 *      is a member of.
 *
 *      The memory is allocated as shared giveable to be
 *      useful for the queue APIs. Call DosFreeMem to free it.
 *
 *      Returns:
 *
 *      --  XWPSEC_CANNOT_GET_MUTEX
 *
 *      --  XWPSEC_INVALID_ID
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      plus the many error codes from LoadDB.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

APIRET sudbQueryUser(XWPSECID uid,
                     PXWPUSERDBENTRY *ppUser)
{
    APIRET arc = NO_ERROR;
    BOOL fLocked;

    if (!ppUser)
        return ERROR_INVALID_PARAMETER;

    if (!(fLocked = LockUserDB()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        const XWPUSERDBENTRY* pUserFound;
        if (!(pUserFound = FindUserFromID(&G_Database.llUsers,
                                          uid)))
            arc = XWPSEC_INVALID_ID;
        else
        {
            PXWPUSERDBENTRY pUserReturn;
            if (!(arc = DosAllocSharedMem((PVOID*)&pUserReturn,
                                          NULL,
                                          pUserFound->cbStruct,
                                          PAG_COMMIT | OBJ_GIVEABLE | PAG_READ | PAG_WRITE)))
            {
                memcpy(pUserReturn,
                       pUserFound,
                       pUserFound->cbStruct);

                // do not give out password
                memset(pUserReturn->szPassword,
                       0,
                       sizeof(pUserReturn->szPassword));

                *ppUser = pUserReturn;
            }
        }
    }

    if (fLocked)
        UnlockUserDB();

    _PmpfF(("leaving"));

    return arc;
}

/*
 *@@ sudbQueryUserName:
 *      shortcut function if the caller just needs the
 *      user name for a given user ID.
 *
 *@@added V1.0.2 (2003-11-13) [umoeller]
 */

APIRET sudbQueryUserName(XWPSECID uid,          // in: user id
                         PSZ pszUserName)       // out: user name (buffer must be XWPSEC_NAMELEN in size)
{
    APIRET arc = NO_ERROR;
    BOOL fLocked;

    if (!(fLocked = LockUserDB()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        const XWPUSERDBENTRY* pUserFound;
        if (!(pUserFound = FindUserFromID(&G_Database.llUsers,
                                          uid)))
            arc = XWPSEC_INVALID_ID;
        else
        {
            memcpy(pszUserName,
                   pUserFound->User.szUserName,
                   XWPSEC_NAMELEN);
        }
    }

    if (fLocked)
        UnlockUserDB();

    return arc;
}

