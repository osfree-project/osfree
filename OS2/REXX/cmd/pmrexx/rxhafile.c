/* static char *SCCSID = "@(#)rxhafile.c	6.4 92/01/10";               */
/*********************  START OF SPECIFICATIONS  ******************************/
/*                                                                            */
/*   SOURCE FILE NAME:  RXHAFILE.C                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  File routines for PMREXX.                             */
/*                                                                            */
/*   COPYRIGHT:         IBM Corporation 1991                                  */
/*                                                                            */
/*   STATUS:            Version 2.00                                          */
/*                                                                            */
/*   FUNCTION:                                                                */
/*           Process the file requests from the user, to allow them to save   */
/*           their output to a file.                                          */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES:                                                         */
/*          This function has dependencies on the following                   */
/*      files for compilation.                                                */
/*          os2.h      - All OS/2 specific definitions                        */
/*          rxhdluse.h - necessary equates used by the file procedures        */
/*          pmrexx.h   - global typedefs, defines and prototypes              */
/*          pmrexxio.h - diaglog control defines                              */
/*                                                                            */
/*   PROCEDURES:                                                              */
/*      CheckDosError    - see if a dos error occurred and take action        */
/*      ErrorNotifyProc  - Display error message box                          */
/*      MapButtonText    - Map Text to Buttons                                */
/*      OpenFile         - Open file with given mode                          */
/*      SaveQuit         - Bring up Save/Quit/Cancel Dialog Box               */
/*      SaveQuitDlgProc  - Handle messages for Save/Quit dialog               */
/*      WriteFile        - Write contents of window to given file             */
/*                                                                            */
/*                                                                            */
/***********************  END OF SPECIFICATIONS  ******************************/
/******************************************************************************/
/* Define Constants for additional includes                                   */
/******************************************************************************/

#define  INCL_WINCOUNTRY               /* Code page support                   */
#define  INCL_WINDIALOGS               /* Dialog boxes                        */
#define  INCL_WINENTRYFIELDS           /* Entry fields                        */
#define  INCL_WINFRAMEMGR              /* Frame manager                       */
#define  INCL_WINHELP                  /* Help manager definitions            */
#define  INCL_WININPUT                 /* Mouse and keyboard input            */
#define  INCL_WINMESSAGEMGR            /* Message management                  */
#define  INCL_WINSHELLDATA             /* Profile calls                       */
#define  INCL_WINSWITCHLIST            /* Task list calls                     */
#define  INCL_WINTIMER                 /* Timer routines                      */
#define  INCL_WINWINDOWMGR             /* General window management           */
#define  INCL_WINMENUS                 /* Menu controls                       */
#define  INCL_WINMLE                   /* Multiple line edit fields           */
#define  INCL_WINPOINTERS              /* Mouse pointers                      */
#define  INCL_WINSTDFILE               /* Standard File dialog                */
#define  INCL_WINSYS                   /* Sytem Values (and colors)           */
#define  INCL_WINSTATICS               /* Static Controls                     */
#define  INCL_WINMESSAGEMGR            /* Win Message functions               */
#define  INCL_WINSTDFONT               /* Standard Font dialog                */
#define  INCL_DOSQUEUES                /* Queue Support                       */
#define  INCL_DOSERRORS                /* OS/2 Errors                         */
#define  INCL_DOSMISC                  /* for DosError function               */
#define  INCL_DOSFILEMGR               /* File Managment                      */
#define  INCL_DOSSEMAPHORES            /* OS/2 semaphores                     */
#define  INCL_RXSYSEXIT                /* REXX system exits                   */

/******************************************************************************/
/* Include required header files                                              */
/******************************************************************************/

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rexxsaa.h>

/******************************************************************************/
/* Include Application Header Files                                           */
/******************************************************************************/

#include "rxhdluse.h"
#include "pmrexx.h"
#include "pmrexxio.h"

/******************************************************************************/
/* Declare character arrays for buttons                                       */
/******************************************************************************/

static char ok[BUTTON_TEXT_LENGTH];

static char cancel[BUTTON_TEXT_LENGTH];
static char retry[BUTTON_TEXT_LENGTH];
static char abortstr[BUTTON_TEXT_LENGTH];
static char ignore[BUTTON_TEXT_LENGTH];
static char yes[BUTTON_TEXT_LENGTH];
static char no[BUTTON_TEXT_LENGTH];
static char enter[BUTTON_TEXT_LENGTH];

static SHORT stringSize = BUTTON_TEXT_LENGTH;/* Define string size            */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: OpenFile                                                */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Open a file with the indicated mode.                         */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Open a file and return the file handle and the action taken  */
/*               to the caller.                                               */
/*                                                                            */
/*   NOTES: None.                                                             */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: int OpenFile                                                 */
/*                                                                            */
/*   INPUT:                                                                   */
/*      PSZ pszFileName      - ptr to ascii string containing file to open    */
/*      PHFILE pFileHandle   - ptr to file handle (returned)                  */
/*      PULONG  pActionTaken - ptr to action taken (returned)                 */
/*      ULONG  iMode         - indicates how to open file                     */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       Returns 0                                                            */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*       Returns ERROR_OPEN_FAILED.                                           */
/*                                                                            */
/*   EFFECTS: None.                                                           */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        None.                                                               */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        DosOpen                                                             */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

