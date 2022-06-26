
/*
 *@@sourcefile contexts.c:
 *      implementation for security contexts, including subject
 *      handles management, ACL setup, and ring-0 interfaces.
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

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVICES
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <setjmp.h>

#include "setup.h"

#include "helpers\except.h"
#include "helpers\dosh.h"
#include "helpers\linklist.h"
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\procstat.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\threads.h"
#include "helpers\tree.h"               // red-black binary trees

#include "helpers\xwpsecty.h"
#include "security\xwpshell.h"
#include "security\ring0api.h"

/* ******************************************************************
 *
 *   Private Definitions
 *
 ********************************************************************/

/*
 *@@ SUBJECTTREENODE:
 *
 */

typedef struct _SUBJECTTREENODE
{
    TREE            Tree;             // tree item (required for tree* to work)
    XWPSUBJECTINFO  SubjectInfo;
} SUBJECTTREENODE, *PSUBJECTTREENODE;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

HFILE       G_hfSec32DD = NULLHANDLE;

// subject infos
TREE        *G_treeSubjects;
    // global linked list of currently active subjects
LONG        G_cSubjects = 0;
    // subjects count
ULONG       G_ulNextHSubject = 1;
    // next HSUBJECT to use (raised with each subject creation);
    // we start with 1 (0 is the special one for root)

// ACL database
TREE        *G_treeACLDB;
    // balanced binary tree of currently loaded ACLs;
    // contains ACLDBTREENODENODE's
LONG        G_cACLDBEntries = 0;
HMTX        G_hmtxACLs = NULLHANDLE;
    // mutex semaphore protecting global data

PRING0BUF   G_pRing0Buf = NULL;         //  system ACL table

THREADINFO  G_tiLogger = {0};

extern PXFILE G_LogFile;            // xwpshell.c

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

BOOL LockACLs(VOID)
{
    return !DosRequestMutexSem(G_hmtxACLs, SEM_INDEFINITE_WAIT);
}

/*
 *@@ UnlockACLs:
 *      unlocks the global security data.
 */

VOID UnlockACLs(VOID)
{
    DosReleaseMutexSem(G_hmtxACLs);
}

/*
 *@@ SecIOCtl:
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

APIRET SecIOCtl(ULONG ulFuncCode,
                PVOID pvData,
                ULONG cbData)
{
    return DosDevIOCtl(G_hfSec32DD,
                       IOCTL_XWPSEC,
                       ulFuncCode,
                       NULL,        // params, we have none
                       0,
                       NULL,
                       pvData,
                       cbData,
                       &cbData);
}

/*
 *@@ FindSubjectInfoFromHandle:
 *      searches the list of subject infos for the
 *      specified subject handle.
 *
 *      Since the subjects tree is sorted according
 *      to handles, this is extremely fast.
 *
 *      Returns the PSUBJECTTREENODE from the list
 *      or NULL if not found.
 *
 *      Preconditions:
 *
 *      --  Call LockACLs() first.
 */

PSUBJECTTREENODE FindSubjectInfoFromHandle(HXSUBJECT hSubject)
{
    PSUBJECTTREENODE pTreeItem
        = (PSUBJECTTREENODE)treeFind(G_treeSubjects,
                                     hSubject,
                                     treeCompareKeys);

    return pTreeItem;
}

/*
 *@@ FindSubjectInfoFromID:
 *      searches the list of subject infos for the
 *      specified (user or group) ID.
 *
 *      Returns the XWPSUBJECTINFO from the list
 *      or NULL if not found.
 *
 *      In the worst case, this needs to traverse
 *      the entire list of subject infos, so this
 *      is not terribly fast.
 *
 *      Preconditions:
 *
 *      --  Call LockACLs() first.
 */

PSUBJECTTREENODE FindSubjectInfoFromID(BYTE bType,       // in: one of SUBJ_USER, SUBJ_GROUP, or SUBJ_PROCESS
                                       XWPSECID id)      // in: ID to search for
{
    PSUBJECTTREENODE pNode = (PSUBJECTTREENODE)treeFirst(G_treeSubjects);
    while (pNode)
    {
        PXWPSUBJECTINFO psi = &pNode->SubjectInfo;

        if ((psi->id == id) && (psi->bType == bType))
            return pNode;

        pNode = (PSUBJECTTREENODE)treeNext((TREE*)pNode);
    }

    return NULL;
}

/* ******************************************************************
 *
 *   Security logging thread
 *
 ********************************************************************/

/*
 *@@ LogEntry:
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

APIRET LogEntry(ULONG idLogBuf,
                PEVENTLOGENTRY pThis,
                PCSZ pcszFormat,
                ...)
{
    APIRET arc = NO_ERROR;

    if (G_LogFile)
    {
        DATETIME dt;
        CHAR szTemp[2000];
        ULONG   ulLength;

        ulLength = sprintf(szTemp,
                           "%04lX|%04lX %08lX %04d-%02d-%02d %02d:%02d:%02d %08lX|%08lX",
                           pThis->ctxt.pid, pThis->ctxt.tid, pThis->idContext,
                           pThis->stamp.year, pThis->stamp.month, pThis->stamp.day,
                           pThis->stamp.hours, pThis->stamp.minutes, pThis->stamp.seconds,
                           idLogBuf, pThis->idEvent);
        if (!(arc = doshWrite(G_LogFile,
                              ulLength,
                              szTemp)))
        {
            va_list arg_ptr;
            va_start(arg_ptr, pcszFormat);
            ulLength = vsprintf(szTemp, pcszFormat, arg_ptr);
            va_end(arg_ptr);

            szTemp[ulLength++] = '\n';

            arc = doshWrite(G_LogFile,
                            ulLength,
                            szTemp);
        }
    }

    return arc;
}

/*
 *@@ LogLoop:
 *
 *@@added V1.0.1 (2003-01-13) [umoeller]
 */

