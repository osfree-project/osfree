
/*
 *@@sourcefile acldb.c:
 *      database for access control lists (ACL's).
 *
 *      ACL's are complicated and difficult to understand.
 *      It is important to separate between two representations
 *      of ACL entries:
 *
 *      --  The ACL database contains entries for (possibly)
 *          each directory and file on the system.
 *
 *          Each ACL entry has the following fields:
 *
 *          -- OWNUSER    name of user who owns this file.
 *          -- OWNGROUP   name of group this file belongs to.
 *          -- OWNERRIGHTS  rights of the owner (RWX).
 *          -- GROUPRIGHTS  rights of members of the group (RWX).
 *          -- OTHERRIGHTS  rights of everyone else (RWX).
 *
 *          This is exactly the Unix way of doing things.
 *          "ls" output is as follows:
 *
 +              -rwxrwxrwx  4  ownuser owngroup  ...
 +               \o/\g/\o/
 +                w  r  t
 +                n  o  h
 +                e  u  e
 +                r  p  r
 *
 *          This ACLDB implementation uses a plain text file
 *          for speed, where each ACL entry consists of a
 *          single line like this:
 +
 +            F "resname" decuid decgid octrights
 *
 *          being:
 *
 *          -- F: a flag marking the type of resource, being:
 *                  -- "R": root directory (drive)
 *                  -- "D": directory
 *                  -- "F": file
 *                  -- "P": process
 *
 *          -- "resname": the name of the resource, being
 *                  -- for "R": Drive letter (e.g. "G:")
 *                  -- for "D": full directory specification (e.g. "G:\DESKTOP")
 *                  -- for "F": full file path (e.g. "G:\DESKTOP\INDEX.HTML")
 *                  -- for "P": full executable path (e.g. "G:\OS2\E.EXE")
 *
 *          -- decuid: decimal user ID of owner
 *
 *          -- decgid: decimal group ID of owning group
 *
 *          -- octrights: rights flags (octal), as on Unix "chmod"
 *
 *          Per definition, if no ACL entry exists for a
 *          resource (i.e. a file or directory), the ACL
 *          of the parent directory applies, climbing up until
 *          the drive level is reached. This way we only have
 *          to create ACL entries if they are supposed to be
 *          different from the parent's ones.
 *
 *          If no ACL entry is found this way (i.e. not even
 *          the drive has an ACL entry), the following default
 *          is used:
 *
 *          -- OWNUSER root
 *          -- OWNGROUP root
 *          -- access denied for all (i.e. ---------).
 *
 *      --  By contrast, an ACLENTRYNODE structure describes
 *          an ACL entry as associated with a subject handle.
 *
 *      <B>Access verification</B>
 *
 *      This implementation verifies access as follows:
 *
 *      1.  XWPSec calls the ACLDB for each protected resource
 *          which is accessed (unless a process is running
 *          on behalf of the superuser, who always gets access).
 *
 *                  Example 1: XWPSec needs "create" access to the
 *                  F:\home\user directory.
 *
 *                  Example 2: XWPSec needs "write" access to the
 *                  F:\home\user\index.html file.
 *
 *      2.  The ACLDB attempts to find an ACLDBENTRYNODE for
 *          that resource.
 *
 *      3.  If such an entry exists, that entry is used. Go to 4.
 *
 *          If no such entry exists, the parent directory is
 *          searched for. If we've not yet reached the root directory
 *          (drive), go back to 2. Otherwise go to 5.
 *
 *      4.  An ACLDBENTRYNODE was found:
 *
 *          a) The subjects list in the ACLDBENTRYNODE is searched
 *             for the "user" subject handle from the process's
 *             security context.
 *
 *          b) If a "user" entry is found, its access rights are
 *             used for verifying access, and NO_ERROR or
 *             ERROR_ACCESS_DENIED are returned.
 *
 *          c) If no "user" entry is found, the subjects list in
 *             the ACLDBENTRYNODE is searched for the "group"
 *             subject handle from the process's security context.
 *
 *          d) If a "user" entry is found, its access rights are
 *             used for verifying access, and NO_ERROR or
 *             ERROR_ACCESS_DENIED are returned.
 *
 *      5.  No entry was found, not even for the root directory:
 *          return ERROR_ACCESS_DENIED.
 *
 *      When a subject handle is created, the ACLDB must
 *      create an ACLENTRYNODE for each ACL entry which
 *      contains a definition for the user or group ID
 *      that the subject handle is created for.
 *
 *      Example:
 *
 *      1.  User "user" logs on, who belongs to group "users".
 *          The user's ID (uid) is 1, the group id (gid) is 1.
 *
 *      2.  XWPSec creates two subject handles, one for the
 *          user "user", one for the group "users".
 *
 *      3.  On creation of the subject handle for "user",
 *          the ACLDB is notified.
 *
 *          It must then go thru the ACL database (whatever
 *          implementation this code uses) and cache all
 *          DB entries for this subject handle (user or group).
 *
 *          Assuming that the user has his own home directory
 *          F:\home\user with only the following ACL entry:
 *
 *          -- OWNUSER    "user"
 *          -- OWNGROUP   "users"
 *          -- OWNERRIGHTS  RWX
 *          -- GROUPRIGHTS  ---
 *          -- OTHERRIGHTS  ---
 *
 *          So on subject handle creation for uid == 1 ("user),
 *          the ACLDB would create an ACLENTRYNODE for F:\home\user
 *          with type ACLTYPE_DIRECTORY and the "RWX" rights.
 *
 *      4.  The same happens for the subject handle for the
 *          "users" group.
 *
 *          There are probably many ACL entries for the "users"
 *          group.
 *
 *          Assuming that F:\home has the following ACL entry:
 *
 *          -- OWNUSER    "root"        // root owns the home parent
 *          -- OWNGROUP   "users"
 *          -- OWNERRIGHTS  R--
 *          -- GROUPRIGHTS  R--
 *          -- OTHERRIGHTS  ---
 *
 *          So on subject handle creation for gid == 1 ("users"),
 *          the ACLDB would create an ACLENTRYNODE for F:\home
 *          with type ACLTYPE_DIRECTORY and the "R--" rights.
 *
 *      5.  Now assume that "user" attempts to create a file
 *          in "F:\home\user" (his own directory).
 *
 *          This results in a saclVerifyAccess call for
 *          "F:\home\user", asking for authorization.
 *
 *          saclVerifyAccess receives the subject handle for
 *          "user" (and also for the "users" group) and will
 *          find out that an ACLENTRYNODE exists for the user
 *          subject handle and "F:\home\user".
 *
 *          Since "RWX" is specified in that entry, access is
 *          granted.
 *
 *      6.  Now assume that "user" attempts to create a file
 *          in "F:\home" (the parent directory).
 *
 *          Again, saclVerifyAccess receives the subject handle
 *          for "user" and the "users" group.
 *
 *          This time, however, since "F:\home" is not owned
 *          by user, no ACLENTRYNODE exists for that, and for
 *          none of the parent directories either.
 *
 *          saclVerifyAccess will then try the group subject
 *          handle (for "users"). "F:\home" is owned by "users",
 *          and access rights are defined as "R--". So creating
 *          a file is not permitted, and access is turned down
 *          gor food.
 *
 *      Remarks:
 *
 *      -- The functions in this file never get called
 *         for processes running on behalf of the
 *         superuser (root). This is because per definition,
 *         the superuser has unrestricted access to the
 *         system, so no ACL entries are needed.
 *
 *@@added V0.9.5 [umoeller]
 *@@header "security\xwpsecty.h"
 */

