
/*
 * xpstat.c:
 *      this is the main (and only) .C file for xpstat.exe.
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

#define  INCL_WIN
#define  INCL_WINWORKPLACE
#define  INCL_DOS
#define  INCL_DOSERRORS
#include <os2.h>
#include <stdio.h>

#include "setup.h"
#include "helpers\pmprintf.h"

#include "helpers\comctl.h"
#include "helpers\cnrh.h"
#include "helpers\dosh.h"
#include "helpers\exeh.h"
#include "helpers\gpih.h"
#include "helpers\linklist.h"
#include "helpers\memdebug.h"
#include "helpers\procstat.h"
#include "helpers\dosh.h"
#include "helpers\stringh.h"
#include "helpers\textview.h"           // PM text view control
#include "helpers\winh.h"
#include "helpers\xwpsecty.h"
#include "helpers\xstring.h"

#include "dlgids.h"

#include "xpstat.h"

                                /*

                                if (fl & PAG_COMMIT)
                                    strcat(szInfo, "commit ");
                                if (fl & PAG_SHARED)
                                    strcat(szInfo, "shared ");
                                if (fl & PAG_FREE)
                                    strcat(szInfo, "free ");
                                if (fl & PAG_BASE)
                                    strcat(szInfo, "base ");
                                if (fl & PAG_READ)
                                    strcat(szInfo, "read ");
                                if (fl & PAG_WRITE)
                                    strcat(szInfo, "write ");
                                if (fl & PAG_EXECUTE)
                                    strcat(szInfo, "exec ");
                                if (fl & PAG_GUARD)
                                    strcat(szInfo, "guard ");
*/

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

HAB         G_hab;
HMQ         G_hmq;

HWND        G_hwndMain = NULLHANDLE;

HMODULE     G_hmodNLS = NULLHANDLE;

HWND        G_hwndProcListCnr = NULLHANDLE,
            G_hwndProcView = NULLHANDLE,
            G_hwndSplit = NULLHANDLE,
            G_hMainMenu = NULLHANDLE;

const char  *pcszClientClass = "ProcInfoClient";

const char  *INIAPP                 = "XWorkplace";
const char  *INIKEY_MAINWINPOS      = "WndPosXpstat";

ULONG       G_ulCurrentView = ID_XPSMI_PIDTREE;

PQTOPLEVEL32 G_pInfo = NULL;

PRECORDCORE G_precSelected = NULL;

BOOL        G_fWordWrap = FALSE;

BOOL        G_fXWPSec = FALSE;

QPROCESS32  SysInitProcess =
                {
                    1,      // rectype
                    NULL,   // pThreads
                    1,      // PID
                    0,      // PPID
                    0,      // FS
                    0,      // state
                    0,      // SID
                    NULLHANDLE, // hmod
                    0,      // thread count
                    0,      // ulPrivSem32Count;
                    0,      // _reserved2_;    // 0 always
                    0,      // usSem16Count;   // count of 16-bit semaphores in pausSem16 array
                    0,      // usModuleCount;  // count of DLLs owned by this process
                    0,      // usShrMemCount;  // count of shared memory items
                    0,      // usFdsCount;     // count of open files; this is mostly way too large
                    0,      // pausSem16;      // ptr to array of 16-bit semaphore handles;
                    0,      // pausModules;    // ptr to array of modules;
                    0,      // pausShrMems;    // ptr to array of shared mem items;
                    0,      // pausFds;        // ptr to array of file handles;
                };

/* ******************************************************************
 *
 *   Miscellaneous
 *
 ********************************************************************/

/*
 *@@ fnComparePID:
 *      compare process IDs. Works with PROCRECORD's only.
 */

SHORT EXPENTRY fnComparePID(PPROCRECORD p1, PPROCRECORD p2, PVOID pStorage)
{
    if (p1->pProcess && p2->pProcess)
    {
        if (p1->pProcess->usPID < p2->pProcess->usPID)
            return (-1);
        else if (p1->pProcess->usPID > p2->pProcess->usPID)
            return (1);
    }

    return (0);
}

/*
 *@@ fnCompareSID:
 *      compare session IDs. Works with PROCRECORD's only.
 */

SHORT EXPENTRY fnCompareSID(PPROCRECORD p1, PPROCRECORD p2, PVOID pStorage)
{
    if (p1->pProcess && p2->pProcess)
    {
        if (p1->pProcess->ulScreenGroupID < p2->pProcess->ulScreenGroupID)
            return (-1);
        else if (p1->pProcess->ulScreenGroupID > p2->pProcess->ulScreenGroupID)
            return (1);
    }

    return (0);
}

/*
 *@@ fnComparePSZ:
 *      compare record titles. Works with all RECORDCORE's.
 */

SHORT EXPENTRY fnComparePSZ(PRECORDCORE p1, PRECORDCORE p2, PVOID pStorage)
{
    if (p1->pszIcon && p2->pszIcon)
    {
        switch (WinCompareStrings(G_hab,
                                  0, 0,
                                  p1->pszIcon,
                                  p2->pszIcon,
                                  0))
        {
            case WCS_LT: return (-1);
            case WCS_GT: return (1);
        }
    }

    return (0);
}

/*
 *@@ SetupWindows:
 *
 */

VOID SetupWindows(HWND hwndClient)
{
    SPLITBARCDATA sbcd;
    XTEXTVIEWCDATA xtxvCData;
    XFMTPARAGRAPH fmtp;
    G_hwndProcListCnr = WinCreateWindow(hwndClient,
                                        WC_CONTAINER,
                                        "",
                                        WS_VISIBLE,
                                        0, 0, 100, 100,
                                        hwndClient,
                                        HWND_TOP,
                                        ID_PROCLISTCNR,
                                        0,
                                        0);

    memset(&xtxvCData, 0, sizeof(xtxvCData));
    xtxvCData.cbData = sizeof(xtxvCData);
    xtxvCData.ulXBorder = 20;
    xtxvCData.ulYBorder = 20;
    G_hwndProcView = WinCreateWindow(hwndClient,
                                     WC_XTEXTVIEW,
                                     "",
                                     WS_VISIBLE | XS_HSCROLL | XS_VSCROLL
                                        | XS_AUTOHHIDE | XS_AUTOVHIDE,
                                     0, 0, 100, 100,
                                     hwndClient,
                                     HWND_TOP,
                                     ID_PROCINFO,
                                     &xtxvCData,
                                     0);

    winhSetWindowFont(G_hwndProcView, "10.System VIO");

    // change default format
    WinSendMsg(G_hwndProcView,
               TXM_QUERYPARFORMAT,
               (MPARAM)0,       // default format
               (MPARAM)&fmtp);
    fmtp.fWordWrap = FALSE;
    fmtp.lLeftMargin = 0;
    fmtp.lFirstLineMargin = 5;
    WinSendMsg(G_hwndProcView,
               TXM_SETPARFORMAT,
               (MPARAM)0,       // default format
               (MPARAM)&fmtp);

    winhSetWindowFont(G_hwndProcListCnr, "9.WarpSans");

    sbcd.ulSplitWindowID = ID_PROCSPLIT;
    sbcd.ulCreateFlags = SBCF_VERTICAL | SBCF_PERCENTAGE | SBCF_3DSUNK | SBCF_MOVEABLE;
    sbcd.lPos = 50;
    sbcd.ulLeftOrBottomLimit = 100;
    sbcd.ulRightOrTopLimit = 100;
    sbcd.hwndParentAndOwner = hwndClient;
    G_hwndSplit = ctlCreateSplitWindow(G_hab,
                                       &sbcd);

                     // fnwpProcInfoClient,
    WinSendMsg(G_hwndSplit,
               SPLM_SETLINKS,
               (MPARAM)G_hwndProcListCnr,
               (MPARAM)G_hwndProcView);

    WinSetFocus(HWND_DESKTOP, G_hwndProcListCnr);
}