VOID LogLoop(PTHREADINFO ptiMyself,
             PLOGBUF pLogBuf)
{
    APIRET  arc;

    while (!ptiMyself->fExit)
    {
        if (!(arc = SecIOCtl(XWPSECIO_GETLOGBUF,
                             pLogBuf,
                             LOGBUFSIZE)))
        {
            ULONG ul;
            PEVENTLOGENTRY pThis = (PEVENTLOGENTRY)((PBYTE)pLogBuf + sizeof(LOGBUF));

            for (ul = 0;
                 ul < pLogBuf->cLogEntries;
                 ++ul)
            {
                PBYTE   pbData = (PBYTE)pThis + sizeof(EVENTLOGENTRY);

                switch (pThis->ulEventCode)
                {
                    case EVENT_OPEN_PRE:
                    {
                        PEVENTBUF_OPEN pOpen = (PEVENTBUF_OPEN)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: OPEN PRE  %04lX=\"%s\" fl-%08lX md-%08lX -> rc %d (rq:%02lX gr:%02lX)",
                                 ul,
                                 pOpen->SFN,
                                 pOpen->szPath,
                                 pOpen->fsOpenFlags,
                                 pOpen->fsOpenMode,
                                 pOpen->rc,
                                 pOpen->PRE.fsRequired,
                                 pOpen->PRE.fsGranted);
                    }
                    break;

                    case EVENT_OPEN_POST:
                    {
                        PEVENTBUF_OPEN pOpen = (PEVENTBUF_OPEN)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: OPEN POST %04lX=\"%s\" fl-%08lX md-%08lX -> rc %d (c:%d)",
                                 ul,
                                 pOpen->SFN,
                                 pOpen->szPath,
                                 pOpen->fsOpenFlags,
                                 pOpen->fsOpenMode,
                                 pOpen->rc,
                                 (pLogBuf->idLogBuf)
                                    ? pOpen->POST.cOpenFiles
                                    : 0);
                    }
                    break;

                    case EVENT_LOADEROPEN:
                    {
                        PEVENTBUF_LOADEROPEN pFile = (PEVENTBUF_LOADEROPEN)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: LOADROPEN %04lX=\"%s\" -> rc %d",
                                 ul,
                                 pFile->SFN,
                                 pFile->szPath,
                                 pFile->rc);
                    }
                    break;

                    case EVENT_GETMODULE:
                    {
                        PEVENTBUF_FILENAME pFile = (PEVENTBUF_FILENAME)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: GETMODULE      \"%s\" -> rc %d",
                                 ul,
                                 pFile->szPath,
                                 pFile->rc);
                    }
                    break;

                    case EVENT_EXECPGM_PRE:
                    case EVENT_CREATEVDM_PRE:
                    {
                        PEVENTBUF_FILENAME pExec = (PEVENTBUF_FILENAME)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: %s \"%s\" -> rc %d",
                                 ul,
                                 (pThis->ulEventCode == EVENT_EXECPGM_PRE) ? "EXEC PRE      " : "CREATEVDM PRE ",
                                 pExec->szPath,
                                 pExec->rc);
                    }
                    break;

                    case EVENT_EXECPGM_ARGS:
                        // @@todo
                    break;

                    case EVENT_EXECPGM_POST:
                    {
                        PEVENTBUF_FILENAME pExec = (PEVENTBUF_FILENAME)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: EXEC POST      \"%s\" -> pid %lX",
                                 ul,
                                 pExec->szPath,
                                 pExec->rc);
                    }
                    break;

                    case EVENT_CREATEVDM_POST:
                    {
                        PEVENTBUF_CLOSE pBuf = (PEVENTBUF_CLOSE)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: CREATEVDM POST -> rc %d",
                                 ul,
                                 pBuf->SFN);       // rc
                    }
                    break;

                    case EVENT_CLOSE:
                    {
                        PEVENTBUF_CLOSE pClose = (PEVENTBUF_CLOSE)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: CLOSE     %04lX (c:%d)",
                                 ul,
                                 pClose->SFN,
                                 (pLogBuf->idLogBuf)
                                    ? pClose->cOpenFiles
                                    : 0);
                    }
                    break;

                    case EVENT_DELETE_PRE:
                    {
                        PEVENTBUF_FILENAME pFile = (PEVENTBUF_FILENAME)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: DEL  PRE       \"%s\" -> rc %d",
                                 ul,
                                 pFile->szPath,
                                 pFile->rc);
                    }
                    break;

                    case EVENT_DELETE_POST:
                    {
                        PEVENTBUF_FILENAME pFile = (PEVENTBUF_FILENAME)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: DEL  POST      \"%s\" -> rc %d",
                                 ul,
                                 pFile->szPath,
                                 pFile->rc);
                    }
                    break;

                    case EVENT_MAKEDIR:
                    {
                        PEVENTBUF_FILENAME pFile = (PEVENTBUF_FILENAME)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: MAKEDIR        \"%s\" -> rc %d",
                                 ul,
                                 pFile->szPath,
                                 pFile->rc);
                    }
                    break;

                    case EVENT_CHANGEDIR:
                    {
                        PEVENTBUF_FILENAME pFile = (PEVENTBUF_FILENAME)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: CHANGEDIR      \"%s\" -> rc %d",
                                 ul,
                                 pFile->szPath,
                                 pFile->rc);
                    }
                    break;

                    case EVENT_REMOVEDIR:
                    {
                        PEVENTBUF_FILENAME pFile = (PEVENTBUF_FILENAME)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: REMOVEDIR      \"%s\" -> rc %d",
                                 ul,
                                 pFile->szPath,
                                 pFile->rc);
                    }
                    break;

                    case EVENT_TRUSTEDPATH:
                    {
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: TRUSTEDPATH",
                                 ul);
                    }
                    break;

                    case EVENT_FINDFIRST:
                    case EVENT_FINDFIRST3X:
                    {
                        PEVENTBUF_FILENAME pFile = (PEVENTBUF_FILENAME)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "%04d: %s    \"%s\" -> rc %d (rq:%02lX gr:%02lX)",
                                 ul,
                                 (pThis->ulEventCode == EVENT_FINDFIRST) ? "FINDFIRST  " : "FINDFIRST3X",
                                 pFile->szPath,
                                 pFile->rc,
                                 pFile->fsRequired,
                                 pFile->fsGranted);
                    }
                    break;

                    case EVENT_FINDPERMISSIONS:
                    {
                        PEVENTBUF_FILENAME pFile = (PEVENTBUF_FILENAME)pbData;
                        LogEntry(pLogBuf->idLogBuf,
                                 pThis,
                                 "      -------(test)- \"%s\" -> 0x%lX",
                                 pFile->szPath,
                                 pFile->rc);
                    }
                    break;
                }

                pThis = (PEVENTLOGENTRY)((PBYTE)pThis + pThis->cbStruct);
            }
        }
        else
        {
            doshWriteLogEntry(G_LogFile,
                              "Error: XWPSECIO_GETLOGBUF returned %d",
                              arc);
            break;
        }
    }
}