/*
 *      Copyright (C) 2000 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>

#include "setup.h"

#include "helpers\dosh.h"
#include "helpers\linklist.h"
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\stringh.h"
#include "helpers\tree.h"               // red-black binary trees

#include "bldlevel.h"

#include "security\xwpsecty.h"

/* ******************************************************************
 *
 *   Private Definitions
 *
 ********************************************************************/

#define ACLTYPE_DRIVE           1
#define ACLTYPE_DIRECTORY       2
#define ACLTYPE_FILE            3
#define ACLTYPE_PROCESS         4

#define ACLACC_READ         0x0001
#define ACLACC_WRITE        0x0002
#define ACLACC_EXEC         0x0004

// octal constants as with Linux
// 1) rights for "other" users
#define ACLACC_READ_OTHER           01
#define ACLACC_WRITE_OTHER          02
#define ACLACC_EXEC_OTHER           04

// 2) rights for group members
#define ACLACC_READ_GROUP           010
#define ACLACC_WRITE_GROUP          020
#define ACLACC_EXEC_GROUP           040

// 3) rights for owner user
#define ACLACC_READ_USER            0100
#define ACLACC_WRITE_USER           0200
#define ACLACC_EXEC_USER            0400

/*
 *@@ ACLSUBJECTNODE:
 *      ACL DB entry node prepared with access rights
 *      for a specific user.
 *
 *      When a subject handle is created, the ACLDB
 *      goes thru all ACL entries and creates ACLSUBJECTNODE's
 *      for that subject (user or group), with the access
 *      rights ready-made.
 *
 *      This is an extended tree node (see helpers\tree.c).
 *      These nodes are sorted according to hSubject,
 *      which is made the tree ID;
 *      the comparison function is fnCompareSubjects.
 */