/*
 *@@ SetupMenu:
 *
 */

VOID SetupMenu(VOID)
{
    WinCheckMenuItem(G_hMainMenu,
                     G_ulCurrentView,
                     TRUE);
    WinCheckMenuItem(G_hMainMenu,
                     ID_XPSMI_WORDWRAP,
                     G_fWordWrap);
}

/*
 *@@ AppendModuleInfo:
 *
 */

VOID AppendModuleInfo(PXSTRING ppszCurrentInfo,
                      PSZ pszModuleName)
{
    CHAR            szTemp[2000];
    PSZ             pszTemp = szTemp;
    PEXECUTABLE     pExec = NULL;
    PSZ             pszExeFormat = "unknown",
                    psz32Bits = "unknown",
                    pszVersion = "not available",
                    pszVendor = "not available",
                    pszDescr = "not available";
    // get module info
    if (exehOpen(pszModuleName,
                     &pExec)
            == NO_ERROR)
    {
        switch (pExec->ulExeFormat)
        {
            case EXEFORMAT_OLDDOS:
                pszExeFormat = "Old DOS";
            break;
            case EXEFORMAT_NE:
                pszExeFormat = "New Executable (NE)";
            break;
            case EXEFORMAT_PE:
                pszExeFormat = "Portable Executable (PE)";
            break;
            case EXEFORMAT_LX:
                pszExeFormat = "Linear Executable (LX)";
            break;
            case EXEFORMAT_TEXT_BATCH:
                pszExeFormat = "Text batch file";
            break;
            case EXEFORMAT_TEXT_CMD:
                pszExeFormat = "Text command file";
            break;
        }

        if (pExec->f32Bits)
            psz32Bits = "yes";
        else
            psz32Bits = "no";

        if (exehQueryBldLevel(pExec) == NO_ERROR)
        {
            if (pExec->pszVendor)
                pszVendor = pExec->pszVendor;
            if (pExec->pszVersion)
                pszVersion = pExec->pszVersion;
            if (pExec->pszInfo)
                pszDescr = pExec->pszInfo;
        }
    }

    pszTemp = szTemp;
    pszTemp += sprintf(pszTemp, "Module format: %s\n", pszExeFormat);
    pszTemp += sprintf(pszTemp, "32-bit module: %s\n", psz32Bits);
    pszTemp += sprintf(pszTemp, "Vendor: %s\n", pszVendor);
    pszTemp += sprintf(pszTemp, "Version: %s\n", pszVersion);
    xstrcat(ppszCurrentInfo, szTemp, 0);
    pszTemp = szTemp;
    pszTemp += sprintf(pszTemp, "Description: %s\n", pszDescr);
    xstrcat(ppszCurrentInfo, szTemp, 0);

    if (pExec)
        exehClose(&pExec);
}

/* ******************************************************************
 *
 *   "Process list" mode
 *
 ********************************************************************/

/*
 *@@ InsertProcessList:
 *      clears the container and inserts all processes by PID.
 *      Gets called by RefreshView().
 */

VOID InsertProcessList(HWND hwndCnr,
                       BOOL fSortBySID)     // in: else PID
{
    APIRET arc = NO_ERROR;
    XFIELDINFO    axfi[2];
    ULONG         i = 0;

    WinSendMsg(hwndCnr,
               CM_REMOVERECORD,
               (MPARAM)NULL,
               MPFROM2SHORT(0,  // all records
                            CMA_FREE | CMA_INVALIDATE));

    BEGIN_CNRINFO()
    {
        cnrhSetView(CV_DETAIL | CA_DETAILSVIEWTITLES);

        if (fSortBySID)
        {
            cnrhSetSortFunc(fnCompareSID);
        }
        else
            cnrhSetSortFunc(fnComparePID);
    } END_CNRINFO(G_hwndProcListCnr);

    // set up cnr details view
    i = 0;
    axfi[i].ulFieldOffset = FIELDOFFSET(PROCRECORD, pszPID);
    if (fSortBySID)
        axfi[i].pszColumnTitle = "SID";
    else
        axfi[i].pszColumnTitle = "PID";
    axfi[i].ulDataType = CFA_STRING;
    axfi[i++].ulOrientation = CFA_LEFT;

    axfi[i].ulFieldOffset = FIELDOFFSET(PROCRECORD, pszModuleName);
    axfi[i].pszColumnTitle = "Process";
    axfi[i].ulDataType = CFA_STRING;
    axfi[i++].ulOrientation = CFA_LEFT;

    cnrhClearFieldInfos(G_hwndProcListCnr, FALSE);  // no invalidate
    cnrhSetFieldInfos(G_hwndProcListCnr,
                      axfi,
                      i,
                      TRUE,
                      0);

    if (G_pInfo)
        prc32FreeInfo(G_pInfo);

    if (G_pInfo = prc32GetInfo(&arc))
    {
        PQPROCESS32 pProcess = G_pInfo->pProcessData;
        PPROCRECORD precFirst,
                    precThis;
        ULONG cProcesses = 0;

        pProcess = G_pInfo->pProcessData;
        while ((pProcess) && (pProcess->ulRecType == 1))
        {
            PQTHREAD32 pThread = pProcess->pThreads;
            for (i = 0;
                 i < pProcess->usThreadCount;
                 i++, pThread++)
                ;
            pProcess = (PQPROCESS32)pThread;
            cProcesses++;
        }

        // insert records
        precFirst = (PPROCRECORD)cnrhAllocRecords(hwndCnr,
                                                  sizeof(PROCRECORD),
                                                  cProcesses);

        pProcess = G_pInfo->pProcessData;
        precThis = precFirst;
        while ((pProcess) && (pProcess->ulRecType == 1))
        {
            PQTHREAD32 pThread = pProcess->pThreads;
            for (i = 0;
                 i < pProcess->usThreadCount;
                 i++, pThread++)
                ;

            DosQueryModuleName(pProcess->usHModule,
                               sizeof(precThis->szModuleName),
                               precThis->szModuleName);

            if (fSortBySID)
                sprintf(precThis->szPID, "0x%04lX", pProcess->ulScreenGroupID);
            else
                sprintf(precThis->szPID, "0x%04lX", pProcess->usPID);
            precThis->pszPID = precThis->szPID;
            sprintf(precThis->szTitle, "%s: %s",
                    precThis->szPID,
                    precThis->szModuleName);
            precThis->pszModuleName = precThis->szModuleName;

            precThis->recc.pszIcon
                = precThis->recc.pszTree
                = precThis->recc.pszName
                = precThis->recc.pszText
                = precThis->szTitle;
            precThis->pProcess = pProcess;


            pProcess = (PQPROCESS32)pThread;
            precThis = (PPROCRECORD)precThis->recc.preccNextRecord;
        }

        cnrhInsertRecords(hwndCnr,
                          NULL,         // parent recc
                          (PRECORDCORE)precFirst,
                          TRUE,
                          NULL,
                          CRA_RECORDREADONLY,
                          cProcesses);

    }
}

