/*************************************************************************
*
*  File Name   : STY_FILE.C
*
*  Description : This module contains the code for the
*                WM_COMMAND messages posted by the standard
*                File menu.
*
*  Concepts    : Open, close, read, write of files.
*                File selection through the use of the
*                standard file dialog.
*
*  API's       : WinSendMsg
*                WinLoadString
*                WinFileDlg
*                DosOpen
*                DosQueryFileInfo
*                DosClose
*                DosRead
*                DosAllocMem
*                DosFreeMem
*                WinPostMsg
*                DosWrite
*                WinQueryTaskTitle
*                WinLoadString
*                WinSetWindowText
*                WinQueryWindowULong
*
*  Copyright (C) 1992, 1996 IBM Corporation
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
#define INCL_WINFRAMEMGR
#define INCL_WINWINDOWMGR
#define INCL_WINSWITCHLIST
#define INCL_WINMLE
#define INCL_WINSTDFILE

#include <os2.h>
#include <string.h>
#include "sty_main.h"
#include "sty_xtrn.h"
#include "sty_help.h"
#include "sty_dlg.h"

/*  Global variables                                                    */
CHAR szFullPath[CCHMAXPATH] = "";

/*********************************************************************
 *  Name: FileNew
 *
 *  Description : Processes the File menu's New item
 *
 *  Concepts : Called whenever New from the File menu is selected.
 *
 *  API's : WinSendMsg
 *
 *  Parameters : mp2 - Message parameter 2
 *
 *  Returns : VOID
 *
 ****************************************************************/
VOID FileNew( MPARAM mp2)
{
   /* Save file if changed                                              */
   if ((BOOL)WinSendMsg(hwndMLE, MLM_QUERYCHANGED, MPVOID, MPVOID))
   {
      SHORT sMsgBoxResponse = MessageBox(hwndMLE, IDMSG_FILECHANGED,
                                         MB_QUERY | MB_YESNOCANCEL, FALSE);

      if (sMsgBoxResponse == MBID_CANCEL)   /* if user cancels the New,  */
         return;                           /* then return               */
      else
         if (sMsgBoxResponse == MBID_YES)
            FileSave(MPVOID);
      /*
       * If sMsgBoxResponse == MBID_NO, continue with New File processing
       */
   }

   /*
    * disable redrawing of the MLE so the text doesn't "flash" when
    * the MLE is cleared
    */
   WinSendMsg(hwndMLE, MLM_DISABLEREFRESH, MPVOID, MPVOID); 

   /*
    * Clear the MLE by selecting all of the text and clearing it
    */
   WinSendMsg(hwndMLE, MLM_SETSEL, MPFROMSHORT(0),
             (MPARAM)WinSendMsg(hwndMLE, MLM_QUERYTEXTLENGTH, MPVOID, MPVOID));

   WinSendMsg(hwndMLE, MLM_CLEAR, MPVOID, MPVOID);

   /*
    * Reset the changed flag
    */
   WinSendMsg(hwndMLE, MLM_SETCHANGED, MPFROMSHORT((BOOL)FALSE), MPVOID);

   /*
    * Enable redrawing of the MLE
    */
   WinSendMsg(hwndMLE, MLM_ENABLEREFRESH, MPVOID, MPVOID);

   /*
    * Reset file name to NULL and update the main title bar
    */
   szFullPath[0] = 0;
   UpdateTitleText(hwndMainFrame);

   /*
    *  This routine currently doesn't use the mp2 parameter but
    *  it is referenced here to prevent an 'Unreferenced Parameter'
    *  warning at compile time.
    */
   mp2;
}   /* End of FileNew()                                                  */

