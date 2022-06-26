
/*
 *@@sourcefile xwpshell.c:
 *      startup wrapper around PMSHELL.EXE. Besides functioning
 *      as a shell starter, XWPShell also maintains users,
 *      processes, and access control lists to interact with
 *      the ring-0 device driver (XWPSEC32.SYS).
 *
 *      See helpers\xwpsecty.h for an introduction of how all
 *      this works.
 *
 *
 *      <B>XWPShell Setup:</B>
 *
 *      In CONFIG.SYS, set RUNWORKPLACE to XWPSHELL.EXE's full path.
 *      This will make XWPShell initialize PM.
 *
 *      Put the user database (xwpusers.xml) into ?:\OS2 on your
 *      boot drive.
 *
 *      Debug setup:
 *
 *      Instead of SET RUNWORKPLACE=...\xwpshell.exe, use cmd.exe
 *      and start XWPSHELL.EXE manually from the command line.
 *      In debug mode, you can terminate XWPShell by entering the
 *      "exit" user name with any dummy password.
 *
 *      Configuration via environment variables:
 *
 *      1.  In CONFIG.SYS, set XWPSHELL to PMSHELL.EXE's full path.
 *          The XWPSHELL environment variable specifies the executable
 *          to be started by XWPShell. If XWPSHELL is not defined,
 *          PMSHELL.EXE is used.
 *
 *      2.  In CONFIG.SYS, set XWPHOME to the full path of the HOME
 *          directory tree, which holds all user desktops and INI files.
 *          If this is not specified, this defaults to ?:\home on the
 *          boot drive.
 *
 *      3.  In CONFIG.SYS, set ETC to the directory where XWPShell
 *          should keep its user data base files (xwpusers.xml
 *          and xwpusers.acc; on a typical OS/2 installation, this
 *          will be ?:\mptn\etc). If this is not specified, this
 *          defaults to ?:\OS2 on the boot drive.
 *
 *@@added V0.9.5 [umoeller]
 *@@header "helpers\xwpsecty.h"
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
#define INCL_DOSQUEUES
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINPOINTERS
#define INCL_WINPROGRAMLIST
#define INCL_WINWORKPLACE
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <ctype.h>
#include <setjmp.h>

#include "setup.h"

#include "helpers\apps.h"
#include "helpers\dialog.h"
#include "helpers\dosh.h"
#include "helpers\except.h"
#include "helpers\lan.h"
#include "helpers\prfh.h"
#include "helpers\procstat.h"           // DosQProcStat handling
#include "helpers\winh.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\threads.h"

#include "helpers\xwpsecty.h"

#include "security\ring0api.h"
#include "security\xwpshell.h"

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

HPOINTER        G_hptrIcon = NULLHANDLE;

// user shell currently running:
// this must only be modified by thread-1!!
HAPP            G_happWPS = NULLHANDLE;
    // HAPP of WPS process or NULLHANDLE if the WPS is not running
PSZ             G_pszEnvironment = NULL;
    // environment of user shell
PPROCESSLIST    G_paPIDsLocalLogon = NULL;

extern PXFILE   G_LogFile = NULL;

PXWPSHELLSHARED G_pXWPShellShared = 0;

HQUEUE          G_hqXWPShell = 0;
THREADINFO      G_tiQueueThread = {0},
                G_tiLanThread = {0};

HWND            G_hwndShellObject = NULLHANDLE,
                G_hwndLanObject = NULLHANDLE;
    // object windows for communication

MSGBOXSTRINGS   G_MsgBoxStrings =
    {
        "~Yes",
        "~No",
        "~OK",
        "~Cancel",
        "~Abort",
        "~Retry",
        "~Ignore",
        "~Help",
        "Yes to ~all",
        "~Help"
    };

/* ******************************************************************
 *
 *   Helpers
 *
 ********************************************************************/

/*
 *@@ Error:
 *
 */

VOID Error(const char* pcszFormat,
           ...)
{
    va_list     args;
    CHAR        szError[1000];
    va_start(args, pcszFormat);
    vsprintf(szError, pcszFormat, args);
    va_end(args);

    doshWrite(G_LogFile, 0, szError);
    doshWrite(G_LogFile, 0, "\n");
    /* winhDebugBox(NULLHANDLE,
             "XWPShell",
             szError); */
    DosBeep(100, 1000);
    WinPostMsg(G_hwndShellObject, WM_QUIT, 0, 0);
}

/* ******************************************************************
 *
 *   Local Logon Dialog
 *
 ********************************************************************/

/*
 *@@ fnwpLogonDlg:
 *      dialog proc for the logon dialog. This takes
 *      the user name and password from the user.
 */