/*
 *@@ DescribeSem32:
 *
 *@@added V0.9.10 (2001-04-08) [umoeller]
 */

VOID DescribeSem32(PSZ pszTemp,
                   PQS32SEM32 pSem32)
{
    pszTemp += sprintf(pszTemp,
                       "  %06lX  %8lX %04d",
                       pSem32->ulHandle,
                       pSem32->ulBlockID,
                       pSem32->fl);
    if (pSem32->fl & QS32_DC_SEM_SHARED)
        pszTemp += sprintf(pszTemp, " shared");
    /* if (pSem32->fl & QS32_DC_SEM_PM)
        pszTemp += sprintf(pszTemp, " PM"); */

    if (pSem32->fl & QS32_DCM_MUTEX_SEM)
        pszTemp += sprintf(pszTemp, " mutex");
    if (pSem32->fl & QS32_DCE_EVENT_SEM)
        pszTemp += sprintf(pszTemp, " event");
    if (pSem32->fl & QS32_DCMW_MUX_SEM)
        pszTemp += sprintf(pszTemp, " muxwait");

    if (pSem32->fl & QS32_DCE_POSTONE)
        pszTemp += sprintf(pszTemp, ", post-one");
    if (pSem32->fl & QS32_DCE_AUTORESET)
        pszTemp += sprintf(pszTemp, ", auto-reset");

    if (pSem32->fl & QS32_DCMW_WAIT_ANY)
        pszTemp += sprintf(pszTemp, ", waitAny");
    if (pSem32->fl & QS32_DCMW_WAIT_ALL)
        pszTemp += sprintf(pszTemp, ", waitAll");
    if (pSem32->fl & QS32_DE_POSTED)
        pszTemp += sprintf(pszTemp, ", %dx posted ", pSem32->usPostCount);
    if (pSem32->fl & QS32_DM_OWNER_DIED)
        pszTemp += sprintf(pszTemp, ", owner died");
    if (pSem32->fl & QS32_DMW_MTX_MUX)
        pszTemp += sprintf(pszTemp, ", contains mutexes");
    if (pSem32->fl & QS32_DE_16BIT_MW)
        pszTemp += sprintf(pszTemp, ", part of 16-bit muxwait");

    if (pSem32->fl & QS32_DHO_SEM_OPEN)
        pszTemp += sprintf(pszTemp, ", opened by device driver");

    if (pSem32->pszName)
        pszTemp += sprintf(pszTemp,
                           "\n                        name: \"%s\"",
                           pSem32->pszName);
    if (pSem32->pvDeviceDriver)
        pszTemp += sprintf(pszTemp,
                           "\n                        ptr: %08lX",
                           pSem32->pvDeviceDriver);

    /* if (pSem32->us_)
        pszTemp += sprintf(pszTemp,
                           "\n                        strange us_: %04lX",
                           pSem32->us_);
    if (pSem32->ulElse)
        pszTemp += sprintf(pszTemp,
                           "\n                        else: %08lX",
                           pSem32->ulElse);
    if (pSem32->ulElse2)
        pszTemp += sprintf(pszTemp,
                           "\n                        else2: %08lX",
                           pSem32->ulElse2); */
    strcpy(pszTemp, "\n");
}

/*
 *@@ ProcessSelected:
 *      gets called when a new process gets selected
 *      to compose the process information string
 *      displayed on the right.
 */