/*********************************************************************
 *  Name: FileOpen
 *
 *  Description : Processes the File menu's Open item.
 *
 *  Concepts : Called whenever New from the File menu is
 *             selected.  Calls the standard file open
 *             dialog to get the file name.  The file name
 *             is passed onto DosOpen which returns the
 *             handle to the file.  The file input
 *             procedure is called and then the file
 *             handle is closed.
 *
 *  API's : WinLoadString
 *          WinFileDlg
 *          DosOpen
 *          DosQueryFileInfo
 *          DosClose
 *          DosRead
 *          DosAllocMem
 *          DosFreeMem
 *
 *  Parameters : mp2 - Message parameter 2
 *
 *  Returns : VOID
 *
 ****************************************************************/
VOID FileOpen( MPARAM mp2)
{
   FILEDLG fileDialog;
   CHAR szTitle[MESSAGELEN], szButton[MESSAGELEN];

   /* Save file if changed                                              */
   if ((BOOL)WinSendMsg(hwndMLE, MLM_QUERYCHANGED, MPVOID, MPVOID))
   {
      SHORT sMsgBoxResponse = MessageBox(hwndMLE, IDMSG_FILECHANGED,
           MB_QUERY | MB_YESNOCANCEL, FALSE);

      if (sMsgBoxResponse == MBID_CANCEL)   /* if user cancels the New,  */
         return;                           /* then return               */
      else
         if (sMsgBoxResponse == MBID_YES)
            FileSave(MPVOID);
      /*
       * If sMsgBoxResponse == MBID_NO, continue with Open File processing
       */
   }

   fileDialog.cbSize = sizeof(FILEDLG);

   if (!WinLoadString(hab, NULLHANDLE, IDS_OPENDLGTITLE, MESSAGELEN, szTitle))
   {
      MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR, TRUE);
      return;
   }

   if (!WinLoadString(hab, NULLHANDLE, IDS_OPENDLGBUTTON, MESSAGELEN, szButton))
   {
      MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR, TRUE);
      return;
   }

   fileDialog.pszTitle = szTitle;
   fileDialog.pszOKButton = szButton;
   fileDialog.ulUser = 0UL;
   fileDialog.fl = FDS_HELPBUTTON | FDS_CENTER | FDS_OPEN_DIALOG;
   fileDialog.pfnDlgProc = (PFNWP)OpenSaveFilterDlgProc;
   fileDialog.lReturn = 0L;
   fileDialog.lSRC = 0L;
   fileDialog.hMod = NULLHANDLE;
   fileDialog.usDlgId = IDD_FILEOPEN;
   fileDialog.x = 0;
   fileDialog.y = 0;

   if (!WinLoadString(hab, NULLHANDLE, IDS_FILEOPENEXT, CCHMAXPATH,
                     fileDialog.szFullFile))
   {
      MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR, TRUE);
      return;
   }

   fileDialog.pszIType = NULL;
   fileDialog.papszITypeList = NULL;
   fileDialog.pszIDrive = NULL;
   fileDialog.papszIDriveList = NULL;
   fileDialog.sEAType = 0;
   fileDialog.papszFQFilename = NULL;
   fileDialog.ulFQFCount = 0UL;

   /*
    *Call the standard file dialog to get the file
    */
   if (!WinFileDlg(HWND_DESKTOP, hwndMain, (PFILEDLG)&fileDialog))
      return;

   /*
    *  Upon sucessful return of a file, open it for reading
    */

   if (fileDialog.lReturn == ID_OK)
   {
      HFILE hfile;
      ULONG ulAction;
      FILESTATUS fileStatus;
      PVOID pvBuf;
      IPT iptOffset = 0;

      if ( DosOpen(fileDialog.szFullFile, &hfile, &ulAction, 0, FILE_NORMAL,
             FILE_OPEN, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, NULL))
      {
         MessageBox(hwndMain, IDMSG_CANNOTOPENINPUTFILE, MB_OK | MB_ERROR,
            FALSE);
         return;
      }
      /*
       * Copy file name into file name buffer
       */
      strcpy(szFullPath, fileDialog.szFullFile);

      /*
       * Get the length of the file
       */

      if (DosQueryFileInfo(hfile, 1, (PVOID)&fileStatus, sizeof(FILESTATUS)))
      {
         MessageBox(hwndMain, IDMSG_CANNOTGETFILEINFO, MB_OK | MB_ERROR,
              FALSE);
         DosClose(hfile);
         return;
      }
      /*
       * Allocate a buffer for the file
       */
      if (DosAllocMem((PPVOID)&pvBuf, (ULONG)fileStatus.cbFileAlloc, fALLOC))
      {
         MessageBox(hwndMain, IDMSG_CANNOTALLOCATEMEMORY, MB_OK | MB_ERROR,
                    FALSE);

         DosClose(hfile);
         return;
      }
      /*
       * Read in the file
       */
      if (DosRead(hfile, pvBuf, fileStatus.cbFileAlloc, &ulAction))
      {
         MessageBox(hwndMain, IDMSG_CANNOTREADFILE, MB_OK | MB_ERROR, FALSE);
         DosClose(hfile);
         return;
      }
      /*
       * Set the file into the MLE
       */
      WinSendMsg(hwndMLE, MLM_SETIMPORTEXPORT, MPFROMP((PBYTE)pvBuf),
                                MPFROMSHORT(fileStatus.cbFileAlloc));

      /*
       * Import to MLE starting at offset 0
       */

      WinSendMsg(hwndMLE, MLM_IMPORT, MPFROMP(&iptOffset),
                              MPFROMSHORT(fileStatus.cbFileAlloc));

      /*
       * Reset the changed flag
       */
      WinSendMsg(hwndMLE, MLM_SETCHANGED, MPFROMSHORT((BOOL)FALSE), NULL);

      DosFreeMem(pvBuf);

      DosClose(hfile);

      UpdateTitleText(hwndMainFrame);
   }
   /*
    *  This routine currently doesn't use the mp2 parameter but
    *  it is referenced here to prevent an 'Unreferenced Parameter'
    *  warning at compile time.
    */
   mp2;
}   /* End of FileOpen()                                                */