STATIC MRESULT EXPENTRY fnwpLogonDlg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    static PXWPLOGGEDON *ppLoggedOn = NULL;

    switch (msg)
    {
        case WM_INITDLG:
            winhSetControlsFont(hwnd, 0, 1000, NULL);
            winhCenterWindow(hwnd);
            ppLoggedOn = (PXWPLOGGEDON*)mp2;
        break;

        case WM_COMMAND:

            switch (SHORT1FROMMP(mp1))
            {
                case DID_OK:
                {
                    HWND    hwndUserID = WinWindowFromID(hwnd, IDDI_USERENTRY);
                    HWND    hwndPassword = WinWindowFromID(hwnd, IDDI_PASSWORDENTRY);
                    CHAR    szUserName[XWPSEC_NAMELEN],
                            szPassword[XWPSEC_NAMELEN];

                    HPOINTER hptrOld = winhSetWaitPointer();

                    if (WinQueryWindowText(hwndUserID,
                                           sizeof(szUserName),
                                           szUserName))
                    {
                        APIRET arc;

                        if (!strcmp(szUserName, "exit"))
                            WinDismissDlg(hwnd, 999);
                        else
                        {
                            static const ULONG aIDs[] =
                            {
                                DID_OK,
                                IDDI_USERENTRY,
                                IDDI_PASSWORDENTRY
                            };

                            WinQueryWindowText(hwndPassword,
                                               sizeof(szPassword),
                                               szPassword);

                            winhEnableControls2(hwnd,
                                                aIDs,
                                                ARRAYITEMCOUNT(aIDs),
                                                FALSE);

                            if (!(arc = slogLogOn(szUserName,
                                                  szPassword,
                                                  TRUE,                 // mark as local user
                                                  ppLoggedOn)))            // receives subject handles
                                WinDismissDlg(hwnd, DID_OK);

                            memset(szPassword,
                                   0,
                                   sizeof(szPassword));

                            WinSetPointer(HWND_DESKTOP, hptrOld);

                            if (arc)
                            {
                                dlghMessageBox(hwnd,
                                               G_hptrIcon,
                                               "XWorkplace Security",
                                               "Error: Invalid user name and/or password given.",
                                               NULL,
                                               MB_OK | MB_SYSTEMMODAL | MB_MOVEABLE,
                                               "9.WarpSans",
                                               &G_MsgBoxStrings);
                            }

                            winhEnableControls2(hwnd,
                                                aIDs,
                                                ARRAYITEMCOUNT(aIDs),
                                                TRUE);
                        }
                    }
                }
                break;
            }
        break;

        case WM_CONTROL:        // Dialogkommando
            break;

        default:
            return WinDefDlgProc(hwnd, msg, mp1, mp2);
    }

    return (MRESULT)0;
}

/*
 *@@ SetNewUserProfile:
 *      changes the current user profile using
 *      PrfReset.
 *
 *      Returns:
 *
 *      --  NO_ERROR: profile was successfully changed;
 *          in that case, *ppszEnvironment has been
 *          set to an environment strings buffer for
 *          use with WinStartApp. This must be freed
 *          by the caller.
 *
 *      --  XWPSEC_NO_USER_PROFILE: OS2.INI doesn't
 *          exist in the new user's home directory.
 *
 *      plus the error codes from appGetEnvironment;
 *
 *@@added V0.9.4 (2000-07-19) [umoeller]
 *@@changed V0.9.19 (2002-04-02) [umoeller]: changed prototype to return APIRET
 */