typedef struct _ACLSUBJECTNODE
{
    HXSUBJECT       hSubject;
                // subject handle (either for a user or a process)
    ULONG           ulXWPAccessRights;
                // access rights as calculated from
                // Unix access rights for this specific subject;
                // any combination of the following:
                // -- XWPACCESS_READ             0x01                // "R"
                // -- XWPACCESS_WRITE            0x02                // "W"
                // -- XWPACCESS_CREATE           0x04                // "C"
                // -- XWPACCESS_EXEC             0x08                // "X"
                // -- XWPACCESS_DELETE           0x10                // "D"
                // -- XWPACCESS_ATRIB            0x20                // "A"
                // -- XWPACCESS_PERM             0x40
} ACLSUBJECTNODE, *PACLSUBJECTNODE;

/*
 *@@ ACLDBENTRYNODE:
 *      this represents one ACL database entry.
 *      This corresponds to a single line in the
 *      database file.
 *
 *      This is an extended tree node (see helpers\tree.c).
 *      These nodes are sorted according to pszName;
 *      there are two comparison functions for this
 *      (fnCompareACLDBNames_Nodes, fnCompareACLDBNames_Data).
 *
 *      Created by LoadACLDatabase.
 */

typedef struct _ACLDBENTRYNODE
{
    TREE        Tree;

    ULONG       ulType;
            // one of the following:
            // -- ACLTYPE_DRIVE         entry is for an entire drive
            // -- ACLTYPE_DIRECTORY     entry is for a directory (or subdirectory)
            // -- ACLTYPE_FILE          entry is for a file

    XWPSECID    uid;
            // user ID (owner of resource)
    XWPSECID    gid;
            // group ID (owner of resource)

    PSZ         pszName;
            // name of the resource to which this entry applies
            // (in a new buffer allocated with malloc());
            // one of the following:
            // -- ACLTYPE_DRIVE:        the drive name (e.g. "G:")
            // -- ACLTYPE_DIRECTORY     the capitalized directory name (e.g. "DESKTOP")
            // -- ACLTYPE_FILE          the capitalized file name (e.g. "INDEX.HTML")

    ULONG       ulUnixAccessRights;
            // Unix access rights flags as stored in ACL database;
            // this is any combination of the following:
            // 1) rights for "other" users
            // -- ACLACC_READ_OTHER           01
            // -- ACLACC_WRITE_OTHER          02
            // -- ACLACC_EXEC_OTHER           04

            // 2) rights for group members
            // -- ACLACC_READ_GROUP           010
            // -- ACLACC_WRITE_GROUP          020
            // -- ACLACC_EXEC_GROUP           040

            // 3) rights for owner user
            // -- ACLACC_READ_USER            0100
            // -- ACLACC_WRITE_USER           0200
            // -- ACLACC_EXEC_USER            0400

    TREE        *treeSubjectRights;
            // member tree containing ACLSUBJECTNODE's,
            // sorted according to ID's; this is
            // initially empty, but one item is added
            // for created subject handle

} ACLDBENTRYNODE, *PACLDBENTRYNODE;

APIRET LoadACLDatabase(PULONG pulLineWithError);

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// ACL database
TREE        *G_treeACLDB;
    // balanced binary tree of currently loaded ACLs;
    // contains ACLDBENTRYNODE's
