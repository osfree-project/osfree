/* static char *SCCSID = "@(#)pmrexxio.c        6.3 91/12/24";                      */
/*********************  START OF SPECIFICATIONS  ******************************/
/*                                                                            */
/*   SOURCE FILE NAME:  pmrexxio.c                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  PM I/O environment for REXX command files.            */
/*                                                                            */
/*      Provide a means for REXX command files to do Pull, Say and            */
/*      Trace interactions when called from a PM application by simulating    */
/*      stdin, stdout and stderr I/O operations.                              */
/*                                                                            */
/*   COPYRIGHT:         IBM Corporation 1991                                  */
/*                                                                            */
/*   STATUS:            Version 2.00                                          */
/*                                                                            */
/*   FUNCTION:                                                                */
/*           This program provides a Presentation Manager Shell for           */
/*      RexxStart programs.                                                   */
/*                                                                            */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES:                                                         */
/*          This function has dependencies on the following                   */
/*      files for compilation.                                                */
/*          pmrexxio.h - Definitions for control of the PMREXXIO              */
/*                       dialog.                                              */
/*          pmrxiodt.h - Definitions necessary for the resource               */
/*                       file compilation.                                    */
/*          rhdtatyp.h - Macros, structures, typedefs and defines             */
/*                       local to and necessary for this program.             */
/*          The C runtime multi-threaded header files                         */
/*                                                                            */
/*  EXECUTION INSTRUCTIONS:                                                   */
/*          PMREXXIO is invoked via a call to RexxCreateIOWindow.             */
/*      This call redirects stdin, stdout and stderr to pips which            */
/*      are processed by PMREXXIO.  The calling application enables           */
/*      execution of a Rexx program by posting message REXX_STARTPROC,        */
/*      passing the id of the thread the Rexx program will execute on.        */
/*      On completion of the Rexx program, the message REXX_ENDPROC           */
/*      should be sent to the window.  RexxDestroyIOWindow should be          */
/*      called to perform cleanup of the I/O resources at program             */
/*      termination.                                                          */
/*                                                                            */
/*  EXPECTED OUTPUT:                                                          */
/*          The REXX procedure file will be executed with all                 */
/*      output directed into PM windows.                                      */
/*                                                                            */
/*  PROCEDURES:                                                               */
/*    RexxCreateIOWindow:  Initialize I/O subsystem                           */
/*    RexxDestroyIOWindow:  Terminate I/O subsystem                           */
/*    EntryDlgProc:   Dialog procedure for the input entry field              */
/*    initialize:     Initialize data structures and threads                  */
/*    io_routine:     Process the REXX IO system exit                         */
/*    MainWndProc:    Main window procedure                                   */
/*    SizeWindow:     Size the output window and input window                 */
/*    ClipBoard:      Process MLE clipboard functions                         */
/*    RXIOCmds:       Process WM_COMMAND messages from the main window proc   */
/*    Import:         Add information from import buffer to the MLE           */
/*    SelectFont:     Interface to the font selection dialog                  */
/*    stdinpipe:      Send input to the standard input of a child process     */
/*    stdoutpipe:     Send standard output from child proc to output window   */
/*                                                                            */
/***********************  END OF SPECIFICATIONS  ******************************/
#include <malloc.h>
#include <process.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define the sections of the OS/2 header files that we need.                 */

#define  INCL_RXSYSEXIT                /* REXX system exits                   */
#define  INCL_RXARI                    /* REXX Asynchronous Request Interface */
#define  INCL_DOSSEMAPHORES            /* OS/2 semaphore support              */
#define  INCL_DOSERRORS                /* OS/2 errors                         */
#define  INCL_DOSPROCESS               /* Process and thread support          */
#define  INCL_DOSQUEUES                /* Queues                              */
#define  INCL_DOSMISC                  /* Miscellaneous doscalls              */
#define  INCL_DOSNLS                   /* NLS (code page) support             */
#define  INCL_DOSMODULEMGR             /* OS/2 module support                 */
#define  INCL_GPILCIDS                 /* Physical and logical fonts with     */
                                       /* lcids                               */
#define  INCL_GPIPRIMITIVES            /* Drawing primitives and primitive    */
                                       /* attributes                          */
#define  INCL_WINBUTTONS               /* Buttons controls                    */
#define  INCL_WINCOUNTRY               /* Code page support                   */
#define  INCL_WINDIALOGS               /* Dialog boxes                        */
#define  INCL_WINENTRYFIELDS           /* Entry fields                        */
#define  INCL_WINERRORS                /* Standard Font dialog                */
#define  INCL_WINFRAMEMGR              /* Frame manager                       */
#define  INCL_WINHELP                  /* Help manager definitions            */
#define  INCL_WININPUT                 /* Mouse and keyboard input            */
#define  INCL_WINMENUS                 /* Menu controls                       */
#define  INCL_WINMESSAGEMGR            /* Message management                  */
#define  INCL_WINMLE                   /* Multiple line edit fields           */
#define  INCL_WINPOINTERS              /* Mouse pointers                      */
#define  INCL_WINSTDFONT               /* Standard Font dialog                */
#define  INCL_WINSHELLDATA             /* Profile calls                       */
#define  INCL_WINSWITCHLIST            /* Task list calls                     */
#define  INCL_WINTIMER                 /* Timer routines                      */
#define  INCL_WINWINDOWMGR             /* General window management           */
#include <os2.h>
#include <rexxsaa.h>                   /* Include the REXX header file        */

/* Defines, typedefs and function prototypes for pmrexx                       */

#include "rhdtatyp.h"
#include "pmrxiodt.h"
#include "pmrexxio.h"

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: RexxCreateIOWindow                                      */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Initialize a Rexx stdin/stdout/stderr transaction subsystem. */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Creates Rexx I/O windows and redirects stdin, stdout, and    */
/*               stderr to pipes.                                             */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:  RexxCreateIOWindow(HWND hwndOwner, PHWND phwndClient)       */
/*                                                                            */
/*   INPUT:                                                                   */
/*       phwndClient is the handle of the I/O window client window.           */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       Exit with handle to I/O frame window.                                */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*       If unable to duplicate the write handle for stderr pipe, exit rc=5.  */
/*       If unable to register the window class, exit rc=1.                   */
/*       If no procedure name was specified, exit with rc=3.                  */
/*       If window was not displayed, exit with return code from the REXX     */
/*         procedure.                                                         */
/*       Otherwise: exit with created frame and client handles.               */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*       Initializes the Rexx Presentation Manager I/O subsystem.             */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       getstring      - To obtain the title bar name and window name.       */
/*       initialize     - Sets up the environment for the program. Starts all */
/*                        necessary threads, sets up the input queues, etc.   */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       WinQueryAnchorBlock         WinRegisterClass                         */
/*       WinCreateStdWindow                                                   */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

HWND RexxCreateIOWindow(
   HWND hwndOwner,                     /* Owner handle                        */
   PHWND phwndClient)                  /* Returned client window              */
 {

   /* We set the creation flags to be the standard window frame flags         */
   /*    but without scroll bars and without a task list entry.               */

  ULONG flCreate = FCF_STANDARD&~(FCF_VERTSCROLL|FCF_HORZSCROLL|FCF_ACCELTABLE);
  PUCHAR windowtext;                   /* Text for title bar                  */
  ULONG  argnum = 1;                   /* Counter for looking at arguments    */
  HAB    hab;                          /* anchor block                        */
  HMODULE handle;                      /* module handle                       */
  HWND    hwndFrame;                   /* frame handle                        */

                                       /* get desktop anchor                  */
  hab = WinQueryAnchorBlock(hwndOwner);
  if (!hab) {                          /* no anchor block?                    */
    return NULLHANDLE;                 /* return with nothing                 */
  }

  if (DosQueryModuleHandle(MODULE_NAME, &handle)) {
    return NULLHANDLE;                 /* return with nothing                 */
  }

  if (!WinRegisterClass(               /* Register Window Class               */
      hab,                             /* Anchor block handle                 */
      PMREXXCLASSNAME,                 /* Window Class name                   */
      (PFNWP)MainWndProc,              /* Address of Window Procedure         */
      CS_SIZEREDRAW,                   /* Class Style                         */
      sizeof(PUCHAR))) {               /* Extras words for a far ptr.         */
    return  NULLHANDLE;                /* Exit if we couldn't register the    */
  }                                    /* class                               */

   /* Get the title bar text                                                  */

  windowtext = getstring(hab, handle, WINDOW_TEXT);

   /* Create the standard window                                              */

  hwndFrame = WinCreateStdWindow(hwndOwner,/* Supplied owner                  */
      0,                               /* Frame Style                         */
      &flCreate,                       /* Control Data                        */
      PMREXXCLASSNAME,                 /* Window Class name                   */
      NULL,                            /* Window Text                         */
      WS_VISIBLE,                      /* Client style                        */
      handle,                          /* Module handle==this module          */
      RXHOST_WIN,                      /* Window ID                           */
      phwndClient);                    /* Client Window handle                */
                                       /* set the window text                 */
  WinSetWindowText(WinWindowFromID(hwndFrame, (USHORT)FID_TITLEBAR),
      windowtext);
  free(windowtext);                    /* release window text                 */
  return hwndFrame;                    /* return created frame handle         */
 }

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: RexxDestroyIOWindow                                     */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Terminate a Rexx stdin/stdout/stderr transaction subsystem.  */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Cleans up the Rexx I/O subsystem.                            */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:  RexxDestroyIOWindow(HWND hwndClient)                        */
/*                                                                            */
/*   INPUT:                                                                   */
/*       phwndClient is the handle of the I/O window client window.           */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*       Terminates the Rexx Presntation Manager I/O subsystem.               */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       WinQueryWindowPtr           WinDestroyHelpInstance                   */
/*       WinAssociateHelpInstance    WinDestroyWindow                         */
/*       DosSuspendThread            RexxDeregisterExit                       */
/*       DosFreeMem                                                           */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