APIRET SetNewUserProfile(HAB hab,                       // in: XWPSHELL anchor block
                         ULONG uid,
                         PCSZ pcszUserName,
                         PSZ *ppszEnvironment)          // out: new environment
{
    APIRET arc = NO_ERROR;
    PSZ pEnv2 = NULL;
    DOSENVIRONMENT Env;

    if (!(arc = appGetEnvironment(&Env)))
    {
        ULONG cbEnv2 = 0;

        CHAR    szNewProfile[CCHMAXPATH];

        CHAR    szHomeBase[CCHMAXPATH];
        PSZ     pszHomeBase;
        if (!(pszHomeBase = getenv("XWPHOME")))
        {
            // XWPHOME not specified:
            sprintf(szHomeBase, "%c:\\home", doshQueryBootDrive());
            pszHomeBase = szHomeBase;
        }

        if (!uid)
            // root gets default profile
            strlcpy(szNewProfile,
                    getenv("USER_INI"),
                    sizeof(szNewProfile));
        else
            // non-root:
            sprintf(szNewProfile,
                    "%s\\%s\\os2.ini",
                    pszHomeBase,
                    pcszUserName);

        if (access(szNewProfile, 0) != 0)
            // OS2.INI doesn't exist:
            arc = XWPSEC_NO_USER_PROFILE;
        else
        {
            do      // for break
            {
                CHAR    szNewVar[1000];
                PSZ     p;
                sprintf(szNewVar, "USER_INI=%s", szNewProfile);
                if (arc = appSetEnvironmentVar(&Env, szNewVar, FALSE))
                    break;

                // set HOME var to home directory
                sprintf(szNewVar, "HOME=%s\\%s", pszHomeBase, pcszUserName);
                if (arc = appSetEnvironmentVar(&Env, szNewVar, FALSE))
                    break;

                // set USER var to user name
                sprintf(szNewVar, "USER=%s", pcszUserName);
                if (arc = appSetEnvironmentVar(&Env, szNewVar, FALSE))
                    break;

                // set USERID var to user name
                sprintf(szNewVar, "USERID=%d", uid);
                if (arc = appSetEnvironmentVar(&Env, szNewVar, FALSE))
                    break;

                if (arc = appConvertEnvironment(&Env, &pEnv2, &cbEnv2))
                    // pEnv != NULL now, which is returned
                    break;

            } while (FALSE);

            if (!arc)
                // user profile exists:
                // call PrfReset
                arc = prfhSetUserProfile(hab,
                                         szNewProfile);

        } // end if (access(szNewProfile, 0) == 0)

        appFreeEnvironment(&Env);

    } // end if (doshGetEnvironment(&Env)

    if (!arc)
        *ppszEnvironment = pEnv2;
    else
        if (pEnv2)
            free(pEnv2);

    return arc;
}

/*
 *@@ StartUserShell:
 *
 */

APIRET StartUserShell(VOID)
{
    APIRET      arc = NO_ERROR;

    XWPSECID    uidLocal;
    PXWPUSERDBENTRY pUser;

    if (    (!(arc = slogQueryLocalUser(&uidLocal)))
         && (!(arc = sudbQueryUser(uidLocal, &pUser)))
       )
    {
        PROGDETAILS pd;

        if (pUser->User.szUserShell[0] == '?')
            pUser->User.szUserShell[0] = doshQueryBootDrive();

        memset(&pd, 0, sizeof(pd));
        pd.Length = sizeof(PROGDETAILS);
        pd.progt.progc = PROG_DEFAULT;
        pd.progt.fbVisible = SHE_VISIBLE;
        pd.pszTitle = "Workplace Shell";
        pd.pszExecutable = pUser->User.szUserShell;
        pd.pszParameters = 0;
        pd.pszStartupDir = 0;
        pd.pszEnvironment = G_pszEnvironment;
                    // new environment with new USER_INI
        pd.swpInitial.fl = SWP_ACTIVATE | SWP_ZORDER | SWP_SHOW;
        pd.swpInitial.hwndInsertBehind = HWND_TOP;

        if (!(G_happWPS = WinStartApp(G_hwndShellObject,
                                      &pd,
                                      NULL,
                                      NULL,
                                      SAF_INSTALLEDCMDLINE | SAF_STARTCHILDAPP)))
        {
            Error("WinStartApp returned FALSE for starting %s",
                  pUser->User.szUserShell);
            arc = XWPSEC_CANNOT_START_SHELL;
        }

        DosFreeMem(pUser);
    }

    return arc;
}

/*
 *@@ LocalLogon:
 *      displays the logon dialog and creates a new
 *      locally logged-on user, including its
 *      user and group subject handles, sets the
 *      new shell environment and user profile
 *      and starts the user shell (normally the WPS).
 *
 *      This gets called from fnwpShellObject when
 *      XM_LOGON comes in.
 *
 *      On success, NO_ERROR is returned and
 *      G_pLocalLoggedOn contains the new user data.
 */