VOID ProcessSelected(VOID)
{
    XSTRING strCurrentInfo;
    xstrInit(&strCurrentInfo, 1000);

    strCurrentInfo.ulDelta = 1000;

    if (G_precSelected)
    {
        PQPROCESS32 pProcess = ((PPROCRECORD)G_precSelected)->pProcess;
        PQTHREAD32  pThread;
        PSZ         pszSessionType = "unknown";
        CHAR        szTemp[2000],
                    szTemp2[200] = "";
        PSZ         pszTemp = szTemp;
        ULONG       i,
                    cMsgQueues = 0;

        pszTemp += sprintf(pszTemp, "PID: 0x%04lX\n", pProcess->usPID);
        pszTemp += sprintf(pszTemp, "Parent PID: 0x%04lX\n", pProcess->usPPID);
        pszTemp += sprintf(pszTemp, "Screen Group ID: 0x%04lX\n", pProcess->ulScreenGroupID);
        xstrcpy(&strCurrentInfo, szTemp, 0);

        if (G_fXWPSec)
        {
            XWPSECID uidOwner;
            APIRET  arc;
            CHAR    szOwnerName[XWPSEC_NAMELEN];
            if (!(arc = xsecQueryProcessOwner(pProcess->usPID, &uidOwner)))
                if (!(arc = xsecQueryUserName(uidOwner, szOwnerName)))
                    sprintf(szTemp, "\nOwning User: %s (ID %d)\n", szOwnerName, uidOwner);
                else
                    sprintf(szTemp, "\nOwning User ID: %d\n", uidOwner);
            else
                sprintf(szTemp, "\nError %d querying owning user ID\n", arc);
        }
        else
            sprintf(szTemp, "\nMulti-user information not available\n");
        xstrcat(&strCurrentInfo, szTemp, 0);

        sprintf(szTemp, "\nModule: %s\n", ((PPROCRECORD)G_precSelected)->szModuleName);
        xstrcat(&strCurrentInfo, szTemp, 0);

        if (pProcess->usPID == 1)
            // sysinit:
            xstrcat(&strCurrentInfo, "\nKernel pseudo-process.\n", 0);
        else
        {
            // regular process:
            AppendModuleInfo(&strCurrentInfo,
                             ((PPROCRECORD)G_precSelected)->szModuleName);
            // get process type
            switch (pProcess->ulProgType)
            {
                case 0:
                    pszSessionType = "Full screen protected mode";
                break;
                case 1:
                    pszSessionType = "Real mode (probably VDM)";
                break;
                case 2:
                    pszSessionType = "VIO windowable protected mode";
                break;
                case 3:
                    pszSessionType = "Presentation manager protected mode";
                break;
                case 4:
                    pszSessionType = "Detached protected mode";
                break;
            }

            pszTemp = szTemp;
            pszTemp += sprintf(pszTemp, "\nProcess type: %d (%s)\n",
                               pProcess->ulProgType,
                               pszSessionType);

            if (pProcess->ulState & STAT_EXITLIST)
                strcpy(szTemp2, "[processing exit list] ");
            if (pProcess->ulState & STAT_EXIT1)
                strcat(szTemp2, "[exiting thread 1] ");
            if (pProcess->ulState & STAT_EXITALL)
                strcat(szTemp2, "[exiting all] ");
            if (pProcess->ulState & STAT_PARSTAT)
                strcat(szTemp2, "[notify parent on exit] ");
            if (pProcess->ulState & STAT_SYNCH)
                strcat(szTemp2, "[parent waiting on exit] ");
            if (pProcess->ulState & STAT_DYING)
                strcat(szTemp2, "[dying] ");
            if (pProcess->ulState & STAT_EMBRYO)
                strcat(szTemp2, "[embryo] ");

            pszTemp += sprintf(pszTemp, "\nulState: 0x%lX %s\n",
                               pProcess->ulState,
                               szTemp2);

            // threads

            pszTemp += sprintf(pszTemp, "\nThreads: %d\n", pProcess->usThreadCount);
            // header for the following
            pszTemp += sprintf(pszTemp, "  TID Slot SleepID  Prty State HMQ==PMQ\n");
            xstrcat(&strCurrentInfo, szTemp, 0);

            // dump threads
            pThread = pProcess->pThreads;
            for (i = 0;
                 i < pProcess->usThreadCount;
                 i++, pThread++)
            {
                CHAR    szState[30] = "block";
                HAB     habhmq = 0;
                HMQ     hmq = 0;

                switch(pThread->ucState)
                {
                    case 1: strcpy(szState, "ready"); break;
                    case 5: strcpy(szState, "runng"); break;
                    case 9: strcpy(szState, "loadd"); break;
                }

                pszTemp = szTemp;
                pszTemp += sprintf(pszTemp,
                                   "  %02d  %04lX %08lX %04lX %s ",
                                   pThread->usTID,
                                   pThread->usSlotID,
                                   pThread->ulSleepID,
                                   pThread->ulPriority,
                                   szState);
                if (hmq = winhFindMsgQueue(pProcess->usPID,
                                           pThread->usTID,
                                           &habhmq))
                {
                    pszTemp += sprintf(pszTemp, "0x%lX", hmq);
                    cMsgQueues++;
                }

                pszTemp += sprintf(pszTemp, "\n");
                xstrcat(&strCurrentInfo, szTemp, 0);
            }

            // dump message queues
            if (cMsgQueues)
            {
                HENUM   henum = WinBeginEnumWindows(HWND_OBJECT);
                HWND    hwndThis;
                pszTemp = szTemp;
                pszTemp += sprintf(pszTemp, "\nMessage queues: %d\n", cMsgQueues);
                // header for the following
                pszTemp += sprintf(pszTemp, "  PMQ        HWND       TID size\n");
                xstrcat(&strCurrentInfo, szTemp, 0);

                while (hwndThis = WinGetNextWindow(henum))
                {
                    CHAR    szClass[200];
                    if (WinQueryClassName(hwndThis, sizeof(szClass), szClass))
                    {
                        if (strcmp(szClass, "#32767") == 0)
                        {
                            // is message queue:
                            PID pidWin = 0;
                            TID tidWin = 0;
                            WinQueryWindowProcess(hwndThis,
                                                  &pidWin,
                                                  &tidWin);
                            if (pidWin == pProcess->usPID)
                            {
                                // is our process:
                                // get message queue for this
                                HMQ     hmq = WinQueryWindowULong(hwndThis, QWL_HMQ);
                                MQINFO  mqi = {0};
                                ULONG   cb, fl;

                                WinQueryQueueInfo(hmq, &mqi, sizeof(mqi));

                                pszTemp = szTemp;
                                pszTemp += sprintf(pszTemp, "  0x%08lX 0x%lX %02d  %d",
                                                   hmq, hwndThis, tidWin, mqi.cmsgs);

                                pszTemp += sprintf(pszTemp, "\n");
                                xstrcat(&strCurrentInfo, szTemp, 0);
                            }
                        }
                    }
                }
                WinEndEnumWindows(henum);
            }

            // dump public 32-bit semaphores
            sprintf(szTemp, "\nPublic 32-bit semaphores:\n");
            xstrcat(&strCurrentInfo, szTemp, 0);
            if (!G_pInfo->pSem32Data)
                xstrcat(&strCurrentInfo, "  cannot get data\n", 0);
            else
            {
                // semaphore data available:
                PQS32SEM32 pFirst = G_pInfo->pSem32Data,
                           pSem32 = pFirst;
                ULONG ul;

                xstrcat(&strCurrentInfo,
                        "  handle  blockid  flags           name\n", 0);

                while (pSem32)
                {
                    DescribeSem32(szTemp, pSem32);
                    xstrcat(&strCurrentInfo, szTemp, 0);

                    pSem32 = pSem32->pNext;
                }
            }

            // dump private 32-bit semaphores
            sprintf(szTemp, "\nPrivate 32-bit semaphores: %d\n", pProcess->ulPrivSem32Count);
            xstrcat(&strCurrentInfo, szTemp, 0);
            if (!pProcess->pvPrivSem32s)
                xstrcat(&strCurrentInfo, "  cannot get data\n", 0);
            else
            {
                // semaphore data available:
                PQS32SEM32 pFirst = (PQS32SEM32)pProcess->pvPrivSem32s,
                           pSem32 = pFirst;
                ULONG ul;

                xstrcat(&strCurrentInfo,
                        "  handle  blockid  flags           name\n", 0);

                for (ul = 0;
                     ul < pProcess->ulPrivSem32Count;
                     ul++)
                {
                    DescribeSem32(szTemp, pSem32);
                    xstrcat(&strCurrentInfo, szTemp, 0);

                    pSem32 = pSem32->pNext;
                }
            }

            // dump 16-bit semaphores
            sprintf(szTemp, "\n16-bit semaphores: %d\n", pProcess->usSem16Count);
            xstrcat(&strCurrentInfo, szTemp, 0);
            if (pProcess->usSem16Count)
            {
                sprintf(szTemp, "  indx ownr flag cUse cReq \n", pProcess->usSem16Count);
                xstrcat(&strCurrentInfo, szTemp, 0);
                for (i = 0;
                     i < pProcess->usSem16Count;
                     i++)
                {
                    USHORT usSemThis = pProcess->pausSem16[i];
                    PQS32SEM16 pSem16 = prc32FindSem16(G_pInfo, usSemThis);
                    pszTemp = szTemp;
                    pszTemp += sprintf(pszTemp, "  %4d", usSemThis);
                    if (pSem16)
                        pszTemp += sprintf(pszTemp,
                                           " %04lX %04lX %04lX %04lX S%s",
                                           pSem16->usSysSemOwner,
                                           pSem16->fsSysSemFlags,
                                           pSem16->usSysSemRefCnt,
                                           pSem16->usSysSemProcCnt,
                                           pSem16->szName);


                    strcat(pszTemp, "\n");
                    xstrcat(&strCurrentInfo, szTemp, 0);
                }
            }

            // dump shared memory
            sprintf(szTemp, "\nShared mem: %d references\n", pProcess->usShrMemCount);
            xstrcat(&strCurrentInfo, szTemp, 0);
            if (pProcess->usShrMemCount)
            {
                sprintf(szTemp, "  shrmID selector cRef\n");
                xstrcat(&strCurrentInfo, szTemp, 0);
                for (i = 0;
                     i < pProcess->usShrMemCount;
                     i++)
                {
                    USHORT  usShrThis = pProcess->pausShrMems[i];
                    PQSHRMEM32 pShrThis = prc32FindShrMem(G_pInfo, usShrThis);
                    pszTemp = szTemp;
                    pszTemp += sprintf(pszTemp, "  0x%04lX", usShrThis);
                    if (pShrThis)
                        pszTemp += sprintf(pszTemp, " 0x%04lX %6d %s",
                                           pShrThis->usSelector,
                                           pShrThis->usRefCount,
                                           pShrThis->acName);
                    strcat(pszTemp, "\n");
                    xstrcat(&strCurrentInfo, szTemp, 0);
                }
            }

            // dump modules
            sprintf(szTemp, "\nModule references (imports): %d\n", pProcess->usModuleCount);
            xstrcat(&strCurrentInfo, szTemp, 0);
            if (pProcess->usModuleCount)
            {
                for (i = 0;
                     i < pProcess->usModuleCount;
                     i++)
                {
                    pszTemp = szTemp;
                    pszTemp += sprintf(pszTemp, "  0x%04lX ", pProcess->pausModules[i]);
                    DosQueryModuleName(pProcess->pausModules[i],
                                       sizeof(szTemp),
                                       pszTemp);
                    strcat(pszTemp, "\n");
                    xstrcat(&strCurrentInfo, szTemp, 0);
                }
            }

            // dump open files
            sprintf(szTemp, "\nOpen files: %d\n", pProcess->usFdsCount);
            xstrcat(&strCurrentInfo, szTemp, 0);
            if (pProcess->usFdsCount)
            {
                xstrcat(&strCurrentInfo,
                         "  sfn  cOpn Flags    Accs  Size hVol attribs\n", 0);
                for (i = 0;
                     i < pProcess->usFdsCount;
                     i++)
                {
                    USHORT usFileID = pProcess->pausFds[i];

                    szTemp[0] = 0;
                    pszTemp = szTemp;

                    pszTemp += sprintf(pszTemp,
                                       "  %04lX",
                                       usFileID);

                    if (usFileID)       // rule out "0" file handles
                    {
                        PQFILEDATA32 pFile = prc32FindFileData(G_pInfo, usFileID);

                        if (pFile)      // rule out pseudo-file handles
                        {
                            CHAR    szAttribs[] = "......";
                            PSZ     pszAccess = szAttribs;
                            CHAR    szSize[20];
                            PQFDS32 pFileData = pFile->paFiles;
                            USHORT  fsAttribs = pFileData->fsAttribs;
                            *pszAccess++ = (fsAttribs & 0x20) ? 'A' : '-';
                            *pszAccess++ = (fsAttribs & 0x10) ? 'D' : '-';
                            *pszAccess++ = (fsAttribs & 0x08) ? 'L' : '-';
                            *pszAccess++ = (fsAttribs & 0x04) ? 'S' : '-';
                            *pszAccess++ = (fsAttribs & 0x02) ? 'H' : '-';
                            *pszAccess++ = (fsAttribs & 0x01) ? 'R' : '-';

                            if (pFileData->ulFileSize > (8*1024*1024))
                                sprintf(szSize, "%4dM", pFileData->ulFileSize / (8*1024*1024));
                            else if (pFileData->ulFileSize > 1024)
                                sprintf(szSize, "%4dK", pFileData->ulFileSize / 1024);
                            else
                                sprintf(szSize, "%4db", pFileData->ulFileSize);

                            pszTemp += sprintf(pszTemp,
                                               " %04d %08lx %04x %s %04lX %s ",
                                               pFile->ulCFiles,
                                               pFileData->flFlags,
                                               pFileData->flAccess,
                                               szSize,
                                               pFileData->usHVolume,
                                               szAttribs);

                            if ((pFileData->flFlags & FSF_NO_SFT_HANDLE_ALLOCTD) == 0)
                                // we do have a file handle:
                                pszTemp += sprintf(pszTemp,
                                                   "%s",
                                                   pFile->szFilename);
                            else
                                // no SFT file handle allocated:
                                strcat(szTemp, " [no SFT handle allocated]");

                        } // end if (pFile)      // rule out pseudo-file handles
                        else
                            strcat(szTemp, " [cannot find SFT entry]");

                    } // end if (usFileID)       // rule out "0" file handles

                    strcat(szTemp, "\n");
                    xstrcat(&strCurrentInfo, szTemp, 0);
                }
            }
        }
    } // end if (G_precSelected)

    xstrcat(&strCurrentInfo, "End of dump\n", 0);

    WinSetWindowText(G_hwndProcView, strCurrentInfo.psz);
    xstrClear(&strCurrentInfo);
}