void RexxDestroyIOWindow(HWND hwnd)
{
  PRHWINDATA pWinData = NULL;          /* pointer to the windata structure    */

  pWinData = WinQueryWindowPtr(WinWindowFromID(hwnd, (ULONG)FID_CLIENT),  //(USHORT)FID_CLIENT),
      (LONG)0); // (SHORT)0);
   /* And now, clean everything up.                                    */
   if (pWinData->HelpInst) {
      WinDestroyHelpInstance(pWinData->HelpInst);
      WinAssociateHelpInstance(NULLHANDLE, pWinData->frame);
   }
                                       /* cleanup semaphores                  */
   DosCloseMutexSem(pWinData->pipe_in.q_sem);
   DosCloseMutexSem(pWinData->trace_in.q_sem);
   DosCloseEventSem(pWinData->pipe_in.q_data);
   DosCloseEventSem(pWinData->trace_in.q_data);

   RexxDeregisterExit(PMRXIO_EXIT, NULL);
   WinDestroyWindow(pWinData->frame);
   DosFreeMem(pWinData);               /* release storage                     */
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: initialize                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Initialize the data structures and threads necessary for the */
/*               program to operate.                                          */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Initializes the input queues and their semaphores, set up    */
/*               the file handles for redirection of standard input/output and*/
/*               error streams, allocate stacks for and begin the threads.    */
/*               subcommand environments.                                     */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:  initialize                                                  */
/*                                                                            */
/*   INPUT:       none                                                        */
/*                                                                            */
/*   EXIT-NORMAL: returns 0                                                   */
/*                                                                            */
/*   EXIT-ERROR:  returns error code from call causing the error.             */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       makepipe       - Creates a pipe with the read and write handles at   */
/*                        desired positions.                                  */
/*       setinherit     - Sets the inheritance characteristcs of the passed   */
/*                        file handle.                                        */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       DosDupHandle                                                         */
/*       DosCreateMutexSem                                                    */
/*       DosCreateEventSem                                                    */
/*       DosGetInfoBlocks                                                     */
/*       RexxRegisterExitExe                                                  */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

  LONG initialize(PRHWINDATA pWinData)
  {
    LONG         rc;                   /* return code                         */
    PTIB         TI;                   /* Thread info block pointer           */
    PPIB         PI;                   /* Process info block pointer          */
    PRHWINDATA   userdata[2];          /* registered user data                */

   /* Initialization Section:                                                 */
   /* First initialize the semaphores for the input queue.                    */
   /* Using a queue and a separate thread ensures that we don't               */
   /*   tie up the window procedure if the pipe backs up.                     */

    DosCreateMutexSem(NULL, &pWinData->pipe_in.q_sem, 0, FALSE);
    DosCreateMutexSem(NULL, &pWinData->trace_in.q_sem, 0, FALSE);
    DosCreateEventSem(NULL, &pWinData->pipe_in.q_data, FALSE, FALSE);
    DosCreateEventSem(NULL, &pWinData->trace_in.q_data, FALSE, FALSE);

   /* Next, set up the redirection pipes to capture standard                  */
   /*  I/O from subcommand processing.                                        */
   /* First make sure all C handles are closed.                               */

    DosClose(STDIN);                   /* close stdin, stdout, stderr         */
    DosClose(STDOUT);
    DosClose(STDERR);

   /* Since we closed all open file handles, we are free to use any *
    * handles we want.                                                        */
   /* First, standard input                                                   */

    makepipe(&pWinData->stdin_r, STDIN, &pWinData->stdin_w, 4, PIPE_SIZE);
    setinherit(pWinData->stdin_r, TRUE);
    setinherit(pWinData->stdin_w, FALSE);

   /* Next, standard output                                                   */

    makepipe(&pWinData->stdout_r, 5, &pWinData->stdout_w, STDOUT, PIPE_SIZE);
    setinherit(pWinData->stdout_w, TRUE);
    setinherit(pWinData->stdout_r, FALSE);

   /* And, finally, standard error                                            */
   /* Just dup the standard output and handle it once.                        */

    pWinData->stderr_w = STDERR;

    if (rc = DosDupHandle(pWinData->stdout_w, &pWinData->stderr_w)) {
      return  rc;
    }

    DosGetInfoBlocks(&TI, &PI);        /* Get the thread information          */
    pWinData->proc_id = PI->pib_ulpid; /* set the process id                  */

    userdata[0] = pWinData;            /* save our anchor                     */
                                       /* register I/O handler                */
    RexxRegisterExitExe(PMRXIO_EXIT, (PFN)io_routine, (PUCHAR)userdata);

   /* And, of course, start the various threads!                              */

    rc = DosCreateThread(&pWinData->in_tid, (PFNTHREAD)stdinpipe, (ULONG)pWinData,
        (ULONG)0, STACK_SIZE);
    if (!rc) rc = DosCreateThread(&pWinData->out_tid, (PFNTHREAD)stdoutpipe, (ULONG)pWinData,
        (ULONG)0, STACK_SIZE);
    return rc;
  }

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: MainWndProc                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Main window procedure for the PMREXX program.                */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This procedure handles all interesting messages for the      */
/*               client area for the PMREXX program. The messages handled     */
/*               are:                                                         */
/*       WM_CREATE      - Allocates and initializes the RHWINDATA structure   */
/*                        (defined in rhdtatyp.h) and places a pointer to it  */
/*                        into the window words.                              */
/*       WM_TIMER       - A 100 millisecond timer is set when output is added */
/*                        to a buffer for later adding to the MLE output      */
/*                        window. Thus if many lines are being added, output  */
/*                        time is improved, and window flashing is reduced.   */
/*                        If the timer pops, then no output has been          */
/*                        added for the 100 milliseconds and it is time to    */
/*                        add the output to the MLE.                          */
/*       WM_ERASEBACKGROUND - Just return TRUE to force a background erase.   */
/*       WM_SIZE        - Calls routine SizeWindow to re-adjust the sizes     */
/*                        of the MLE and input areas.                         */
/*       WM_PAINT       - Paints the header for the input window, and lets    */
/*                        the child controls do the rest.                     */
/*       WM_COMMAND     - All WM_COMMAND messages are processed in routine    */
/*                        RXIOCmds.                                           */
/*       WM_CHAR        - Handles tab and backtab keys to switch the focus    */
/*                        window and the break key as a halt procedure.       */
/*       REXX_MAKE_VISIBLE - Is a user defined message requesting that        */
/*                        the window be made visible. The window is initially */
/*                        created invisible and is not made visible until     */
/*                        something is output, or until some input is         */
/*                        requested. This way, a REXX procedure that does not */
/*                        need this window, will not clutter up the screen    */
/*                        with it (e.g. a DM exec that uses DM for all I/O).  */
/*       REXX_MAKE_INVISIBLE - Is a user defined message requesting that      */
/*                        the window be made invisible.                       */
/*       REXX_ENDPROC   - This signals the end of a Rexx procedure.  The      */
/*                        I/O subsystem is flushed and reinitialized.         */
/*       REXX_STARTPROC - This signals the start of a Rexx procedure.  This   */
/*                        identifies the thread running the procedure.        */
/*       RXIOB_MSGBOX   - Puts up a message box with the requested message.   */
/*       START_TIMER    - Starts a timer. When the timer pops, the buffered   */
/*                        output is added to the MLE. See also, WM_TIMER and  */
/*                        ADD_STDOUTWIN. This message is posted by the thread */
/*                        reading the input pipe.                             */
/*       ADD_STDOUTWIN  - This user defined message is a request to add       */
/*                        output to the output window. To reduce window       */
/*                        flicker and improve performance, data is internally */
/*                        buffered until the buffer is full, or until there   */
/*                        is a lull in output, before being sent on to the    */
/*                        MLE window. See also the WM_TIMER message.          */
/*       WM_SAVEAPPLICATION - Save the current font and window information    */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*               Standard window procedure linkage is followed.               */
/*                                                                            */
/*   INPUT:                                                                   */
/*               Standard window procedure inputs are passed in.              */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       getfixedfont    - Get a fixed font for the default display            */
/*       getstring       - Retrieve a string resource                          */
/*       RestoreWindow   - Gets saved window position from user profile        */
/*       RHWinErrorBox   - Display an error message box                        */
/*       SizeWindow      - Sizes all the child windows (MLE & Entry field) to  */
/*                         fit the frame window.                               */
/*       RXIOCmds        - This routine processes the various WM_COMMAND       */
/*                         messages.                                           */
/*       Import          - Sends the data from the internal buffer to the MLE. */
/*                         See the WM_TIMER and ADD_STDOUTWIN messages.        */
/*       SaveWindow      - Saves current window position to user profile       */
/*       SetCheckMark    - Check/uncheck a menu item                           */
/*       ProcessSaveQuit - Interface to the Save/Quit dialog                  */
/*       SearchDlgProc   - Display Search Dialog                                */
/*                                                                            */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       DosAllocMem                       WinLoadString                      */
/*       DosCreateMutexSem                 WinMessageBox                      */
/*       DosFreeMem                        WinPostMsg                         */
/*       DosReleaseMutexSem                WinQueryAnchorBlock                */
/*       DosRequestMutexSem                WinQueryWindow                     */
/*       DosSuspendThread                  WinQueryWindowPos                  */
/*       GpiCharStringAt                   WinQueryWindowPtr                  */
/*       GpiQueryCurrentPosition           WinReleasePS                       */
/*       RexxDeregisterExit                WinSendMsg                         */
/*       WinAlarm                          WinSetFocus                        */
/*       WinAssociateHelpInstance          WinSetWindowPtr                    */
/*       WinCreateHelpInstance             WinShowWindow                      */
/*       WinDefWindowProc                  WinStartTimer                      */
/*       WinDestroyHelpInstance            WinStopTimer                       */
/*       WinGetPS                          WinWindowFromID                    */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

  MRESULT EXPENTRY MainWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {
    ULONG      rc;                     /* return code                         */
    PRHWINDATA pWinData = NULL;        /* pointer to the windata structure    */
    HPS        hps;                    /* presentation space handle           */
    PIMPORT    im_ptr;                 /* Pointer to buffer for data waiting  */
    ULONG      postcount;              /* number of posted lines              */
    FONTMETRICS fm;                    /* font size information               */
    HWND       hEntry;                 /* entry dialog handle                 */
                                       /* Creation flags for the MLE requests */
                                       /* scroll bars.                        */
    ULONG flCreate = MLS_HSCROLL|MLS_VSCROLL|MLS_BORDER;

    pWinData = WinQueryWindowPtr(hwnd, (SHORT)0);

    switch (msg) {

      case  WM_CREATE :

         /* When the window is created, we also create a structure to hold    */
         /*   all the window specific data.                                   */

        if (pWinData = malloc(sizeof(*pWinData))) {
          memset(pWinData, '\0', sizeof(*pWinData));

          pWinData->hab = WinQueryAnchorBlock(hwnd);
                                       /* and client handle                   */
          pWinData->client = hwnd;
                                       /* save frame handle                   */
          pWinData->frame = WinQueryWindow(hwnd, QW_PARENT);
          if (DosQueryModuleHandle(MODULE_NAME, &pWinData->hResource)) {
            return NULL;               /* return with nothing                 */
          }
          pWinData->imp_ptr = NULL;    /* This is the timer to be used for    */
                                       /* import time-out.                    */

          pWinData->timer = (USHORT)WinStartTimer(pWinData->hab, (HWND)0,
                                                 (USHORT)0, (ULONG)0);
                                       /* get code page information           */
          DosQueryCtryInfo(sizeof(COUNTRYINFO), &pWinData->CountryCode,
              &pWinData->CountryInfo, &postcount);
                                       /* Attempt to find a fixed font for the*/
                                       /* display, if we are not successful,  */
                                       /* we will use the system font.        */
          getfixedfont(hwnd, &(pWinData->MleFontAttrs.fAttrs));
                                       /* system font attributes              */
          pWinData->infocus = FALSE;
                                       /* get the input text                  */
                                       /* prompt from the resource file       */

          pWinData->RxResult = TRUE;   /* append or overwrite flag            */

          pWinData->pszInputText = getstring(pWinData->hab,
          pWinData->hResource, INPUT_TITLE);

          WinSetWindowPtr(hwnd, (SHORT)0, pWinData);
                                       /* Now allocate a segment for the      */
                                       /* import buffer for the MLE           */

          pWinData->imp_ptr = (PIMPORT)(pWinData->ImportBuffer);
                                       /* initialize the buffer               */
          im_ptr = pWinData->imp_ptr;
          im_ptr->total_size = ALLOCSIZE;
          im_ptr->inuse = HDRSIZE;
          *(PUCHAR)(im_ptr->data) = '\0';
                                       /* create the semaphore                */
          DosCreateMutexSem(NULL, &im_ptr->sem, 0, FALSE);

   /* Set up the threads and initialize the data structures                   */

          rc = initialize(pWinData);

   /* If initialization failed, alert the user and exit                       */

          if (rc) {
            WinAlarm(HWND_DESKTOP, WA_ERROR);
            WinMessageBox(HWND_DESKTOP, NULLHANDLE,
                getstring(pWinData->hab, pWinData->hResource, INIT_FAILED),
                getstring(pWinData->hab, pWinData->hResource, RXHB_MSGBOXHDR), 1,
                MB_CANCEL|MB_CUACRITICAL|MB_MOVEABLE);
          }
                                       /* Find out the character              */
                                       /* height for the input entry field    */
          hps = WinGetPS(pWinData->client);
          GpiQueryFontMetrics(hps, (LONG)sizeof fm, &fm);
          pWinData->in_hgt = (ULONG )fm.lMaxBaselineExt+2;
          WinReleasePS(hps);
                                       /* Create the child MLE for            */
                                       /* output                              */
          pWinData->outmle = WinCreateWindow(pWinData->client, (PCSZ)WC_MLE, NULL,
              flCreate, 0, 0, 0, 0, pWinData->client, HWND_TOP,
              MLE_WNDW, NULL, NULL);

          if (!pWinData->outmle) {     /* If we couldn't do it, tell the user */
                                       /* why.                                */
            RHWinErrorBox(pWinData->hab, hwnd, pWinData->hResource,
                MLE_CREATE_ERROR, MB_OK|MB_CUAWARNING);
          }                            /* Delay redrawing window until        */
                                       /* we're done updating it              */
          WinEnableWindowUpdate(pWinData->client, FALSE);
                                       /* Set the format for the MLE          */

          WinSendMsg(pWinData->outmle, MLM_FORMAT, MLE_TXT_FMT, (MPARAM)0);
                                       /* Make the MLE readonly               */

          WinSendMsg(pWinData->outmle, MLM_SETREADONLY, MPFROMLONG(TRUE),
              (MPARAM)0);
                                       /* set the new font                    */
          WinSendMsg(pWinData->outmle, MLM_SETFONT,
              &(pWinData->MleFontAttrs.fAttrs), (MPARAM)0);
                                       /* Update the window                   */
          WinShowWindow(pWinData->outmle, TRUE);
                                       /* set the title width                 */
          pWinData->wtitle = (strlen((const char *)pWinData->pszInputText)+5)*(USHORT)
              fm.lAveCharWidth;
                                       /* Load the dialog box for the input   */
                                       /* window                              */
          hEntry = WinLoadDlg(pWinData->client, pWinData->client,
              (PFNWP)EntryDlgProc, (HMODULE)pWinData->hResource, RH_EF1, NULL);

          pWinData->insle = hEntry;    /* save input window handle Set the    */
                                       /* maximum input length to ANSLEN      */
          WinSendMsg(hEntry, EM_SETTEXTLIMIT, MPFROMSHORT(ANSLEN), (MPARAM)0);
                                       /* Set the offset to the first         */
                                       /* character in the input box to 0     */
          WinSendMsg(hEntry, EM_SETFIRSTCHAR, (MPARAM)0, (MPARAM)0);
                                       /* Set the first character to be       */
                                       /* selected as 0                       */
          WinSendMsg(hEntry, EM_SETSEL, (MPARAM)0, (MPARAM)0);
                                       /* Update the windows                  */
          WinShowWindow(hEntry, TRUE);
          WinEnableWindowUpdate(hEntry, TRUE);
          WinEnableWindowUpdate(pWinData->client, TRUE);
          WinSetFocus(HWND_DESKTOP, pWinData->outmle);
        }
        else {                         /* If we couldn't allocate the data    */
                                       /* structure, then we cannot continue, */
                                       /* so alert the user, and exit.        */
          WinSendMsg(WinWindowFromID(hwnd, (USHORT)FID_CLIENT),
              RXIOB_MSGBOX,
              MPFROM2SHORT(RX_ALLOCERR, RXHB_MSGBOXHDR),
              MPFROMLONG(MB_ENTER|MB_CUACRITICAL|MB_MOVEABLE));
          return (MRESULT)TRUE;
        }
        break;

      case  WM_TIMER :

        if (hwnd) {                    /* We have gone for the designated time*/
                                       /* without adding any new data to the  */
                                       /* output buffer, so it is now time to */
                                       /* import the information to the MLE   */
                                       /* output window.                      */
          WinStopTimer(pWinData->hab, hwnd, pWinData->timer);
                                       /* Get the semaphore so that the       */
                                       /* input pipe thread won't interfere.  */

          DosRequestMutexSem(pWinData->imp_ptr->sem, SEM_INDEFINITE_WAIT);
          Import(pWinData);
                                       /* Now clear the                       */
                                       /* thread so that the input pipe can   */
                                       /* continue.                           */
          DosReleaseMutexSem(pWinData->imp_ptr->sem);
        }

        break;

      case  WM_ERASEBACKGROUND :

        return (MRESULT)TRUE;          /* Return TRUE to indicate we want the */
                                       /* background cleared to the system    */
                                       /* default window background color.    */
      case  WM_SIZE :

        SizeWindow(hwnd, pWinData);    /* Re-size the client area windows.    */
        break;

      case  WM_PAINT :

        if (hps = WinGetPS(hwnd)) {    /* The title for the input window needs*/
                                       /* to be re-painted by us. The child   */
                                       /* windows do all the rest of the      */
                                       /* drawing for us.                     */
          POINTL p;
          SWP wp;
          HWND hOwner;
          PUCHAR ptr;                  /* Ensure that we get a proper handle  */
                                       /* to the client area.                 */

          hOwner = WinQueryWindow(hwnd, QW_PARENT);
                                       /* Calculate the proper positioning.   */
          WinQueryWindowPos(WinWindowFromID(hOwner, FID_CLIENT), &wp);

          p.x = BORDER_MARGIN *2;
          p.y = wp.cy-pWinData->in_hgt-(BORDER_MARGIN *2);
          ptr = pWinData->pszInputText; /* Output the title, and find the      */
                                       /* resulting position for the next     */
                                       /* character. That is where we will    */
                                       /* start the input box.                */
          GpiCharStringAt(hps, &p, (LONG)strlen((const char *)ptr), ptr);
          GpiQueryCurrentPosition(hps, &p);
          pWinData->wtitle = (ULONG )p.x;
          WinReleasePS(hps);
        }
        break;

      case  WM_COMMAND :
                                       /* WM_COMMAND processing in RXIOCMDS   */
        return (RXIOCmds(hwnd, mp1, pWinData));
        break;

      case  WM_CHAR :

         /* Check for a valid (complete) key message.                         */

        if (!(SHORT1FROMMP(mp1)&KC_KEYUP) &&
            (SHORT1FROMMP(mp1)&KC_VIRTUALKEY) ) {

          switch (SHORT2FROMMP(mp2)) {
            case  VK_TAB :
            case  VK_BACKTAB :         /* For tab and back-tab, switch focus  */
                                       /* windows                             */
              return (MRESULT)WinSetFocus(HWND_DESKTOP,
                  pWinData->infocus?pWinData->outmle: pWinData->insle);
              break;

            case VK_NEWLINE:
            case  VK_ENTER :           /* For an enter key, process the command */
              WinSendMsg(pWinData->insle, WM_CHAR, MPFROM2SHORT(KC_VIRTUALKEY, NULL),
                         MPFROM2SHORT(NULL, VK_NEWLINE));
              break;

            case  VK_DELETE :           /* For a break key, set the halt       */
                                       /* procedure indicator.                */
              WinSendMsg(hwnd, WM_COMMAND, MPFROMSHORT(RXIOA_DELETE), (MPARAM)0);
              break;

            default  :
              break;
          }                            /* endswitch                           */
        }
        break;

      case  REXX_STARTPROC :

         /* This is the request to start everything up. It is sent by the     */
         /*   main procedure at startup time.                                 */

        pWinData->rexx_tid = LONGFROMMP(mp1);

        break;

      case  REXX_ENDPROC : {

        PLIST_ENTRY q_elem;            /* pointer for an element in the input */
                                       /* queue                               */
                                       /* Clear out any entries on input queue*/
        if (!DosRequestMutexSem(pWinData->pipe_in.q_sem, SEM_INDEFINITE_WAIT)) {
          for (q_elem = pWinData->pipe_in.q_1st; q_elem; q_elem = q_elem->next) {
             free(q_elem->ptr);
             free(q_elem);
          }
                                       /* Set the queue to empty              */
          DosResetEventSem(pWinData->pipe_in.q_data, &postcount);
                                       /* Clear the list                      */
          pWinData->pipe_in.q_1st = NULL;
                                       /* We're done with the queue           */
          DosReleaseMutexSem(pWinData->pipe_in.q_sem);
        }

        DosReleaseMutexSem(pWinData->imp_ptr->sem);

         /* Clear the input pipe, so that any unprocessed data is deleted.    */
         /* The only way to do this is to close the pipe, and then reopen it. */
        DosClose(pWinData->stdin_r);
        DosClose(pWinData->stdin_w);
        makepipe(&pWinData->stdin_r, STDIN, &pWinData->stdin_w, 4, PIPE_SIZE);
        setinherit(pWinData->stdin_r, TRUE);
        setinherit(pWinData->stdin_w, FALSE);

        WinSetFocus(HWND_DESKTOP, pWinData->insle);
        break;
      }

      case  REXX_MAKE_INVISIBLE :

        if (pWinData->visible) {
          pWinData->visible = FALSE;   /* reset flag to invisible             */
            WinShowWindow(pWinData->frame, FALSE);
                                       /* Make it invisible.                  */
        }
        break;

      case  REXX_MAKE_VISIBLE :

         /* At start-up, the window is left invisible. When output or         */
         /*   trace mode input occurs for the first time, this message is     */
         /*   sent to make the window visible. It is at this time that        */
         /*   the Help Manager gets initialized.                              */

        if (!pWinData->visible) {
          HELPINIT hmiData;

          pWinData->visible = TRUE;    /* Set so we don't get called          */
                                       /* unnecessarily                       */
          RestoreWindow(hwnd, pWinData);/* From last invocation               */
          WinShowWindow(pWinData->frame, TRUE);
                                       /* Make it visible. Initialize the Help*/
                                       /* Manager data structure              */
          memset(&hmiData, '\0', sizeof hmiData);
          hmiData.cb = sizeof(hmiData);
          hmiData.pszHelpWindowTitle =
              getstring(pWinData->hab, pWinData->hResource, PMREXX_HELP_TITLE);
          hmiData.phtHelpTable = (PHELPTABLE)(PMREXX_HELPTABLE|RESOURCE_FLAG);
          hmiData.pszHelpLibraryName =
              getstring(pWinData->hab, pWinData->hResource, PMREXX_HELPFILENAME);
          hmiData.fShowPanelId = CMIC_HIDE_PANEL_ID;
          hmiData.hmodHelpTableModule = pWinData->hResource;
          hmiData.hmodAccelActionBarModule = pWinData->hResource;
          hmiData.idAccelTable = 0;
          hmiData.idActionBar = 0;
                                       /* Attempt to create help instance     */

          if (pWinData->HelpInst = WinCreateHelpInstance(pWinData->hab,
              &hmiData)) {
                                       /* If the create succeeded, then we    */
                                       /* need to associate it with the frame */
                                       /* window.                             */

            if (!WinAssociateHelpInstance(pWinData->HelpInst, pWinData->frame)) {
                                       /* If the associate failed, inform the */
                                       /* user and destroy the instance.      */
              RHWinErrorBox(pWinData->hab, hwnd, pWinData->hResource,
                  ERROR_HELPASSOC, MB_OK|MB_CUAWARNING);
              WinDestroyHelpInstance(pWinData->HelpInst);
              pWinData->HelpInst = NULLHANDLE;
            }
          }

          else {
                                       /* If the create failed, inform the    */
                                       /* user.                               */
            RHWinErrorBox(pWinData->hab, hwnd, pWinData->hResource,
                ERROR_HELPCREATE, MB_OK|MB_CUAWARNING);
          }
          WinSetFocus(HWND_DESKTOP, pWinData->insle);

        }
        break;

      case  RXIOB_MSGBOX : {           /* Put up a message box. The mp1       */
                                       /* parameter has the resource ids for  */
                                       /* the message and the title.          */
          USHORT msgid = SHORT1FROMMP(mp1);
          PUCHAR header = getstring(pWinData->hab, pWinData->hResource,
              SHORT2FROMMP(mp1));
          PUCHAR content = getstring(pWinData->hab, pWinData->hResource,
              SHORT1FROMMP(mp1));
                                       /* If we are                           */
                                       /* displaying the ended message, set   */
                                       /* the owner to the desktop so we don't*/
                                       /* steal the focus back from other     */
                                       /* apps.                               */

          LONG  rc = WinMessageBox(HWND_DESKTOP, hwnd, content, header,
                                   msgid, SHORT1FROMMP(mp2));
          free(header);
          free(content);
          return  MPFROMSHORT(rc);
        }
        break;

      case  START_TIMER :

         /* Start a timer to force output after a period of inactivity.       */

        if (!pWinData->visible) {      /* Make the window visible, if not     */
                                       /* already.                            */
          WinSendMsg(WinWindowFromID(hwnd, (USHORT)FID_CLIENT),
                     REXX_MAKE_VISIBLE, (MPARAM)0, (MPARAM)0);
        }
        WinStartTimer(pWinData->hab, hwnd, pWinData->timer, TIMER_DELAY);
        pWinData->imp_queued = FALSE;
        break;

      case  ADD_STDOUTWIN :

         /* Add an output line to the MLE buffer, and start the timer.        */

        if (!pWinData->visible) {      /* Make the window visible, if not     */
                                       /* already.                            */
          WinSendMsg(WinWindowFromID(hwnd, (USHORT)FID_CLIENT),
                     REXX_MAKE_VISIBLE, (MPARAM)0, (MPARAM)0);
        }
        if (pWinData->client == hwnd) {
          HWND    wnd = pWinData->outmle;
          PIMPORT im_ptr = pWinData->imp_ptr;
          PUCHAR  ptr = (PUCHAR)im_ptr->data;
          PUCHAR  tmp1 = (PUCHAR)mp1;  /* Obtain the buffer semaphore to      */
                                       /* prevent interference with the input */
                                       /* pipe thread.                        */

          DosRequestMutexSem(im_ptr->sem, SEM_INDEFINITE_WAIT);
                                       /* If the string is too                */
                                       /* large, break it up into pieces and  */
                                       /* add to the window.                  */
          while (strlen((const char *)tmp1) > (ULONG)IMPORTSIZE-HDRSIZE-1) {
                                       /* Save the character we are about to  */
                                       /* null out                            */
            UCHAR tch = *(tmp1+IMPORTSIZE-HDRSIZE-1);
                                       /* Add smaller string to the window    */
            *(tmp1+IMPORTSIZE-HDRSIZE-1) = '\0';

            WinSendMsg(hwnd, ADD_STDOUTWIN, tmp1, (MPARAM)mp1);
                                       /* Then bump the pointer and restore   */
                                       /* the character.                      */
            tmp1 += IMPORTSIZE - HDRSIZE - 1;
            *tmp1 = tch;
          }                            /* endwhile See if there is room left  */
                                       /* in the buffer.                      */

          while (((im_ptr->total_size)-(im_ptr->inuse)) <= strlen((const char *)tmp1)) {
                                       /* need to import what we have         */
            WinStopTimer(pWinData->hab, hwnd, pWinData->timer);
                                       /* empty the buffer                    */
            Import(pWinData);
          }

          strcat(ptr, (const char *)tmp1);           /* Copy the data to the end of the     */
                                       /* buffer                              */
          im_ptr->inuse = HDRSIZE+strlen((const char *)ptr);/* Reset the use count          */
                                       /* and free the semaphore              */
          DosReleaseMutexSem(im_ptr->sem);
          free(mp1);                   /* free the text                       */
                                       /* Start the timer to force output if  */
                                       /* nothing added for the specified     */
                                       /* time.                               */
          WinStartTimer(pWinData->hab, hwnd, pWinData->timer, TIMER_DELAY);
          pWinData->imp_queued = FALSE;
          return (MRESULT)TRUE;
        }

        return (MRESULT)FALSE;
        break;

      case  HM_QUERY_KEYS_HELP :

        return (MRESULT)RX_KEYSHELP;

      case  WM_SAVEAPPLICATION :

        SaveWindow(pWinData);
        break;

      default  :
        break;
    }
    return (WinDefWindowProc(hwnd, msg, mp1, mp2));

  }

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: RXIOCmds                                                */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Process WM_COMMAND messages for the window procedure.        */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This routine processes all the WM_COMMAND messages that have */
/*               been passed into the MainWndProc. The messages handled are:  */
/*       RXIOA_TRCSTEP  - This requests that the REXX trace be advanced one   */
/*                        step. It is equivalent to an input of a blank line  */
/*                        to a REXX trace read. It is only valid when REXX is */
/*                        waiting for trace input.                            */
/*       RXIOA_TRCLAST  - This requests that the REXX re-trace the last       */
/*                        clause executed. Is is equivalent to an input of an */
/*                        equal sign to a REXX trace read. It is only valid   */
/*                        when REXX is waiting for trace input.               */
/*       RXIOA_TRCOFF   - This requests that the REXX turn off interactive    */
/*                        tracing. Is is equivalent to an input of a "TRACE   */
/*                        OFF" statement to a REXX trace read. It is only     */
/*                        valid when REXX is waiting for trace input.         */
/*       RXIOA_TRACE   - This is a request to turn interactive tracing on    */
/*                        or off. It is only valid when the REXX procedure    */
/*                        has ended and the user has not yet restarted it.    */
/*                        This is because REXX only looks at the trace flag   */
/*                        when it is first called to execute a procedure.     */
/*       RXIOA_HALT     - This requests that the REXX HALT indicator be       */
/*                        raised, requesting the procedure to be interrupted. */
/*       RXIOA_CUT      - This is a request to remove the marked area of the  */
/*                        focus window and place it onto the clipboard.       */
/*       RXIOA_DELETE   - This is a request to delete the marked area of the  */
/*                        focus window.                                       */
/*       RXIOA_PASTE    - Paste from the clipboard into the input window.     */
/*       RXIOA_COPY     - This is a request to copy the marked area of the    */
/*                        focus window into the clipboard.                    */
/*       RXIOA_ALL      - This is a request to mark the entire output area.   */
/*       RXIOA_FONTS    - Present the user with a dialog to select a font for */
/*                        the output window.                                  */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*       MRESULT RXIOCmds(HWND hwnd, MPARAM mp1, PRHWINDATA pWinData)         */
/*                                                                            */
/*   INPUT:                                                                   */
/*       hwnd           - The window handle of the window that received the   */
/*                        command.                                            */
/*       mp1            - The first parameter of the WM_COMMAND message.      */
/*       pWinData       - A pointer to the RHWINDATA structure of the window. */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       returns TRUE if the file was saved successfully or user response     */
/*       from SAVEAS dialog.                                                  */
/*   EXIT-ERROR:                                                              */
/*       returns FALSE if the file was not saved successfully.                */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       Add_Q_Element  - Adds an entry to an input queue.                    */
/*       RHWinErrorBox  - Display an error message box.                       */
/*       ClipBoard        - Perform clipboard related commands.               */
/*       SizeWindow     - Resize the child windows within the client area.  */
/*       SelectFont     - Interface to the font selection dialog              */
/*       SetCheckMark   - Check/uncheck a menu item                           */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       DosRequestMutexSem             WinInvalidateRect                     */
/*       DosReleaseMutexSem             WinLoadDlg                            */
/*       GpiQueryFontMetrics            WinLoadString                         */
/*       WinAlarm                       WinPostMessage                        */
/*       WinCreateWindow                WinReleasePS                          */
/*       WinDlgBox                      WinSendMsg                            */
/*       WinEnableWindowUpdate          WinShowWindow                         */
/*       WinGetPS                       WinWindowFromID                       */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

MRESULT RXIOCmds(HWND hwnd, MPARAM mp1, PRHWINDATA pWinData)
{
    CHAR Temp = '\0';                  /* temporary character                 */
                                       /* dialog                              */
    PLIST_ENTRY q_elem;                /* pointer for an element in the input */
                                       /* queue                               */

    switch (LOUSHORT(mp1)) {

      /************************************************************************/
      /* Append or overwrite output data it MLE                              */
      /************************************************************************/

      case  RXIOA_RESULTS :

        pWinData->RxResult = !pWinData->RxResult;
        SetCheckMark(pWinData->frame, RXIOA_RESULTS, pWinData->RxResult);
        WinSendMsg(pWinData->client, WM_COMMAND,
           MPFROMSHORT(PMRXA_RESULTCHANGE), (MPARAM)0);
        break;

      case  RXIOA_TRCSTEP :

         /* Trace the next clause. This is accomplished by inputting a        */
         /*   null line.                                                      */

        if (q_elem = malloc(sizeof(*q_elem))) {
          q_elem->ptr = strdup("");
          q_elem->len = strlen(q_elem->ptr);
          Add_Q_Element(&pWinData->trace_in, q_elem);
        }
        break;

      case  RXIOA_TRCLAST :

         /* Re-trace the previous clause. This is accomplished by             */
         /*   inputting an equal (=) sign alone on a line.                    */

        if (q_elem = malloc(sizeof(*q_elem))) {
          q_elem->ptr = strdup("=");
          q_elem->len = strlen(q_elem->ptr);
          Add_Q_Element(&pWinData->trace_in, q_elem);
        }
        break;

      case  RXIOA_TRCOFF :

         /* Turn tracing off by calling RexxResetTrace                        */

        RexxResetTrace(pWinData->proc_id, pWinData->rexx_tid);
        pWinData->tracebit = FALSE;
                                       /* Clear the checkmark                 */
                                       /* send a null line to end pause       */
        if (q_elem = malloc(sizeof(*q_elem))) {
          q_elem->ptr = strdup("trace off");
          q_elem->len = strlen(q_elem->ptr);
          Add_Q_Element(&pWinData->trace_in, q_elem);
        }

        SetCheckMark(pWinData->frame, RXIOA_TRACE, FALSE);
        WinSendMsg(pWinData->client, WM_COMMAND,
           MPFROMSHORT(PMRXA_FLAGCHANGE), (MPARAM)0);

        break;

      case  RXIOA_TRACE :

         /* Change the state of the interactive trace indiciator. This        */
         /*   will turn tracing on immediately (with the next phrase          */
         /*   executed).                                                      */

        if (pWinData->tracebit = (BOOL)(!pWinData->tracebit)) {
          RexxSetTrace(pWinData->proc_id, pWinData->rexx_tid);
        }
        else {
          RexxResetTrace(pWinData->proc_id, pWinData->rexx_tid);
        }
        SetCheckMark(pWinData->frame, RXIOA_TRACE, pWinData->tracebit);
        WinSendMsg(pWinData->client, WM_COMMAND,
           MPFROMSHORT(PMRXA_FLAGCHANGE), (MPARAM)1);


        break;

      case  RXIOA_HALT :

         /* Change the Halt Procedure indicator. This will cause REXX to      */
         /*   terminate execution of the procedure when the next phrase       */
         /*   is executed.                                                    */

        RexxSetHalt(pWinData->proc_id, pWinData->rexx_tid);
        /* If REXX is waiting for input, it won't halt until it gets          */
        /* some input, so feed it a blank line, just in case                  */

        if (q_elem = malloc(sizeof(*q_elem))) {
          q_elem->ptr = strdup("");
          q_elem->len = 0;
          if (pWinData->trace_inp) {
            Add_Q_Element(&pWinData->trace_in, q_elem);
          } else {
            Add_Q_Element(&pWinData->pipe_in, q_elem);
          } /* endif */
        }
        if (!pWinData->RxHalt) {
          pWinData->RxHalt = TRUE;
          SetCheckMark(pWinData->frame, RXIOA_HALT, pWinData->RxHalt);
        }
        break;

      /************************************************************************/
      /* These messages relate to clipboard operations                        */
      /************************************************************************/

      case  RXIOA_CUT :

        if (pWinData->infocus) {       /* clear the input window, because it  */
                                       /* has the focus                       */
          WinSendMsg(pWinData->insle, EM_CUT, (MPARAM)0, (MPARAM)0);
        }
        else {                         /* clear the MLE                       */
          ClipBoard(hwnd, MLM_CUT, RXIOH_ERR_CUT, pWinData);
        }
        break;

      case  RXIOA_DELETE :

        if (pWinData->infocus) {       /* clear the input window, because it  */
                                       /* has the focus                       */
          WinSendMsg(pWinData->insle, EM_CLEAR, (MPARAM)0, (MPARAM)0);
        }
        else {                         /* clear the MLE                       */
          ClipBoard(hwnd, MLM_CLEAR, RXIOH_ERR_DELETE, pWinData);
        }
        break;

      case  RXIOA_PASTE :

         /* put the text from the clipboard into the input window             */

        WinSendMsg(pWinData->insle, EM_PASTE, (MPARAM)0, (MPARAM)0);
        break;

      case  RXIOA_COPY :

        if (pWinData->infocus) {       /* Copy text from input window         */
          WinSendMsg(pWinData->insle, EM_COPY, (MPARAM)0, (MPARAM)0);
        }
        else {                         /* Get the text from the MLE           */
          ClipBoard(hwnd, MLM_COPY, RXIOH_ERR_COPY, pWinData);
        }
        break;

      /************************************************************************/
      /* Select all                                                           */
      /************************************************************************/

      case  RXIOA_ALL : {

          ULONG  usFileSize;

          WinEnableWindowUpdate(hwnd, FALSE);
                                       /* disable updates Get length of the   */
                                       /* output in the MLE                   */
          usFileSize = (ULONG )WinSendMsg(WinWindowFromID(hwnd,
                                          (USHORT)MLE_WNDW),
                                          MLM_QUERYTEXTLENGTH,
                                          (MPARAM)0, (MPARAM)0);
                                       /* Select all of the output in the MLE */
          WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                     MLM_SETSEL, (MPARAM)0L, MPFROMSHORT(usFileSize));
          WinInvalidateRect(hwnd, NULL, TRUE);
                                       /* update the window                   */
          WinEnableWindowUpdate(hwnd, TRUE);
                                       /* Make MLE so that it can't be undone */
          WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                     MLM_RESETUNDO, (MPARAM)0, (MPARAM)0);
          WinSetFocus(HWND_DESKTOP, pWinData->outmle);
        }
        break;


      /************************************************************************/
      /* DeSelect all                                                         */
      /************************************************************************/

      case  RXIOA_DALL : {

          ULONG  usFileSize;

          WinEnableWindowUpdate(hwnd, FALSE);
                                       /* disable updates Get length of the   */
                                       /* output in the MLE                   */
          usFileSize = 0;
                                     /* DeSelect all of the output in the MLE */
          WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                     MLM_SETSEL, (MPARAM)0L, MPFROMSHORT(usFileSize));
          WinInvalidateRect(hwnd, NULL, TRUE);
                                       /* update the window                   */
          WinEnableWindowUpdate(hwnd, TRUE);
                                       /* Make MLE so that it can't be undone */
          WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                     MLM_RESETUNDO, (MPARAM)0, (MPARAM)0);
          WinSetFocus(HWND_DESKTOP, pWinData->outmle);
        }
        break;

      /************************************************************************/
      /* Present Font dialog; the dialog will change the PS attribute         */
      /************************************************************************/

      case  RXIOA_FONTS :
        WinSendMsg(WinWindowFromID(hwnd, (USHORT) MLE_WNDW),
                   MLM_RESETUNDO, (MPARAM)0, (MPARAM)0);
        SelectFont(pWinData);
        break;


      /************************************************************************/
      /* Display the Search dialog.                                           */
      /************************************************************************/

      case  RXIOA_SEARCH :
        {
         CHAR           szSearchString[100];
         MLE_SEARCHDATA mlesrch;
         USHORT         usResult;
         SEARCHINFO     searchinfo;

         searchinfo.size = sizeof(SEARCHINFO);
         usResult = WinDlgBox(HWND_DESKTOP, hwnd, (PFNWP)SearchDlgProc,
                             (HMODULE)NULL, PMREXX_SEARCH, &searchinfo);
         if (usResult == PB_OK) {
           mlesrch.cb = sizeof(mlesrch);                   /* Structure size           */
           mlesrch.pchFind = searchinfo.szSearchString;    /* Search string            */
           mlesrch.pchReplace = NULL;                      /* No replacement string    */
           mlesrch.cchFind = 0;                            /* Not used                 */
           mlesrch.cchReplace = 0;                         /* Not used                 */
           mlesrch.iptStart = -1;                          /* Start at cursor position */
           mlesrch.iptStop = -1;                           /* Stop at end of file      */
           mlesrch.cchFound = 0;                            /* See if string was round  */
           if (searchinfo.caseSensitive) {
             WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                        MLM_SEARCH, MPFROMLONG(MLFSEARCH_CASESENSITIVE | MLFSEARCH_SELECTMATCH),
                        MPFROMP(&mlesrch));
           } else {
             WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                        MLM_SEARCH, MPFROMLONG(MLFSEARCH_SELECTMATCH),
                        MPFROMP(&mlesrch));
           } /* endif */
           if (mlesrch.cchFound < 1) {
             WinMessageBox(HWND_DESKTOP, NULLHANDLE,
                 getstring(pWinData->hab, pWinData->hResource, SEARCH_ERROR),
                 getstring(pWinData->hab, pWinData->hResource, SEARCH_TITLE), 1,
                 MB_OK|MB_INFORMATION|MB_MOVEABLE);
           } else {
             if (pWinData->infocus) {
               pWinData->infocus = FALSE;
               WinSetFocus(HWND_DESKTOP, pWinData->outmle);
             } /* endif */
           } /* endif */
         } /* endif */
        }
        break;
      /************************************************************************/
      /* Display Help for help panel                                          */
      /************************************************************************/

      case  RXIOM_HELPHELP :

        if (pWinData->HelpInst != NULLHANDLE) {
          WinSendMsg(pWinData->HelpInst, HM_DISPLAY_HELP, (MPARAM)0, (MPARAM)0);
        }
        else {
          WinSendMsg(pWinData->client,
                     RXIOB_MSGBOX,
                     MPFROM2SHORT(NO_HELP_MGR, HELP_ERR),
                     MPFROMLONG(MB_ENTER|MB_CUACRITICAL|MB_MOVEABLE));
        }
        break;

      /************************************************************************/
      /* Display Extended help panel                                          */
      /************************************************************************/

      case  RXIOM_EXTHELP :

        if (pWinData->HelpInst != NULLHANDLE) {
          WinSendMsg(pWinData->HelpInst, HM_EXT_HELP, (MPARAM)0, (MPARAM)0);
        }
        else {
          WinSendMsg(pWinData->client,
                     RXIOB_MSGBOX,
                     MPFROM2SHORT(NO_HELP_MGR, HELP_ERR),
                     MPFROMLONG(MB_ENTER|MB_CUACRITICAL|MB_MOVEABLE));
        }
        break;

      /************************************************************************/
      /* Display keys help panel                                              */
      /************************************************************************/

      case  RXIOM_KEYSHELP :

        if (pWinData->HelpInst != NULLHANDLE) {
          WinSendMsg(pWinData->HelpInst, HM_KEYS_HELP, (MPARAM)0, (MPARAM)0);
        }
        else {
          WinSendMsg(pWinData->client, RXIOB_MSGBOX,
                     MPFROM2SHORT(NO_HELP_MGR, HELP_ERR),
                     MPFROMLONG(MB_ENTER|MB_CUACRITICAL|MB_MOVEABLE));
        }
        break;

      /************************************************************************/
      /* Display help index                                                   */
      /************************************************************************/

      case  RXIOM_INDEXHELP :

        if (pWinData->HelpInst != NULLHANDLE) {
          WinSendMsg(pWinData->HelpInst, HM_HELP_INDEX, (MPARAM)0, (MPARAM)0);
        }
        else {
          WinSendMsg(pWinData->client, RXIOB_MSGBOX,
                     MPFROM2SHORT(NO_HELP_MGR, HELP_ERR),
                     MPFROMLONG(MB_ENTER|MB_CUACRITICAL|MB_MOVEABLE));
        }
        break;

      /************************************************************************/
      /* Option not implemented: Warn the user                                */
      /************************************************************************/

      default  :
        WinAlarm(HWND_DESKTOP, WA_ERROR);
    }                                  /* endswitch                           */
    return (MRESULT)FALSE;
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: ClipBoard                                               */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Process the MLE clipboard functions by passing the message   */
/*               on to the MLE window.                                        */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               All of the clipboard related functions are handled very      */
/*               simliarly and so are just passed directly on to the MLE      */
/*               window for processing. In case of a failure, a message       */
/*               box is popped up to notify the user.                         */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          CipBoard(hWnd, MLECMsg, ErrMsg, pWinData)                         */
/*                                                                            */
/*   INPUT:                                                                   */
/*       hWnd           - The handle of the window that received the message  */
/*       MLECMsg        - The message to be processed.                        */
/*       ErrMsg         - The resource ID of the error message to use in      */
/*                        case of an error.                                   */
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
/*       WinSendMsg                                                           */
/*       WinWindowFromID                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

  void ClipBoard(HWND hwnd, USHORT MLECMsg, ULONG ErrMsg,
      PRHWINDATA pWinData) {

   /* If an error occurs, put up a message box                                */

    if (!((MRESULT)WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
        MLECMsg, (MPARAM)0, (MPARAM)0)))

      WinSendMsg(pWinData->client, RXIOB_MSGBOX,
                 MPFROM2SHORT(ErrMsg, RXIOH_ERR_CLIPBRD),
                 MPFROMLONG(MB_ENTER|MB_CUACRITICAL|MB_MOVEABLE));
  }

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: Import                                                  */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Add the information collected into the import buffer into    */
/*               the MLE window.                                              */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Adding information to the MLE involves considerable          */
/*               overhead. To do so on each individual line would be          */
/*               unacceptably slow for any considerable amount of output.     */
/*               Therefore, output is collected into a buffer and output      */
/*               to the MLE when the buffer is full, or there has been no     */
/*               output added for some period of time. This is the routine    */
/*               to take the information collected in the buffer area and     */
/*               add it to the output window.                                 */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void Import(PRHWINDATA pWinData)                                  */
/*                                                                            */
/*   INPUT:                                                                   */
/*       pWinData       - Pointer to global control block.                    */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       RHWinErrorBox  - Display an error message box.                       */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       WinPostMsg                                                           */
/*       WinQueryWindow                                                       */
/*       WinSendMsg                                                           */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

  void Import(PRHWINDATA pWinData) {
    IPT ipt = (IPT)NULL;               /* Integer pointer type for MLE        */


    if (pWinData->imp_ptr->inuse > HDRSIZE) {

      /* Get the number of characters currently in the MLE                    */

      ipt = (IPT)WinSendMsg(pWinData->outmle, MLM_QUERYTEXTLENGTH,
          (MPARAM)0, (MPARAM)0);

      /* If we had an error setting up the transfer buffer, display a message */

      if (!WinSendMsg(pWinData->outmle, MLM_SETIMPORTEXPORT,
          pWinData->imp_ptr->data,
          MPFROMLONG(pWinData->imp_ptr->total_size-HDRSIZE))) {
        RHWinErrorBox(pWinData->hab, pWinData->outmle, pWinData->hResource,
            MLE_IMEXPORT_ERROR, MB_OK|MB_CUACRITICAL);
        WinPostMsg(WinQueryWindow(pWinData->outmle, QW_PARENT),
                   WM_CLOSE, (MPARAM)0, (MPARAM)0);
        return ;
      }
      WinEnableWindowUpdate(pWinData->outmle, FALSE);

      /* Set the MLE to not translate the input                               */
      WinSendMsg(pWinData->outmle, MLM_FORMAT, MPFROMSHORT(MLFIE_NOTRANS),
          (MPARAM)0);

      /* Set the MLE so we can add data to it                                 */
      WinSendMsg(pWinData->outmle, MLM_SETREADONLY, FALSE, (MPARAM)0);

      /* Add the data to the MLE, if we get an error, display a message       */
                                       /* upon return, point to end of data   */
      if (!WinSendMsg(pWinData->outmle, MLM_IMPORT, &ipt,
          MPFROMLONG(pWinData->imp_ptr->inuse-HDRSIZE))) {

        RHWinErrorBox(pWinData->hab, pWinData->outmle, pWinData->hResource,
            MLE_IMPORT_ERROR, MB_OK|MB_CUACRITICAL);

         /* It probably failed because the line is too long, so append a      */
         /* new line and try again.                                           */
        WinSendMsg(pWinData->outmle, MLM_SETSEL, MPFROMLONG(ipt),
            MPFROMLONG(ipt));
        WinSendMsg(pWinData->outmle, MLM_INSERT, "\r", (MPARAM)0);

         /* Move the selection to the end of the input                        */
        ipt = (IPT)WinSendMsg(pWinData->outmle, MLM_QUERYTEXTLENGTH, (MPARAM)0,
            (MPARAM)0);

        if (!WinSendMsg(pWinData->outmle, MLM_IMPORT, &ipt,
            MPFROMLONG(pWinData->imp_ptr->inuse-HDRSIZE) )) {
                                       /* If we still couldn't import         */
                                       /* anything, give up.                  */
          WinPostMsg(WinQueryWindow(pWinData->outmle, QW_PARENT),
                     WM_CLOSE, (MPARAM)0, (MPARAM)0);
        }
      }

      /* Update the ptr structure                                             */

      *(PUCHAR)(pWinData->imp_ptr->data) = '\0';
      pWinData->imp_ptr->inuse = HDRSIZE;

      /* Move the cursor to the end of the new data                           */
      WinSendMsg(pWinData->outmle, MLM_SETSEL, MPFROMLONG(ipt), MPFROMLONG(ipt));

      WinEnableWindowUpdate(pWinData->outmle, TRUE);

      /* Set the MLE to normal export mode                                    */
      WinSendMsg(pWinData->outmle, MLM_FORMAT, MLE_TXT_FMT, (MPARAM)0);

      /* Set the MLE back to readonly                                         */
      WinSendMsg(pWinData->outmle, MLM_SETREADONLY, MPFROMLONG(TRUE), (MPARAM)0);

    }
  }

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: SizeWindow                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Size the MLE output window and the Entry Field input window  */
/*               to fit the client area.                                      */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Whenever the client window is sized, this routine is         */
/*               called to resize the MLE output window and the Entry Field   */
/*               control used for input to fit properly within the new        */
/*               client area.                                                 */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void SizeWindow(HWND hwnd, PRHWINDATA pWinData)                   */
/*                                                                            */
/*   INPUT:                                                                   */
/*       hwnd           - The handle of the client window.                    */
/*       pWinData       - A pointer to the RHWINDATA structure of the window. */
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
/*       WinQueryWindow                                                       */
/*       WinQueryWindowPos                                                    */
/*       WinSetWindowPos                                                      */
/*       WinWindowFromID                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

  void SizeWindow(HWND hwnd, PRHWINDATA pWinData) {
    HWND hOwner;                       /* Window handle of parent window      */
    SWP windowPos;                     /* info on window position             */
    ULONG  in_hgt = pWinData->in_hgt;  /* Height for input entry field        */


    if (!pWinData->visible)
      return ;                         /* If the window is not visible, do    */
                                       /* nothing                             */
    hOwner = WinQueryWindow(hwnd, QW_PARENT);

   /* Get Window positions                                                    */
    WinQueryWindowPos(WinWindowFromID(hOwner, (USHORT) FID_CLIENT), &windowPos);

   /* adjust MLE and input field, if created                                  */
    if (pWinData->outmle && pWinData->insle) {

      WinSetWindowPos(pWinData->outmle,   /* set window a bit smaller         */
                      HWND_TOP,
                      (SHORT)BORDER_MARGIN,
                      (SHORT)BORDER_MARGIN-1,
                      (SHORT)(windowPos.cx-(BORDER_MARGIN *2)+1),
                      (SHORT)(windowPos.cy-in_hgt-(BORDER_MARGIN *6)),
                      SWP_SIZE|SWP_MOVE|SWP_ZORDER|SWP_SHOW);

      WinSetWindowPos(pWinData->insle,   /* set window a bit smaller          */
                      HWND_TOP,
                      (SHORT)(BORDER_MARGIN *3+pWinData->wtitle),
                      (SHORT)(windowPos.cy-in_hgt-BORDER_MARGIN *3),
                      (SHORT)(windowPos.cx-BORDER_MARGIN *5-pWinData->wtitle),
                      (SHORT)pWinData->in_hgt,
                      SWP_SIZE|SWP_MOVE|SWP_ZORDER|SWP_SHOW);
    }
    return ;
  }

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: io_routine                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Process the REXX IO exit.                                    */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This function has been registered to handle the IO exits     */
/*               for the REXX procedure. The interface is defined by REXXSAA. */
/*               For the input requests (RXSIOTRD and RXSIODTR), first the    */
/*               window is made visible, if it isn't already, and then the    */
/*               trace options are enabled/disabled as appropriate. Then      */
/*               a return code is set to allow normal operation of I/O by     */
/*               REXX.                                                        */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          LONG  io_routine( LONG  exitno, LONG  subfunc, PUCHAR parmblock)  */
/*                                                                            */
/*   INPUT:                                                                   */
/*       The arguments are defined by the REXX interface specifications.      */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       Remove_Q_Element: Remove a queue element, wait if none there         */
/*       SetCheckMark   - Check/uncheck a menu item                           */
/*       SetOptions     - Enable/disable menu items.                          */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

