/*************************************************************************
*
*  File Name   : Sty_main.c
*
*  Description : This PM sample application demonstates the
*                standard menus, dialogs, and controls found
*                in most PM applications.
*
*  Concepts    : PM window creation
*                Menu Creation
*                Dialog box creation
*                Presentation parameters
*                Initizalition and display of PM controls
*                Help creation and initialization
*
*  API's       :
*  -------------
*  DosAllocMem              WinFontDlg             WinCreateStdWindow
*  DosBeep                  WinGetMsg              WinCreateWindow
*  DosClose                 WinGetPs               WinDefDlgProc
*  DosExit                  WinInitialize          WinDefDlgProc
*  DosExitList              WinIsWindow            WinDefWindowProc
*  DosFreeMem               WinLoadMenu            WinDestroyHelpINstance
*  DosOpen                  WinLoadString          WinDestroyWIndow
*  DosRead                  WinMapWIndowPoints     WinDismissDlg
*  DosWrite                 WinMessageBox          WinDispatchMsg
*  GpiCreateLogFont         WinOpenClipbrd         WinDlgBox
*  GpiDeleteSetID           WinPopupMenu           WinEndPaint
*  GpiQueryFontMetrics      WinPostMsg             WinSendMsg
*  GpiQueryFonts            WinQueryClipbrdFmtInfo WinSetDlgItemText
*  GpiSetCharSet            WinQueryFocus          WinSetFocus
*  WinSendDlgItemMsg        WinQueryTaskTitle      WinSetPresParms
*  WinTerminate             WinQueryWindowPos      WinSetWindosPos
*  WinAlarm                 WinQueryWindowRect     WinSetWindowText
*  WinAssociateHelpInstance WinQueryWindowText     WinTerminate
*  WinBeginPaint            WinQueryWindowUShort   WinWindosFormID
*  WinCloseClipbrd          WinRegisterClass       WinDestroyMsgQueue
*  WinCreateHelpInstance    WinReleasePS           WinFileDlg
*  WinCreateMsgQueue        WinRemovePresParms     WinFillRect
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

/* Include files, macros, defined constants, and externs                */

#define  INCL_WINHELP
//#define  INCL_WIN

#include <os2.h>
#include "sty_main.h"
#include "sty_xtrn.h"
#include "sty_help.h"

#define RETURN_SUCCESS      0            /* successful return in DosExit */
#define RETURN_ERROR        1            /* error return in DosExit      */
#define BEEP_WARN_FREQ      60           /* frequency of warning beep    */
#define BEEP_WARN_DUR      100           /* duration of warning beep     */


/*  Global variables                                                     */

HWND hwndMainFrame;                  /* handle to the main frame window */
HWND hwndMain;                       /* handle to the main client window */
HDC  hdcMain;                        /* handle to the DC of the client */
HAB hab;                             /* anchor block for the process */
HMQ hmq;                             /* handle to the process' message queue */
CHAR szAppName[MAXNAMEL];            /* buffer for application name string */
CHAR szUntitled[MESSAGELEN];         /* buffer for "Untitled" string */
BOOL fHelpEnabled;                   /* flag to determine if help is enabled */

/****************************************************************
 *  Name:   main()
 *
 *  Description: Entry point of program.
 *
 *  Concepts: Obtains anchor block handle and creates message
 *            queue.  Calls the initialization routine.
 *            Creates the main frame window which creates the
 *            main client window.  Polls the message queue
 *            via Get/Dispatch Msg loop.  Upon exiting the
 *            loop, exits.
 *
 *  API's   :  WinInitilize
 *             DosBeep
 *             WinCreateMsgQueue
 *             WinTerminate
 *             WinCreateStdWindow
 *             WinSetWindowText
 *             WinGetMsg
 *             WinDispatchMsg
 *
 *  Parameters: None
 *
 *  Returns:
 *          1 - if sucessful execution completed
 *          0 - if error
\****************************************************************/
int main (VOID)
{
   QMSG qmsg;          /* message structure */
   ULONG ulCtlData;      /* frame control data */

   hab = WinInitialize(0);

   if(!hab)
   {
      DosBeep(BEEP_WARN_FREQ, BEEP_WARN_DUR);
      return(RETURN_ERROR);
   }

   hmq = WinCreateMsgQueue(hab, 0);

   if(!hmq)
   {
      DosBeep(BEEP_WARN_FREQ, BEEP_WARN_DUR);
      WinTerminate(hab);
      return(RETURN_ERROR);
   }

   if(!Init())
   {
      MessageBox(HWND_DESKTOP, IDMSG_INITFAILED, MB_OK | MB_ERROR, TRUE);
      return(RETURN_ERROR);
   }

   /* NOTE:  clean up from here is handled by the DosExitList processing */

   /* create the main window */
   ulCtlData = FCF_STANDARD;

   hwndMainFrame = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE,
                                      (PVOID)&ulCtlData, (PSZ)szAppName,
                                      (PSZ)NULL, WS_VISIBLE,
                                      (HMODULE)NULL, IDR_MAIN,
                                      (PHWND)&hwndMain);

   if(!hwndMainFrame)
   {
      MessageBox(HWND_DESKTOP, IDMSG_MAINWINCREATEFAILED, MB_OK | MB_ERROR,
                  TRUE);
      return(RETURN_ERROR);
   }

   WinSetWindowText(hwndMainFrame, szAppName);

   InitHelp();

   /* Get/Dispatch Message loop                                  */
   while(WinGetMsg(hab, (PQMSG)&qmsg, 0, 0, 0))
       WinDispatchMsg(hab, (PQMSG)&qmsg);

   DestroyHelpInstance();

   return(RETURN_SUCCESS);

} /* End of main porcedure  */