ULONG  OpenFile(PSZ pszFileName,PHFILE pFileHandle,PULONG  pActionTaken,ULONG
                iMode)
{
  ULONG  DosRC;                        /* Return Code                         */
  ULONG  openflag,openmode;            /* bitmaps for DosOpen                 */

/******************************************************************************/
/* Set openflag and openmode based on iMode                                   */
/******************************************************************************/

  switch (iMode) {
    case 0 :                           /* Read-only mode                      */
      openflag = OPEN_ACTION_OPEN_IF_EXISTS;/* Open if exists, else fail      */
      openmode = OPEN_SHARE_DENYWRITE; /* R/O access, deny Write              */
      break;
    case 1 :                           /* Write-only mode, new file           */
      openflag = OPEN_ACTION_CREATE_IF_NEW;/* Fail if exists                  */
      openmode = OPEN_FLAGS_WRITE_THROUGH|OPEN_SHARE_DENYWRITE|
          OPEN_ACCESS_WRITEONLY;       /* R/W access, deny Write Synchronous  */
                                       /* Write                               */
      break;
    case 2 :                           /* Write-only mode, replace            */
      openflag = OPEN_ACTION_REPLACE_IF_EXISTS|OPEN_ACTION_CREATE_IF_NEW;/*   */
                                       /* Replace if exists                   */
      openmode = OPEN_FLAGS_WRITE_THROUGH|OPEN_SHARE_DENYWRITE|
          OPEN_ACCESS_WRITEONLY;       /* R/W access, deny Write Synchronous  */
                                       /* Write                               */
      break;
    default  :                         /* Default processing                  */
      return (ERROR_OPEN_FAILED);
  }                                    /* endswitch                           */

/******************************************************************************/
/* Call DosOpen to open the file and return the return code                   */
/* from the DosOpen call.                                                     */
/******************************************************************************/

  *pActionTaken = 0;
  DosRC = DosOpen(pszFileName,         /* File name                           */
                  pFileHandle,         /* File handle                         */
                  pActionTaken,        /* Action taken                        */
                  0L,                  /* File's new size                     */
                  0,                   /* File attribute bits                 */
                  openflag,            /* Open flag                           */
                  openmode,            /* Open mode                           */
                  0L                   /* Reserved, must be 0                 */
      );
  return (DosRC);
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: WriteFile                                               */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Write contents of the window to a file.                      */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Write contents of the window to a file.                      */
/*                                                                            */
/*                                                                            */
/*   NOTES: None.                                                             */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: BOOL WriteFile                                               */
/*                                                                            */
/*   INPUT:                                                                   */
/*      HWND hwnd       - window handle                                       */
/*      PSZ pszFileName - file name to write to                               */
/*      PSZ pszPgmName  - Rexx program executing                              */
/*      BOOL bConfirm  - indicates if fail or replace on write (TRUE => FAIL) */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       Returns TRUE.                                                        */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*       Returns FALSE.                                                       */
/*                                                                            */
/*   EFFECTS: None.                                                           */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES: OpenFile, CheckDosError                                     */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        DosWrite                                                            */
/*        WinLoadString,                                                      */
/*        WinMessageBox,                                                      */
/*        WinSendMsg,                                                         */
/*        WinWindowFromID,                                                    */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

BOOL WriteFile(HAB  hab,               /* Window Anchor Block                 */
               HWND hwnd,              /* Window Handle                       */
               PSZ  pszFileName,       /* File Name                           */
               PSZ  pszPgmName,        /* Rexx program name                   */
               BOOL bConfirm)          /* Confirm                             */
{

  ULONG ulFileSize;                    /* File Size                           */
  ULONG ulFmtSize;                     /* Format Size                         */
  HFILE hFile;                         /* File Handle                         */
  ULONG actionTaken;                   /* Action Taken                        */
  CHAR *buffer;                        /* Pointer to buffer                   */
  ULONG DosRC;                         /* OpenFile Return Code                */
  ULONG UserResponse;                  /* User Response                       */
  ULONG iMode;                         /* OpenFile mode                       */
  ULONG textLen;                       /* Text Length                         */
  LONG  bytesCopied = 0;               /* Number of bytes copied              */
  ULONG usWriteSize;                   /* Write Size                          */

  DosError(FERR_DISABLEHARDERR);       /* Disable exception popups            */

                                       /* Determine filesize                  */
  ulFileSize = (LONG)WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                                MLM_QUERYTEXTLENGTH,
                                (MPARAM)0, (MPARAM)0);

  ulFmtSize = (LONG)WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                               MLM_QUERYFORMATTEXTLENGTH, 0,
                               (MPARAM)ulFileSize);

   /***************************************************************************/
   /* If file size is greater or equal to limit set text length to            */
   /* FFFE hex otherwise set it to the text format size.                      */
   /***************************************************************************/

  textLen = (ulFileSize >= (ULONG)0xffff)?0xfffe:ulFmtSize;
                                       /* Allocate buffer                     */
  DosRC = DosAllocMem((PVOID)&buffer, textLen+1, PAG_COMMIT+PAG_WRITE);

   /***************************************************************************/
   /* Determine if a DosError occurred in allocating the buffer               */
   /* and take appropriate action                                             */
   /***************************************************************************/

  if ((UserResponse = CheckDosError(hab, hwnd, DosRC, pszPgmName))
      == MBID_CANCEL)
    return (FALSE);

                                       /* Import text from window to buffer   */
  WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
             MLM_SETIMPORTEXPORT, (MPARAM)buffer, MPFROMSHORT(textLen));

  iMode = (bConfirm)?1:2;              /* Set the file mode and user response */
  UserResponse = MBID_RETRY;

   /***************************************************************************/
   /* Try to open the file as long as user response is set to retry           */
   /***************************************************************************/

  do {
    DosRC = OpenFile((PSZ)pszFileName, &hFile, &actionTaken, iMode);

                                       /* If open fails, print message        */
    if ((bConfirm) && (DosRC == ERROR_OPEN_FAILED)) {

      CHAR pszOverWrite[MSG_SIZE];     /* Declare variables for messages      */
      CHAR pszOverWriteMsg[MSG_SIZE];
      CHAR pszWarning[MSG_SIZE];
      CHAR pszWarningMsg[MSG_SIZE];

                                       /* Load string for overwrite prompt    */
      WinLoadString(hab, (HMODULE)NULL, RXHD_EXISTS,
                    sizeof(pszOverWrite), pszOverWrite);

         /*********************************************************************/
         /* Copy overwrite prompt and filename to pszOverWriteMsg             */
         /*********************************************************************/

      sprintf(pszOverWriteMsg, pszOverWrite, pszFileName);

                                       /* Load Warning message                */
      WinLoadString(hab, (HMODULE)NULL, RXH_WARNING,
                    sizeof(pszWarning), pszWarning);

         /*********************************************************************/
         /* Copy warning prompt and program name to pszWarningMsg             */
         /*********************************************************************/

      sprintf(pszWarningMsg, pszWarning, pszPgmName);

         /*********************************************************************/
         /* Create, display, and operate the message box window               */
         /*********************************************************************/

      if (WinMessageBox(HWND_DESKTOP, hwnd, pszOverWriteMsg, pszWarningMsg, 0,
          MB_YESNO|MB_CUAWARNING|MB_MOVEABLE) == MBID_YES) {

        bConfirm = FALSE;              /* yes, open specified file            */
        DosRC = OpenFile((PSZ)pszFileName, &hFile, &actionTaken, 2);
      }

      else                             /* otherwise return failure            */
        return (FALSE);
    }
                                       /* check and handle errors             */
    if ((UserResponse = CheckDosError(hab, hwnd, DosRC, pszPgmName)) ==
        MBID_CANCEL)
      return (FALSE);
  }

     /*************************************************************************/
     /* While the user selects retry, attempt to write to file                */
     /* from the buffer                                                       */
     /*************************************************************************/

  while (UserResponse == MBID_RETRY);

  do {

       /***********************************************************************/
       /* Get the size of the date to be written from the window              */
       /***********************************************************************/
    usWriteSize = (ULONG )WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                                     MLM_EXPORT, &bytesCopied, &ulFileSize);

    do {

                                       /* Write the buffer to the file        */
      DosRC = DosWrite(hFile, buffer, usWriteSize, &actionTaken);

         /*********************************************************************/
         /* If an error occurs, handle the error. Return if canceled          */
         /*********************************************************************/
      if (DosRC) {

        if ((UserResponse = CheckDosError(hab, hwnd, DosRC, pszPgmName)) ==
            MBID_CANCEL)
          return (FALSE);
      }
    }

    while (UserResponse == MBID_RETRY);/* Do-While loop for retry             */
  }

  while (ulFileSize);                  /* Do-While loop for file write        */

                                       /* Write EOF marker and do cleanup     */
  DosRC = DosWrite(hFile, "\032", 1, &actionTaken);

  DosClose(hFile);
  DosFreeMem(buffer);

  DosError(FERR_ENABLEHARDERR);        /* Enable exception popups and return  */
  return (TRUE);
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: CheckDosError                                           */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Determine if a DosError occurred, and decide what action     */
/*               to take.                                                     */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Check the return code and see what sort of action should be  */
/*               taken.                                                       */
/*                                                                            */
/*   NOTES:      MB_?????? constants are used to define which buttons are     */
/*               to be displayed.  This is a fulblown Dialog box , not a      */
/*               message box, so these values mean nothing to the dialog      */
/*               manager here.  We just use them because they are convenient  */
/*               and we use the same text that they would have in a message   */
/*               box.                                                         */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: ULONG  CheckDosError                                         */
/*                                                                            */
/*   INPUT:                                                                   */
/*      HAB  hab  - PM anchor block                                           */
/*      HWND hClientWnd - window handle                                       */
/*      ULONG  DosError    - error code from DosCalls                         */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       Returns TRUE                                                         */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*       Returns FALSE                                                        */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        DosErrClass                                                         */
/*        WinDlgBox                                                           */
/*        WinLoadString                                                       */
/*        WinQueryWindowULong                                                 */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