LONG  io_routine(LONG  exitno, LONG  subfunc, PUCHAR parmblock)
{
    LONG           rc;                 /* rc=1 says use standard I/O routines */
    PLIST_ENTRY    q_elem;             /* entry in queue                      */
    RXSIOSAY_PARM *sparm = (RXSIOSAY_PARM *)parmblock;/* struc for IO exit    */
    PRHWINDATA     userdata[2];        /* registered user data                */
    PRHWINDATA     pWinData;           /* global data anchor                  */
    USHORT         exists;             /* existance flag                      */

                                       /* retrieve the anchor                 */
    if (RexxQueryExit(PMRXIO_EXIT, NULL, &exists, (PUCHAR)userdata))
                                       /* failed?                             */
      return RXEXIT_RAISE_ERROR;       /* raise a system failure error        */

    rc = RXEXIT_NOT_HANDLED;           /* let others go through               */
    pWinData = userdata[0];            /* get saved global anchor             */

    if (exitno == RXSIO) {

      switch (subfunc) {

        case  RXSIODTR :               /* For trace read we want to enable the*/
                                       /* trace menu items so we must perform */
                                       /* the read ourselves.                 */
                                       /* Enable all the options              */
          SetOptions(pWinData->frame, RXIOA_TRCSTEP, TRUE);
          SetOptions(pWinData->frame, RXIOA_TRCLAST, TRUE);
          SetOptions(pWinData->frame, RXIOA_TRCOFF, TRUE);

          if (!pWinData->tracebit) {   /* Set tracing active flag in menu     */
            SetCheckMark(pWinData->frame, RXIOA_TRACE,
                (BOOL)(!pWinData->tracebit));
          }
          pWinData->trace_inp = TRUE;  /* Set the trace active flag for IO    */
          pWinData->in_type = TRC_IN;

          q_elem = Remove_Q_Element(&pWinData->trace_in);/* And get input     */
          pWinData->in_type = STD_IN;
          pWinData->trace_inp = FALSE; /* Reset the trace active flag for IO  */
                                       /* Disable all the options             */
          SetOptions(pWinData->frame, RXIOA_TRCSTEP, FALSE);
          SetOptions(pWinData->frame, RXIOA_TRCLAST, FALSE);
          SetOptions(pWinData->frame, RXIOA_TRCOFF, FALSE);

          if (!pWinData->tracebit) {   /* Reset the trace flag as appropriate */
            SetCheckMark(pWinData->frame, RXIOA_TRACE, pWinData->tracebit);
          }

         /* Copy the input line to the return area.                           */

          strcpy(sparm->rxsio_string.strptr, q_elem->ptr);
          sparm->rxsio_string.strlength = strlen(q_elem->ptr);
          free(q_elem->ptr);           /* And free the memory.                */
          free(q_elem);
          rc = RXEXIT_HANDLED;         /* Set the return code to              */
          break;                       /* input complete.                     */

        case  RXSIOTRD :               /* If a read occurs and the window is  */
                                       /* not visible, make it visible.       */

          if (!pWinData->visible) {

            while (!WinPostMsg(pWinData->client, (USHORT)REXX_MAKE_VISIBLE,
                (MPARAM)0, (MPARAM)0)) {
              DosSleep((ULONG)TIMER_DELAY);
            }
          }
          break;

        default  :                     /* For all else, let REXX do the       */
          break;                       /* work.                               */
      }                                /* endswitch                           */
    }
    return  rc;
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: stdinpipe                                               */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Send input lines to the standard input of child processes.   */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This function is the thread that waits for input for child   */
/*               processes started by calls to the subcommand environment,    */
/*               passes that input through to the pipe connected to their     */
/*               standard input handle.                                       */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void stdinpipe()                                                  */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       Remove_Q_Element - Removes an entry from an input queue. Waits for   */
/*                          one if the queue is empty.                        */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       DosWaitEventSem                                                      */
/*       DosWrite                                                             */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

  void stdinpipe(PRHWINDATA pWinData) {
    PLIST_ENTRY q_elem;                /* element from queue                  */
    UCHAR       str[ANSLEN+5];         /* data from queue entry               */
    ULONG       written;               /* bytes written by DosWrite           */

    while (TRUE) {
      q_elem = Remove_Q_Element(&pWinData->pipe_in);
      strcpy(str, q_elem->ptr);
      strcat(str, "\r\n");

      DosWrite(pWinData->stdin_w, str, strlen((const char *)str), &written);
      free(q_elem->ptr);
      free(q_elem);
    }
    DosExit(EXIT_THREAD, 0);           /* end the thread                      */
  }

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: stdoutpipe                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Send lines from the standard output of child processes to    */
/*               the output window.                                           */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This function is the thread that waits for output from child */
/*               processes started by calls to the subcommand environment,    */
/*               passes that output through to the output window.             */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void stdoutpipe()                                                 */
/*                                                                            */
/*   INPUT:                                                                   */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       addline - Macro to add a line to the output window                   */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       DosPostEventSem                                                      */
/*       DosRequestMutexSem                                                   */
/*       DosReleaseMutexSem                                                   */
/*       DosWaitEventSem                                                      */
/*       DosSleep                                                             */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

  void stdoutpipe(PRHWINDATA pWinData) {
    UCHAR  buffer[MAXWIDTH];           /* holds data from standard out        */
    UCHAR  tchar = '\0';               /* temporary character variable        */
    PUCHAR tptr;                       /* character pointer                   */
    ULONG  len;                        /* counter                             */
    ULONG  nread;                      /* bytes read                          */

    USHORT         exists;             /* existance flag                      */

                                       /* retrieve the anchor                 */
    RexxQueryExit(PMRXIO_EXIT, NULL, &exists, NULL);

    while (!pWinData->stopoutpipe) {

      len = 0;                         /* initialize count back to 0          */
      buffer[0] = '\0';                /* initialize buffer                   */

      /* Loop here until we see a new line, or the buffer is full             */
      do {
        if (!DosRead(pWinData->stdout_r,&tchar, 1, &nread) && nread) {
          if (tchar == '\r') {
            DosRead(pWinData->stdout_r,&tchar, 1, &nread);
            if (tchar != '\n')
              buffer[len++] = '\n';
          }
          buffer[len++] = (UCHAR)tchar;
        }
      }

      while (len < MAXWIDTH-1 && tchar != '\n');

      buffer[len] = END_STR;           /* Terminate the string                */

      if (pWinData->stopoutpipe)       /* shutdown flag?                      */
        break;                         /* time to leave                       */

      /* Now add the data into the import structure, but first get            */
      /* the semaphore.                                                       */

      DosRequestMutexSem(pWinData->imp_ptr->sem, SEM_INDEFINITE_WAIT);

      /* If there is no more room in the import area, add the data            */
      if (pWinData->imp_ptr->total_size <= pWinData->imp_ptr->inuse+len+2) {
        DosReleaseMutexSem(pWinData->imp_ptr->sem);
        addline(pWinData->client, strdup((const char *)buffer));

         /* Give the MLE a chance to add the data                             */

        DosSleep((ULONG)TIMER_DELAY);

      /* There is room in the import area, so add the data                    */

      }

      else {
        tptr = (PUCHAR)pWinData->imp_ptr;
        strcat((tptr + pWinData->imp_ptr->inuse), (const char *)buffer);
        pWinData->imp_ptr->inuse += len;
        DosReleaseMutexSem(pWinData->imp_ptr->sem);

         /* Set (or reset) the timer                                          */
        if (!pWinData->imp_queued) {
          pWinData->imp_queued = TRUE;

          while (!WinPostMsg(pWinData->frame, START_TIMER, NULL, NULL)) {
            DosSleep((ULONG)TIMER_DELAY);
          }
        }
      }
    }                                  /* endwhile                            */
    DosExit(EXIT_THREAD, 0);           /* end the thread                      */
  }

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: EntryDlgProc                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Dialog procedure for the input entry field.                  */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This is the Dialog Procedure that is registered to handle    */
/*               the messages coming into the dialog that establishes the     */
/*               input entry field. The messages that are handled are:        */
/*       WM_BUTTON1DBLCLK - Copy the marked area from the output window into  */
/*                          the input window.                                 */
/*       WM_BUTTON2DBLCLK - Same as pressing ENTER in the input window.       */
/*       WM_CHAR - several characters are processed here. These include:      */
/*         VK_NEWLINE   - take the data in the input window and place it on   */
/*                        current input queue, then clear the entry field.    */
/*         VK_UP        - place the last entered entry into the input field.  */
/*               All other messages and keys are handled by default           */
/*               processing.                                                  */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          This is defined by the PM interface for dialog procedures.        */
/*                                                                            */
/*   INPUT:                                                                   */
/*       item           - The id of the item to enable/disable                */
/*       option         - TRUE to enable the option, FALSE to disable.        */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       addline        - Macro to add a line to the output window            */
/*       Add_Q_Element  - Adds an entry to an input queue.                    */
/*                                                                            */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       WinDefDlgProc                                                        */
/*       WinQueryWindow                                                       */
/*       WinQueryWindowPtr                                                    */
/*       WinQueryWindowText                                                   */
/*       WinSendMsg                                                           */
/*       WinSetFocus                                                          */
/*       WinSetWindowText                                                     */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

MRESULT EXPENTRY EntryDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
    static PUCHAR last_str[2] =  { NULL,NULL };
    LONG       len = ANSLEN;           /* length of input buffer              */
    UCHAR      str[ANSLEN+1];          /* holds data from input buffer        */
    LONG       len1;                   /* number of characters input          */
    WNDPARAMS  wp;                     /* struc to update input window        */
    HWND       hOwner;                 /* parent window handle                */
    IPT        start,end;              /* Insertion point for MLE             */
    PRHWINDATA pWinData;               /* pointer to WinData structure        */

    hOwner = WinQueryWindow(hwnd, QW_PARENT);
    pWinData = WinQueryWindowPtr(hOwner, (SHORT)0);

    switch (msg) {

      case  WM_SETFOCUS :

        pWinData->infocus = (BOOL)(mp2?TRUE:FALSE);
        break;

      case  WM_BUTTON1DBLCLK :

        start = (IPT)WinSendMsg(pWinData->outmle, MLM_QUERYSEL,
            MPFROMSHORT(1), (MPARAM)0);
        end = (IPT)WinSendMsg(pWinData->outmle, MLM_QUERYSEL,
                              MPFROMSHORT(2), (MPARAM)0);

        if (end > start) {
          PUCHAR ptr = (PUCHAR)malloc((LONG )(end-start+2));
          ULONG len = end-start;
          ULONG got;

          if (ptr && WinSendMsg(pWinData->outmle, MLM_SETIMPORTEXPORT, ptr,
              MPFROMLONG(end-start+1))) {

            if (got = (ULONG)WinSendMsg(pWinData->outmle, MLM_EXPORT,
                MPFROMLONG(&start), MPFROMLONG(&len))) {
              PUCHAR ptr1 = ptr;
              PUCHAR ptr2;

              while (!*ptr1)
                ptr1++;                /* Skip over any nulls.                */
              ptr2 = ptr1;             /* Now find the end                    */

              while (*ptr2 &&          /* at NULL, newline or                 */
                  (*ptr2 != '\n') &&   /* carraige return.                    */
                  *ptr2 != '\r' && ptr2-ptr < (INT)got)
                *ptr2++;
              *(ptr2) = '\0';
              WinSetWindowText(hwnd,
                               ptr1);  /* Put into SLE                        */
            }
          }

          if (ptr) {
            free(ptr);
          }
        }
        break;

      case  WM_BUTTON2DBLCLK :

        WinSendMsg(hwnd, WM_CHAR, MPFROM2SHORT(KC_VIRTUALKEY, NULL),
                   MPFROM2SHORT(NULL, VK_NEWLINE));
        break;

      case  WM_CHAR :

        if (!(SHORT1FROMMP(mp1)&KC_KEYUP) &&
            (SHORT1FROMMP(mp1)&KC_VIRTUALKEY) ) {

          switch (SHORT2FROMMP(mp2)) {

            case  VK_BREAK :
              WinSendMsg(pWinData->client, WM_COMMAND,
                  MPFROMSHORT(RXIOA_HALT), (MPARAM)0);
              break;

            case  VK_NEWLINE :
            case  VK_ENTER : {

                PLIST_ENTRY qe = malloc(sizeof *qe);

                len1 = WinQueryWindowText(hwnd, (SHORT)len, str);

                if (qe) {
                  PUCHAR str1 = malloc(len1+4);

                  if (last_str[pWinData->in_type]) {
                    free(last_str[pWinData->in_type]);
                  }
                  memcpy(str1, str, len1);
                  strcpy(str1+len1, "\n");
                                       /* only add input if executing         */
                  last_str[pWinData->in_type] = strdup((const char *)str1);

                  addline(pWinData->client, strdup((const char *)str1));
                  qe->len = len1;
                  qe->ptr = strdup((const char *)str);
                  *(str+len1) = '\0';

                  if (pWinData->trace_inp) {
                    Add_Q_Element(&pWinData->trace_in, qe);
                  }

                  else {
                    Add_Q_Element(&pWinData->pipe_in, qe);
                  }
                  free(str1);
                }
              }
              WinSendMsg(hwnd, EM_SETSEL, MPFROM2SHORT(0, len1), (MPARAM)0);
              WinSendMsg(hwnd, EM_CLEAR, (MPARAM)0, (MPARAM)0);
              return (MRESULT)TRUE;
              break;

            case  VK_UP :

              if (last_str[pWinData->in_type]) {
                WinSendMsg(hwnd, EM_SETSEL, MPFROM2SHORT(0, len1), (MPARAM)0);
                WinSendMsg(hwnd, EM_CLEAR, (MPARAM)0, (MPARAM)0);
                *(last_str[pWinData->in_type] +
                    strlen((const char *)last_str[pWinData->in_type])-1) = '\0';
                wp.fsStatus = WPM_TEXT+WPM_CCHTEXT;
                wp.cchText = (SHORT)strlen((const char *)last_str[pWinData->in_type]);
                wp.pszText = strdup((const char *)last_str[pWinData->in_type]);
                WinSendMsg(hwnd, WM_SETWINDOWPARAMS, &wp, (MPARAM)0);
                *(last_str[pWinData->in_type] +
                    strlen((const char *)last_str[pWinData->in_type])) = '\r';
                return (MRESULT)TRUE;
              }
              break;

            case  VK_TAB :
            case  VK_BACKTAB :

              return (MRESULT)WinSetFocus(HWND_DESKTOP,
                  pWinData->infocus?pWinData->outmle: pWinData->insle);
              break;

            default  :
              break;
          }
        }
      default  :
        break;
    }
    return  WinDefDlgProc(hwnd, msg, mp1, mp2);
  }

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: SelectFont                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Interface to the Font Selection dialog.                      */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This routine sets up the parameters and calls the Font       */
/*               Selection dialog routines. Responses from the dialog will    */
/*               be via WM_CONTROL messages to the parent window procedure.   */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          ULONG  SelectFont(PRHWINDATA pWinData)                            */
/*                                                                            */
/*   INPUT:                                                                   */
/*       pWinData       - Pointer to the WinData structure of the parent      */
/*                        window.                                             */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       FontDialog - Load and initialize the Font dialog                     */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       WinProcessDlg                                                        */
/*       WinSendMsg                                                           */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

  ULONG  SelectFont(PRHWINDATA pWinData) {
    PFONTDLG   fntd;                   /* font dialog control                 */
    BOOL       rc;                     /* success/failure flag                */

    fntd = &pWinData->MleFontAttrs;    /* point to control structure          */
    fntd->cbSize = sizeof(FONTDLG);
    if (!fntd->pszFamilyname)         {/* do first time set up               */
                                       /* set the buffer                     */
      fntd->pszFamilyname = pWinData->MleFamilyName;
      strcpy(fntd->pszFamilyname, fntd->fAttrs.szFacename);
      fntd->fxPointSize = MAKEFIXED(8,0);
    }
    fntd->pszPreview = getstring(pWinData->hab, pWinData->hResource,
        SAMPLE_TEXT);
    fntd->pszTitle = getstring(pWinData->hab, pWinData->hResource,
        FONT_TITLE);
    fntd->fl = FNTS_CENTER + FNTS_HELPBUTTON;
    fntd->clrFore = CLR_NEUTRAL;    /* Selected foreground color */
    fntd->clrBack = CLR_BACKGROUND; /* Selected Background color */
    fntd->usWeight = FWEIGHT_NORMAL;
    fntd->usWidth = FWIDTH_NORMAL;
    fntd->hpsScreen = WinGetPS(pWinData->outmle);
    fntd->fAttrs.usCodePage = (USHORT)pWinData->CountryInfo.codepage;
    fntd->clrFore = CLR_BLACK;
    fntd->clrBack = CLR_WHITE;
    fntd->usFamilyBufLen = MAXWIDTH;
    fntd->pszPtSizeList = NULL;        /* use default point sizes*/
    rc = TRUE;

    /* Disable the window for update, and call the font dialog   */
    WinEnableWindowUpdate(pWinData->outmle, FALSE);
    if (!WinFontDlg(HWND_DESKTOP, pWinData->frame, fntd) ||
        fntd->lReturn != DID_OK) {
      rc = FALSE;
      } else {

       /**********************************************************/
       /* If outline font, calculate the maxbaselineext and      */
       /*  avecharwidth for the point size selected              */
       /**********************************************************/
       if ( fntd->fAttrs.fsFontUse == FATTR_FONTUSE_OUTLINE )
       {
         HDC   hDC;
         LONG  lxFontResolution;
         LONG  lyFontResolution;
         SIZEF sizef;

         /********************************************************/
         /* Query device context for the screen and then query   */
         /* the resolution of the device for the device context. */
         /********************************************************/
         hDC = GpiQueryDevice(fntd->hpsScreen);
         DevQueryCaps( hDC, CAPS_HORIZONTAL_FONT_RES,
                       (LONG)1, &lxFontResolution);
         DevQueryCaps( hDC, CAPS_VERTICAL_FONT_RES,
                       (LONG)1, &lyFontResolution);

         /********************************************************/
         /* Calculate the size of the character box, based on the*/
         /* point size selected and the resolution of the device.*/
         /* The size parameters are of type FIXED, NOT int.      */
         /*  NOTE: 1 point == 1/72 of an inch.                   */
         /********************************************************/
         sizef.cx = (FIXED)(((fntd->fxPointSize) / 72 ) *
                           lxFontResolution );
         sizef.cy = (FIXED)(((fntd->fxPointSize) / 72 ) *
                           lyFontResolution );

         fntd->fAttrs.lMaxBaselineExt =
                               MAKELONG( HIUSHORT( sizef.cy ), 0 );
         fntd->fAttrs.lAveCharWidth =
                               MAKELONG( HIUSHORT( sizef.cx ), 0 );
        }/* endif fsFontUse == FATTR_FONTUSE_OUTLINE */


      } /* endif !WinFontDlg */
    WinReleasePS(fntd->hpsScreen);

    if (rc) {
      WinSendMsg(pWinData->outmle,     /* set font of multiline edit field    */
          MLM_SETFONT, MPFROMP((PFATTRS)&fntd->fAttrs), (MPARAM)0);
    }
    WinEnableWindowUpdate(pWinData->outmle, TRUE);
    free(fntd->pszTitle);
    free(fntd->pszPreview);
    return (TRUE);
  }


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: SearchDlgProc                                           */
/*                                                                            */
/*   DESCRIPTIVE NAME: Search Dialog Procedure                                */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This routine is the dialog procedure for diaply of the       */
/*               Search dialog.                                               */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: Standard Dialog procedure interface.                         */
/*                                                                            */
/*   INPUT: Standard Dialog procedure interface.                              */
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
/*        WinDefDlgProc                                                       */
/*        WinDismissDlg                                                       */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