/*********************************************************************
 *  Name: FileSave
 *
 *  Description : Processes the File menu's Save item.
 *
 *  Concepts : Gets file name for untitled files, opens file
 *             writes file to disk, closes file.
 *
 *  API's : DosOpen
 *          DosClose
 *
 *  Parameters : mp2 - Message parameter 2
 *
 *  Returns : VOID
 *
 ****************************************************************/
VOID FileSave( MPARAM mp2)
{
   HFILE hfile;
   ULONG ulAction;
   /*
    * If the file currently is untitled, we will need to get a file
    * name from the user before we can open the file.  Getting a
    * file name is normally done during the FileSaveAs operation
    * so we will treat this save as a SaveAs and call FileSaveAs().
    * If the file is titled, then we save the file.
    *
    * NOTE:  This routine will be called by FileSaveAs(), but only
    *  after a valid file name has been obtained.  So, FileSaveAs()
    *  will not be called again from this routine.
    */
   if (szFullPath[0] == 0)
   {
      FileSaveAs(mp2);
      return;
   }

   /*
    * Open the file
    */
   if ( DosOpen(szFullPath, &hfile, &ulAction, 0, FILE_NORMAL, FILE_OPEN |
       FILE_CREATE, OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE, NULL))
   {
      MessageBox(hwndMain, IDMSG_CANNOTOPENOUTPUTFILE, MB_OK|MB_ERROR, FALSE);
      return;
   }

   WriteFileToDisk(hfile);

   DosClose(hfile);
   return;
}   /*  End of FileSave()                                               */

/*********************************************************************
 *  Name: FileSaveAs
 *
 *  Description : Processes the File menu's Save As item.
 *
 *  Concepts : Called whenever Save As from the File menu is
 *             selected.
 *
 *  API's : DosOpen
 *          DosClose
 *
 *  Parameters : mp2 - Message parameter 2
 *
 *  Returns : VOID
 *
 ********************************************************************/