/* ******************************************************************
 *
 *   "Process tree" mode
 *
 ********************************************************************/

/*
 *@@ InsertProcTreeRecord:
 *
 */

PPROCRECORD InsertProcTreeRecord(HWND hwndCnr,
                                 PPROCRECORD precParent,
                                 PQPROCESS32 pProcess)
{
    PPROCRECORD prec = (PPROCRECORD)cnrhAllocRecords(hwndCnr,
                                                     sizeof(PROCRECORD),
                                                     1);
    if (pProcess->usPID != 1)
        DosQueryModuleName(pProcess->usHModule,
                           sizeof(prec->szModuleName),
                           prec->szModuleName);
    else
        strcpy(prec->szModuleName, "[sysinit]");
    prec->pszModuleName = prec->szModuleName;

    sprintf(prec->szPID, "0x%04lX",
                pProcess->usPID);

    prec->pszPID = prec->szPID;
    sprintf(prec->szTitle, "%s: %s",
            prec->szPID,
            prec->szModuleName);

    prec->recc.pszIcon
        = prec->recc.pszTree
        = prec->recc.pszName
        = prec->recc.pszText
        = prec->szTitle;
    prec->pProcess = pProcess;

    // insert records
    cnrhInsertRecords(hwndCnr,
                      (PRECORDCORE)precParent,  // parent recc
                      (PRECORDCORE)prec,
                      FALSE,
                      NULL,
                      CRA_RECORDREADONLY | CRA_EXPANDED,
                      1);
    return (prec);
}

/*
 *@@ InsertProcessesWithParent:
 *
 */