/****************************************************************
 *  Name:   MainWndProc
 *
 *  Description : Window procedure for the main clent window.
 *
 *  Concepts : Processes the messages sent to the main client
 *             window.  This routine processes the basic
 *             messages all client windows should process and
 *             passes all others onto UserWndProc where the
 *             developer can process any others.
 *
 *  API's : None
 *
 * Parameters   : hwnd - Window handle to which message is addressed
 *                msg - Message type
 *                mp1 - First message parameter
 *                mp2 - Second message parameter
 *
 *  Returns:  Return values are determined by each message
 *
 ****************************************************************/
MRESULT EXPENTRY MainWndProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
      case WM_CREATE:
         return(InitMainWindow(hwnd, mp1, mp2));
         break;

      case WM_PAINT:
         MainPaint(hwnd);
         break;

      case WM_COMMAND:
         MainCommand(mp1, mp2);
         break;

      case WM_INITMENU:
         InitMenu(mp1, mp2);
         break;

      case HM_QUERY_KEYS_HELP:
         return (MRESULT)PANEL_HELPKEYS;     /* return id of key help panel */
         break;

    /*
     * Any messages not processed are passed on
     * to the user's window proc.  It is
     * responsible for passing any messages it
     * doesn't handle onto WinDefWindowProc()
     */
      default:
         return(UserWndProc(hwnd, msg, mp1, mp2));
         break;

   }
   return (MRESULT)0; /* all window procedures should return 0 as a default */

}   /* End of MainWndProc() */

/*********************************************************************
 *  Name:   MessageBox
 *
 *  Description : Message Box procedure
 *
 *  Concepts : Displays the warning message box with the message
 *             given in idMsg retrived from the message table;
 *             Called wherever an error occurs and a message should
 *             be displayed to the user
 *
 *  API's : WinLoadString
 *          WinLoadMessage
 *          WinAlarm
 *          WinMessageBox
 *
 *  Parameters :  hwndOwner - window handle of the owner of the
 *                            message box
 *                idMsg     - id of message to be retrieved from
 *                            resource file
 *                fsStyle   - message box style
 *                fBeep     - flag to sound alarm
 *
 *  Returns: SHORT
 *
\****************************************************************/
SHORT MessageBox(HWND hwndOwner, SHORT idMsg, SHORT fsStyle, BOOL fBeep)
{
   CHAR szText[MESSAGELEN];
   CHAR szTitle[MESSAGELEN];
   PSZ  pszTitle = NULL;

   if(!WinLoadMessage(hab, (HMODULE)NULL, idMsg, MESSAGELEN, (PSZ)szText))
   {
      WinAlarm(HWND_DESKTOP, WA_ERROR);
      return MBID_ERROR;
   }

   if(fBeep)
      WinAlarm(HWND_DESKTOP, WA_ERROR);

   if ( (fsStyle & MB_WARNING)                        /* use warning title ? */
       && WinLoadString(hab, (HMODULE)0, IDS_WARNING, MESSAGELEN, szTitle) )
         pszTitle = szTitle;
   else
      pszTitle = NULL; /* default to error title */

   return(WinMessageBox(HWND_DESKTOP, hwndOwner, szText, pszTitle,
          IDD_MSGBOX, fsStyle));

}   /* End of MessageBox() */

/****************************************************************\
 *  Name:   MainCommand
 *
 *  Purpose: Main window WM_COMMAND processing procedure
 *
 *  Concepts : Routine is called whenever a WM_COMMAND message is
 *             posted to the main window.  A switch statement
 *             branches on the id of the menu item that posted the
 *             message and the appropriate action for that item is
 *             taken.  Any menu ids that are not part of the
 *             standard menu set are passed onto the user defined
 *             WM_COMMAND processing procedure.
 *
 *  API's : WinPostMsg
 *
 *  Parameters :  mp1 - First message parameter
 *                mp2 - Second message parameter
 *
 *  Returns: VOID
 *
\****************************************************************/
VOID MainCommand(MPARAM mp1, MPARAM mp2)
{
   switch(SHORT1FROMMP(mp1))
   {
      case IDM_EXIT:
         WinPostMsg( hwndMain, WM_QUIT, NULL, NULL );
         break;

      case IDM_FILENEW:
         FileNew(mp2);
         break;

      case IDM_FILEOPEN:
         FileOpen(mp2);
         break;

      case IDM_FILESAVE:
         FileSave(mp2);
         break;

      case IDM_FILESAVEAS:
         FileSaveAs(mp2);
         break;

      case IDM_EDITUNDO:
         EditUndo(mp2);
         break;

      case IDM_EDITCUT:
         EditCut(mp2);
         break;

      case IDM_EDITCOPY:
         EditCopy(mp2);
          break;

      case IDM_EDITPASTE:
         EditPaste(mp2);
         break;

      case IDM_EDITCLEAR:
         EditClear(mp2);
         break;

      case IDM_HELPUSINGHELP:
         HelpUsingHelp(mp2);
         break;

      case IDM_HELPGENERAL:
         HelpGeneral(mp2);
         break;

      case IDM_HELPKEYS:
         HelpKeys(mp2);
         break;

      case IDM_HELPINDEX:
         HelpIndex(mp2);
         break;

      case IDM_HELPPRODINFO:
         HelpProdInfo(mp2);
         break;
    /*
     * User command processing routine is called
     * here so any ids not procecessed here can be
     * processed
     */
      default:
         UserCommand(mp1, mp2);
         break;
   }
}   /* MainCommand() */