VOID FileSaveAs( MPARAM mp2)
{
   /*
    * Infinite loop until we break out of it
    */
   while(TRUE)
   {
      HFILE hfile;
      ULONG ulAction;
   /*
    * If no file name, then get a file name
    */
      if (!GetFileName())
         return;
   /*
    * See if the file exists.  If it does, then confirm that the
    * user wants to overwrite it.  If he doesn't, then get a new
    * file name
    */
      if ( DosOpen(szFullPath,         /* file name from, GetFileName()    */
                   &hfile,             /* handle of opened file            */
                   &ulAction,
                   0,
                   FILE_NORMAL,
                   FILE_OPEN | FILE_CREATE,
                   OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE,
                   NULL))
     {
        MessageBox(hwndMain, IDMSG_CANNOTOPENOUTPUTFILE, MB_OK|MB_ERROR, FALSE);
        return;
     }
     else
        DosClose(hfile);

     /*
      * If file exists, ask if we want to overwrite it
      */
     if (ulAction == FILE_EXISTED)
     {
        SHORT sMsgBoxResponse = MessageBox(hwndMLE, IDMSG_OVERWRITEFILE, 
                                           MB_QUERY | MB_YESNOCANCEL, FALSE);

       if (sMsgBoxResponse == MBID_CANCEL)
          return;

        if (sMsgBoxResponse == MBID_YES)
           break;
       /*
        * If user selected no, repeat the sequence
        */
      }
      else
         break;                      /* file didn't exist               */
   }                                 /* while(TRUE)                     */
   UpdateTitleText(hwndMainFrame);
   /*
    * Now that we have a valid file name, save the file.  This is
    * normally done under the File Save function so we can just
    * call the FileSave() function here.  Note that FileSave() will
    * not call FileSaveAs() back since there is a valid file name
    */
    FileSave(mp2);
    return;
}       /* End of FileSaveAs()                                          */

/****************************************************************
 *  Name: WriteFileToDisk
 *
 *  Description : Writes the current file to the file in szFileName
 *
 *  Concepts : Called from FileSave and FileSaveAs when a file is
 *             to be saved to disk.  Routine uses the file handle
 *             specified and gets the text from the MLE and
 *             writes the text to the file.
 *
 *  API's : WinSendmsg
 *          DosAllocMem
 *          DosWrite
 *          DosFreeMem
 *
 *  Parameters : hfile - handle of file to save
 *
 *  Returns: Void
 *
 ****************************************************************/
VOID WriteFileToDisk( HFILE hfile)
{
   ULONG ulWrite;
   PVOID pvBuf;
   ULONG ulOffset;             /* offset in buffer                    */
   ULONG cbExport;             /* # of bytes to export from the MLE   */
   /*
    * Get the length of the file
    */
   ULONG ulFileLen = (ULONG)WinSendMsg(hwndMLE, MLM_QUERYTEXTLENGTH, MPVOID, MPVOID);

   if (!ulFileLen)
      return;

   /*
    * Allocate a buffer for the file
    */
   if (DosAllocMem((PPVOID) &pvBuf, ulFileLen, fALLOC))
   {
      MessageBox(hwndMLE, IDMSG_CANNOTALLOCATEMEMORY, MB_OK | MB_ERROR,
           FALSE);
      return;
   }

   /*
    * Get the file from the MLE
    */
   cbExport = ulFileLen;
   WinSendMsg(hwndMLE, MLM_SETIMPORTEXPORT, MPFROMP((PBYTE)pvBuf),
        MPFROMLONG(cbExport));

   /*
   * Export MLE starting at offset 0
   */
   ulOffset = 0UL;
   WinSendMsg(hwndMLE, MLM_EXPORT, MPFROMP(&ulOffset), MPFROMLONG(&cbExport));

   /*
    * Write the file
    */
   if (DosWrite(hfile, pvBuf, ulFileLen, &ulWrite))
   {
      MessageBox(hwndMLE, IDMSG_CANNOTWRITETOFILE, MB_OK | MB_ERROR, FALSE);
      return;
   }

   /*
    * Reset the changed flag
    */
   WinSendMsg(hwndMLE, MLM_SETCHANGED, MPFROMSHORT((BOOL)FALSE), MPVOID);

   DosFreeMem(pvBuf);
}          /*   End of WriteFileToDisk()                                */

