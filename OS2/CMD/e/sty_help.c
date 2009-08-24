/*************************************************************************
*
*  File Name   : STY_HELP.C
*
*  Description : This module contains all the routines for
*                interfacing with the IPF help manager.
*
*  Concepts    : Help initialization and termination, handling of
*                help menu items.
*
*  API's       : WinCreateHelpInstance
*                WinAssociateHelpInstance
*                WinLoadString
*                WinSendMsg
*                WinDlgBox
*                WinDestroyHelpInstance
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

/*  Include files, macros, defined constants, and externs       */

#define  INCL_WINHELP

#include <os2.h>
#include <string.h>
#include "sty_main.h"
#include "sty_xtrn.h"
#include "sty_dlg.h"
#include "sty_help.h"

#define HELPLIBRARYNAMELEN  20

/*  Global variables                                            */
static HWND hwndHelpInstance;
static CHAR szLibName[HELPLIBRARYNAMELEN];
static CHAR szWindowTitle[HELPLIBRARYNAMELEN];


/*  Entry point declarations                                    */
MRESULT EXPENTRY ProdInfoDlgProc(HWND hwnd, USHORT msg,
                                 MPARAM mp1, MPARAM mp2);

/***************************************************************
 *  Name: InitHelp
 *
 *  Description : Initializes the IPF help facility
 *
 *  Concepts : Initializes the HELPINIT structure and creates the
 *             help instance.  If successful, the help instance
 *             is associated with the main window
 *
 *  API's : WinCreateHelpInstance
 *          WinAssociateHelpInstance
 *          WinLoadString
 *
 *  Parameters : None
 *
 *  Returns:  None
 *
 ****************************************************************/
VOID InitHelp(VOID)
{
  HELPINIT helpInit;

  /* if we return because of an error, Help will be disabled                */
  fHelpEnabled = FALSE;

  /* inititalize help init structure                                        */
  helpInit.cb = sizeof(HELPINIT);
  helpInit.ulReturnCode = 0L;

  helpInit.pszTutorialName = (PSZ)NULL;   /* if tutorial added,             */
                                          /* add name here                  */
  helpInit.phtHelpTable = (PHELPTABLE)MAKELONG(STYLE_HELP_TABLE,
                           0xFFFF);
  helpInit.hmodHelpTableModule = (HMODULE)0;
  helpInit.hmodAccelActionBarModule = (HMODULE)0;
  helpInit.idAccelTable = 0;
  helpInit.idActionBar = 0;

  if (!WinLoadString(hab, (HMODULE)0, IDS_HELPWINDOWTITLE,
                    HELPLIBRARYNAMELEN, (PSZ)szWindowTitle))
  {
     MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR, FALSE);
     return;
  }
  helpInit.pszHelpWindowTitle = (PSZ)szWindowTitle;
  helpInit.fShowPanelId = CMIC_HIDE_PANEL_ID;

  if (!WinLoadString(hab, (HMODULE)0, IDS_HELPLIBRARYNAME,
       HELPLIBRARYNAMELEN, (PSZ)szLibName))
  {
     MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR, FALSE);
     return;
  }

  helpInit.pszHelpLibraryName = (PSZ)szLibName;

  /* creating help instance                                                 */
  hwndHelpInstance = WinCreateHelpInstance(hab, &helpInit);

  if (!hwndHelpInstance || helpInit.ulReturnCode)
  {
     MessageBox(hwndMainFrame, IDMSG_HELPLOADERROR, MB_OK | MB_ERROR, TRUE);
     return;
  }

  /* associate help instance with main frame                                */
  if (!WinAssociateHelpInstance(hwndHelpInstance, hwndMainFrame))
  {
     MessageBox(hwndMainFrame, IDMSG_HELPLOADERROR, MB_OK | MB_ERROR, TRUE);
     return;
  }

  /* help manager is successfully initialized so set flag to TRUE           */
  fHelpEnabled = TRUE;

}   /*                                          End of InitHelp()           */

/*********************************************************************
 *  Name: HelpUsingHelp
 *
 *  Description : Processes the WM_COMMAND message posted
 *                by the Using Help item of the Help menu
 *
 *  Concepts : Called from MainCommand when the Using Help menu item
 *             is selected.  Sends an HM_DISPLAY_HELP message to the
 *             help instance so that the default Using Help is
 *             displayed.
 *
 *  API's : WinSendMsg
 *
 *  Parameters :  mp2 - Message parameter 2
 *
 *  Returns: None
 *
 ****************************************************************/
VOID  HelpUsingHelp(MPARAM mp2)
{
/*    this just displays the system help for help panel                       */
  if (fHelpEnabled)
  {
    if ((BOOL)WinSendMsg(hwndHelpInstance, HM_DISPLAY_HELP, NULL, NULL))
    {
       MessageBox(hwndMain, IDMSG_HELPDISPLAYERROR, MB_OK | MB_ERROR, FALSE);
    }
  }
 /*
  * This routine currently doesn't use the mp2 parameter but
  * it is referenced here to prevent an 'Unreferenced Parameter'
  * warning at compile time.
  */
  mp2;
}   /* End of HelpUsingHelp()                                                */

/*********************************************************************
 *  Name: HelpUsingHelp
 *
 *  Description : Processes the WM_COMMAND message posted
 *                by the General Help item of the Help menu
 *
 *  Concepts : Called from MainCommand when the General Help menu item
 *             is selected.  Sends an HM_EXT_HELP message to the
 *             help instance so that the default Extended Help is
 *             displayed.
 *
 *  API's : WinSendMsg
 *
 *  Parameters :  mp2 - Message parameter 2
 *
 *  Returns: None
 *
 ****************************************************************/
