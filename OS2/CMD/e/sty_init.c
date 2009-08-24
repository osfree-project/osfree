/*************************************************************************
*
*  File Name   : STY_INIT.C
*
*  Description : This module contains the code for application
*                initialization as well as the code for exit list
*                processing
*
*  Concepts    : Application initialization and Exit list processing
*
*  API's       : DosExit
*                DosExitList
*                WinCreateWindow
*                WinDestroyMsgQueue
*                WinDistroyWindow
*                WinIsWindow
*                WinLoadString
*                WinQueryWindowRect
*                WinRegisterClass
*                WinTerminate
*
*  Copyright (C) 1992 IBM Corporation
*
*      DISCLAIMER OF WARRANTIES.  The following [enclosed] code is
*      sample code created by IBM Corporation. This sample code is not
*      part of any standard or IBM product and is provided to you solely
*      for  the purpose of assisting you in the development of your
*      applications.  The code is provided "AS IS", without
*      warranty of any kind.  IBM shall not be liable for any damages
*      arising out of your use of the sample code, even if they have been
*      advised of the possibility of such damages.                                                    *
*
************************************************************************/

/*  Include files, macros, defined constants, and externs               */

#define  INCL_WINWINDOWMGR
#define  INCL_WINMLE
#define  INCL_DOSPROCESS

#include <os2.h>
#include <string.h>
#include "sty_main.h"
#include "sty_xtrn.h"
#include "sty_dlg.h"

#define RETURN_ERROR        1               /* error return in DosExit  */

/*  Global variables                                                    */

HWND hwndMLE;
extern LONG lClrForeground;                         /* color for window text */
extern LONG lClrBackground;                          /* color for background */
extern LONG lClrDefaultForeground;                  /* color for window text */
extern LONG lClrDefaultBackground;                   /* color for background */

/*********************************************************************
 *  Name: Init
 *
 *  Description : Performs initialization functions required
 *                before the main window can be created.
 *
 *  Concepts :  Window class registration and addition of exit
 *              procedure to Exit list.
 *
 *  API's : DosExit
 *          DosExitList
 *          WinLoadString
 *          WinRegisterClass
 *
 *  Parameters :  None
 *
 *  Returns: TRUE  - initialization is successful
 *           FALSE - initialization failed
 *
 ****************************************************************/
BOOL Init(VOID)
{
    /* Add ExitProc to the exit list to handle the exit processing.  If
     * there is an error, then terminate the process since there have
     * not been any resources allocated yet
     */
    if(DosExitList(EXLST_ADD, (PFNEXITLIST)ExitProc))  {
        MessageBox(HWND_DESKTOP,
                   IDMSG_CANNOTLOADEXITLIST,
                   MB_OK | MB_ERROR,
                   TRUE);
        DosExit(EXIT_PROCESS, RETURN_ERROR);
    }

    /* load application name from resource file */
    if(!WinLoadString(hab, (HMODULE)0, IDS_APPNAME, MAXNAMEL, szAppName))
        return FALSE;

    /* load "untitled" string */
    if(!WinLoadString(hab, (HMODULE)0, IDS_UNTITLED, MESSAGELEN, szUntitled))
        return FALSE;

    /* register the main client window class */
    if(!WinRegisterClass(hab,
                        (PSZ)szAppName,
                        (PFNWP)MainWndProc,
                        CS_SIZEREDRAW | CS_CLIPCHILDREN,
                        0))  {
        return FALSE;
    }

    /*
     * Add any command line processing here
     */
    return TRUE;
}               /* End of Init()          */

/*********************************************************************
 *  Name: InitMainWindow
 *
 *  Description : Performs initialization functions required
 *                when the main window can be created.
 *
 *  Concepts : Performs initialization functions required
 *             when the main window is created.  Called once
 *             during the WM_CREATE processing when the main
 *             window is created.
 *
 *
 *  API's : WinQueryWindowRect
 *          WinCreateWindow
 *
 *  Parameters : hwnd - client window handle
 *               mp1 - First message parameter
 *               mp2 - Second message parameter
 *
 *  Returns: TRUE  - initialization failed
 *           FALSE - initialization is successful
 *
 ****************************************************************/
MRESULT InitMainWindow(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    RECTL rcl;

    UpdateTitleText(((PCREATESTRUCT)PVOIDFROMMP(mp2))->hwndParent);

    WinQueryWindowRect(hwnd, (PRECTL)&rcl);

    /* create MLE window the same size as the client */

    hwndMLE = WinCreateWindow(hwnd, WC_MLE, (PSZ)NULL,
                              MLS_HSCROLL | MLS_VSCROLL | WS_VISIBLE,
                              rcl.xLeft,  rcl.yBottom,
                              rcl.xRight, rcl.yTop,
                              hwnd, HWND_TOP, ID_MLE, NULL, NULL);
    if(!hwndMLE)
        return (MRESULT)TRUE;

    /*
     * Set default colors
     */

    WinSendMsg(hwndMLE, MLM_SETTEXTCOLOR, MPFROMLONG(lClrDefaultForeground),
                                                                        NULL);
    WinSendMsg(hwndMLE, MLM_SETBACKCOLOR, MPFROMLONG(lClrDefaultBackground),
                                                                        NULL);
    WinSendMsg(hwndMLE, MLM_RESETUNDO, NULL, NULL);

    /* return FALSE to continue window creation, TRUE to abort it */
    return (MRESULT)FALSE;

    /*
     *  This routine currently doesn't use the mp1 and mp2 parameters so
     *  it is referenced here to prevent an 'Unreferenced Parameter'
     *  warning at compile time
     */
    mp1;
    mp2;
}        /* End of InitMainWindow() */

/*********************************************************************
 *  Name:  ExitProc
 *
 *  Description : Cleans up certain resources when the
 *                application terminates
 *
 *  Concepts : Global resources, such as the main window an
 *             message queue, are destroyed and any system
 *             resources used are freed
 *
 *  API's : WinIsWindow
 *          WinDestroyWindow
 *          WinDestroyMsgQueue
 *          WinTerminate
 *          DosExitList
 *
 *  Parameters : USHORT -  termination code
 *
 *  Returns:  Returns EXLST_EXIT to the DosExitList handler
 *
 ****************************************************************/
VOID APIENTRY ExitProc(USHORT usTermCode)
{
 /* destroy the main window if it exists                       */
    if(WinIsWindow(hab, hwndMainFrame))
        WinDestroyWindow(hwndMainFrame);

    /*
     *      Any other system resources used
     *      (e.g. memory or files) should be freed here
     */

    WinDestroyMsgQueue(hmq);

    WinTerminate(hab);

    /*
     * Termination complete
     */
    DosExitList(EXLST_EXIT, (PFNEXITLIST)ExitProc);

    /*
     * This routine currently doesn't use the usTermCode parameter so
     *  it is referenced here to prevent an 'Unreferenced Parameter'
     *  warning at compile time
     */
    usTermCode;
}                                  /*     End of ExitProc()           */