ULONG       G_cACLSubjects = 0;
HMTX        G_hmtxACLs = NULLHANDLE;
    // mutex semaphore protecting global data

/* ******************************************************************
 *
 *   Initialization
 *
 ********************************************************************/

/*
 *@@ saclInit:
 *      initializes XWPSecurity.
 */

APIRET saclInit(VOID)
{
    APIRET arc = NO_ERROR;

    if (G_hmtxACLs == NULLHANDLE)
    {
        // first call:
        arc = DosCreateMutexSem(NULL,       // unnamed
                                &G_hmtxACLs,
                                0,          // unshared
                                FALSE);     // unowned
        if (arc == NO_ERROR)
        {
            ULONG   ulLineWithError;
            treeInit(&G_treeACLDB);
            arc = LoadACLDatabase(&ulLineWithError);

        }
    }
    else
        arc = XWPSEC_NO_AUTHORITY;

    return arc;
}

/* ******************************************************************
 *
 *   Private Helpers
 *
 ********************************************************************/

/*
 *@@ LockACLs:
 *      locks the global security data by requesting
 *      its mutex.
 *
 *      Always call UnlockACLs() when you're done.
 */

APIRET LockACLs(VOID)
{
    APIRET arc = NO_ERROR;

    arc = DosRequestMutexSem(G_hmtxACLs,
                             SEM_INDEFINITE_WAIT);

    return arc;
}

/*
 *@@ UnlockACLs:
 *      unlocks the global security data.
 */

APIRET UnlockACLs(VOID)
{
    return (DosReleaseMutexSem(G_hmtxACLs));
}

/*
 *@@ fnCompareSubjects:
 *
 */

int TREEENTRY fnCompareSubjects(unsigned long id1, unsigned long id2)
{
    if (id1 < id2)
        return -1;
    if (id1 > id2)
        return +1;
    return 0;
}

/*
 *@@ fnCompareACLDBNames_Nodes:
 *
 */

int TREEENTRY fnCompareACLDBNames_Nodes(TREE *t1, TREE *t2)
{
    int i;
    i = strcmp( ((PACLDBENTRYNODE)t1)->pszName,
                ((PACLDBENTRYNODE)t2)->pszName);
    if (i < 0) return -1;
    if (i > 0) return +1;
    return 0;
}

/*
 *@@ fnCompareACLDBNames_Data:
 *
 */

int TREEENTRY fnCompareACLDBNames_Data(TREE *t1, void *pName)
{
    int i;
    i = strcmp( ((PACLDBENTRYNODE)t1)->pszName,
                (PSZ)pName);
    if (i < 0) return -1;
    if (i > 0) return +1;
    return 0;
}

/* ******************************************************************
 *
 *   ACL Database
 *
 ********************************************************************/

/*
 *@@ LoadACLDatabase:
 *      loads the ACL database.
 *
 *      Called on startup by saclInit.
 *
 *      The caller must lock the ACLDB before using this.
 */