ULONG  CheckDosError(HAB hab, HWND hClientWnd,ULONG  DosError,
    PSZ  pszPgmName)
{
  CHAR pszMsg[MSG_SIZE];               /* string for error message            */
  ULONG  usButtons;                    /* bit-map of which buttons to display */
  USHORT usTitle;                      /* Number of title to put on error box */
  CHAR pszTitle[TITLE_SIZE];           /* string of title to put on error box */
  CHAR pszTitleText[TITLE_SIZE];       /* Formatted title text                */
  CHAR pszFormattedErr[MSG_SIZE];      /* contains string version of DosError */
  CHAR pszLocus[MSG_SIZE];             /* string for what device is causing   */
                                       /* error                               */
  CHAR pszClass[MSG_SIZE];             /* what is the class of the error      */
  ULONG  ErrClass;                     /* code for class of error             */
  ULONG  ErrAction;                    /* code for action to take on error    */
  ULONG  ErrLocus;                     /* code for what device is causing the */
                                       /* prob.                               */
  RXHA_ERR rxhaErr;                    /* passed indirectly to ErrorNotifyProc*/
  USHORT usMsgNum;                     /* ID of message string to diplay      */

   /***************************************************************************/
   /* Find out which dos error has occured                                    */
   /***************************************************************************/

  if (!DosError)
    return (MBID_OK);

  else {
    WinSetPointer(HWND_DESKTOP,        /* End wait issued in INITDLG of dialog*/
        WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE));

    switch (DosError) {
      case  ERROR_NOT_READY :          /* Drive is not ready                  */
        usMsgNum = RXH_NOT_READY;
        break;
      case  ERROR_PATH_NOT_FOUND :     /* Path could not be found             */
        usMsgNum = RXH_BAD_PATH;
        break;
      case  ERROR_TOO_MANY_OPEN_FILES :/* No more file handles                */
        usMsgNum = RXH_TOO_MANY_OPEN;
        break;
      case  ERROR_ACCESS_DENIED :      /* System lock in effect               */
        usMsgNum = RXH_ACCESS_DENIED;
        break;
      case  ERROR_INVALID_ACCESS :     /* Bad access attempt                  */
        usMsgNum = RXH_INV_ACCESS;
        break;
      case  ERROR_NOT_DOS_DISK :       /* Unrecognized device                 */
        usMsgNum = RXH_NOT_DISK;
        break;
      case  ERROR_SHARING_VIOLATION :  /* Appl. lock on file                  */
        usMsgNum = RXH_SHARE_VIOL;
        break;
      case  ERROR_SHARING_BUFFER_EXCEEDED :/* Sharing buffer exceeded         */
        usMsgNum = RXH_SHARE_BUFF_EXC;
        break;
      case  ERROR_CANNOT_MAKE :        /* Cannot create the file              */
        usMsgNum = RXH_CANNOT_MAKE;
        break;
      case  ERROR_DRIVE_LOCKED :       /* Drive is locked                     */
        usMsgNum = RXH_DRIVE_LOCKED;
        break;
      case  ERROR_DISK_FULL :          /* No space left                       */
        usMsgNum = RXH_DISK_FULL;
        break;
      case  ERROR_NOT_ENOUGH_MEMORY :  /* Can't allocate memory               */
        usMsgNum = RXH_OUTOF_MEM;
        break;
      case  ERROR_WRITE_PROTECT :      /* Disk is read-only                   */
        usMsgNum = RXH_WRITE_PROT;
        break;
      case  ERROR_FILENAME_EXCED_RANGE :/* Filename is too long               */
        usMsgNum = RXH_NAME_LONG;
        break;
      case  ERROR_INVALID_NAME :       /* Illegal filename                    */
        usMsgNum = RXH_BAD_NAME;
        break;
      default  :
        usMsgNum = RXH_APP_ERROR;      /* generic messages...                 */
        break;
    }

      /************************************************************************/
      /* Classify error, and set message box buttons accordingly              */
      /************************************************************************/

    usTitle = RXH_WARNING;             /* set default Icon and title          */
    rxhaErr.usIconStyle = SPTR_ICONWARNING;

    DosErrClass(DosError, &ErrClass, &ErrAction, &ErrLocus);

    switch (ErrAction) {
      case  ERRACT_RETRY :             /* retry immediatly                    */
      case  ERRACT_DLYRET :            /* Delay and retry                     */
      case  ERRACT_USER :              /* Bad user input - get new values     */
      case  ERRACT_INTRET :            /* Retry after user intervention       */
        usButtons = MB_RETRYCANCEL;    /* Retry and Cancel Buttons            */
        break;

      case  ERRACT_ABORT :             /* Terminate in an ordely Fashion      */
      case  ERRACT_PANIC :             /* Terminate immediatly                */
        usButtons = MB_CANCEL;         /* Show a Cancel Button                */
        usTitle = RXH_CRITICAL;        /* Critical Title on dlg box           */
        rxhaErr.usIconStyle = SPTR_ICONERROR;/* Error Icon                    */
        break;

      case  ERRACT_IGNORE :            /* Ignore Error                        */
        usButtons = MB_ABORTRETRYIGNORE;/* abort retry and ignore buttons     */
        usTitle = RXH_WARNING;         /* title with Warning                  */
        rxhaErr.usIconStyle = SPTR_ICONWARNING;/* warning Icon                */
        break;

      default  :
        usButtons = 0;
        break;
    }

   /***************************************************************************/
   /* Load the message and title from application resources                   */
   /***************************************************************************/

    WinLoadString(hab, (HMODULE)NULL, usTitle, sizeof(pszTitle), pszTitle);

    WinLoadString(hab, (HMODULE)NULL, usMsgNum, sizeof(pszMsg), pszMsg);

    WinLoadString(hab, (HMODULE)NULL, (USHORT)(RXH_ERRL_START+ErrLocus),
        sizeof(pszLocus), pszLocus);

    WinLoadString(hab, (HMODULE)NULL, (USHORT)(RXH_ERRC_START+ErrClass),
        sizeof(pszClass), pszClass);

   /***************************************************************************/
   /* Display message to user; return choice to caller                        */
   /***************************************************************************/

    sprintf(pszTitleText, pszTitle, pszPgmName);/*  format title              */
    rxhaErr.Title = pszTitleText;      /* pass to ErrorNotifyProc             */
    sprintf(pszFormattedErr, "%i", DosError);/* error number                  */
    rxhaErr.RetCode = pszFormattedErr;
    rxhaErr.Locus = pszLocus;
    rxhaErr.Class = pszClass;
    rxhaErr.message = pszMsg;
    rxhaErr.buttonStyle = usButtons;
    rxhaErr.action = 1;
    rxhaErr.hab = hab;
    {
      ULONG  retVal;

                                       /* Start dialog box                    */
      retVal = WinDlgBox(HWND_DESKTOP, hClientWnd, (PFNWP)ErrorNotifyProc,
                         (HMODULE)NULL, RXHD_ERROR, &rxhaErr);

      return (rxhaErr.action);         /* return action code                  */
    }
  }                                    /* end: error occurred                 */
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: ErrorNotifyProc                                         */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Display error message box.                                   */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Creates and handles messages for error box.                  */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: MRESULT EXPENTRY ErrorNotifyProc                   */
/*                                                                            */
/*   INPUT:                                                                   */
/*      HWND hwnd      - window handle                                        */
/*      ULONG  msg     - message to process                                   */
/*      MPARAM lParam1 - parameter 1                                          */
/*      MPARAM lParam2 - parameter 2                                          */
/*                                                                            */
/*   The messages handled are:                                                */
/*      WM_INITDLG - initialize the error box                                 */
/*      WM_CLOSE   - close dialog box                                         */
/*      WM_COMMAND - the following WM_COMMAND message are handled             */
/*         BUTTON1 - perform the action assigned to button 1                  */
/*         BUTTON2 - perform the action assigned to button 2                  */
/*         BUTTON3 - perform the action assigned to button 3                  */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       Returns TRUE                                                         */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*       Returns FALSE                                                        */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        InitDlgSysMenu - Remove unwanted options from the system menu       */
/*        MapButtonText  - Change button text to Numbers                      */
/*        PositionDlg    - Position dialog box                                */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        WinDefDlgProc                   WinSendMsg                          */
/*        WinDismissDlg                   WinShowWindow                       */
/*        WinLoadString                   WinWindowFromID                     */
/*        WinQueryWindowText              WinSetWindowText                    */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