/*
 *@@ fntLogger:
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

void _Optlink fntLogger(PTHREADINFO ptiMyself)
{
    APIRET  arc;

    PLOGBUF pLogBuf;

    if (!(arc = DosAllocMem((PVOID*)&pLogBuf,
                            LOGBUFSIZE,
                            PAG_COMMIT | PAG_READ | PAG_WRITE | OBJ_TILE)))
    {
        TRY_LOUD(excpt1)
        {
            LogLoop(ptiMyself,
                    pLogBuf);
        }
        CATCH(excpt1)
        {
            doshWriteLogEntry(G_LogFile,
                              "Crash in " __FUNCTION__);
        } END_CATCH();

        DosFreeMem(pLogBuf);
    }

    doshWriteLogEntry(G_LogFile,
                      "Logger thread exiting");
}

/* ******************************************************************
 *
 *   Initialization, status
 *
 ********************************************************************/

/*
 *@@ scxtBuildPIDList:
 *
 *      Caller must free() the list if NO_ERROR is returned.
 *
 *@@added V1.0.2 (2003-11-13) [umoeller]
 */

APIRET scxtGetRunningPIDs(PPROCESSLIST *ppList)
{
    APIRET arc = NO_ERROR;

    // build array of process IDs to send down with SECIO_REGISTER
    PQTOPLEVEL32 pInfo;
    if (pInfo = prc32GetInfo2(QS32_PROCESS | QS32_THREAD, &arc))
    {
        // count processes; i can't see a field for this
        PQPROCESS32 pProcThis = pInfo->pProcessData;
        ULONG cProcs = 0;
        while (pProcThis && pProcThis->ulRecType == 1)
        {
            PQTHREAD32  t = pProcThis->pThreads;
            ++cProcs;
            // for next process, skip the threads info;
            // the next process block comes after the
            // threads
            t += pProcThis->usThreadCount;
            pProcThis = (PQPROCESS32)t;
        }

        if (cProcs)
        {
            PPROCESSLIST    pList;
            ULONG           cbStruct =   sizeof(PROCESSLIST)
                                       + (cProcs - 1) * sizeof(ULONG);

            if (!(pList = malloc(cbStruct)))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                PUSHORT     ppidThis = &pList->apidTrusted[0];

                pList->cbStruct = cbStruct;
                pList->cTrusted = cProcs;

                // start over
                pProcThis = pInfo->pProcessData;
                while (pProcThis && pProcThis->ulRecType == 1)
                {
                    PQTHREAD32  t = pProcThis->pThreads;

                    *ppidThis++ = pProcThis->usPID;

                    // for next process, skip the threads info;
                    // the next process block comes after the
                    // threads
                    t += pProcThis->usThreadCount;
                    pProcThis = (PQPROCESS32)t;
                }

                *ppList = pList;
            }
        }

        prc32FreeInfo(pInfo);
    }

    return arc;
}