APIRET LocalLogon(VOID)
{
    APIRET      arc = NO_ERROR;

    PXWPLOGGEDON pLogon = NULL;
    switch (WinDlgBox(HWND_DESKTOP,
                      NULLHANDLE,      // owner
                      fnwpLogonDlg,
                      NULLHANDLE,
                      IDD_LOGON,
                      &pLogon))
    {
#ifdef __DEBUG__
        case 999:
            // in debug builds, allow exit
            WinPostMsg(G_hwndShellObject, WM_QUIT, 0, 0);
        break;
#endif

        case DID_OK:
        {
            // now go switch the security context of XWPShell
            // itself (and that of our parent process, which
            // might be another PMSHELL)
            ULONG pid;
            arc = scxtSetSecurityContext((pid = doshMyPID()),
                                         pLogon->cSubjects,
                                         pLogon->aSubjects);
            _Pmpf(("Got %d for setting context for PID 0x%lX", arc, pid));

            if (!arc)
            {
                arc = scxtSetSecurityContext((pid = doshMyParentPID()),
                                             pLogon->cSubjects,
                                             pLogon->aSubjects);
                _Pmpf(("Got %d for setting context for parent PID 0x%lX", arc, pid));
            }

            if (!arc)
            {
                G_pszEnvironment = NULL;

                if (arc = scxtGetRunningPIDs(&G_paPIDsLocalLogon))
                    Error("scxtGetRunningPIDs returned %d.", arc);

                if (arc = SetNewUserProfile(WinQueryAnchorBlock(G_hwndShellObject),
                                            pLogon->uid,
                                            pLogon->szUserName,
                                            &G_pszEnvironment))
                {
                    Error("SetNewUserProfile returned %d.", arc);
                    arc = XWPSEC_INVALID_PROFILE;
                }
                else
                {
                    // success:
                    // start the user's shell, which will switch the
                    // XWPShell security context first
                    arc = StartUserShell();
                }

                if (arc)
                    slogLogOff(pLogon->uid);
            }
        }
        break;

        default:
            arc = XWPSEC_NOT_AUTHENTICATED;
        break;
    }

    if (pLogon)
        free(pLogon);

    return arc;
}

/*
 *@@ LocalLogoff:
 *      logs off the current local user. Shuts down the
 *      system if we run into any errors.
 *
 *      This kills all processes that were started in
 *      the user's session, so beware.
 *
 */

VOID LocalLogoff(VOID)
{
    APIRET arc;
    XWPSECID    uidLocal;
    if (arc = slogQueryLocalUser(&uidLocal))
        Error("slogQueryLocalUser returned %d", arc);
    else
    {
        PQTOPLEVEL32    pInfo;
        USHORT          pidSelf = doshMyPID();
        _Pmpf(("uidLocal is 0x%lX", uidLocal));

        // now kill each process that
        // 1) matches the user ID that is logging off locally --and--
        // 2) was not running when the user logged on locally
        if (    (pInfo = prc32GetInfo(&arc))
             && (G_paPIDsLocalLogon)
           )
        {

#if 0
            PQPROCESS32 pProcThis = pInfo->pProcessData;
            while (pProcThis && pProcThis->ulRecType == 1)
            {
                PQTHREAD32  t = pProcThis->pThreads;

                if (pProcThis->usPID != pidSelf)
                {
                    PXWPSECURITYCONTEXTCORE pContext;
                    if (!scxtFindSecurityContext(pProcThis->usPID,
                                                 &pContext))
                    {
                        XWPSUBJECTINFO  si;
                        si.hSubject = pContext->aSubjects[0];
                        if (!scxtQuerySubjectInfo(&si))
                        {
                            _Pmpf(("pid 0x%lX -> uid 0x%lX", pProcThis->usPID, si.id));
                            if (si.bType == SUBJ_USER)
                                if (si.id == uidLocal)
                                {
                                    // alright, this process was running on behalf
                                    // of the user who's logging off:
                                    // kill it if it wasn't running when he logged on
                                    BOOL fWasRunning = FALSE;
                                    ULONG ul;

                                    for (ul = 0;
                                         ul < G_paPIDsLocalLogon->cTrusted;
                                         ++ul)
                                    {
                                        if (G_paPIDsLocalLogon->apidTrusted[ul] == pProcThis->usPID)
                                        {
                                            fWasRunning = TRUE;
                                            break;
                                        }
                                    }

                                    if (!fWasRunning)
                                    {
                                        PQMODULE32  pMod;
                                        PCSZ        pcszModule = "?";
                                        arc = DosKillProcess(DKP_PROCESS,
                                                             pProcThis->usPID);
                                        if (pMod = prc32FindModule(pInfo,
                                                                   pProcThis->usHModule))
                                            pcszModule = pMod->pcName;

                                        doshWriteLogEntry(G_LogFile,
                                                          "DosKillProcess for 0x%lX (%s) returned %d",
                                                          pProcThis->usPID,
                                                          pcszModule,
                                                          arc);
                                    }
                                 }
                        }

                        free(pContext);
                    }
                }

                // next process block comes after the
                // threads
                t += pProcThis->usThreadCount;
                pProcThis = (PQPROCESS32)t;
            }
#endif

            prc32FreeInfo(pInfo);
        }

        if (G_paPIDsLocalLogon)
        {
            free(G_paPIDsLocalLogon);
            G_paPIDsLocalLogon = NULL;
        }

        // log off the local user
        // (this deletes the subject handles)
        arc = slogLogOff(uidLocal);

        if (G_pszEnvironment)
        {
            free(G_pszEnvironment);
            G_pszEnvironment = NULL;
        }
    }

    if (arc != NO_ERROR)
        Error("WM_APPTERMINATENOTIFY: arc %d on logoff",
            arc);


}