VOID  HelpGeneral(MPARAM mp2)
{
/*   this just displays the system General help panel                       */
   if (fHelpEnabled)
   {
     if ((BOOL)WinSendMsg(hwndHelpInstance, HM_EXT_HELP, NULL, NULL))
     {
        MessageBox(hwndMain, IDMSG_HELPDISPLAYERROR, MB_OK | MB_ERROR, FALSE);
     }
   }
   /*
    * This routine currently doesn't use the mp2 parameter but
    * it is referenced here to prevent an 'Unreferenced Parameter'
    * warning at compile time.
    */
    mp2;

}       /* End of HelpGeneral()                                      */

/*********************************************************************
 *  Name: HelpKeys
 *
 *  Description : Processes the WM_COMMAND message posted
 *                by the Keys Help item of the Help menu
 *
 *  Concepts : Called from MainCommand when the Keys Help menu item
 *             is selected.  Sends an HM_KEYS_HELP message to the
 *             help instance so that the default Extended Help is
 *             displayed.
 *
 *  API's : WinSendMsg
 *
 *  Parameters :  mp2 - Message parameter 2
 *
 *  Returns: None
 *
 ****************************************************************/
VOID  HelpKeys(MPARAM mp2)
{
/* this just displays the system keys help panel                              */
  if (fHelpEnabled)
  {
    if ((BOOL)WinSendMsg(hwndHelpInstance, HM_KEYS_HELP, NULL, NULL))
    {
      MessageBox(hwndMain, IDMSG_HELPDISPLAYERROR, MB_OK | MB_ERROR, FALSE);
    }
  }
/*
 * This routine currently doesn't use the mp2 parameter but
 * it is referenced here to prevent an 'Unreferenced Parameter'
 * warning at compile time.
 */
  mp2;
}   /*   End of HelpKeys()                                                    */

/*********************************************************************
 *  Name: HelpIndex
 *
 *  Description : Processes the WM_COMMAND message posted
 *                by the Index Help item of the Help menu
 *
 *  Concepts : Called from MainCommand when the Index Help menu item
 *             is selected.  Sends an HM_INDEX_HELP message to the
 *             help instance so that the default Extended Help is
 *             displayed.
 *
 *  API's : WinSendMsg
 *
 *  Parameters :  mp2 - Message parameter 2
 *
 *  Returns: None
 *
 ****************************************************************/
VOID  HelpIndex(MPARAM mp2)
{
/* this just displays the system help index panel                             */
  if (fHelpEnabled)
  {
    if ((BOOL)WinSendMsg(hwndHelpInstance, HM_HELP_INDEX, NULL, NULL))
    {
       MessageBox(hwndMain, IDMSG_HELPDISPLAYERROR, MB_OK | MB_ERROR, FALSE);
    }
  }
 /*
  * This routine currently doesn't use the mp2 parameter but
  * it is referenced here to prevent an 'Unreferenced Parameter'
  * warning at compile time.
  */
  mp2;
}   /* End of HelpIndex()                                                     */

/*********************************************************************
 *  Name: HelpProdInfo
 *
 *  Description : Processes the WM_COMMAND message posted
 *                by the Index Help item of the Help menu
 *
 *  Concepts : Called from MainCommand when the Product
 *             information Help menu item is selected.
 *             Calls WinDlgBox to display the Product
 *             Information dialog.
 *
 *  API's : WinDlgBox
 *
 *  Parameters :  mp2 - Message parameter 2
 *
 *  Returns: None
 *
 ****************************************************************/
VOID  HelpProdInfo( MPARAM mp2)
{
/* display the Product Information dialog                                     */
  WinDlgBox(HWND_DESKTOP, hwndMain, (PFNWP)ProdInfoDlgProc, (HMODULE)0,
            IDD_PRODINFO, (PVOID)NULL);
 /*
  * This routine currently doesn't use the mp2 parameter but
  * it is referenced here to prevent an 'Unreferenced Parameter'
  * warning at compile time.
  */
    mp2;

}   /* End of HelpProdInfo()                                                  */

/*********************************************************************
 *  Name: DisplayHelp
 *
 *  Description : Displays the help panel indicated
 *
 *  Concepts : Displays the help panel whose id is passed to
 *             the routine.  Called whenever a help panel is
 *             desired to be displayed, usually from the
 *             WM_HELP processing of the dialog boxes.
 *
 *  API's : WinSendMsg
 *
 *  Parameters : idPanel - Id of the halp panel to be displayed
 *
 *  Returns: None
 *
 ****************************************************************/
VOID DisplayHelpPanel(SHORT idPanel)
{
  if (fHelpEnabled)
  {
    if ((BOOL)WinSendMsg(hwndHelpInstance, HM_DISPLAY_HELP,
            MPFROM2SHORT(idPanel, NULL), MPFROMSHORT(HM_RESOURCEID)))
    {
      MessageBox(hwndMainFrame, IDMSG_HELPDISPLAYERROR,
                 MB_OK | MB_ERROR, TRUE);
    }
  }
}   /*    End of DisplayHelpPanel()                                   */

/*********************************************************************
 *  Name: DestroyHelpInstance
 *
 *  Description : Destroys the help instance for the application
 *
 *  Concepts : Called after exit from message loop. Calls
 *             WinDestroyHelpInstance() to destroy the
 *             help instance.
 *
 *  API's : WinDestroyHelpInstance
 *
 *  Parameters : None
 * None
 *  Returns:
 *
 ****************************************************************/
VOID DestroyHelpInstance(VOID)
{
  if ((BOOL)hwndHelpInstance)
  {
     WinDestroyHelpInstance(hwndHelpInstance);
  }

}   /*        End of DestroyHelpInstance()                                    */