/*
 *@@ InitRing0:
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

APIRET InitRing0(VOID)
{
    APIRET  arc = NO_ERROR;
    ULONG   ulActionTaken = 0;

    // return no error if driver is not present,
    // only if it is and _then_ something goes wrong;
    // people are allowed to run XWPShell without the
    // driver
    if (!DosOpen("XWPSEC$",
                 &G_hfSec32DD,
                 &ulActionTaken,
                 0,
                 FILE_NORMAL,
                 OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE | OPEN_FLAGS_FAIL_ON_ERROR,
                 NULL))
    {
        // driver opened:
        PPROCESSLIST    pList;
        if (!(arc = scxtGetRunningPIDs(&pList)))
        {
            // alright, REGISTER these and ENABLE LOCAL SECURITY
            // by calling DosDevIOCtl
            if (!(arc = SecIOCtl(XWPSECIO_REGISTER,
                                 pList,
                                 pList->cbStruct)))
            {
                // this worked:
                // start the logger thread
                thrCreate(&G_tiLogger,
                          fntLogger,
                          NULL,
                          "Logger",
                          THRF_WAIT,
                          0);
            }

            doshWriteLogEntry(G_LogFile,
                              __FUNCTION__ ": XWPSECIO_REGISTER returned %d (0x%lX)",
                              arc, arc);

            free(pList);
        }

        if (arc)
        {
            DosClose(G_hfSec32DD);
            G_hfSec32DD = NULLHANDLE;
        }
    }

    return arc;
}

/*
 *@@ scxtInit:
 *      initializes XWPSecurity.
 */

APIRET scxtInit(VOID)
{
    APIRET arc;

    if (!(arc = DosCreateMutexSem(NULL,       // unnamed
                                  &G_hmtxACLs,
                                  0,          // unshared
                                  FALSE)))    // unowned
    {
        BOOL    fLocked;
        ULONG   ulLineWithError;

        treeInit(&G_treeSubjects, &G_cSubjects);
        treeInit(&G_treeACLDB, &G_cACLDBEntries);

        if (!(fLocked = LockACLs()))
            arc = XWPSEC_CANNOT_GET_MUTEX;
        else
        {
            // create the magic zero subject handle
            XWPSUBJECTINFO si;
            BOOL fRefresh;
            si.id = 0;
            si.bType = SUBJ_USER;
            if (!(arc = scxtCreateSubject(&si, &fRefresh)))
                if (!(arc = saclLoadDatabase(&ulLineWithError)))
                    // try to open the driver
                    arc = InitRing0();

            if (fLocked)
                UnlockACLs();
        }
    }

    return arc;
}

/*
 *@@ scxtExit:
 *      cleans up when XWPShell exits.
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

VOID scxtExit(VOID)
{
    if (G_hfSec32DD)
    {
        SecIOCtl(XWPSECIO_DEREGISTER,
                 NULL,
                 0);
        DosClose(G_hfSec32DD);
        G_hfSec32DD = NULLHANDLE;
    }
}

/*
 *@@ scxtCreateACLEntry:
 *      helper API to be called by saclLoadDatabase as
 *      implemented by the ACL database implementation
 *      to insert an ACL entry into the global database.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  XWPSEC_DB_ACL_DUPRES: an entry for the
 *          given resource already existed.
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

APIRET scxtCreateACLEntry(PACLDBTREENODE pNewEntry)
{
    pNewEntry->Tree.ulKey = (ULONG)pNewEntry->szResName;
    if (treeInsert(&G_treeACLDB,
                   &G_cACLDBEntries,
                   (TREE*)pNewEntry,
                   treeCompareStrings))
        return XWPSEC_DB_ACL_DUPRES;

    return NO_ERROR;
}

/*
 *@@ scxtQueryStatus:
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

APIRET scxtQueryStatus(PXWPSECSTATUS pStatus)
{
    APIRET  arc = NO_ERROR;

    ZERO(pStatus);

    if (G_hfSec32DD)
    {
        XWPSECSTATUS r0s;

        pStatus->fLocalSecurity = TRUE;

        if (!(arc = SecIOCtl(XWPSECIO_QUERYSTATUS,
                             &r0s,
                             sizeof(r0s))))
        {
            memcpy(pStatus, &r0s, sizeof(r0s));
            pStatus->fLocalSecurity = TRUE;
        }
    }

    return arc;
}

/* ******************************************************************
 *
 *   Subject entry points
 *
 ********************************************************************/

/*
 *@@ scxtCreateSubject:
 *      creates a new subject handle, either for
 *      a user or a group, or raises the usage count
 *      for a subject if a handle for the given ID
 *      has already been created previously.
 *
 *      This checks for whether a subject handle exists
 *      already for the specified ID. If so, NO_ERROR is
 *      returned, and the existing subject's usage count
 *      is incremented. pSubjectInfo then receives the
 *      existing HXSUBJECT and usage count.
 *
 *      See XWPSUBJECTINFO for the definition of a subject.
 *
 *      Required input in XWPSUBJECINFO:
 *
 *      --  id: for a user subject: the user ID;
 *              for a group subject: the group ID
 *
 *      --  bType: one of SUBJ_USER, SUBJ_GROUP, SUBJ_PROCESS
 *
 *      This is NOT validated and better be valid.
 *
 *      Output in XWPSUBJECINFO, if NO_ERROR is returned:
 *
 *      --  hSubject: new or existing subject handle.
 *
 *      --  cUsage: usage count.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  XWPSEC_INTEGRITY
 *
 *      Postconditions:
 *
 *      --  Caller must call scxtRefresh afterwards to rebuild
 *          the system ACL table and send it down to the driver.
 *          This is not done here because the caller may create
 *          many subject handles and only refresh the table after
 *          all subject handles have been created. However, this
 *          operation _must_ be synchronous, so you may not
 *          defer it to a different thread.
 */