APIRET LoadACLDatabase(PULONG pulLineWithError)
{
    APIRET  arc = NO_ERROR;

    CHAR    szUserDB[CCHMAXPATH];
    PSZ     pszUserDB = NULL,
            pszDBPath = NULL;
    CHAR    szDBPath[CCHMAXPATH];
    FILE    *UserDBFile;

    ULONG   ulLineCount = 0;

    pszDBPath = getenv("XWPUSERDB");
    if (!pszDBPath)
    {
        // XWPUSERDB not specified:
        // default to "?:\os2" on boot drive
        sprintf(szDBPath, "%c:\\OS2", doshQueryBootDrive());
        pszDBPath = szDBPath;
    }
    sprintf(szUserDB, "%s\\xwpusers.acc", pszDBPath);

    UserDBFile = fopen(szUserDB, "r");
    if (!UserDBFile)
        arc = _doserrno;
    else
    {
        CHAR        szLine[300];
        PSZ         pLine = NULL;
        while ((pLine = fgets(szLine, sizeof(szLine), UserDBFile)) != NULL)
        {
            ULONG   ulType = 0;

            // skip spaces, tabs
            while (     (*pLine)
                     && ( (*pLine == ' ') || (*pLine == '\t') )
                  )
                pLine++;
            switch (*pLine)
            {
                case 'R':       // root directory (drive)
                    ulType = ACLTYPE_DRIVE;
                break;

                case 'D':       // directory
                    ulType = ACLTYPE_DIRECTORY;
                break;

                case 'F':       // file
                    ulType = ACLTYPE_FILE;
                break;

                case 'P':       // process
                    ulType = ACLTYPE_PROCESS;
                break;
            }

            if (ulType)
            {
                PSZ pszName = 0;
                pLine++;        // on space now
                while (     (*pLine)
                         && ( (*pLine == ' ') || (*pLine == '\t') )
                      )
                if (!*pLine)
                    arc = XWPSEC_DB_ACL_SYNTAX;
                else
                {
                    // on '"' now
                    pszName = strhQuote(pLine, '"', &pLine);
                    if (!pszName)
                        arc = XWPSEC_DB_ACL_SYNTAX;
                    else
                    {
                        // OK, got name:
                        // pLine points to space after '"' now
                        while (     (*pLine)
                                 && ( (*pLine == ' ') || (*pLine == '\t') )
                              )
                        if (!*pLine)
                            arc = XWPSEC_DB_ACL_SYNTAX;
                        else
                        {
                            ULONG   uid = 0,
                                    gid = 0;
                            CHAR    szRights[20];
                            if (sscanf(pLine, "%d %d %s", &uid, &gid, szRights) != 3)
                                arc = XWPSEC_DB_ACL_SYNTAX;
                            else
                            {
                                if (strlen(szRights) != 9)
                                    arc = XWPSEC_DB_ACL_SYNTAX;
                                else
                                {
                                    // OK, got fields:
                                    ULONG aulRights[]          // rwxrwxrwx
                                        = {
                                                // first digits: user read, write, execute
                                                ACLACC_READ_USER,
                                                ACLACC_WRITE_USER,
                                                ACLACC_EXEC_USER,

                                                // second digits: group read, write, execute
                                                ACLACC_READ_GROUP,
                                                ACLACC_WRITE_GROUP,
                                                ACLACC_EXEC_GROUP,

                                                // third digits: other read, write, execute
                                                ACLACC_READ_OTHER,
                                                ACLACC_WRITE_OTHER,
                                                ACLACC_EXEC_OTHER
                                          };

                                    PACLDBENTRYNODE pNewEntry
                                        = (PACLDBENTRYNODE)malloc(sizeof(ACLDBENTRYNODE));
                                    if (!pNewEntry)
                                        arc = ERROR_NOT_ENOUGH_MEMORY;
                                    else
                                    {
                                        ULONG   ulDigit = 0;

                                        memset(pNewEntry, 0, sizeof(*pNewEntry));

                                        pNewEntry->ulType = ulType;
                                        pNewEntry->pszName = pszName;
                                        pNewEntry->uid = uid;
                                        pNewEntry->gid = gid;

                                        pLine = szRights;
                                        for (ulDigit = 0;
                                             ulDigit < 9;
                                             ulDigit++)
                                        {
                                            if (*pLine != '-')
                                                // access granted:
                                                pNewEntry->ulUnixAccessRights
                                                        |= aulRights[ulDigit];
                                            pLine++;
                                        }

                                        treeInit(&pNewEntry->treeSubjectRights);

                                        treeInsertNode(&G_treeACLDB,
                                                       (TREE*)pNewEntry,
                                                       fnCompareACLDBNames_Nodes,
                                                       FALSE);

                                        _Pmpf(("LoadACLDatabase: got entry \"%s\" -> 0x%lX",
                                                pNewEntry->pszName,
                                                pNewEntry->ulUnixAccessRights));
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (arc != NO_ERROR)
                break;  // while

            ulLineCount++;
        } // end while ((pLine = fgets(szLine, sizeof(szLine), UserDBFile)) != NULL)
        fclose(UserDBFile);
    }

    *pulLineWithError = ulLineCount;

    return arc;
}

/*
 *@@ ConvertACLEntriesForSubject:
 *      loads all entries for the specified user or
 *      group from the ACL database.
 *
 *      This gets called on subject handle creation
 *      by saclSubjectHandleCreated.
 *
 *      If the subject handle is for a user,
 *      pSubjectInfo->bType is SUBJ_USER.
 *      If the subject handle is for a group,
 *      pSubjectInfo->bType is SUBJ_GROUP.
 *
 *      This must go thru the database and check each
 *      ACL entry if it applies to the specified user
 *      or group id (pSubjectInfo->id). If so,
 *      it must add an ACLENTRYNODE to pNewSubjectNode
 *      (which was just created for the new subject handle).
 *
 *      Preconditions:
 *
 *      -- The ACL database is locked by the caller.
 */

VOID ConvertACLEntriesForSubject(TREE* pNode,       // in: current ACLDBENTRYNODE
                                 PVOID pvData)      // in: points to an XWPSUBJECTINFO

{
    APIRET arc = NO_ERROR;

    PXWPSUBJECTINFO pSubjectInfo = (PXWPSUBJECTINFO)pData;
    PACLDBENTRYNODE pACLDBNode = (PACLDBENTRYNODE)pNode;

    // create new subject node
    PACLSUBJECTNODE pSubjectNode = (PACLSUBJECTNODE)malloc(sizeof(ACLSUBJECTNODE));
    if (!pSubjectNode)
        arc = ERROR_NOT_ENOUGH_MEMORY;
    else
    {
        // node created:
        pACLDBNode->Tree.id = pData->pSubjectInfo->hSubject;

        // convert Unix access rights (rwxrwxrwx)
        // to XWPSec (OS/2) access rights for this user
        switch (pACLDBNode->ulType)
        {
            case ACLTYPE_DRIVE:
            case ACLTYPE_DIRECTORY:
                // drive or directory:

                if (pSubjectInfo->bType == SUBJ_USER)
                {
                    // subject info is a "user" entry:
                    // then id is a uid
                    if (pACLDBNode->uid == pSubjectInfo->id)
                    {
                        // user subject owns this file:
                        // then owner rights apply...
                        if (pACLDBNode->ulUnixAccessRights & ACLACC_READ_USER)
                            ulUnixAccessRights |= ACLACC_READ;
                        if (pACLDBNode->ulUnixAccessRights & ACLACC_WRITE_USER)
                            ulUnixAccessRights |= ACLACC_WRITE;
                        if (pACLDBNode->ulUnixAccessRights & ACLACC_EXEC_USER)
                            ulUnixAccessRights |= ACLACC_EXEC;
                    }
                }
                else if (pSubjectInfo->bType == SUBJ_GROUP)
                {
                    // subject info is a "group" entry:
                    // then id is a gid
                    if (pACLDBNode->uid == pSubjectInfo->id)
                }

            break;

            case ACLTYPE_FILE:
            break;
        }


        // insert node into tree
        treeInsertID(&pACLDBNode->treeSubjectRights,
                     (TREE*)pSubjectNode,
                     fnCompareSubjects,
                     FALSE);        // no duplicates
    }

    // return arc;
}

/*
 *@@ FreeACLEntries:
 *      frees all ACL entries associated with the
 *      specified subject handle.
 *
 *          @@todo
 *
 *      The caller must lock the ACLDB before using this.
 */

APIRET FreeACLEntries(PACLSUBJECTNODE pNewNode)
{
    APIRET arc = NO_ERROR;

    return arc;
}

/*
 *@@ FindACLDBEntry:
 *
 */

PACLDBENTRYNODE FindACLDBEntry(const char *pcszName)
{
    PACLDBENTRYNODE pEntry = treeFindEQData(&G_treeACLDB,
                                            (PVOID)pcszName,
                                            fnCompareACLDBNames_Data);
    return (pEntry);
}

/* ******************************************************************
 *
 *   Access Control APIs
 *
 ********************************************************************/

/*
 *@@ saclSubjectHandleCreated:
 *      this gets called whenever a subject handle
 *      is created on the system.
 *
 *      It is guaranteed that this function only gets
 *      called once per subject handle, even if the
 *      subject handle represents a group.
 *
 *      The ACLDB must load the ACL entries related
 *      to the subject's user or group ID so that
 *      ACL entries can later be used for authorization.
 *
 *      If this returns something != NO_ERROR, a
 *      security violation is raised.
 *
 *      Preconditions:
 *
 *      -- This never gets called for root's subject
 *         handle (whose uid is 0) because per definition,
 *         there are no ACL entries for root. Root has
 *         unrestricted access to the machine. As a
 *         result, saclVerifyAccess never gets called
 *         for root either.
 *
 *      -- This does get called for the admin group's
 *         subject handle however because users of the
 *         admin group can be restricted.
 */

APIRET saclSubjectHandleCreated(PCXWPSUBJECTINFO pSubjectInfo)
{
    APIRET arc = NO_ERROR;

    BOOL fLocked = (LockACLs() == NO_ERROR);
    if (!fLocked)
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        CONVERTDATA ConvertData;

        ConvertData.pSubjectInfo = pSubjectInfo;

        // go thru the ACLDB and create subject nodes
        // in each DB node...
        treeTraverse(G_treeACLDB,
                     ConvertACLEntriesForSubject,
                     &ConvertData,
                     1);
    }

    if (fLocked)
        UnlockACLs();

    return arc;
}

/*
 *@@ saclSubjectHandleDeleted:
 *
 *
 *      Preconditions: See saclSubjectHandleCreated.
 */

APIRET saclSubjectHandleDeleted(HXSUBJECT hSubject)
{
    APIRET arc = NO_ERROR;

    BOOL fLocked = (LockACLs() == NO_ERROR);
    if (!fLocked)
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
    }

    if (fLocked)
        UnlockACLs();

    return arc;
}

/*
 *@@ saclVerifyAccess:
 *      this function is called every single time
 *      when XWPSec needs to have access to a
 *      directory authorized.
 *
 *      This must return either NO_ERROR (if access
 *      is granted) or ERROR_ACCESS_DENIED (if
 *      access is denied).
 *
 *      In pContext, this function receives the
 *      XWPSECURITYCONTEXT of the process which
 *      needs to have access verified.
 *
 *      This function then needs to look up the
 *      ACL's for the directory specified in pcszDir,
 *      based on the user and group subject handles
 *      provided in the security context.
 *
 *      It is up to this function to determine
 *      how user and group rights are evaluated
 *      and which one should have priority.
 *
 *      Preconditions:
 *
 *      -- This never gets called for processes running
 *         on behalf of root's subject handle (whose uid
 *         is 0) because per definition, there are no ACL
 *         entries for root. Root has unrestricted access
 *         to the machine.
 *
 *      -- This does get called for processes running on
 *         behalf of a non-superuser belonging the "admin"
 *         group however because users of the admin group
 *         can be restricted.
 */

APIRET saclVerifyAccess(PCXWPSECURITYCONTEXT pContext,   // in: security context of process
                           const char *pcszDir,             // in: fully qualified directory name
                           ULONG ulRequiredAccess)          // in: OR'ed XWPACCESS_* flags
{
    APIRET arc = NO_ERROR;

    BOOL fLocked = (LockACLs() == NO_ERROR);

    _Pmpf(("Authorizing dir %s", pcszDir));

    if (!fLocked)
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        PACLDBENTRYNODE pACLEntry = NULL;       // default: not found

        PSZ pszDir2 = strdup(pcszDir);
        nlsUpper(pszDir2);

        while (TRUE)        // run at least once
        {
            PSZ p;
            // find matching ACL entry
            pACLEntry = FindACLDBEntry(pszDir2);
            if (pACLEntry)
                // item found:
                break;

            // no ACL entry found:
            // try parent directory
            p = strrchr(pszDir2, '\\');
            if (!p)
                // we've reached root:
                // that means ACL's don't even
                // exist for the root directory,
                // so deny access
                break;
            else
                // overwrite '\' with 0,
                // so we can search for parent directory;
                // e.g. C:\DESKTOP\PROGRAMS\GAMES
                *p = 0;
                // now  G:\DESKTOP\PROGRAMS
        }

        if (!pACLEntry)
            // no ACL entry found:
            arc = ERROR_ACCESS_DENIED;
        else
            // ACL entry found:
            arc = CompareAccessDir(ulRequiredAccess,
                                   pACLEntry);

        free(pszDir2);
    }

    if (fLocked)
        UnlockACLs();

    return NO_ERROR;
    // return arc;
}