/*********************************************************************
 *  Name : GetFileName
 *
 *  Description : Gets the name of the save file.
 *
 *  Concepts : Called when the user is needs to supply a name
 *             for the file to be saved.  Calls the standard
 *             file open dialog to get the file name.
 *
 *  API's : WinLoadString
 *          WinFileDlg
 *
 *  Parameters : None
 *
 *  Returns: Void
 *
 ****************************************************************/
BOOL GetFileName(VOID)
{
   FILEDLG fileDialog;
   CHAR szTitle[MESSAGELEN], szButton[MESSAGELEN];

   if (!WinLoadString(hab, NULLHANDLE, IDS_SAVEDLGTITLE, MESSAGELEN, szTitle))
   {
      MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR, TRUE);
      return FALSE;
   }

   if (!WinLoadString(hab, NULLHANDLE, IDS_SAVEDLGBUTTON, MESSAGELEN, szButton))
   {
      MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR, TRUE);
      return FALSE;
   }

   fileDialog.cbSize = sizeof(FILEDLG);       /* Size of FILEDLG structure. */
   fileDialog.fl = FDS_HELPBUTTON |   /* FDS_ flags. Alter behavior of dlg. */
                    FDS_CENTER | FDS_SAVEAS_DIALOG | FDS_ENABLEFILELB;
   fileDialog.ulUser = 0UL;                          /* User defined field. */
   fileDialog.lReturn = 0L;           /* Result code from dialog dismissal. */
   fileDialog.lSRC = 0L;                             /* System return code. */
   fileDialog.pszTitle = szTitle;        /* String to display in title bar. */
   fileDialog.pszOKButton = szButton;    /* String to display in OK button. */
                                      /* Entry point to custom dialog proc. */
   fileDialog.pfnDlgProc = (PFNWP)OpenSaveFilterDlgProc;
   fileDialog.pszIType = NULL;         /* Pointer to string containing      */
                                       /*   initial EA type filter. Type    */
                                       /*   does not have to exist in list. */
   fileDialog.papszITypeList = NULL;   /* Pointer to table of pointers that */
                                       /*   point to null terminated Type   */
                                       /*   strings. End of table is marked */
                                       /*   by a NULL pointer.              */
   fileDialog.pszIDrive = NULL;          /* Pointer to string containing    */
                                         /*   initial drive. Drive does not */
                                         /*   have to exist in drive list.  */
   fileDialog.papszIDriveList = NULL;  /* Pointer to table of pointers that */
                                       /*   point to null terminated Drive  */
                                       /*   strings. End of table is marked */
                                                      /* by a NULL pointer. */
   fileDialog.hMod = (HMODULE)0;            /* Custom File Dialog template. */
   strcpy(fileDialog.szFullFile, szFullPath); /* Initial or selected fully  */
                                              /*   qualified path and file. */
   fileDialog.papszFQFilename = NULL;  /* Pointer to table of pointers that */
                                       /*   point to null terminated FQFname*/
                                       /*   strings. End of table is marked */
                                       /*   by a NULL pointer.              */
   fileDialog.ulFQFCount = 0UL;                 /* Number of files selected */
   fileDialog.usDlgId = IDD_FILESAVE;                  /* Custom dialog id. */
   fileDialog.x = 0;                          /* X coordinate of the dialog */
   fileDialog.y = 0;                          /* Y coordinate of the dialog */
   fileDialog.sEAType = 0;                      /* Selected file's EA Type. */


   /*
    * Get the file
    */
   if (!WinFileDlg(HWND_DESKTOP, hwndMLE, (PFILEDLG)&fileDialog))
      return FALSE;

   if (fileDialog.lReturn != ID_OK)
      return FALSE;

   /*
    * Copy file name and path returned into buffers
    */
   strcpy(szFullPath, fileDialog.szFullFile);

   return TRUE;

}   /* End of GetFileName() */