/* ******************************************************************
 *
 *   Queue thread
 *
 ********************************************************************/

/*
 *@@ GiveMemToCaller:
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

APIRET GiveMemToCaller(PVOID p,
                       ULONG pid)
{
    APIRET arc;

    arc = DosGiveSharedMem((PBYTE)p,
                           pid, // caller's PID
                           PAG_READ | PAG_WRITE);

    // free this for us; usage count is 2 presently,
    // so the chunk will be freed after the caller
    // has issued DosFreeMem also
    DosFreeMem((PBYTE)p);

    return arc;
}

/*
 *@@ ProcessQueueCommand:
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 *@@changed V1.0.1 (2003-01-05) [umoeller]: added create user
 */

APIRET ProcessQueueCommand(PXWPSHELLQUEUEDATA pQD,
                           ULONG pid)
{
    APIRET  arc = NO_ERROR;

    // check size of struct; when we add fields, this can change,
    // and we don't want to blow up because of this V1.0.1 (2003-01-05) [umoeller]
    if (pQD->cbStruct != sizeof(XWPSHELLQUEUEDATA))
        return XWPSEC_STRUCT_MISMATCH;

#ifdef __DEBUG__
    TRY_LOUD(excpt1)
#else
    TRY_QUIET(excpt1)
#endif
    {
        // prepare security context so we can check if the
        // calling process has sufficient authority for
        // processing this request
        PXWPSECURITYCONTEXTCORE psc;
        if (!(arc = scxtFindSecurityContext(pid,
                                            &psc)))
        {
            switch (pQD->ulCommand)
            {
                case QUECMD_QUERYSTATUS:
                    arc = scxtQueryStatus(&pQD->Data.Status);
                break;

                case QUECMD_QUERYLOCALUSER:
                {
                    // no authority needed for this
                    XWPSECID    uidLocal;
                    if (    (!(arc = slogQueryLocalUser(&uidLocal)))
                         && (!(arc = sudbQueryUser(uidLocal,
                                                   &pQD->Data.pLocalUser)))
                       )
                    {
                        // this has allocated a chunk of shared memory, so give
                        // this to the caller
                        arc = GiveMemToCaller(pQD->Data.pLocalUser,
                                              pid);
                    }
                }
                break;

                case QUECMD_QUERYUSERS:
                    if (    (!(arc = scxtVerifyAuthority(psc,
                                                         XWPPERM_QUERYUSERINFO)))
                         && (!(arc = sudbQueryUsers(&pQD->Data.QueryUsers.cUsers,
                                                    &pQD->Data.QueryUsers.paUsers)))
                       )
                    {
                        // this has allocated a chunk of shared memory, so give
                        // this to the caller
                        arc = GiveMemToCaller(pQD->Data.QueryUsers.paUsers,
                                              pid);
                    }
                break;

                case QUECMD_QUERYGROUPS:
                    if (    (!(arc = scxtVerifyAuthority(psc,
                                                         XWPPERM_QUERYUSERINFO)))
                         && (!(arc = sudbQueryGroups(&pQD->Data.QueryGroups.cGroups,
                                                     &pQD->Data.QueryGroups.paGroups)))
                       )
                    {
                        // this has allocated a chunk of shared memory, so give
                        // this to the caller
                        arc = GiveMemToCaller(pQD->Data.QueryGroups.paGroups,
                                              pid);
                    }
                break;

                case QUECMD_QUERYUSERNAME:
                    arc = sudbQueryUserName(pQD->Data.QueryUserName.uid,
                                            pQD->Data.QueryUserName.szUserName);
                break;

                case QUECMD_QUERYPROCESSOWNER:
                {
                    PXWPSECURITYCONTEXTCORE psc2;
                    XWPSUBJECTINFO  si;

                    if (!(arc = scxtFindSecurityContext(pQD->Data.QueryProcessOwner.pid,
                                                        &psc2)))  // queried process
                    {
                        pQD->Data.QueryProcessOwner.hsubj0
                        = si.hSubject
                        = psc2->aSubjects[0];

                        if (!(arc = scxtQuerySubjectInfo(&si)))
                            if (si.bType == SUBJ_USER)
                                pQD->Data.QueryProcessOwner.uid = si.id;
                            else
                                pQD->Data.QueryProcessOwner.uid = -1;        // @@todo privileged process
                    }
                }
                break;

                case QUECMD_CREATEUSER:
                    if (!(arc = scxtVerifyAuthority(psc,
                                                    XWPPERM_CREATEUSER)))
                    {
                        XWPUSERDBENTRY ue;
                        #define COPYITEM(a) memcpy(ue.User.a, pQD->Data.CreateUser.a, sizeof(ue.User.a))
                        COPYITEM(szUserName);
                        COPYITEM(szFullName);
                        memcpy(ue.szPassword, pQD->Data.CreateUser.szPassword, sizeof(ue.szPassword));
                        if (!(arc = sudbCreateUser(&ue)))
                            pQD->Data.CreateUser.uidCreated = ue.User.uid;
                    }
                break;

                case QUECMD_QUERYPERMISSIONS:
                    arc = scxtQueryPermissions(pQD->Data.QueryPermissions.szResource,
                                               psc->cSubjects,
                                               psc->aSubjects,
                                               &pQD->Data.QueryPermissions.flAccess);
                break;

                case QUECMD_SWITCHUSER:
                {
                    PXWPLOGGEDON pLogon;
                    if (!(arc = slogLogOn(pQD->Data.SwitchUser.szUserName,
                                          pQD->Data.SwitchUser.szPassword,
                                          FALSE,        // not local
                                          &pLogon)))
                    {
                        if (!(arc = scxtSetSecurityContext(pid,
                                                           pLogon->cSubjects,
                                                           pLogon->aSubjects)))
                            pQD->Data.SwitchUser.uid = pLogon->uid;

                        free(pLogon);
                    }
                }
                break;

                default:
                    // unknown code:
                    arc = XWPSEC_QUEUE_INVALID_CMD;
                break;
            }

            free(psc);
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    return arc;
}

/*
 *@@ fntQueueThread:
 *      "queue" thread started by main() to process the
 *      XWPShell command queue. This allows other processes
 *      ("clients") to send commands to XWPShell.
 *
 *      The way this works is that a client allocates
 *      an XWPSHELLQUEUEDATA struct as shared memory,
 *      gives this to the XWPShell process, and writes
 *      an entry into the queue. We then process the
 *      command and post hevData when done to signal
 *      to the caller that data is available.
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 */

void _Optlink fntQueueThread(PTHREADINFO ptiMyself)
{
    while (!ptiMyself->fExit)
    {
        APIRET      arc = NO_ERROR;
        REQUESTDATA rq;
        ULONG       ulDummySize;
        PULONG      pulDummyData;
        BYTE        bPriority;

        // block on the queue until a command comes in
        if (!(arc = DosReadQueue(G_hqXWPShell,
                                 &rq,
                                 &ulDummySize,
                                 (PVOID*)&pulDummyData,
                                 0,                     // remove first element
                                 DCWW_WAIT,
                                 &bPriority,            // priority
                                 NULLHANDLE)))           // event semaphore, ignored for DCWW_WAIT
        {
            // got a command:
            PXWPSHELLQUEUEDATA  pQD = (PXWPSHELLQUEUEDATA)(rq.ulData);
            HEV hev = pQD->hevData;

            /* _PmpfF(("got queue item, pQD->ulCommand: %d",
                        pQD->ulCommand)); */

            if (!DosOpenEventSem(NULL,
                                 &hev))
            {
                TRY_LOUD(excpt1)
                {
                    pQD->arc = ProcessQueueCommand(pQD,
                                                   // caller's pid
                                                   rq.pid);
                }
                CATCH(excpt1)
                {
                } END_CATCH();

                DosPostEventSem(hev);
                DosCloseEventSem(hev);
            }

            // free shared memory for this process... it was
            // given to us by the client, so we must lower
            // the resource count (client will still own it)
            DosFreeMem(pQD);
        }
        else
            _PmpfF(("DosReadQueue returned %d", arc));
    }
}

/* ******************************************************************
 *
 *   LAN thread
 *
 ********************************************************************/

/*
 *@@ fnwpLanObject:
 *
 *@@added V1.0.2 (2003-11-13) [umoeller]
 */

MRESULT EXPENTRY fnwpLanObject(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    SERVICEBUF2 buf2;

    switch (msg)
    {
        /*
         *@@ LANM_STARTREQ:
         *      ensures that at least the requester is running. Note
         *      that if the REQUESTER service is started, this will
         *      usually start PEER (or even SERVER) as well.
         */

        case LANM_STARTREQ:
        {
            BOOL fNeedsStart = FALSE;
            APIRET rc;

            doshWriteLogEntry(G_LogFile,
                              "processing LANM_STARTREQ");
            // if requester is not running, then this will yield an error
            rc = lanServiceControl("REQUESTER",
                                   SERVICE_CTRL_INTERROGATE,
                                   &buf2);
            doshWriteLogEntry(G_LogFile,
                              "SERVICE_CTRL_INTERROGATE(REQUESTER) -> rc %d",
                              rc);

            switch (rc)
            {
                case NERR_WkstaNotStarted:      // 2138
                case NERR_ServiceNotInstalled:  // 2184
                    fNeedsStart = TRUE;
            }

            doshWriteLogEntry(G_LogFile,
                              "Needs start: %d",
                              fNeedsStart);

            if (fNeedsStart)
            {
                rc = lanServiceInstall("REQUESTER", &buf2);

                doshWriteLogEntry(G_LogFile,
                                  "lanServiceInstall(REQUESTER) -> rc %d",
                                  rc);

                if (!rc)
                {
                    // wait until status is no longer "starting"
                    while (1)
                    {
                        DosSleep(1000);
                        if (    (lanServiceControl("REQUESTER",
                                                   SERVICE_CTRL_INTERROGATE,
                                                   &buf2))
                             || ((buf2.svci2_status & SERVICE_INSTALL_STATE) != SERVICE_INSTALL_PENDING)
                           )
                            break;
                    }
                }
            }

            doshWriteLogEntry(G_LogFile,
                              "done with LANM_STARTREQ",
                              rc);
        }
        break;

        default:
            mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ fntLanThread:
 *
 *@@added V1.0.2 (2003-11-13) [umoeller]
 */

void _Optlink fntLanThread(PTHREADINFO ptiMyself)
{
    APIRET rc = lanInit();

    doshWriteLogEntry(G_LogFile,
                      "lanInit -> rc %d",
                      rc);

    // if this system doesn't have LAN requester or peer installed,
    // then just quit this thread!
    if (!rc)
    {
        // LAN present:
        // only then create object window

        if (G_hwndLanObject = winhCreateObjectWindow(WC_LAN_OBJECT,
                                                     NULL))
        {
            QMSG qmsg;

            doshWriteLogEntry(G_LogFile,
                              "LAN thread object window is 0x%lX",
                              G_hwndLanObject);

            WinPostMsg(G_hwndLanObject,
                       LANM_STARTREQ,
                       NULL,
                       NULL);

            while (WinGetMsg(ptiMyself->hab, &qmsg, NULLHANDLE, 0, 0))
                WinDispatchMsg(ptiMyself->hab, &qmsg);
        }
    }
}

/* ******************************************************************
 *
 *   Object window
 *
 ********************************************************************/

VOID DumpEnv(PDOSENVIRONMENT pEnv)
{
    PSZ     *ppszThis = pEnv->papszVars;
    PSZ     pszThis;
    ULONG   ul = 0;
    _Pmpf(("cVars: %d", pEnv->cVars));

    for (ul = 0;
         ul < pEnv->cVars;
         ul++)
    {
        pszThis = *ppszThis;
        _Pmpf(("var %d: %s", ul, pszThis));
        // next environment string
        ppszThis++;
    }
}

/*
 *@@ fnwpShellObject:
 *      winproc for XWPShell's object window on thread 1.
 *      This is created by main().
 */

MRESULT EXPENTRY fnwpShellObject(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    PTHREADINFO ptiSESLogonEvent = NULL;

    switch (msg)
    {
        /*
         *@@ SHM_LOGON:
         *
         */

        case SHM_LOGON:
        {
            // display logon dialog, create subject handles,
            // set environment, start shell, ...
            APIRET arc;

            if (arc = LocalLogon())
            {
                // error:
                // repost to try again
                WinPostMsg(hwndObject,
                           SHM_LOGON,
                           0, 0);
            }
        }
        break;

        /*
         * WM_APPTERMINATENOTIFY:
         *      gets posted by PM when an application
         *      started with WinStartApp terminates.
         *      In this case, it's the WPS. So we log
         *      off.
         */

        case WM_APPTERMINATENOTIFY:
        {
            HAPP    happ = (HAPP)mp1;
            if (happ == G_happWPS)
            {
                // PMSHELL.EXE (or other user shell) has terminated:
                APIRET arc = NO_ERROR;

                G_happWPS = NULLHANDLE;

                // check if the shell wants a restart or
                // a new logon
                if (G_pXWPShellShared->fNoLogonButRestart)
                {
                    StartUserShell();
                }
                else
                {
                    LocalLogoff();

                    // show logon dlg again
                    WinPostMsg(hwndObject,
                               SHM_LOGON,
                               0, 0);
                }
            }
        }
        break;

        case SHM_ERROR:
        {
            PSZ     pszError = "Unknown error.";

            switch ((ULONG)mp1)
            {
                case XWPSEC_RING0_NOT_FOUND:
                    pszError = "XWPSEC32.SYS driver not installed.";
                break;
            }

            winhDebugBox(0,
                     "XWPShell Error",
                     pszError);
        }
        break;

        case SHM_MESSAGE:
            winhDebugBox(0,
                     "XWPShell Message",
                     (PSZ)mp1);
            free(mp1);
        break;

        default:
            mrc = WinDefWindowProc(hwndObject, msg, mp1, mp2);
    }

    return mrc;
}

/*
 * main:
 *      program entry point.
 */

int main(int argc, char *argv[])
{
    APIRET      arc = NO_ERROR;

    HAB         hab;
    HMQ         hmq;
    DATETIME    DT;
    ULONG       cbFile = 0;

    CHAR        szLog[500];
    CHAR        szBoot[] = "?:";
    PSZ         pszLogDir;
    if (DosScanEnv("LOGFILES",      // new eCS 1.1 setting
                   &pszLogDir))
    {
        // variable not set:
#ifdef __EWORKPLACE__
        return FALSE;
#else
        szBoot[0] = doshQueryBootDrive();
        pszLogDir = szBoot;
#endif
    }

    sprintf(szLog,
            "%s\\xwpshell.log",
            pszLogDir);

    doshOpen(szLog,
             XOPEN_READWRITE_NEW, // XOPEN_READWRITE_APPEND,        // not XOPEN_BINARY
             &cbFile,
             &G_LogFile);

    DosGetDateTime(&DT);
    sprintf(szLog,
            "\n\nXWPShell startup -- %04d-%02d-%02d %02d:%02d:%02d\n",
            DT.year, DT.month, DT.day,
            DT.hours, DT.minutes, DT.seconds);
    doshWrite(G_LogFile,
              0,
              szLog);
    doshWrite(G_LogFile,
              0,
              "---------------------------------------\n");

    if (!(hab = WinInitialize(0)))
        return 99;

    if (!(hmq = WinCreateMsgQueue(hab, 0)))
        return 99;

    winhInitGlobals();      // V1.0.1 (2002-11-30) [umoeller]

    if (winhAnotherInstance("\\SEM32\\XWPSHELL.MTX", FALSE))
    {
        // already running:
        winhDebugBox(NULLHANDLE,
                 "XWorkplace Security",
                 "Another instance of XWPSHELL.EXE is already running. "
                 "This instance will terminate now.");
        arc = -1;
    }
    else
    {
        TRY_LOUD(excpt1)
        {
            // since this program will never stop running, make
            // sure we survive even shutdown
            WinCancelShutdown(hmq, TRUE);

            G_hptrIcon = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 1);

            // allocate XWPSHELLSHARED
            if (arc = DosAllocSharedMem((PVOID*)&G_pXWPShellShared,
                                        SHMEM_XWPSHELL,
                                        sizeof(XWPSHELLSHARED),
                                        PAG_COMMIT | PAG_READ | PAG_WRITE))
                Error("DosAllocSharedMem returned %d.", arc);
            // create master queue
            else if (arc = DosCreateQueue(&G_hqXWPShell,
                                          QUE_FIFO | QUE_NOCONVERT_ADDRESS,
                                          QUEUE_XWPSHELL))
                Error("DosCreateQueue returned %d.", arc);
            // initialize subsystems
            else if (arc = scxtInit())
                Error("Error %d initializing security contexts.", arc);
            else if (arc = sudbInit())
                Error("Error %d initializing user database.", arc);
            else if (arc = slogInit())
                Error("Error %d initializing logon management.", arc);
            // create shell object (thread 1)
            else if (!WinRegisterClass(hab,
                                       WC_SHELL_OBJECT,
                                       fnwpShellObject,
                                       0,
                                       sizeof(ULONG)))
                arc = -1;
            else if (!(G_hwndShellObject = winhCreateObjectWindow(WC_SHELL_OBJECT,
                                                                  NULL)))
                arc = -1;
            else if (!WinRegisterClass(hab,
                                       WC_LAN_OBJECT,
                                       fnwpLanObject,       // obj window only created on LAN thread
                                       0,
                                       sizeof(ULONG)))
                arc = -1;
            else
            {
                QMSG qmsg;
                // OK:

                // create the queue thread
                thrCreate(&G_tiQueueThread,
                          fntQueueThread,
                          NULL,
                          "Queue thread",
                          THRF_WAIT,
                          0);

                // create the LAN thread
                thrCreate(&G_tiLanThread,
                          fntLanThread,
                          NULL,
                          "Lan thread",
                          THRF_WAIT | THRF_PMMSGQUEUE,
                          0);

                // do a logon first
                WinPostMsg(G_hwndShellObject,
                           SHM_LOGON,
                           0, 0);

                // enter standard PM message loop
                while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
                    WinDispatchMsg(hab, &qmsg);
            }
        }
        CATCH(excpt1)
        {
        } END_CATCH();

        scxtExit();
    }

    // clean up on the way out
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    doshWriteLogEntry(G_LogFile,
                      "XWPShell exiting");
    doshClose(&G_LogFile);

    return arc;
}