APIRET scxtCreateSubject(PXWPSUBJECTINFO pSubjectInfo,  // in/out: subject info
                         BOOL *pfNeedsRefresh)          // out: if set to TRUE, caller must call scxtRefresh
{
    APIRET arc = NO_ERROR;

    BOOL fLocked;
    if (!(fLocked = LockACLs()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        PSUBJECTTREENODE pExisting;
        if (pExisting = FindSubjectInfoFromID(pSubjectInfo->bType,
                                              pSubjectInfo->id))
        {
            // subject exists:
            // increase usage count
            pExisting->SubjectInfo.cUsage++;
            // output to caller
            pSubjectInfo->hSubject = pExisting->SubjectInfo.hSubject;
            pSubjectInfo->cUsage = pExisting->SubjectInfo.cUsage;
        }
        else
        {
            // not created yet:
            // allocate new subject info
            PSUBJECTTREENODE pNewSubject;
            if (!(pNewSubject = (PSUBJECTTREENODE)malloc(sizeof(SUBJECTTREENODE))))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                ULONG   ulNewSubjectHandle;
                // new subject handle (globally unique for each boot)
                if (    (pSubjectInfo->bType == SUBJ_USER)
                     && (pSubjectInfo->id == 0)
                   )
                    // root logging on:
                    ulNewSubjectHandle = 0;
                else
                    // non-root: use new subject handle
                    ulNewSubjectHandle = G_ulNextHSubject++;

                // set key
                pNewSubject->Tree.ulKey = ulNewSubjectHandle;

                // append new item
                if (treeInsert(&G_treeSubjects,
                               &G_cSubjects,
                               (TREE*)pNewSubject,
                               treeCompareKeys))
                {
                    arc = XWPSEC_INTEGRITY;
                    free(pNewSubject);
                }
                else
                {
                    // set up data:

                    // new subject handle
                    pNewSubject->SubjectInfo.hSubject = ulNewSubjectHandle;
                    // copy ID (user or group)
                    pNewSubject->SubjectInfo.id = pSubjectInfo->id;
                    // copy type
                    pNewSubject->SubjectInfo.bType = pSubjectInfo->bType;
                    // set usage count
                    pNewSubject->SubjectInfo.cUsage = 1;

                    // output to caller
                    pSubjectInfo->hSubject = pNewSubject->SubjectInfo.hSubject;
                    pSubjectInfo->cUsage = pNewSubject->SubjectInfo.cUsage;

                    _Pmpf(("Created hsubj 0x%lX for %s %d",
                            pSubjectInfo->hSubject,
                            (pSubjectInfo->bType == SUBJ_USER) ? "user" : "non-user",
                            pSubjectInfo->id));

                    *pfNeedsRefresh = TRUE;
                }
            }
        }
    }

    if (fLocked)
        UnlockACLs();

    return arc;
}

/*
 *@@ scxtDeleteSubject:
 *      lowers the usage count of the given subject handle and
 *      deletes it if it is no longer in use.
 *
 *      Returns:
 *
 *      -- NO_ERROR
 *
 *      -- XWPSEC_INVALID_HSUBJECT: SUBJ_USER only; user already
 *         has a subject handle.
 *
 *      Postconditions:
 *
 *      --  Caller must call scxtRefresh afterwards to rebuild
 *          the system ACL table and send it down to the driver.
 *          This is not done here because the caller may create
 *          many subject handles and only refresh the table after
 *          all subject handles have been created. However, this
 *          operation _must_ be synchroneous, so you may not
 *          defer it to a different thread.
 */