VOID InsertProcessesWithParent(HWND hwndCnr,
                               ULONG ulParentPID,           // initially 0
                               PPROCRECORD precParent)
{
    PQPROCESS32 pProcess = G_pInfo->pProcessData;
    ULONG       i;

    /* if (ulParentPID == 0)
    {
        PPROCRECORD prec = InsertProcTreeRecord(hwndCnr,
                                                NULL,
                                                &SysInitProcess);        // "sysinit" process
        InsertProcessesWithParent(hwndCnr,
                                  1,    // pid of sysinit
                                  prec);     // preccParent
    } */

    while ( (pProcess) && (pProcess->ulRecType == 1) )
    {
        PQTHREAD32 pThread = pProcess->pThreads;
        for (i = 0;
             i < pProcess->usThreadCount;
             i++, pThread++)
            ;

        if (pProcess->usPPID == ulParentPID)
        {
            PPROCRECORD prec = InsertProcTreeRecord(hwndCnr,
                                                    precParent,
                                                    pProcess);
            // recurse for processes which have this proc as parent
            InsertProcessesWithParent(hwndCnr,
                                      pProcess->usPID,
                                      prec);     // preccParent
        }

        // next process
        pProcess = (PQPROCESS32)pThread;
    }
}

/*
 *@@ InsertProcessTree:
 *
 */

VOID InsertProcessTree(HWND hwndCnr)
{
    APIRET arc = NO_ERROR;
    XFIELDINFO    axfi[2];
    ULONG         i = 0;

    WinSendMsg(hwndCnr,
               CM_REMOVERECORD,
               (MPARAM)NULL,
               MPFROM2SHORT(0,  // all records
                            CMA_FREE | CMA_INVALIDATE));

    // clear cnr details view
    cnrhClearFieldInfos(G_hwndProcListCnr, FALSE);  // no invalidate

    BEGIN_CNRINFO()
    {
        cnrhSetView(CV_TREE | CV_TEXT | CA_TREELINE);
        cnrhSetSortFunc(fnComparePID);
        cnrhSetTreeIndent(20);
    } END_CNRINFO(G_hwndProcListCnr);

    if (G_pInfo)
    {
        prc32FreeInfo(G_pInfo);
        G_pInfo = NULL;
    }
    G_pInfo = prc32GetInfo(&arc);

    if (G_pInfo)
    {
        PQPROCESS32 pProcess = G_pInfo->pProcessData;
        PPROCRECORD precFirst,
                    precThis;
        ULONG cProcesses = 0;

        pProcess = G_pInfo->pProcessData;
        while ((pProcess) && (pProcess->ulRecType == 1))
        {
            PQTHREAD32 pThread = pProcess->pThreads;
            for (i = 0;
                 i < pProcess->usThreadCount;
                 i++, pThread++)
                ;
            pProcess = (PQPROCESS32)pThread;
            cProcesses++;
        }

        // insert records
        InsertProcessesWithParent(hwndCnr,
                                  0,
                                  NULL);     // preccParent

        cnrhInvalidateAll(hwndCnr);
    }
}

/* ******************************************************************
 *
 *   "Module tree" mode
 *
 ********************************************************************/

/*
 *@@ InsertModule2Parent:
 *
 */

PMODRECORD InsertModule2Parent(HWND hwndCnr,
                               PQMODULE32 pModule,
                               PMODRECORD precParent)
{
    PSZ     p;
    PMODRECORD prec = (PMODRECORD)cnrhAllocRecords(hwndCnr,
                                                   sizeof(MODRECORD),
                                                   1);
    DosQueryModuleName(pModule->usHModule,
                       sizeof(prec->szModuleName),
                       prec->szModuleName);
    prec->pszModuleName = prec->szModuleName;

    p = strrchr(prec->szModuleName, '\\');
    if (p)
        p++;
    else
        p = prec->szModuleName;

    prec->recc.pszIcon
        = prec->recc.pszTree
        = prec->recc.pszName
        = prec->recc.pszText
        = p;

    prec->pModule = pModule;

    // insert records
    cnrhInsertRecords(hwndCnr,
                      (PRECORDCORE)precParent,  // parent recc
                      (PRECORDCORE)prec,
                      FALSE,
                      NULL,
                      CRA_RECORDREADONLY | CRA_COLLAPSED,
                      1);

    return (prec);
}

/*
 *@@ InsertModulesTree:
 *
 */

VOID InsertModulesTree(HWND hwndCnr)
{
    APIRET arc = NO_ERROR;
    XFIELDINFO    axfi[2];
    ULONG         i = 0;

    WinSendMsg(hwndCnr,
               CM_REMOVERECORD,
               (MPARAM)NULL,
               MPFROM2SHORT(0,  // all records
                            CMA_FREE | CMA_INVALIDATE));

    // clear cnr details view
    cnrhClearFieldInfos(G_hwndProcListCnr, FALSE);  // no invalidate

    BEGIN_CNRINFO()
    {
        cnrhSetView(CV_TREE | CV_TEXT | CA_TREELINE);
        cnrhSetSortFunc(fnComparePSZ);
        cnrhSetTreeIndent(20);
    } END_CNRINFO(G_hwndProcListCnr);

    if (G_pInfo)
    {
        prc32FreeInfo(G_pInfo);
        G_pInfo = NULL;
    }
    G_pInfo = prc32GetInfo(&arc);

    if (G_pInfo)
    {
        PQMODULE32 pModule = G_pInfo->pModuleData;

        while (pModule)
        {
            // insert records; this recurses
            InsertModule2Parent(hwndCnr,
                                pModule,
                                NULL);     // preccParent
            pModule = pModule->pNext;
        }
    }
}

/*
 *@@ ModuleSelected:
 *
 */