MRESULT EXPENTRY SearchDlgProc(HWND hdlg, ULONG msg, MPARAM mp1,
                               MPARAM mp2)
{
   HWND hwndEntryFld;
   static PSZ szSearchString;
   static PSEARCHINFO psearchinfo;

    switch (msg) {
      case  WM_CLOSE :
         WinSendMsg(hdlg, WM_COMMAND, MPFROMSHORT(PB_CANCEL),
                    MPFROM2SHORT (CMDSRC_PUSHBUTTON, TRUE));
         return 0;

      case  WM_INITDLG :
         psearchinfo = (PSEARCHINFO)mp2;
         return 0;

      case  WM_COMMAND :
      {
        switch(SHORT1FROMMP(mp1))
        {
          case PB_OK:
            hwndEntryFld = WinWindowFromID(hdlg, RX_SEARCH_FIELD);
            WinQueryWindowText(hwndEntryFld, 100,
               psearchinfo->szSearchString);
            if ((BOOL)WinSendDlgItemMsg(hdlg, RX_CASEBOX,
                                        BM_QUERYCHECK, NULL, NULL)) {
               psearchinfo->caseSensitive = TRUE;
            } else {
               psearchinfo->caseSensitive = FALSE;
            } /* endif */
            WinDismissDlg(hdlg, PB_OK);
            return 0;

          case PB_CANCEL:
           WinDismissDlg(hdlg, PB_CANCEL);
           break;
        }
      }

    }
    return WinDefDlgProc(hdlg, msg, mp1, mp2);
}