APIRET scxtDeleteSubject(LHANDLE hSubject,
                         BOOL *pfNeedsRefresh)          // out: if set to TRUE, caller must call scxtRefresh
{
    APIRET arc = NO_ERROR;
    BOOL fLocked;

    if (!hSubject)
        // cannot delete root
        return XWPSEC_INVALID_HSUBJECT;

    if (!(fLocked = LockACLs()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        PSUBJECTTREENODE psi;
        if (!(psi = FindSubjectInfoFromHandle(hSubject)))
            // not found:
            arc = XWPSEC_INVALID_HSUBJECT;
        else
            // decrease usage count
            if (!(psi->SubjectInfo.cUsage))
                arc = XWPSEC_INTEGRITY;
            else
            {
                psi->SubjectInfo.cUsage--;
                if (!(psi->SubjectInfo.cUsage))
                {
                    // usage count reached zero:
                    if (treeDelete(&G_treeSubjects,
                                   &G_cSubjects,
                                   (TREE*)psi))
                        arc = XWPSEC_INTEGRITY;
                    else
                    {
                        free(psi);
                        *pfNeedsRefresh = TRUE;
                    }
                }
            }
    }

    if (fLocked)
        UnlockACLs();

    return arc;
}

/*
 *@@ scxtQuerySubjectInfo:
 *      returns subject info for a subject handle.
 *
 *      On input, specify hSubject in pSubjInfo.
 *
 *      Returns:
 *
 *      -- NO_ERROR: user found, pSubjectInfo was filled.
 *
 *      -- XWPSEC_INVALID_HSUBJECT: specified subject handle
 *         does not exist.
 */

APIRET scxtQuerySubjectInfo(PXWPSUBJECTINFO pSubjectInfo)   // in/out: subject info

{
    APIRET arc = NO_ERROR;

    BOOL fLocked;
    if (!(fLocked = LockACLs()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        PSUBJECTTREENODE p;
        if (!(p = FindSubjectInfoFromHandle(pSubjectInfo->hSubject)))
            arc = XWPSEC_INVALID_HSUBJECT;
        else
            memcpy(pSubjectInfo,
                   &p->SubjectInfo,
                   sizeof(XWPSUBJECTINFO));
    }

    if (fLocked)
        UnlockACLs();

    return arc;
}

/*
 *@@ scxtFindSubject:
 *      returns the subject handle for the given user, group,
 *      or process ID.
 *
 *      Returns:
 *
 *      -- NO_ERROR: user found, pSubjectInfo was filled.
 *
 *      -- XWPSEC_INVALID_ID: no subject handle exists
 *         for the given ID.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

APIRET scxtFindSubject(BYTE bType,              // in: one of SUBJ_USER, SUBJ_GROUP, or SUBJ_PROCESS
                       XWPSECID id,             // in: user, group, or process ID
                       HXSUBJECT *phSubject)    // out: subject handle if NO_ERROR
{
    APIRET arc = NO_ERROR;

    BOOL fLocked;
    if (!(fLocked = LockACLs()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        PSUBJECTTREENODE p;
        if (!(p = FindSubjectInfoFromID(bType, id)))
            arc = XWPSEC_INVALID_ID;
        else
            *phSubject = p->SubjectInfo.hSubject;
    }

    if (fLocked)
        UnlockACLs();

    return arc;
}

/* ******************************************************************
 *
 *   Security context entry points
 *
 ********************************************************************/

/*
 *@@ scxtFindSecurityContext:
 *      this attempts to find a security context for a process
 *      ID.
 *
 *      This returns:
 *
 *      --  NO_ERROR: *ppContext was set to a newly allocated
 *          security context, which is to be free()'d by caller.
 *
 *      --  ERROR_INVALID_PARAMETER: ring 0 didn't like this stuff.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  XWPSEC_INVALID_PID: security context doesn't exist.
 */

APIRET scxtFindSecurityContext(USHORT pid,
                               PXWPSECURITYCONTEXTCORE *ppContext)
{
    APIRET          arc = NO_ERROR;
    USHORT          *pcSubjects = NULL;
    HXSUBJECT       *paSubjects;

    PSECIOCONTEXT   pContext = NULL;
    PXWPLOGGEDON    pLogon = NULL;

    if (G_hfSec32DD)
    {
        ULONG cb =    sizeof(SECIOCONTEXT)
                    + 10 * sizeof(HXSUBJECT);
        if (!(pContext = malloc(cb)))
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            pContext->pid = pid;
            pContext->cSubjects = 10;

            if (arc = SecIOCtl(XWPSECIO_QUERYCONTEXT,
                               pContext,
                               cb))
                switch (arc & 0xFF)
                {

                    case ERROR_I24_BAD_UNIT:
                        arc = XWPSEC_INVALID_PID;
                    break;

                    case ERROR_I24_BAD_LENGTH:
                        arc = ERROR_NOT_ENOUGH_MEMORY; // @@todo we should reallocate!
                    break;

                    default:
                        arc = ERROR_INVALID_PARAMETER;
                }
            else
            {
                pcSubjects = &pContext->cSubjects;
                paSubjects = pContext->aSubjects;
            }

            _Pmpf(("got %d from XWPSECIO_GETCONTEXT for pid 0x%lX (hdl 0/%d is 0x%lX)",
                   arc,
                   pContext->pid,
                   pContext->cSubjects,
                   pContext->aSubjects[0]));
        }
    }
    else
    {
        // driver not running: use local user then
        XWPSECID        uid;
        if (    (!(arc = slogQueryLocalUser(&uid)))
             && (!(arc = slogQueryLogon(uid,
                                        &pLogon)))
           )
        {
            pcSubjects = &pLogon->cSubjects;
            paSubjects = pLogon->aSubjects;
        }
    }

    if (!arc)
    {
        PXWPSECURITYCONTEXTCORE pReturn;
        ULONG cb =   sizeof(XWPSECURITYCONTEXTCORE)
                   + ((*pcSubjects)
                        ? (*pcSubjects - 1) * sizeof(HXSUBJECT)
                        : 0);
        if (!(pReturn = (PXWPSECURITYCONTEXTCORE)malloc(cb)))
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            pReturn->cSubjects = *pcSubjects;
            memcpy(pReturn->aSubjects,
                   paSubjects,
                   *pcSubjects * sizeof(HXSUBJECT));

            *ppContext = pReturn;
        }
    }

    if (pContext)
        free(pContext);
    if (pLogon)
        free(pLogon);

    return arc;
}

/*
 *@@ scxtSetSecurityContext:
 *
 *@@added V1.0.2 (2003-11-13) [umoeller]
 */

APIRET scxtSetSecurityContext(USHORT pid,
                              ULONG cSubjects,
                              HXSUBJECT *paSubjects)
{
    APIRET arc;
    SECIOCONTEXT    *pContext;
    ULONG cb =   sizeof(SECIOCONTEXT)
               + ((cSubjects)
                    ? (cSubjects - 1) * sizeof(HXSUBJECT)
                    : 0);
    if (!(pContext = (SECIOCONTEXT*)malloc(cb)))
        arc = ERROR_NOT_ENOUGH_MEMORY;
    else
    {
        pContext->pid = pid;
        pContext->cSubjects = cSubjects;
        memcpy(pContext->aSubjects,
               paSubjects,
               cSubjects * sizeof(HXSUBJECT));
        if (arc = SecIOCtl(XWPSECIO_SETCONTEXT,
                           pContext,
                           cb))
            switch (arc & 0xFF)
            {
                case ERROR_I24_BAD_UNIT:
                    arc = XWPSEC_INVALID_PID;
                break;

                case ERROR_I24_GEN_FAILURE:
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                break;

                default: // ERROR_I24_INVALID_PARAMETER:
                    arc = ERROR_INVALID_PARAMETER;
            }

        _Pmpf(("got %d from XWPSECIO_SETCONTEXT for pid 0x%lX (hdl 0/%d is 0x%lX)",
               arc,
               pContext->pid,
               pContext->cSubjects,
               pContext->aSubjects[0]));
    }

    return arc;
}