VOID ModuleSelected(VOID)
{
    XSTRING strCurrentInfo;
    xstrInit(&strCurrentInfo, 40);

    if (G_precSelected)
    {
        PMODRECORD  precSelected = (PMODRECORD)G_precSelected;
        PQMODULE32  pModule = precSelected->pModule,
                    pModule2;
        PQPROCESS32 pProcess = G_pInfo->pProcessData;

        CHAR        szTemp[2000],
                    szTemp2[200] = "";
        PSZ         pszTemp = szTemp;

        ULONG   i,
                cProcCount = 0;

        pszTemp += sprintf(pszTemp, "Module name: %s\n", precSelected->szModuleName);
        pszTemp += sprintf(pszTemp, "Module handle: 0x%04lX\n", pModule->usHModule);
        xstrcpy(&strCurrentInfo, szTemp, 0);

        // module flags
        pszTemp = szTemp;
        pszTemp += sprintf(pszTemp, "Module flags: 0x%04lX\n", pModule->fFlat);
        pszTemp += sprintf(pszTemp, "Segments: %d\n\n", pModule->cObjects);
        xstrcat(&strCurrentInfo, szTemp, 0);

        // module info
        AppendModuleInfo(&strCurrentInfo,
                         precSelected->szModuleName);

        // find processes using this module
        pszTemp = szTemp;
        pszTemp += sprintf(pszTemp, "\nProcesses using this module directly:\n");
        xstrcat(&strCurrentInfo, szTemp, 0);

        pProcess = G_pInfo->pProcessData;
        while ((pProcess) && (pProcess->ulRecType == 1))
        {
            PQTHREAD32 pThread = pProcess->pThreads;
            if (pProcess->usModuleCount)
            {
                for (i = 0;
                     i < pProcess->usModuleCount;
                     i++)
                {
                    if (pProcess->pausModules[i] == pModule->usHModule)
                    {
                        // this proc uses this module:
                        // store process name
                        strcpy(szTemp, "    ");
                        DosQueryModuleName(pProcess->usHModule,
                                           sizeof(szTemp) - 4,
                                           szTemp + 4);
                        strcat(szTemp, "\n");
                        xstrcat(&strCurrentInfo, szTemp, 0);
                        cProcCount++;
                    }
                }
            }

            // next process
            for (i = 0;
                 i < pProcess->usThreadCount;
                 i++, pThread++)
                ;
            pProcess = (PQPROCESS32)pThread;
        }

        pszTemp = szTemp;
        if (cProcCount == 0)
            pszTemp += sprintf(pszTemp, "    none\n");
        else
            pszTemp += sprintf(pszTemp, "Total: %d processes\n", cProcCount);
        xstrcat(&strCurrentInfo, szTemp, 0);

        // other modules using this module
        pszTemp = szTemp;
        pszTemp += sprintf(pszTemp, "\nOther modules using this module:\n");
        xstrcat(&strCurrentInfo, szTemp, 0);

        cProcCount = 0;
        pModule2 = G_pInfo->pModuleData;
        while (pModule2)
        {
            if (pModule2->ulRefCount)
            {
                for (i = 0;
                     i < pModule2->ulRefCount;
                     i++)
                {
                    if (pModule2->ausModRef[i] == pModule->usHModule)
                    {
                        pszTemp = szTemp;
                        DosQueryModuleName(pModule2->usHModule,
                                           sizeof(szTemp2),
                                           szTemp2);
                        pszTemp += sprintf(pszTemp, "    0x%04lX: %s\n",
                                           pModule2->usHModule,
                                           szTemp2);
                        xstrcat(&strCurrentInfo, szTemp, 0);
                        cProcCount++;
                    }
                }
            }

            pModule2 = pModule2->pNext;
        }

        pszTemp = szTemp;
        if (cProcCount == 0)
            pszTemp += sprintf(pszTemp, "    none\n");
        else
            pszTemp += sprintf(pszTemp, "Total: %d modules\n", cProcCount);
        xstrcat(&strCurrentInfo, szTemp, 0);

        // references
        pszTemp = szTemp;
        pszTemp += sprintf(pszTemp, "\nModule references (imports, inserted into tree): %d\n", pModule->ulRefCount);
        xstrcat(&strCurrentInfo, szTemp, 0);

        if (pModule->ulRefCount)
        {
            BOOL        fMarkSelected = FALSE;
            for (i = 0;
                 i < pModule->ulRefCount;
                 i++)
            {
                pszTemp = szTemp;
                DosQueryModuleName(pModule->ausModRef[i],
                                   sizeof(szTemp2),
                                   szTemp2);
                pszTemp += sprintf(pszTemp, "    0x%04lX: %s\n",
                                   pModule->ausModRef[i],
                                   szTemp2);
                xstrcat(&strCurrentInfo, szTemp, 0);

                if (    (!precSelected->fSubModulesInserted)
                     && (pModule->ausModRef[i] != pModule->usHModule)
                   )
                {
                    PQMODULE32 pSubModule = prc32FindModule(G_pInfo,
                                                            pModule->ausModRef[i]);
                    if (pSubModule)
                        InsertModule2Parent(G_hwndProcListCnr,
                                            pSubModule,
                                            precSelected);
                    fMarkSelected = TRUE;
                }
            }

            if (fMarkSelected)
                precSelected->fSubModulesInserted = TRUE;
        }
    } // end if (G_precSelected)

    xstrcat(&strCurrentInfo, "End of dump\n", 0);

    WinSetWindowText(G_hwndProcView, strCurrentInfo.psz);
    xstrClear(&strCurrentInfo);
}

/* ******************************************************************
 *
 *   General view stuff
 *
 ********************************************************************/

/*
 *@@ RefreshView:
 *
 */

VOID RefreshView(VOID)
{
    switch (G_ulCurrentView)
    {
        case ID_XPSMI_PIDLIST:
            InsertProcessList(G_hwndProcListCnr, FALSE);
        break;

        case ID_XPSMI_SIDLIST:
            InsertProcessList(G_hwndProcListCnr, TRUE);
        break;

        case ID_XPSMI_PIDTREE:
            InsertProcessTree(G_hwndProcListCnr);
        break;

        case ID_XPSMI_MODTREE:
            InsertModulesTree(G_hwndProcListCnr);
        break;
    }
}

BOOL APIENTRY PrintCallback(ULONG ulPage,
                            ULONG ulUser)
{
    CHAR    szMsg[1000];
    sprintf(szMsg, "Page %d. Continue?", ulPage);

    if (winhYesNoBox("Printing", szMsg) == MBID_YES)
        return (TRUE);
    else
        return (FALSE);
}

/*
 *@@ fnwpProcInfoClient:
 *
 */

MRESULT EXPENTRY fnwpProcInfoClient(HWND hwndClient, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_CREATE:
            SetupWindows(hwndClient);
            RefreshView();
            mrc = (MPARAM)FALSE;        // continue;
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *
         */

        case WM_WINDOWPOSCHANGED:
        {
            // this msg is passed two SWP structs:                    WM_SIZE
            // one for the old, one for the new data
            // (from PM docs)
            PSWP pswpNew = PVOIDFROMMP(mp1);
            // PSWP pswpOld = pswpNew + 1;

            // resizing?
            if (pswpNew->fl & SWP_SIZE)
            {
                WinSetWindowPos(G_hwndSplit, HWND_TOP,
                                0, 0,
                                pswpNew->cx, pswpNew->cy, // sCXNew, sCYNew,
                                SWP_SIZE);
            }

            // return default NULL
        break; }

        case WM_CONTROL:
        {
            USHORT  usID = SHORT1FROMMP(mp1);
            USHORT  usNotifyCode = SHORT2FROMMP(mp1);

            if (usID == ID_PROCLISTCNR)
                switch (usNotifyCode)
                {
                    case CN_EMPHASIS:
                    {
                        PNOTIFYRECORDEMPHASIS pnre = (PNOTIFYRECORDEMPHASIS)mp2;

                        if (pnre->pRecord)
                            if (pnre->pRecord->flRecordAttr & CRA_SELECTED)
                            {
                                G_precSelected = pnre->pRecord;

                                switch (G_ulCurrentView)
                                {
                                    case ID_XPSMI_PIDLIST:
                                    case ID_XPSMI_SIDLIST:
                                    case ID_XPSMI_PIDTREE:
                                        ProcessSelected();
                                    break;

                                    case ID_XPSMI_MODTREE:
                                        ModuleSelected();
                                    break;
                                }
                            }
                    break; }
                }

        break; }

        case WM_COMMAND:
        {
            SHORT sCommand = SHORT1FROMMP(mp1);
            switch (sCommand)
            {
                case ID_XPSMI_EXIT:
                    WinPostMsg(WinQueryWindow(hwndClient, QW_PARENT),
                               WM_CLOSE,
                               0, 0);
                break;

                case ID_XPSMI_PIDLIST:
                case ID_XPSMI_SIDLIST:
                case ID_XPSMI_PIDTREE:
                case ID_XPSMI_MODTREE:
                    if (G_ulCurrentView != sCommand)
                    {
                        WinCheckMenuItem(G_hMainMenu,
                                         G_ulCurrentView,
                                         FALSE);
                        G_ulCurrentView = sCommand;
                        RefreshView();
                        SetupMenu();
                    }
                break;

                case ID_XPSMI_REFRESH:
                    RefreshView(); // InsertProcessList(G_hwndProcListCnr);
                break;

                case ID_XPSMI_WORDWRAP:
                    G_fWordWrap = !G_fWordWrap;
                    SetupMenu();
                    WinSendMsg(G_hwndProcView,
                               TXM_SETWORDWRAP,
                               (MPARAM)G_fWordWrap,
                               0);
                break;
            }
        break; }

        case WM_CHAR:
        {
            USHORT usFlags    = SHORT1FROMMP(mp1);
            USHORT usch       = SHORT1FROMMP(mp2);
            USHORT usvk       = SHORT2FROMMP(mp2);

            if (    (usFlags & KC_VIRTUALKEY)
                 && (usvk == VK_TAB)
               )
            {
                if ((usFlags & KC_KEYUP) == 0)
                    if (WinQueryFocus(HWND_DESKTOP) == G_hwndProcListCnr)
                        WinSetFocus(HWND_DESKTOP, G_hwndProcView);
                    else
                        WinSetFocus(HWND_DESKTOP, G_hwndProcListCnr);
            }
            else
            {
                if (    (usch == 'p')
                     && (usFlags & KC_CTRL)
                   )
                {
                    CHAR szRet[100];
                    DosBeep(1000, 100);
                    sprintf(szRet, "return code: %d",
                            txvPrintWindow(G_hwndProcView,
                                           "Process dump",
                                           PrintCallback));

                    winhDebugBox(0, "print", szRet);
                }
            }
            mrc = (MPARAM)TRUE;
        break; }

        case WM_CLOSE:
            winhSaveWindowPos(G_hwndMain,
                              HINI_USER,
                              INIAPP,
                              INIKEY_MAINWINPOS);
            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
        break;

        default:
            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
    }

    return (mrc);
}