/*********************************************************************
 *  Name : UpdateTitleText
 *
 *  Description : Updates the text in the main window's title bar
 *                to display the app name, followed by the
 *                separator, followed by the file name
 *
 *  Concepts : Called at init time and when the text file is
 *             changed gets the program name, appends the
 *             separator, and appends the file name.
 *
 *  API's : WinQueryTaskTitle
 *          WinLoadString
 *          WinSetWindowText
 *
 *  Parameters : hwnd - handle of the main window
 *
 *  Returns : Void
 *
 ****************************************************************/
VOID UpdateTitleText(HWND hwnd)
{
   CHAR szBuf[CCHMAXPATH + MAXNAMEL];
   CHAR szSeparator[TITLESEPARATORLEN + 1];
   PSZ pszTitle;

   WinQueryTaskTitle(0UL, szBuf, MAXNAMEL);

   WinLoadString(hab, NULLHANDLE, IDS_TITLEBARSEPARATOR, TITLESEPARATORLEN,
                  szSeparator);

   strcat(szBuf, szSeparator);

   if (szFullPath[0] == '\0')
      pszTitle = szUntitled;
   else
      pszTitle = szFullPath;

   strcat(szBuf, pszTitle);

   /***********************************************************************
   * If title is longer that maximum allowable, show the most significant *
   * portion of it (filename end of path).                                *
   ***********************************************************************/
   if (strlen(szBuf) + 1 > MAXNAMEL)
      pszTitle = &szBuf[strlen(szBuf) - MAXNAMEL + 1];
   else
      pszTitle = szBuf;

   WinSetWindowText(WinWindowFromID(hwnd, FID_TITLEBAR), pszTitle);
   return;
}          /*  End of UpdateTitleText()                                      */


/***********************************************************
 * Name         : OpenSaveFilterProc
 *
 * Description  : Procedure to handle wm_help messages for
 *                the file open/save dialog.
 *
 * Concepts     : This routine handles the WM_HELP messages for
 *                the dialog boxs created with the WinFileDlg
 *                Checks the flags used on the call to determine
 *                the correct help panel to display.
 *
 * API's        : WinQueryWIndowULong
 *                WinSendMessage
 *
 * Parameters   : hwnd - Window handle to which message is addressed
 *                msg - Message type
 *                mp1 - First message parameter
 *                mp2 - Second message parameter
 *
 *  Returns : Dependent upon message sent
 **************************************************************/
MRESULT APIENTRY OpenSaveFilterDlgProc(HWND hwnd, USHORT msg,
                                       MPARAM mp1, MPARAM mp2)
{
   if(msg == WM_HELP)
   {
      /*
       * Get a pointer to the file dialog structure.
       */
      PFILEDLG pOpenSaveFileDlgStruct = (PFILEDLG)WinQueryWindowULong(hwnd,
                                                                     QWL_USER);

      /*
       * If this is an the File Open dialog, display the file open help
       * panel.
       */
      if (pOpenSaveFileDlgStruct->fl & FDS_OPEN_DIALOG)
      {
         DisplayHelpPanel(PANEL_FILEOPEN);
         return (MRESULT)FALSE ;
      }

      /*
       * If this is an the File Save or Save As dialog, display the file
       * Save As help panel.
       */

      if (pOpenSaveFileDlgStruct->fl & FDS_SAVEAS_DIALOG)
      {
         DisplayHelpPanel(PANEL_FILESAVEAS);
         return (MRESULT)FALSE ;
      }
   }
   return WinDefFileDlgProc( hwnd, msg, mp1, mp2 );
}                     /* End of OpenSaveFilterDlgProc  */