/*
 *@@ scxtVerifyAuthority:
 *      returns NO_ERROR only if the specified process
 *      has sufficient authority to perform the
 *      given action.
 *
 *      Otherwise this returns XWPSEC_INSUFFICIENT_AUTHORITY.
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

APIRET scxtVerifyAuthority(PXWPSECURITYCONTEXTCORE pContext,
                           ULONG flActions)
{
    if (!pContext || !flActions)
        return ERROR_INVALID_PARAMETER;

    // presently, we only allow root to do anything
    if (    (pContext->cSubjects)
         && (!pContext->aSubjects[0])       // null is root
       )
        return NO_ERROR;

    return XWPSEC_INSUFFICIENT_AUTHORITY;
}

/* ******************************************************************
 *
 *   ACL entry points
 *
 ********************************************************************/

/*
 *@@ scxtRefresh:
 *      rebuilds the system ACL table and sends it down to
 *      the driver.
 *
 *      This roughly works as follows:
 *
 *      At any time after initialization, there is a global tree
 *      of ACLDBTREENODE structs, one for each resource that has
 *      an access control list assigned to it. Each such list in
 *      turn consists of a linked list of ACLDBPERM structures,
 *      one for each user/group-with-permissions pair.
 *
 *      The ring-0 driver does not care for groups or users, but
 *      instead only checks subject handles. We must therefore
 *
 *      1)  build a list of all subject handles that currently
 *          exist (i.e. all users and groups that are currently
 *          active through logons);
 *
 *      2)  go thru the entire ACLDB tree and create a second tree
 *          for the driver, with subject handles and permissions;
 *
 *      3)  send the thing down to the driver.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

APIRET scxtRefresh(VOID)
{
    APIRET  arc = NO_ERROR;
    BOOL    fLocked;

    if (!(fLocked = LockACLs()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        PACLDBTREENODE  pNode;
        ULONG           cbTotal = sizeof(RING0BUF);
        ULONG           cACLs = 0;
        BOOL            fAllocated;

        PRING0BUF       pRing0Buf = NULL;

        // run thru this two times: once for knowing the
        // required size of the buffer, once for actually
        // writing this

        for (fAllocated = 0;
             fAllocated < 2;
             ++fAllocated)
        {
            PRESOURCEACL    pResourceACL= (PRESOURCEACL)((PBYTE)pRing0Buf + sizeof(RING0BUF));

            for (pNode = (PACLDBTREENODE)treeFirst(G_treeACLDB);
                 pNode;
                 pNode = (PACLDBTREENODE)treeNext((TREE*)pNode))
            {
                PLISTNODE   pListNode;
                ULONG       cAccessesThis = 0;
                ULONG       cbResName
                    =   pNode->usResNameLen + 1
                      + 3
                      - ((pNode->usResNameLen + 1 + 3) & 0x03);     // DWORD-alignment
                ULONG       cbResourceACL =   sizeof(RESOURCEACL)
                                            + cbResName - 1;

                PACCESS     pAccess = (PACCESS)((PBYTE)pResourceACL->szName + cbResName);
                    // warning, use this pointer only if (fAllocated)

                // set up ACCESS structs for the RESOURCEACL
                for (pListNode = lstQueryFirstNode(&pNode->llPerms);
                     pListNode;
                     pListNode = pListNode->pNext)
                {
                    PACLDBPERM  pPerm = (PACLDBPERM)pListNode->pItemData;
                    HXSUBJECT   hSubj;
                    if (!scxtFindSubject(pPerm->bType,
                                        pPerm->id,
                                        &hSubj))
                    {
                        // this user/group is currently in use as subject:
                        // then produce an ACCESS entry for it
                        if (fAllocated)
                        {
                            pAccess->hSubject = hSubj;
                            pAccess->flAccess = pPerm->fbPerm;
                            ++pAccess;
                        }

                        ++cAccessesThis;
                        cbResourceACL += sizeof(ACCESS);
                    }
                }

                if (!cAccessesThis)
                {
                    // if we found no subjects for this resource,
                    // write out one blocker ACCESS struct with
                    // the -1 subject
                    if (fAllocated)
                    {
                        pAccess->hSubject = -1;
                        pAccess->flAccess = 0;
                    }

                    ++cAccessesThis;
                    cbResourceACL += sizeof(ACCESS);
                }

                if (fAllocated)
                {
                    pResourceACL->cbStruct = cbResourceACL;
                    pResourceACL->cAccesses = cAccessesThis;
                    pResourceACL->cbName = cbResName;       // includes padding
                    memcpy(pResourceACL->szName,
                           pNode->szResName,
                           pNode->usResNameLen + 1);

                    _Pmpf(("built %d ACCESS structs for resource \"%s\" (%d bytes in res)",
                           pResourceACL->cAccesses,
                           pResourceACL->szName,
                           cbResourceACL));
                }

                cbTotal += cbResourceACL;
                pResourceACL = (PRESOURCEACL)((PBYTE)pResourceACL + cbResourceACL);
                ++cACLs;
            }

            if (!fAllocated)
            {
                // end of first loop:
                if (!(arc = DosAllocMem((VOID**)&pRing0Buf,
                                        cbTotal,
                                        PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE)))
                {
                    pRing0Buf->cbTotal = cbTotal;
                    pRing0Buf->cACLs = cACLs;
                }
                else
                {
                    pRing0Buf = 0;
                    break;
                }
            }
        }

        // free previous ACL table, if any
        if (G_pRing0Buf)
            DosFreeMem(G_pRing0Buf);
        G_pRing0Buf = pRing0Buf;

        if (!arc)
        {
            // send the pack down to ring 0
            switch (0xFF & SecIOCtl(XWPSECIO_SENDACLS,
                                    G_pRing0Buf,
                                    G_pRing0Buf->cbTotal))
            {
                case NO_ERROR:
                break;

                case XWPERR_I24_NOT_ENOUGH_MEMORY:
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                break;

                case XWPERR_I24_INVALID_ACL_FORMAT:
                    arc = XWPSEC_DB_ACL_INTEGRITY;
                break;

                default:
                    arc = XWPSEC_INTEGRITY;
            }

            doshWriteLogEntry(G_LogFile,
                              "XWPSECIO_SENDACLS yielded rc %d",
                              arc);
        }
    }

    if (G_pRing0Buf)
    {
        PSZ psz;
        if (psz = strhCreateDump((PBYTE)G_pRing0Buf,
                                 G_pRing0Buf->cbTotal,
                                 4))
        {
            ULONG len = strlen(psz);
            doshWrite(G_LogFile,
                      len + 1,
                      psz);
            free(psz);
        }
    }

    if (fLocked)
        UnlockACLs();

    return arc;
}

/*
 *@@ FindAccess:
 *
 *      Preconditions:
 *
 *      --  Caller must have checked that G_pbSysACLs is not NULL.
 *
 *      --  Caller must hold ACLDB mutex.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

PACCESS FindAccess(PCSZ pcszResource,
                   PULONG pcAccesses)       // out: array item count
{
    PBYTE   pbCurrent = (PBYTE)G_pRing0Buf + sizeof(RING0BUF);
    ULONG   ul;

    for (ul = 0;
         ul < G_pRing0Buf->cACLs;
         ++ul)
    {
        PRESOURCEACL pEntry = (PRESOURCEACL)pbCurrent;
        _Pmpf(("    cmp \"%s\"", pEntry->szName));
        if (!stricmp(pcszResource,
                     pEntry->szName))
        {
            *pcAccesses = pEntry->cAccesses;
            return (PACCESS)((PBYTE)pEntry->szName + pEntry->cbName);
        }

        pbCurrent += pEntry->cbStruct;
    }

    return NULL;
}

/*
 *@@ QueryPermissions:
 *      returns the ORed XWPACCESS_* flags for the
 *      given resource based on the security context
 *      represented by the given subject handles.
 *
 *      Preconditions:
 *
 *      --  Caller must hold ACLDB mutex.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

ULONG QueryPermissions(PSZ pszResource,             // in: buf with res name (trashed!)
                       ULONG cSubjects,
                       const HXSUBJECT *paSubjects)
{
    PACCESS paAccesses;
    ULONG   cAccesses;

    _Pmpf(("checking permissions for \"%s\":", pszResource));

    if (!*paSubjects || !G_pRing0Buf)
        // null subject handle: root may do anything
        return XWPACCESS_ALL;

    while (TRUE)
    {
        PSZ p;

        if (paAccesses = FindAccess(pszResource,
                                    &cAccesses))
        {
            ULONG   flAccess = 0;
            ULONG   ulA,
                    ulS;

            _Pmpf(("found %d ACCESS structs for \"%s\" @ofs 0x%lX:",
                   cAccesses,
                   pszResource,
                   (ULONG)paAccesses - (ULONG)G_pRing0Buf));

            for (ulA = 0;
                 ulA < cAccesses;
                 ++ulA)
            {
                for (ulS = 0;
                     ulS < cSubjects;
                     ++ulS)
                {
                    if (paSubjects[ulS] == paAccesses[ulA].hSubject)
                    {
                        _Pmpf(("  acc[%d]: hSubj matches 0x%lX: ORing flAccess 0x%lX",
                                ulA, paSubjects[ulS], paAccesses[ulA].flAccess));
                        flAccess |= paAccesses[ulA].flAccess;
                    }
                    else
                        _Pmpf(("  acc[%d]: hSubj 0x%lX != hSubj 0x%lX (flAccess 0x%lX)",
                                ulA, paAccesses[ulA].hSubject,
                                paSubjects[ulS], paAccesses[ulA].flAccess));
                }
            }

            return flAccess;
        }

        // not found: climb up to parent
        if (p = strrchr(pszResource, '\\'))
            *p = 0;
        else
            break;
    }

    return 0;
}

/*
 *@@ scxtQueryPermissions:
 *
 *      Returns one of:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

APIRET scxtQueryPermissions(PCSZ pcszResource,          // in: resource name
                            ULONG cSubjects,            // in: no. of subjects in paSubjects array
                            const HXSUBJECT *paSubjects,    // in: array of subject handles
                            PULONG pulAccess)           // out: XWPACCESS_* flags
{
    APIRET  arc = NO_ERROR;
    BOOL    fLocked;

    if (!(fLocked = LockACLs()))
        arc = XWPSEC_CANNOT_GET_MUTEX;
    else
    {
        PSZ p;
        if (!(p = strdup(pcszResource)))
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            *pulAccess = QueryPermissions(p,
                                          cSubjects,
                                          paSubjects);
            free(p);
        }
    }

    if (fLocked)
        UnlockACLs();

    return arc;
}