/*
 *@@ LoadNLS:
 *      load NLS interface.
 *
 */

BOOL LoadNLS(VOID)
{
    CHAR        szNLSDLL[2*CCHMAXPATH];
    BOOL Proceed = TRUE;

    if (PrfQueryProfileString(HINI_USER,
                              "XWorkplace",
                              "XFolderPath",
                              "",
                              szNLSDLL, sizeof(szNLSDLL)) < 3)

    {
        WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                      "xpstat was unable to determine the location of the "
                      "XWorkplace National Language Support DLL, which is "
                      "required for operation. The OS2.INI file does not contain "
                      "this information. "
                      "xpstat cannot proceed. Please re-install XWorkplace.",
                      "xpstat: Fatal Error",
                      0, MB_OK | MB_MOVEABLE);
        Proceed = FALSE;
    }
    else
    {
        CHAR    szLanguageCode[50] = "";

        // now compose module name from language code
        PrfQueryProfileString(HINI_USERPROFILE,
                              "XWorkplace", "Language",
                              "001",
                              (PVOID)szLanguageCode,
                              sizeof(szLanguageCode));
        strcat(szNLSDLL, "\\bin\\xfldr");
        strcat(szNLSDLL, szLanguageCode);
        strcat(szNLSDLL, ".dll");

        // try to load the module
        if (DosLoadModule(NULL,
                          0,
                          szNLSDLL,
                          &G_hmodNLS))
        {
            CHAR    szMessage[2000];
            sprintf(szMessage,
                    "xpstat was unable to load \"%s\", "
                    "the National Language DLL which "
                    "is specified for XWorkplace in OS2.INI.",
                    szNLSDLL);
            WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                          szMessage,
                          "xpstat: Fatal Error",
                          0, MB_OK | MB_MOVEABLE);
            Proceed = FALSE;
        }
    }
    return (Proceed);
}

/*
 * main:
 *      program entry point.
 */

int main(int argc, char *argv[])
{
    QMSG        qmsg;

    if (!(G_hab = WinInitialize(0)))
        return FALSE;

    if (!(G_hmq = WinCreateMsgQueue(G_hab, 0)))
        return FALSE;

    winhInitGlobals();      // V1.0.1 (2002-11-30) [umoeller]

    if (!xsecQueryStatus(NULL))
        G_fXWPSec = TRUE;

    // now attempt to find the XWorkplace NLS resource DLL,
    // which we need for all resources (new with XWP 0.9.0)
    if (LoadNLS())
    {
        SWP swpFrame;

        swpFrame.x = 100;
        swpFrame.y = 100;
        swpFrame.cx = 500;
        swpFrame.cy = 500;
        swpFrame.hwndInsertBehind = HWND_TOP;
        swpFrame.fl = SWP_MOVE | SWP_SIZE;

        if (    WinRegisterClass(G_hab,
                                 (PSZ)pcszClientClass,
                                 fnwpProcInfoClient,
                                 CS_SIZEREDRAW | CS_SYNCPAINT,
                                 sizeof(PVOID))
             && txvRegisterTextView(G_hab)
            )
        {
            HPOINTER hptrMain = WinLoadPointer(HWND_DESKTOP,
                                               NULLHANDLE,
                                               1);
            HWND hwndClient;
            G_hwndMain = winhCreateStdWindow(HWND_DESKTOP,
                                             &swpFrame,
                                             FCF_SYSMENU
                                             | FCF_SIZEBORDER
                                             | FCF_TITLEBAR
                                             | FCF_MINMAX
                                             | FCF_NOBYTEALIGN,
                                             WS_CLIPCHILDREN,
                                             "xpstat",
                                             0,
                                             (PSZ)pcszClientClass,
                                             WS_VISIBLE,
                                             0,
                                             NULL,
                                             &hwndClient);
            G_hMainMenu = WinLoadMenu(G_hwndMain,
                                      NULLHANDLE,
                                      ID_XPSM_MAIN);

            // now position the frame and the client:
            // 1) frame
            if (!winhRestoreWindowPos(G_hwndMain,
                                      HINI_USER,
                                      INIAPP,
                                      INIKEY_MAINWINPOS,
                                      SWP_MOVE | SWP_SIZE))
                // INI data not found:
                WinSetWindowPos(G_hwndMain,
                                HWND_TOP,
                                100, 100,
                                500, 500,
                                SWP_MOVE | SWP_SIZE);

            SetupMenu();

            // add to task list
            winhAddToTasklist(G_hwndMain,
                              hptrMain);
            // finally, show window
            WinShowWindow(G_hwndMain, TRUE);

            while (WinGetMsg(G_hab, &qmsg, 0, 0, 0))
                WinDispatchMsg(G_hab, &qmsg);
        }
    } // end if (proceed)

    // clean up on the way out
    WinDestroyMsgQueue(G_hmq);
    WinTerminate(G_hab);

    return TRUE;
}