MRESULT EXPENTRY ErrorNotifyProc(HWND hwnd,ULONG msg,MPARAM lParam1,MPARAM
                                 lParam2)
{
  static PRXHA_ERR prxhaErr;           /* we only get prxhaerr on a WM_INITDLG*/
                                       /* message                             */
  CHAR buttonText[BUTTON_TEXT_LENGTH]; /* Text of button user clicked on      */


  switch (msg) {
    case  WM_INITDLG :

         /* Start Wait was issued on the call to this dialog                  */

      PositionDlg(hwnd);               /* position dialog window              */
      InitDlgSysMenu(hwnd, FALSE);     /* remove items on system menu we dont */
                                       /* want                                */
      prxhaErr = (PRXHA_ERR)lParam2;   /* get all of our data                 */
      WinSetWindowText(hwnd,           /* display title                       */
                       prxhaErr->Title);
                                       /* display error code                  */
      WinSetWindowText(WinWindowFromID(hwnd, (USHORT)RC), prxhaErr->RetCode);
                                       /* display area of error               */
      WinSetWindowText(WinWindowFromID(hwnd, (USHORT)LOCUS), prxhaErr->Locus);
                                       /* display class of error              */
      WinSetWindowText(WinWindowFromID(hwnd, (USHORT)CLASS), prxhaErr->Class);

                                       /* display message                     */
      WinSetWindowText(WinWindowFromID(hwnd, (USHORT)MESSAGE),
          prxhaErr->message);

      stringSize = BUTTON_TEXT_LENGTH; /* Load the button strings             */

      WinLoadString(prxhaErr->hab, (HMODULE)NULL, OK, stringSize, ok);
      WinLoadString(prxhaErr->hab, (HMODULE)NULL, CANCEL, stringSize, cancel);
      WinLoadString(prxhaErr->hab, (HMODULE)NULL, RETRY, stringSize, retry);
      WinLoadString(prxhaErr->hab, prxhaErr->hmod, ABORT, stringSize, abortstr);
      WinLoadString(prxhaErr->hab, (HMODULE)NULL, IGNORE, stringSize, ignore);
      WinLoadString(prxhaErr->hab, (HMODULE)NULL, YES, stringSize, yes);
      WinLoadString(prxhaErr->hab, (HMODULE)NULL, NO, stringSize, no);
      WinLoadString(prxhaErr->hab, (HMODULE)NULL, BENTER, stringSize, enter);

      if (ON_m(prxhaErr->buttonStyle, MB_OKCANCEL)) {/* buttons are OK,       */
                                       /* Cancel, and button3 not shown       */
        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON1), ok);
        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON2), cancel);
        WinShowWindow(WinWindowFromID(hwnd, (USHORT)BUTTON3), FALSE);
      }

      else

      if (ON_m(prxhaErr->buttonStyle, MB_RETRYCANCEL)) {/* buttons are Retry, */
                                       /* Cancel and Button3 not shown        */

        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON1), retry);
        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON2), cancel);
        WinShowWindow(WinWindowFromID(hwnd, (USHORT)BUTTON3), FALSE);
      }

      else

      if (ON_m(prxhaErr->buttonStyle, MB_ABORTRETRYIGNORE)) {/* buttons are   */
                                       /* Abort, Retry, and Ignore            */

        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON1), abortstr);
        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON2), retry);
        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON3), ignore);
      }

      else

      if (ON_m(prxhaErr->buttonStyle, MB_YESNO)) {/* Buttons are Yes, No and  */
                                       /* button3 not shown                   */

        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON1), yes);
        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON2), no);
        WinShowWindow(WinWindowFromID(hwnd, (USHORT)BUTTON3), FALSE);
      }

      else

      if (ON_m(prxhaErr->buttonStyle, MB_YESNOCANCEL)) {/* Buttons are Yes No */
                                       /* and Cancel                          */

        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON1), yes);
        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON2), no);
        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON3), cancel);
      }

      else

      if (ON_m(prxhaErr->buttonStyle, MB_CANCEL)) {/* Buttons are Cancel and  */
                                       /* buttons 2 & 3 not shown             */

        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON1), cancel);
        WinShowWindow(WinWindowFromID(hwnd, (USHORT)BUTTON2), FALSE);
        WinShowWindow(WinWindowFromID(hwnd, (USHORT)BUTTON3), FALSE);
      }

      else

      if (ON_m(prxhaErr->buttonStyle, MB_ENTER)) {/* Buttons are Enter and    */
                                       /* buttons 2 & 3 not shown             */

        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON1), enter);
        WinShowWindow(WinWindowFromID(hwnd, (USHORT)BUTTON2), FALSE);
        WinShowWindow(WinWindowFromID(hwnd, (USHORT)BUTTON3), FALSE);
      }

      else

      if (ON_m(prxhaErr->buttonStyle, MB_ENTERCANCEL)) {/* Buttons are Enter, */
                                       /* Cancel and Button 3 not shown       */

        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON1), enter);
        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON2), cancel);
        WinShowWindow(WinWindowFromID(hwnd, (USHORT)BUTTON3), FALSE);
      }

      else {                           /* Buttons are OK and buttons 2 & 3 not*/
                                       /* shown                               */
        WinSetWindowText(WinWindowFromID(hwnd, (USHORT)BUTTON1), ok);
        WinShowWindow(WinWindowFromID(hwnd, (USHORT)BUTTON2), FALSE);
        WinShowWindow(WinWindowFromID(hwnd, (USHORT)BUTTON3), FALSE);
      }
      WinSendMsg(WinWindowFromID(hwnd,RXHD_ICON),
                 SM_SETHANDLE,
                 MPFROMHWND(WinQuerySysPointer(HWND_DESKTOP,
                                        prxhaErr->usIconStyle, FALSE) ),
                                        (MPARAM)0);
      WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP,
          SPTR_ARROW, FALSE));
      break;
    case  WM_CLOSE :
      return ((MRESULT)TRUE);
      break;
    case  WM_COMMAND :

      switch (LOUSHORT(lParam1)) {
        case  BUTTON1 :
        case  BUTTON2 :
        case  BUTTON3 :                /* read the text from the button       */
          WinQueryWindowText(WinWindowFromID(hwnd, LOUSHORT(lParam1)),
                             stringSize, buttonText);
                                       /* convert text to an number and set   */
                                       /* return code                         */

          prxhaErr->action = MapButtonText(buttonText);/* dismiss the dialog  */
                                       /* box                                 */

          WinDismissDlg(hwnd, TRUE);
          break;
        default  :
          return (MRESULT)(TRUE);
          break;
      }
      break;
    default  :
      break;
  }
  return (WinDefDlgProc(hwnd, msg, lParam1, lParam2));
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: MapButtonText                                           */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Map button text to integers.                                 */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Compares the input string to the different buttons and       */
/*               returns the button number.                                   */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: ULONG  MapButtonText                                         */
/*                                                                            */
/*   INPUT:                                                                   */
/*      PSZ pszText - ptr to ascii string containing text for button          */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       Returns action for specified text                                    */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*       None.                                                                */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

ULONG  MapButtonText(PSZ pszText)
{

  if (!strcmp(pszText, ok))            /* Map text to the correct button      */
    return (MBID_OK);

  if (!strcmp(pszText, cancel))
    return (MBID_CANCEL);

  if (!strcmp(pszText, abortstr))
    return (MBID_ABORT);

  if (!strcmp(pszText, retry))
    return (MBID_RETRY);

  if (!strcmp(pszText, ignore))
    return (MBID_IGNORE);

  if (!strcmp(pszText, yes))
    return (MBID_YES);

  if (!strcmp(pszText, no))
    return (MBID_NO);

  if (!strcmp(pszText, enter))
    return (MBID_ENTER);
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: SaveQuit                                                */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Bring up Save/Quit/Cancel Dialog                             */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Sets up call to Save/Quit dialog procedure.                  */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: ULONG  SaveQuit                                              */
/*                                                                            */
/*   INPUT:                                                                   */
/*      HAB hab - anchor block handle                                         */
/*      HWND hwnd - window handle                                             */
/*      PSZ pszFName - filename to save output in                             */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       Returns TRUE if Save/Quit processed                                  */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*       Returns FALSE if user hits CANCEL                                    */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        WinLoadString                                                       */
/*        WinDlgBox                                                           */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

ULONG  SaveQuit(HAB hab,HWND hwnd,PSZ pszFName)
{
  char pszSaveQuitMsg[120];            /* string for save/quit message        */
  char pszMsg[MSG_SIZE];               /* save/quit message with filename     */
                                       /* inserted                            */
  char pszFormFN[FNAME_SIZE];          /* Holds formatted filename            */
  ULONG  UserResponse;                 /* What did they do?                   */
  RX_SAVQUIT_STRUCT rx_SaveQuit;       /* data structure for this dialog box  */


  if (strlen(pszFName)) {              /* user already entered filename       */
    rx_SaveQuit.bNamedFile = TRUE;
  }
  else {
    rx_SaveQuit.bNamedFile = FALSE;    /* else say it's untitled              */
    WinLoadString(hab,                 /* load the untitled string from       */
                                       /* resource                            */
        (HMODULE)NULL, RX_UNTITLED, FNAME_SIZE, pszFName);
  }

  WinLoadString(hab,                   /* load the save/quit message          */
      (HMODULE)NULL, SAVE_QUIT, sizeof(pszSaveQuitMsg), pszSaveQuitMsg);

  sprintf(pszMsg, pszSaveQuitMsg, pszFormFN);/* insert the filename into      */
                                       /* message                             */
  rx_SaveQuit.pszSaveQuitMsg = pszMsg; /* load up all the parameters into     */
                                       /* struct                              */
  rx_SaveQuit.hab = hab;
  rx_SaveQuit.hClientWnd = hwnd;

  UserResponse =
      WinDlgBox(HWND_DESKTOP,          /* Parent                              */
                hwnd,                  /* Owner                               */
                (PFNWP)SaveQuitDlgProc,/* Dlg Proc                            */
                (HMODULE)NULL,         /* Resource is Loaded                  */
                RXD_SAVEQUIT,          /* Dialog ID                           */
                &rx_SaveQuit);         /* Create parms                        */

  return (UserResponse);
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: SaveQuitDlgProc                                         */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Handle messages for Save/Quit dialog.                        */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               See above.                                                   */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: MRESULT EXPENTRY SaveQuitDlgProc                             */
/*                                                                            */
/*   INPUT:                                                                   */
/*      HWND hDlg - window handle                                             */
/*      USHORT message - message to process                                   */
/*      MPARAM lparm1 - parameter 1                                           */
/*      MPARAM lparm2 - parameter 2                                           */
/*                                                                            */
/*   The messages handled are:                                                */
/*      WM_INITDLG - initialize the error box                                 */
/*      WM_CLOSE   - close dialog box                                         */
/*      WM_COMMAND - the following WM_COMMAND message are handled             */
/*         RXHD_B_SAVE - user pressed SAVE                                    */
/*         RXHD_B_SAVEAS - user pressed SAVEAS                                */
/*         ESCAPE - user pressed CANCEL                                       */
/*         RXHD_B_DISCARD - user pressed DISCARD                              */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       Returns TRUE if processed correctly                                  */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*       Returns FALSE otherwise                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        InitDlgSysMenu                                                      */
/*        PositionDlg                                                         */
/*                                                                            */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        WinDefDlgProc                     WinSendMsg                        */
/*        WinDismissDlg                     WinSetDlgItemText                 */
/*        WinEnableWindow                   WinWindowFromID                   */
/*        WinQuerySysPointer                                                  */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

MRESULT EXPENTRY SaveQuitDlgProc(HWND hDlg,ULONG message,MPARAM
                                 lparm1,MPARAM lparm2)
{
  static PRX_SAVQUIT_STRUCT ppszParms; /* static because we only get it once  */
  MPARAM mpResponse;                   /* return code                         */

/******************************************************************************/
/* Retrieve the handles of buttons and list boxes.                            */
/******************************************************************************/

  switch (message) {
    case  WM_INITDLG :                 /* Initialize the Dialog               */
      PositionDlg(hDlg);               /* position the Dialog box             */
      InitDlgSysMenu(hDlg, TRUE);      /* strip system menue                  */

      /************************************************************************/
      /* Create the Path, and Init Boxes/Buttons                              */
      /************************************************************************/

      ppszParms = (PRX_SAVQUIT_STRUCT)lparm2;

      if (!(ppszParms->bNamedFile)) {  /* Gray Save Button                    */
        WinEnableWindow(WinWindowFromID(hDlg, (USHORT)RXHD_B_SAVE), FALSE);
      }

      /* display the warning ICON                                             */

      WinSendMsg(WinWindowFromID(hDlg, (USHORT)RXHD_ICON),
                 SM_SETHANDLE,
                 MPFROMHWND(WinQuerySysPointer(HWND_DESKTOP,
                            SPTR_ICONWARNING, FALSE) ), NULL);

      /* display the save quit message                                        */

      WinSetDlgItemText(hDlg, RXHD_CHANGETEXT, (*ppszParms).pszSaveQuitMsg);
      break;

    case  WM_CLOSE :
      WinDismissDlg(hDlg, 0);          /* close with a return code of 0       */
      return ((MRESULT)TRUE);
      break;

    case  WM_COMMAND :                 /* Process the Buttons                 */

      switch (LOUSHORT(lparm1)) {
        case  RXHD_B_SAVE :            /* send message to Client to get file  */
                                       /* saved                               */
          mpResponse = WinSendMsg(ppszParms->hClientWnd, WM_COMMAND,
                                  MPFROMSHORT(PMRXA_SAVE), 0L);

          if (mpResponse)              /* if it worked close dialog box       */
            WinDismissDlg(hDlg, LOUSHORT(mpResponse));
          break;

        case  RXHD_B_SAVEAS :          /* let the user pick a new name to save*/
                                       /* as                                  */
          mpResponse = WinSendMsg(ppszParms->hClientWnd, WM_COMMAND,
                                  MPFROMSHORT(PMRXA_SAVEAS), 0L);

          if                           /* Cancel or error on save as          */
              ((mpResponse != (MPARAM)0) && (mpResponse != (MPARAM)ESCAPE)) {
            WinDismissDlg(hDlg, LOUSHORT(mpResponse));
          }
          break;

        case  ESCAPE :                 /* Cancel                              */
          WinDismissDlg(hDlg, 0);
          break;

        case  RXHD_B_DISCARD :         /* discard changes                     */
          WinDismissDlg(hDlg, 2);
          break;

        default  :
          return (MRESULT)TRUE;
          break;
      }
      break;
    default  :
      return (WinDefDlgProc(hDlg, message, lparm1, lparm2));
  }
  return ((MRESULT)NULL);
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: PositionDlg                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Position the Font dialog.                                    */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This routine positions the Font dialog relative to the owner */
/*               window and the desktop.                                      */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: PositionDlg(HWND hDlg)                                       */
/*                                                                            */
/*   INPUT:                                                                   */
/*      hDlg            - Handle to the dialog window.                        */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        WinQuerySysValue                                                    */
/*        WinQueryWindow                                                      */
/*        WinQueryWindowPos                                                   */
/*        WinSetWindowPos                                                     */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/
BOOL PositionDlg(HWND hDlg)
{
   SWP  DlgPos;                        /* Postion of dialog window            */
   SWP  ClientPos;                     /* Position of client window           */
   LONG Desktop_cx;                    /* Desktop horizontal coordinate       */
   LONG Desktop_cy;                    /* Desktop vertical coordinate         */

                                       /* Get window position                 */
   WinQueryWindowPos(WinQueryWindow(hDlg, QW_OWNER), &ClientPos);
   WinQueryWindowPos(hDlg, &DlgPos);
   Desktop_cx=WinQuerySysValue(HWND_DESKTOP, SV_CXFULLSCREEN);
   Desktop_cy=WinQuerySysValue(HWND_DESKTOP, SV_CYFULLSCREEN);

                                       /* calculate y value from Client       */
   DlgPos.y = (SHORT)(ClientPos.y+ClientPos.cy - DlgPos.cy - DLG_OFFSET_TOP);

                                       /* Don't let go off the top of the page*/
   DlgPos.y = (SHORT)( (LONG)(DlgPos.y + DlgPos.cy) > Desktop_cy)
                     ? (SHORT)(Desktop_cy-(LONG)DlgPos.cy) : DlgPos.y;
                                       /* Position within bottom too          */
   DlgPos.y= (SHORT)((DlgPos.y > DLG_OFFSET_Y ) ? DlgPos.y : DLG_OFFSET_Y);

   DlgPos.x=(ClientPos.cx>DlgPos.cx)   /* Center box under the Title          */
            ? (ClientPos.x+((ClientPos.cx/(SHORT)2)-(DlgPos.cx/(SHORT)2)))
            : (ClientPos.x-((DlgPos.cx/(SHORT)2)-(ClientPos.cx/(SHORT)2)));

   ClientPos.x = (ClientPos.x > 0) ? ClientPos.x : (SHORT)0;

                                       /* Don't let it go off the left edge   */
   DlgPos.x= (DlgPos.x > DLG_OFFSET_X ) ? DlgPos.x : (SHORT)DLG_OFFSET_X;
                                       /* or the right edge                   */
   DlgPos.x = (SHORT)(((LONG)DlgPos.x + DlgPos.cx) > Desktop_cx)
              ? (SHORT)(Desktop_cx-(LONG)DlgPos.cx-DLG_OFFSET_X) : DlgPos.x;

                                       /* Adjust dlgbox, (relative to owner)  */
   WinSetWindowPos( hDlg,
                    HWND_TOP,
                    DlgPos.x, DlgPos.y,
                    DlgPos.cx, DlgPos.cy,
                    SWP_SIZE | SWP_MOVE | SWP_ZORDER);

   return(TRUE);
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: InitDlgSysMenu                                          */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Initialize the system menu for dialogues.                    */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Removes items from the system menu that we don't want.       */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: void InitDlgSysMenu                                          */
/*                                                                            */
/*   INPUT:                                                                   */
/*      HWND hDlg   - window handle                                           */
/*      BOOL bClose - Disable close option (TRUE)                             */
/*                                                                            */
/*   EXIT-NORMAL: void                                                        */
/*                                                                            */
/*   EXIT-ERROR:  void                                                        */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       WinSendMsg                                                           */
/*       WinWindowFromID                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

void InitDlgSysMenu( HWND hDlg, BOOL bClose )
{
  HWND hSysMenu;                         /* handle to the system menu window */

                                         /* get handle to system menu        */
   hSysMenu=WinWindowFromID(hDlg, (USHORT)FID_SYSMENU);

   /******************************************************************
   * Remove Items from the system menu that we don't want            *
   *******************************************************************/
   WinSendMsg(hSysMenu,                 /* no Switch to ..   option  */
               MM_DELETEITEM,
               MPFROM2SHORT(SC_TASKMANAGER,TRUE),
               MPFROMSHORT(NULL));

   WinSendMsg(hSysMenu,                /* no Minimize option         */
               MM_DELETEITEM,
               MPFROM2SHORT(SC_MINIMIZE,TRUE),
               MPFROMSHORT(NULL));

   WinSendMsg(hSysMenu,                /* no Maximize option         */
               MM_DELETEITEM,
               MPFROM2SHORT(SC_MAXIMIZE,TRUE),
               MPFROMSHORT(NULL));

   WinSendMsg(hSysMenu,                /* no Size option             */
               MM_DELETEITEM,
               MPFROM2SHORT(SC_SIZE,TRUE),
               MPFROMSHORT(NULL));

   WinSendMsg(hSysMenu,                /* no Restore option          */
               MM_DELETEITEM,
               MPFROM2SHORT(SC_RESTORE,TRUE),
               MPFROMSHORT(NULL));


   if (!bClose)      /* disable close option if necessary */
      WinSendMsg (hSysMenu,
                MM_SETITEMATTR,
                MPFROM2SHORT(SC_CLOSE, TRUE),
                MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
   return;
}
