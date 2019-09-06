/* static char *SCCSID = "@(#)pmrexx.c  6.4 92/01/10";                        */
/*********************  START OF SPECIFICATIONS  ******************************/
/*                                                                            */
/*   SOURCE FILE NAME:  pmrexx.c                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:  PM host environment for REXX command files.           */
/*                                                                            */
/*      Provide a means to execute REXX command files from within a           */
/*      PM environment, allowing the REXX/DM bindings to be used.             */
/*                                                                            */
/*   COPYRIGHT:         IBM Corporation 1991                                  */
/*                                                                            */
/*   STATUS:            Version 2.00                                          */
/*                                                                            */
/*   FUNCTION:                                                                */
/*           This program provides a Presentation Manager Shell for           */
/*      REXX programs.                                                        */
/*                                                                            */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES:                                                         */
/*          This function has dependencies on the following                   */
/*      files for compilation.                                                */
/*          pmrexx.h   - Definitions necessary for the resource               */
/*                       file compilation.                                    */
/*          pmrexxio.h - typedefs and prototypes necessary for use of         */
/*                       the REXX I/O shell dll.                              */
/*          The C runtime multi-threaded header files                         */
/*          See README.C in \TOOLKT12\C for a list and description of the     */
/*          system files that are needed.                                     */
/*                                                                            */
/*  EXECUTION INSTRUCTIONS:                                                   */
/*          The PMREXX program is executed by specifying the                  */
/*      PMREXX program with arguments corresponding to the name               */
/*      of the REXX procedure file, followed by any arguments to              */
/*      the REXX procedure. For example, from a command line:                 */
/*          PMREXX REXXTRY say 'Hi'                                           */
/*      would call PMREXX to execute the REXX procedure file                  */
/*      "REXXTRY.CMD" with arguments of "say 'Hi'" being passed to            */
/*      the procedure. For the same effect from the start programs            */
/*      list, add PMREXX.EXE as the program (specify a path if it             */
/*      is not in the system path) and "REXXTRY say 'Hi'" as the              */
/*      parameters to be passed.                                              */
/*                                                                            */
/*  EXPECTED OUTPUT:                                                          */
/*          The REXX procedure file will be executed with all                 */
/*      output directed into PM windows.                                      */
/*                                                                            */
/*  PROCEDURES:                                                               */
/*    main:           Main entry point                                        */
/*    CallRexx:       Interface to the Rexx interpreter                       */
/*    CheckRexxFile:  Validate program as a Rexx file                         */
/*    PMRexxSubProc:  Subclass winproc of Rexx I/O window                     */
/*    PMRXCmds:       Routine to process subclassed WM_COMMAND messages       */
/*    SelectFile:     Put up a file selection dialog                          */
/*    getstring:      Retrieve a module resource string                       */
/*    init_routine:   Rexx initialization exit                                */
/*    SetOption:      Enable/Disable a menu option                            */
/*    SetCheckMark:   Check/Uncheck a menu option                             */
/*    SysErrorBoxM:   Display message box with a system error message         */
/*    CuaLogoDlgProc: Display About help                                      */
/*    SysErrorBox:    Display message in a message box                        */
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
#define  INCL_DOSERRORS                /* OS/2 errors                         */
#define  INCL_DOSMODULEMGR             /* OS/2 module support                 */
#define  INCL_DOSPROCESS               /* Process and thread support          */
#define  INCL_DOSSEMAPHORES            /* Semaphores                          */
#define  INCL_DOSQUEUES                /* Queues                              */
#define  INCL_DOSMISC                  /* Miscellaneous doscalls              */
#define  INCL_DOSNLS                   /* NLS (code page) support             */
#define  INCL_GPILCIDS                 /* Physical and logical fonts with     */
                                       /* lcids                               */
#define  INCL_GPIPRIMITIVES            /* Drawing primitives and primitive    */
                                       /* attributes                          */
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
#define  INCL_WINACCELERATORS          /* Accelerator APIs                    */
#include <os2.h>
#include <rexxsaa.h>                   /* Include the REXX header file        */
#include "pmrexxio.h"
#include "pmrexx.h"

/******************************************************************************/
/*               G L O B A L    V A R I A B L E   S E C T I O N               */
/******************************************************************************/

HAB     hab;                           /* Anchor block handle                 */
PFNWP   SuperProc;                     /* Superclass window proc              */
HWND    hwndClient;                    /* Client Window handle                */
HWND    hwndFrame;                     /* Frame Window handle                 */
ULONG   rexx_tid;                      /* ThreadIDs of the REXX thread        */
PID     proc_id;                       /* process ID for PMREXX               */
CHAR    pgmtitle[TITLE_SIZE+FNAME_SIZE+10];/* Holds title for title bar       */
CHAR    procname[FNAME_SIZE];          /* Name of procedure to run            */
RXSTRING rxargv;                       /* Structure to pass parameters to REXX*/
BOOL    trace = FALSE;                 /* Flag to indicate if user gave a /T  */
BOOL    Qflag = FALSE;                 /* Flag to indicate if user gave a /Q  */
BOOL    Results = TRUE;                /* Flag to append or overwrite results */
INT     ProcRC;                        /* process return code                 */
CHAR    pszFileName[FNAME_SIZE];       /* buffer to hold filename             */

/* exit_list is a list of the system exits PMREXX register's with REXX        */

static RXSYSEXIT exit_list[] =  {
    { PMRXIO_EXIT,   RXSIO },
    { "PMREXXINIT",  RXINI },
    { NULL,          RXENDLST }};

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: main                                                    */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Execute the PMREXX program.                                  */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This is the main entry point to the program. It sets up      */
/*               the window, executes the message loop, and cleans up         */
/*               on termination (exit from the message loop).                 */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:  main(LONG  argc, CHAR *argv[])                              */
/*                                                                            */
/*   INPUT:                                                                   */
/*      argc and argv are the standard C argument structures passed           */
/*      in to a program.                                                      */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       Exit with return code 0.                                             */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*       If unable to register the window class, exit rc=1.                   */
/*       If no procedure name was specified, exit with rc=3.                  */
/*       If window was not displayed, exit with return code from the REXX     */
/*         procedure.                                                         */
/*       Otherwise: exit rc=0.                                                */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*       Executes the REXX procedure specified as the first argument with the */
/*       input and output coming from/going to a PM window.                   */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       CheckRexxFile  - See if the file starts with a comment line          */
/*       getstring      - To obtain the title bar name and window name.       */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       DosExit                     WinInitialize                            */
/*       DosGetInfoBlocks            WinMessageBox                            */
/*       DosGetCp                    WinPostMessage                           */
/*       WinAddSwitchEntry           WinQueryWindowProcess                    */
/*       WinAlarm                    WinRemoveSwitchEntry                     */
/*       WinCreateMsgQueue           WinSetCp                                 */
/*       WinSubclassWindow           WinSetWindowText                         */
/*       WinDestroyMsgQueue          WinTerminate                             */
/*       WinSendMsg                  WinUpdateWindow                          */
/*       WinDispatchMsg              WinWindowFromID                          */
/*       WinGetMessage                                                        */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

void main(LONG argc, CHAR *argv[])
{
  HMQ      hmq;                        /* Message Queue handle                */
  QMSG     qmsg;                       /* Message structure                   */
  LONG     argnum = 1;                 /* Counter for looking at arguments    */
  PUCHAR   trueargs;                   /* Pointer for actual argument string  */
  HSWITCH  hswPMRexx = NULLHANDLE;     /* Handle to task list entry           */
  SWCNTRL  swlEntry;                   /* Structure for task list entry       */
  ULONG    idThread;                   /* Holds id of this thread             */
  ULONG    CpSize;                     /* Size of returned code page info     */
  PTIB     tib;                        /* thread block pointer                */
  PPIB     pib;                        /* process block pointer               */
  ULONG    CodePage;                   /* Code page we are running in         */
   /* We set the creation flags to be the standard window frame flags         */
   /*    but without scroll bars and without a task list entry.               */
  ULONG    flCreate = FCF_STANDARD&~(FCF_VERTSCROLL|FCF_HORZSCROLL);

   /* We want the original arguments to pass to REXX, not the mangled         */
   /*    version we get from C.                                               */

  DosGetInfoBlocks(&tib, &pib);
  proc_id = pib->pib_ulpid;            /* save process id                     */
  trueargs = pib->pib_pchenv;          /* point to true argument string       */

  pszFileName[0] = '\0';               /* set initial file name               */

   /* Set up the PM environment                                               */
  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue(hab, 50);

                                       /* Create application msg queue        */
                                       /* Get NLS Info                        */
  DosQueryCp(sizeof(CodePage), &CodePage, &CpSize);
  //DosGetCp(sizeof(CodePage), &CodePage, &CpSize);
  WinSetCp(hmq, (USHORT)CodePage);     /* And set for PM                      */
                                       /* Create Rexx I/O window              */
  hwndFrame = RexxCreateIOWindow(HWND_DESKTOP, &hwndClient);
  SuperProc = WinSubclassWindow(hwndClient, (PFNWP)PMRexxSubProc);
                                       /* now add additional menu items       */

                                       /* now load the accelerator table      */
  WinSetAccelTable(hab, WinLoadAccelTable(hab,
      (HMODULE)NULL, RXHOST_WIN), hwndFrame);

      /* See if the trace bit is supposed to be turned on                     */
  if (argc > argnum && !stricmp("/t", argv[argnum])) {

    trace = TRUE;                      /* Set the trace flag                  */
    argnum++;                          /* If it was, that's an extra arg to   */
                                       /* skip                                */
  }

  /* See if the qflag option was specified to suppress pmrexx msg box*/
  if (argc > argnum && !strcmpi("/q", argv[argnum])) {

     /* Turn on  qflag */
     Qflag = TRUE;

     argnum++;               /* If it was, that's an extra arg to skip */

  } /* endif */

  if (argc <= argnum) {                /* There must at least have been a     */
                                       /* procedure name specified If not,    */
                                       /* alert the user and exit.            */
    WinAlarm(HWND_DESKTOP, WA_ERROR);
    WinMessageBox(HWND_DESKTOP,
                  NULLHANDLE,
                  getstring(BAD_NUM_PARMS),
                  getstring(RXHA_MSGBOXHDR),
                  1,
                  MB_CANCEL|MB_CUACRITICAL|MB_MOVEABLE);
    WinSetFocus(HWND_DESKTOP, hwndFrame);
    WinPostMsg(hwndFrame, WM_QUIT, (MPARAM)0, (MPARAM)0);
    ProcRC = BAD_NUM_PARMS;
  }

  else {                               /* procedure name was specified        */
    PUCHAR format = getstring(RXH_TITLE);
    PUCHAR pgmname = getstring(PROGRAM_NAME);/* Create the window/program     */
                                       /* title                               */

    strcpy(procname, argv[argnum]);
    sprintf(pgmtitle, (const char *)format, pgmname, procname);

         /* Check to see if the file starts with a comment or not             */

    if (!CheckRexxFile(procname)) {    /* It doesn't start with a comment     */
                                       /* line, so put up a message box saying*/
                                       /* that the procedure file could not be*/
                                       /* found.                              */
      *procname = '\0';
      WinMessageBox(HWND_DESKTOP,
                    NULLHANDLE, getstring(RXHM_PROC_NOT_FOUND),
                    getstring(RXHA_MSGBOXHDR), 1,
                    MB_CANCEL|MB_CUACRITICAL|MB_MOVEABLE);
      WinSetFocus(HWND_DESKTOP, hwndFrame);
      WinPostMsg(hwndFrame, WM_QUIT, (MPARAM)0, (MPARAM)0);
      ProcRC = PROC_NOT_FOUND;
    }

         /* Update the title bar to the newly created name                    */
    WinSetWindowText(WinWindowFromID(hwndFrame,(USHORT)FID_TITLEBAR),
                     pgmtitle);
    free(format);
    free(pgmname);
    argnum++;

    if (argnum < argc) {               /* In this section, we obtain the      */
                                       /* arguments to pass to the REXX       */
                                       /* procedure from the original         */
                                       /* arguments as stored in the          */
                                       /* environment segment. We cannot      */
                                       /* accept the *argv[] version, because */
                                       /* the C runtime library will have     */
                                       /* changed things like spacing,        */
                                       /* quotation marks, etc.               */

      while (*trueargs++)
        ;                              /* Skip the first argument (the        */
                                       /* command/program name)               */

      while (--argnum) {               /* Skip the procedure name and the "/t"*/
                                       /* option, if there                    */

        while (' ' == *trueargs) {     /* Skip leading white space            */
          trueargs++;
        }

        do {                           /* Then skip the argument              */

          if ('"' == *trueargs) {      /* If quote delimited                  */
            trueargs++;                /* skip to matching quote              */

            while (*trueargs && '"' != *trueargs++)
              ;
          }

          else
            trueargs++;
        }


        while (' ' != *trueargs);

      }
      trueargs++;                      /* Skip next space Set up the remaining*/
                                       /* argument string for passing to the  */
                                       /* REXX procedure.                     */
      rxargv.strptr = (PCHAR)strdup((const char *)trueargs);
      rxargv.strlength = strlen(rxargv.strptr);

    }

    else {                             /* no additional arguments             */
      rxargv.strlength = 0;
      rxargv.strptr = NULL;
    }

                                       /* register I/O handler                */
    RexxRegisterExitExe("PMREXXINIT", (PFN)init_routine, NULL);
    DosCreateThread(&rexx_tid, (PFNTHREAD)CallRexx, (ULONG)0, (ULONG)0,
        STACK_SIZE);
  }

  WinUpdateWindow(hwndFrame);

   /* Get the switch list handle                                              */

  WinQueryWindowProcess(hwndFrame, &swlEntry.idProcess, &idThread);

  hswPMRexx = WinQuerySwitchHandle(hwndFrame, swlEntry.idProcess);

  if (!WinQuerySwitchEntry(hswPMRexx, &swlEntry)) {

    strcpy(swlEntry.szSwtitle, pgmtitle);
    WinChangeSwitchEntry(hswPMRexx, &swlEntry);
  }

   /* Main message processing loop - get and dispatch messages until          */
   /* WM_QUIT received                                                        */

  while (WinGetMsg(hab, &qmsg, (HWND)NULL, 0, 0))
    WinDispatchMsg(hab, &qmsg);

                                       /* Delete subclassing                  */
  WinSubclassWindow(hwndClient, (PFNWP)SuperProc);
                                       /* Destroy Rexx I/O window             */
  RexxDestroyIOWindow(hwndFrame);
                                       /* drop exit handlers                  */
  RexxDeregisterExit("PMREXXINIT", NULL);
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);

   /* Exit the process and return the return code specified by ProcRC         */

  DosExit(EXIT_PROCESS, ProcRC);
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: CallRexx                                                */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Execute the REXX procedure file via the RexxStart interface. */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This function is the thread that executes the REXX           */
/*               procedure. It disables the RESTART option and calls the      */
/*               Rexx interpreter.  On return, it reenables the RESTART       */
/*               options, outputs a message box indicating the procedure has  */
/*               ended, and terminates the thread.                            */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void _stdcall CallRexx(void                                       */
/*                                                                            */
/*   INPUT:                                                                   */
/*       The arguments for the procedure have already been placed in the      */
/*       global variable 'rxargv'.                                            */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       SetCheckMark   - Check/uncheck a menu item                           */
/*       SetOptions     - Enable/disable menu items.                          */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

void CallRexx(void)
{
  RXSTRING retval;                     /* return value from call to REXXSAA   */
  SHORT    rc;                         /* return code                         */

  SetOptions(PMRXA_RESTART, FALSE);    /* Don't allow restart while active    */
  SetOptions(RXIOA_HALT, TRUE);        /* Enable HALT option                  */
                                       /* enable I/O system                   */
  while (!WinPostMsg(hwndClient, REXX_STARTPROC,
      MPFROMLONG(rexx_tid), (MPARAM)0));
    DosSleep(0L);

   MAKERXSTRING(retval, NULL, 0);      /* null out RXSTRING                   */
   /* Now start the REXX procedure. Check for a not-found error.           */
   if (PROC_NOT_FOUND == RexxStart((!rxargv.strlength) ? 0 : 1,
                                   (!rxargv.strlength)? NULL : &rxargv,
                                  procname,
                                  NULL,
                                  NULL,
                                  RXCOMMAND,
                                  exit_list,
                                  &rc,
                                  &retval )) {

      /* If the procedure was not found, let the window procedure put up   */
      /*   a message box indicating the problem. Set the return code to    */
      /*   exit with.                                                      */
      while (!WinPostMsg(hwndClient, REXX_PROC_NF, NULL, NULL))
         DosSleep(0L);
      ProcRC = PROC_NOT_FOUND;

   }
   else {
     DosSleep((ULONG)TIMER_DELAY);
     ProcRC = rc;
  }
                                       /* disable I/O system                  */
  while (!WinPostMsg(hwndClient, REXX_ENDPROC, (MPARAM)0, (MPARAM)0))
    DosSleep(0L);

  SetOptions(PMRXA_RESTART, TRUE);     /* Re-enable the re-start option       */
  SetCheckMark(RXIOA_HALT, FALSE);
  SetOptions(RXIOA_HALT, FALSE);       /* Don't allow halt if not active      */
                                       /* display message to user             */
  while (!WinPostMsg(hwndClient, REXX_PROC_ENDED, (MPARAM)0, (MPARAM)0))
    DosSleep(0L);
  DosExit(EXIT_THREAD, 0);             /* end the thread                      */
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME:  CheckRexxFile                                          */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*              Verify that this is a REXX procedure file.                    */
/*                                                                            */
/*   FUNCTION:  This function will check that the file starts with a comment  */
/*              line to verify that it is a REXX procedure file.              */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*            ULONG  CheckRexxFile(PUCHAR procname)                           */
/*                                                                            */
/*   INPUT:                                                                   */
/*       procname - ptr to an asciiz string containing the filename to run    */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       returns TRUE if the file starts with a comment line                  */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*       returns FALSE otherwise                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        DosSearchPath                                                       */
/**************************** END OF SPECIFICATIONS ***************************/

ULONG  CheckRexxFile(PUCHAR procname)
{
  CHAR fullname[FNAME_SIZE];           /* buffer for fully qualified path     */
  CHAR tempname[FNAME_SIZE];           /* holds name to search for            */
  FILE *fptr = NULL;                   /* ptr to file                         */
  CHAR *pathchar,*colon;               /* ptr to '\' and ':'                  */
  CHAR buff[10];                       /* holds input line from file          */
  ULONG  i;                            /* counter variable                    */

  strcpy(tempname, (const char *)procname);          /* copy name to temp holder            */

  for (i = 0; i < 2 && !fptr; i++) {   /* loop through twice                  */

    if (!(fptr = fopen(tempname, "r"))) {/* couldn't open the file            */
      pathchar = getstring(BACKPATH_DELIMETER);/* load in '\' and             */
      colon = getstring(DRIVE_DELIMETER);/* ':'                               */

         /* Check to see it they entered a path                               */

      if (!strchr((const char *)procname, *pathchar) && !strchr((const char *)procname, *colon)) {
                                       /* They only entered a filename, so    */
                                       /* search the DPATH for it             */

        if (!DosSearchPath(SEARCH_CUR_DIRECTORY|SEARCH_ENVIRONMENT|
            SEARCH_IGNORENETERRS, "PATH", tempname, fullname, sizeof(fullname)
            )) {
          fptr = fopen(fullname, "r"); /* Try and open the file               */
        }
      }
    }

      /* If we still haven't found it stick on a .CMD, and try again          */

    if (!fptr) {
      strcat(tempname, ".cmd");
    }
  }

  if (fptr) {                          /* We found it                         */
    fgets(buff, 10, fptr);             /* read in the first line              */

      /* Check if the first two characters are a comment                      */

    if (*buff == '/' && *(buff+1) == '*') {

         /* It's OK, so copy whatever name we found to procname               */

      strcpy(procname, tempname);
      fclose(fptr);                    /* Close the file                      */
      return  TRUE;                    /* Indicate success                    */
    }

    fclose(fptr);                      /* Close the file                      */
  }
  return  FALSE;                       /* Indicate failure                    */
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: PMRexxSubProc                                           */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Subclass handler for PMREXX I/O window.                      */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This procedure handles all messages over and above the       */
/*               ones handled by the main PMREXX window.                      */
/*               are:                                                         */
/*       REXX_PROC_NF   - Puts up a message box indicating that the REXX      */
/*                        procedure was not found.                            */
/*       REXX_PROC_ENDED- Puts up a message box indicating to the user that   */
/*                        the REXX procedure has terminated.                  */
/*       RXHB_MSGBOX    - Puts up a message box with the requested message.   */
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
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       getstring      - Retrieve a string resource                          */
/*       PMRXCmds       - This routine processes the various WM_COMMAND       */
/*                        messages.                                           */
/*       SetCheckMark   - Check/uncheck a menu item                           */
/*       SysErrorBoxM   - SysErrorBox with added user control                 */
/*       ProcessSaveQuit - Interface to the Save/Quit dialog                  */
/*                                                                            */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       DosReleaseMutexSem                WinMessageBox                      */
/*       DosRequestMutexSem                WinPostMsg                         */
/*       WinSendMsg                        WinQueryWindow                     */
/*       WinSetFocus                       WinQueryWindowPos                  */
/*       WinWindowFromID                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

MRESULT EXPENTRY PMRexxSubProc(HWND hwnd, ULONG msg, MPARAM mp1,
                              MPARAM mp2)
{

    switch (msg) {

      case  WM_COMMAND :
                                       /* All WM_COMMAND processed in PMRXCmds*/
        return (PMRXCmds(hwnd, mp1, mp2));
        break;

      case  WM_CHAR :

         /* Check for a valid (complete) key message.                         */

        if (!(SHORT1FROMMP(mp1)&KC_KEYUP) &&
            (SHORT1FROMMP(mp1)&KC_VIRTUALKEY) ) {

          switch (SHORT2FROMMP(mp2)) {
            case  VK_BREAK :           /* For a break key, set the halt       */
                                       /* procedure indicator.                */
              WinSendMsg(hwnd, WM_COMMAND, MPFROMSHORT(RXIOA_HALT), (MPARAM)0);
              break;

            case  VK_ENTER :           /* For a break key, set the halt       */
              WinSendMsg(hwnd, WM_CHAR, MPFROM2SHORT(KC_VIRTUALKEY, NULL),
                         MPFROM2SHORT(NULL, VK_NEWLINE));
        break;
            default  :
              break;
          }                            /* endswitch                           */
        }
        break;

      case  DOS_ERROR :

         /* Place the information on the base system error into a message     */
         /*   box.                                                            */

        SysErrorBoxM(hwnd, SHORT1FROMMP(mp1), mp2, SHORT2FROMMP(mp1));
        break;

      case  REXX_PROC_NF :

         /* Tell the user that REXX could not find the procedure, and then    */
         /*    exit.                                                          */

        WinAlarm(HWND_DESKTOP, WA_ERROR);
        WinSendMsg(hwndClient, RXHB_MSGBOX,
            MPFROM2SHORT(RXHM_PROC_NOT_FOUND, RXHA_MSGBOXHDR),
            MPFROMLONG(MB_CANCEL|MB_CUACRITICAL|MB_MOVEABLE));
        WinPostMsg(hwndClient, WM_CLOSE, (MPARAM)0, (MPARAM)0);
        break;

      case  REXX_PROC_ENDED :

         /* Tell the user that the procedure has completed processing.        */
        if (!Qflag) {
          WinSendMsg(hwndClient, RXHB_MSGBOX,
              MPFROM2SHORT(RXHM_PROC_ENDED, PROGRAM_NAME),
              MPFROMLONG(MB_OK|MB_INFORMATION|MB_MOVEABLE));
          WinSetFocus(HWND_DESKTOP, hwndFrame);
        }
        else {
          /* quit immediately if QUIET flag was selected                      */
          WinPostMsg( hwndFrame, WM_QUIT, 0L, 0L );
        }
        break;

      case  RXHB_MSGBOX : {            /* Put up a message box. The mp1       */
                                       /* parameter has the resource ids for  */
                                       /* the message and the title.          */
          USHORT msgid = SHORT1FROMMP(mp1);
          PUCHAR header = getstring(SHORT2FROMMP(mp1));
          PUCHAR content = getstring(SHORT1FROMMP(mp1));/* If we are          */
                                       /* displaying the ended message, set   */
                                       /* the owner to the desktop so we don't*/
                                       /* steal the focus back from other     */
                                       /* apps.                               */

          LONG  rc = WinMessageBox(HWND_DESKTOP,
              (RXHM_PROC_ENDED == msgid?HWND_DESKTOP: hwnd),
              content, header, msgid, SHORT1FROMMP(mp2));
          free(header);
          free(content);
          return  MPFROMSHORT(rc);
        }
        break;

      case  WM_CLOSE :

         /* Check to see if the window needs to be saved, but only if the     */
         /* user has entered a filename, otherwise just exit.                 */

        if ((!strlen(pszFileName)) || ProcessSaveQuit(hwnd)) {
          WinSendMsg(hwndClient, WM_SAVEAPPLICATION, NULL, NULL);
          WinPostMsg( hwndFrame, WM_QUIT, 0L, 0L );
          return (MRESULT)(FALSE);
        }

        else {                         /* Don't Close if Cancel on            */
                                       /* ProcessSaveQuit                     */
          return (MRESULT)(TRUE);
        }
        break;

      default  :
        break;
    }
    return (*SuperProc)(hwnd, msg, mp1, mp2);
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: PMRXCmds                                                */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Process WM_COMMAND messages for the window procedure.        */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This routine processes all the WM_COMMAND messages that have */
/*               are handled by the Subclassed window.  Messages handled are: */
/*                                                                            */
/*       PMRXA_RESTART  - This requests that the REXX procedure be re-run.    */
/*                        This command is only valid if the previous instance */
/*                        of the procedure has ended.                         */
/*       PMRXA_EXIT     - This is a request to close the application.         */
/*       PMRXA_SAVEAS   - Save the MLE to the same file as last time.         */
/*       PMRXA_SAVE     - Present the user with a dialog to select a file for */
/*                        saving the MLE.                                     */
/*       PMRXA_ABOUT    - Present the About dialog.                           */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*       MRESULT PMRXCmds(HWND hwnd, MPARAM mp1, MPARAM MP2)                  */
/*                                                                            */
/*   INPUT:                                                                   */
/*       hwnd           - The window handle of the window that received the   */
/*                        command.                                            */
/*       mp1            - The first parameter of the WM_COMMAND message.      */
/*       mp2            - The second parameter of the WM_COMMAND message.     */
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
/*       getstring      - Retrieve a string resource                          */
/*       SelectFile     - Interface to the file selection dialog              */
/*       SetCheckMark   - Check/uncheck a menu item                           */
/*       WriteFile      - Write the data from the MLE into a file             */
/*                                                                            */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       WinAlarm                       WinPostMessage                        */
/*       WinSendMsg                     WinLoadString                         */
/*       WinDlgBox                      WinWindowFromID                       */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

MRESULT PMRXCmds(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    CHAR pszFMask[FNAME_SIZE];         /* buffer to hold mask for wildcard    */
                                       /* searches                            */
    CHAR pszTitle[TITLE_SIZE];         /* text for Save As title bar          */
    CHAR pszFName[FNAME_SIZE];         /* buffer for formatted filename       */
    CHAR Temp = '\0';                  /* temporary character                 */
    ULONG  UserResponse;               /* holds user response from Save As    */
    ULONG  usFileSize;



    switch (LOUSHORT(mp1)) {

      /************************************************************************/
      /* Flip the trace flag                                                  */
      /************************************************************************/

      case  PMRXA_FLAGCHANGE:
        if (mp2 == 0) {
           trace = FALSE;
        } else {
           trace = TRUE;
        } /* endif */
        break;

      /************************************************************************/
      /* Flip the result flag                                                 */
      /************************************************************************/

      case  PMRXA_RESULTCHANGE:

        Results = !Results;
        break;

      /************************************************************************/
      /* Rerun the program                                                    */
      /************************************************************************/

      case  PMRXA_RESTART :
        if (!Results) {

          WinEnableWindowUpdate(hwnd, FALSE);
                                       /* disable updates Get length of the   */
                                       /* output in the MLE                   */
          usFileSize = (ULONG )WinSendMsg(WinWindowFromID(hwnd,
                                          (USHORT)MLE_WNDW),
                                          MLM_QUERYTEXTLENGTH,
                                          (MPARAM)0, (MPARAM)0);
                                       /* Select all of the output in the MLE */
          WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                     MLM_DELETE, (MPARAM)0L, MPFROMSHORT(usFileSize));
          WinInvalidateRect(hwnd, NULL, TRUE);
                                       /* update the window                   */
          WinEnableWindowUpdate(hwnd, TRUE);
                                       /* Make MLE so that it can't be undone */
          WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                     MLM_RESETUNDO, (MPARAM)0, (MPARAM)0);
        } /* endif */

        DosCreateThread(&rexx_tid, (PFNTHREAD)CallRexx, (ULONG)0, (ULONG)0,
            STACK_SIZE);

        break;

      /************************************************************************/
      /* Shutdown and get out                                                 */
      /************************************************************************/

      case PMRXA_EXIT:

         WinPostMsg( hwnd, WM_CLOSE, 0L, 0L );
         break;

      /************************************************************************/
      /* Save the file according to user file specification                   */
      /************************************************************************/

      case  PMRXA_SAVE :

        WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
            MLM_RESETUNDO, (MPARAM)0, (MPARAM)0);

        if (!strlen(pszFileName)) {    /* User hasn't added a filename  */
                                       /* yet, so put up the Save As dialog   */
          WinSendMsg(hwnd, WM_COMMAND, MPFROMSHORT(PMRXA_SAVEAS), (MPARAM)0);
        }

        else {                         /* We have the filename, so just write */
                                       /* it out.                             */

          if (WriteFile(hab, hwnd, pszFileName, procname, FALSE)) {
            WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                       MLM_SETCHANGED, MPFROMSHORT(FALSE), (MPARAM)0);
            return (MRESULT)(TRUE);
          }

          else {
            return (MRESULT)(FALSE);
          }
        }
        break;

      /************************************************************************/
      /* Bring up the Save As dialog to get the filename                      */
      /************************************************************************/

      case  PMRXA_SAVEAS :
        WinLoadString(hab, (HMODULE)NULL, PMRXHB_SAVETITLE,
            sizeof(pszTitle), pszTitle);
        WinLoadString(hab, (HMODULE)NULL, PMRXHB_FILEMASK,
            sizeof(pszFMask), pszFMask);
        *pszFName = '\0';

        if (ESCAPE != (UserResponse = SelectFile( pszTitle,
            pszFMask, pszFName))) {

          if (WriteFile(hab, hwnd, pszFName, procname, TRUE)) {
            WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                       MLM_SETCHANGED, MPFROMSHORT(FALSE), NULL);
            strcpy(pszFileName, pszFName);
          }
        }
        WinSendMsg(WinWindowFromID(hwnd, (USHORT)MLE_WNDW),
                   MLM_RESETUNDO, (MPARAM)0, (MPARAM)0);
        return (MRESULT)(UserResponse);
        break;

      /************************************************************************/
      /* Display the About dialog.                                            */
      /************************************************************************/

      case  PMRXA_ABOUT :

        WinDlgBox(HWND_DESKTOP, hwndFrame, (PFNWP)CuaLogoDlgProc,
                       (HMODULE)0, PMREXX_CUALOGO, (PVOID)NULL);
        break;


      /************************************************************************/
      /* Option not handled here, pass on to default win proc                 */
      /************************************************************************/

      default  :
        return (*SuperProc)(hwnd, WM_COMMAND, mp1, mp2);
    }

    return 0;
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: SelectFile                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME: Bring up Select File dialog procedure.                 */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This routine sets up the file structure before calling the   */
/*               file selection dialog.                                       */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: ULONG  SelectFile                                            */
/*                                                                            */
/*   INPUT:                                                                   */
/*      PSZ pszTitle - current title text                                     */
/*      PSZ pszFileMask - file mask to search for                             */
/*      PSZ pszFName - filename (returned)                                    */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*      Returns ENTER or ESCAPE                                               */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*      None.                                                                 */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        getstring                                                           */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        WinFileDlg                                                          */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

  ULONG  SelectFile(PSZ pszTitle, PSZ pszFileMask, PSZ pszFName) {
    BOOL     fSuccess = FALSE;
    HWND     hwndDlg;                  /* file dialog handle                  */
    FILEDLG  fdl;                      /* open/saveas dialog data             */
    PSZ      pszDefaultDrive = NULL ;
    static   PSZ   ppszDefaultDriveList[] = { NULL } ;
    PSZ      pszDefaultEAType = NULL ;
    static   PSZ   ppszDefaultEATypeList[] = { NULL } ;


  fdl.pfnDlgProc     = NULL;
  fdl.cbSize         = sizeof(FILEDLG);
  fdl.fl             = FDS_HELPBUTTON | FDS_CENTER | FDS_SAVEAS_DIALOG | FDS_ENABLEFILELB ;
  fdl.ulUser         = 0L ;
  fdl.lReturn        = 0L;
  fdl.lSRC           = FDS_SUCCESSFUL ;
  fdl.hMod           = (HMODULE)NULL ;
  fdl.usDlgId        = 0 ;
  fdl.x              = 0 ;
  fdl.y              = 0 ;
  fdl.pszIDrive      = pszDefaultDrive ;
  fdl.papszIDriveList= (PAPSZ) ppszDefaultDriveList ;
  fdl.pszIType       = pszDefaultEAType ;
  fdl.papszITypeList = (PAPSZ) ppszDefaultEATypeList ;
  fdl.pszOKButton    = NULL;

  strcpy(fdl.szFullFile, pszFileMask);
  fdl.pszTitle = pszTitle;

  /* Call the Standard Open/SaveAs Dialog Procedure. */

  hwndDlg = WinFileDlg(HWND_DESKTOP, hwndClient, &fdl);

  /* Check which button was selected. */

  switch ((SHORT)fdl.lReturn) {

    case DID_OK_PB:
      strcpy(pszFName, fdl.szFullFile);
      fSuccess = TRUE;
      break;

    case DID_CANCEL_PB:
      fSuccess = FALSE;
      break ;
  }

  return fSuccess;
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: getstring                                               */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Allocate memory for, and obtain a string resource.           */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This function gets a string resource from the                */
/*               program file. It accepts as input, the resource ID           */
/*               of the string to get, and returns a far pointer to           */
/*               the string in allocated storage. The caller should           */
/*               free the storage (via free()) when done with the             */
/*               string.                                                      */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:  getstring(ULONG  string_name)                               */
/*                                                                            */
/*   INPUT:                                                                   */
/*      string_name - The resource ID that identifies the string in the       */
/*                    resource file.                                          */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*      Returns a pointer to the string.                                      */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*      Returns a null pointer.  Use WinGetLastError to find the cause.       */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*      Assumptions made include:                                             */
/*          The maximum width string to be obtained is MAXWIDTH-1 characters. */
/*          MAXWIDTH is defined in file PMREXX.H.                             */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       WinLoadString                                                        */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

PUCHAR getstring(ULONG  string_name)
{
  UCHAR buffer[MAXWIDTH];
  ULONG chars;

  buffer[0] = '\0';
  /* Get the message into our local buffer, then make a copy to return.      */
  chars = WinLoadString(hab, (HMODULE)NULL,
      (ULONG)string_name, MAXWIDTH, buffer);
  return  strdup((const char *)buffer);
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: init_routine                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Process the REXX Initialization exit                         */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This function has been registered to handle the RXINI exit.  */
/*               for the REXX procedure. This issues a call to RexxSetTrace   */
/*               to turn on tracing if it has been requested from the onset.  */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          LONG  init_routine( LONG  exitno, LONG  subfunc, PUCHAR parmblock)*/
/*                                                                            */
/*   INPUT:                                                                   */
/*       The arguments are defined by the REXX interface specifications.      */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        RexxSetTrace                                                        */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

LONG  init_routine(LONG  exitno, LONG  subfunc, PUCHAR parmblock)
{
  if (trace)                           /* tracing at start?                   */
    RexxSetTrace(proc_id, rexx_tid);   /* turn it on now                      */
  return RXEXIT_HANDLED;               /* return with no errors               */
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: SetOptions                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Enable/disable menu items.                                   */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This function is called to individually enable or disable a  */
/*               particular item in the menu structure.                       */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void SetOptions( ULONG  item, BOOL option )                       */
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
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       DosSleep                                                             */
/*       WinPostMsg                                                           */
/*       WinWindowFromID                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

void SetOptions(ULONG item,BOOL option)
{

   /* This call may be issued from a non-PM thread. So we have to use Post    */
   /*   instead of Send. If the Post fails, give the window procedure a       */
   /*   chance to empty the message queue and try again.                      */

  while (!WinPostMsg(WinWindowFromID(hwndFrame, (USHORT)FID_MENU),
      MM_SETITEMATTR, MPFROM2SHORT(item, TRUE),
      MPFROM2SHORT(MIA_DISABLED, option? ~MIA_DISABLED:MIA_DISABLED))) {
    DosSleep(0L);                      /* This give the window thread a chance*/
                                       /* to run                              */
  }
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: SetCheckMark                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Check/Uncheck menu items.                                    */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This function is called to place a check mark next to, or    */
/*               remove a check mark from a particular item in the menu       */
/*               structure.                                                   */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void SetCheckMark( ULONG item, BOOL option )                      */
/*                                                                            */
/*   INPUT:                                                                   */
/*       item           - The id of the item to check/uncheck.                */
/*       option         - TRUE to check the item, FALSE to uncheck.           */
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
/*       DosSleep                                                             */
/*       WinPostMsg                                                           */
/*       WinWindowFromID                                                      */
/**************************** END OF SPECIFICATIONS ***************************/

void SetCheckMark(ULONG item,BOOL option)
{

   /* This call may be issued from a non-PM thread. So we have to use Post    */
   /*   instead of Send. If the Post fails, give the window procedure a       */
   /*   chance to empty the message queue and try again.                      */

  while (!WinPostMsg(WinWindowFromID(hwndFrame, (USHORT)FID_MENU),
      MM_SETITEMATTR, MPFROM2SHORT(item, TRUE), MPFROM2SHORT(MIA_CHECKED,
      option?MIA_CHECKED:~MIA_CHECKED))) {
    DosSleep(0L);                      /* This give the window thread a chance*/
                                       /* to run                              */
  }
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME:  SysErrorBoxM                                           */
/*                                                                            */
/*   DESCRIPTIVE NAME: System Error Box with Message                          */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Pop up a window with the error information for a base        */
/*               system error, along with an added user control.              */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          ULONG  SysErrorBoxM(HWND hwndOwner, ULONG  errorcd,               */
/*                              PSZ message, ULONG  fsStyle )                 */
/*                                                                            */
/*   INPUT:                                                                   */
/*       hwndOwner  -  Owners window handle                                   */
/*       errorcd    -  Error code                                             */
/*       message    -  User provided message                                  */
/*       fsStyle    -  Style flags                                            */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       returns ID of button pushed.                                         */
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
/*       WinMessageBox                                                        */
/*       DosGetMessage                                                        */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

ULONG  SysErrorBoxM(HWND hwndOwner,ULONG  errorcd,PSZ message,ULONG  fsStyle)
{
  CHAR *szErrMsg;                      /* Buffer to hold message returned     */
  CHAR szErrTitle[80];                 /* Buffer to hold title for message box*/
  ULONG  cbMsg = ONE_K-1;              /* Length of message buffer            */
  ULONG  rc;                           /* return code variable                */
  CHAR *msgfile;                       /* Holds name of message file          */
  CHAR *errformat;                     /* Holds format string for title       */


  if (!(szErrMsg = malloc(ONE_K)))
    return 0;                          /* If we couldn't get space, just      */
                                       /* return                              */
  errformat = getstring(SYS_ERROR_TEXT);/* Get the format string for msg      */
  msgfile = getstring(MSG_FILE);       /* Get name of system message file     */
  sprintf(szErrTitle, errformat, errorcd);

                                       /* retrieve message with DosGetMessage */
  rc = DosGetMessage(NULL,             /* pointer to table of character       */
                                       /* pointers                            */
                     0,                /* number of pointers in table         */
                     szErrMsg,         /* pointer to buffer for return message*/
                     cbMsg+1,          /* length of buffer                    */
                     errorcd,          /* message number to retrieve          */
                     msgfile,          /* name of file containing message     */
                     &cbMsg);          /* length of returned message          */

  free(errformat);                     /* release string memory               */
  free(msgfile);
  szErrMsg[cbMsg] = '\0';              /* terminate message with a NULL       */

   /************************************************************************
   * Since the message returned may have imbedded carraige returns and/or  *
   * linefeeds we must remove them.                                        *
   ************************************************************************/
  while (--cbMsg) {

    if (szErrMsg[cbMsg] == '\n'|szErrMsg[cbMsg] == '\r') {
      szErrMsg[cbMsg] = ' ';
    }
  }
                                       /* add "user" message to system one    */
  if (message) {
    strcat(szErrMsg, "\n");
    strcat(szErrMsg, message);
  }
                                       /* Pop up a Message box                */
  rc = WinMessageBox(HWND_DESKTOP, hwndOwner, szErrMsg, szErrTitle, 0, fsStyle);

   /* Free the message buffer, and return.                                    */

  free(szErrMsg);
  return  rc;
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: CuaLogoDlgProc                                          */
/*                                                                            */
/*   DESCRIPTIVE NAME: Logo Dialog Procedure                                  */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This routine is the dialog procedure for diaply of the       */
/*               About dialog.                                                */
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

MRESULT EXPENTRY CuaLogoDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    LONG timeout;
    HINI hini;
    LONG ldefault = 2000;               /* Default logo to 2 seconds  @D08*/
    UCHAR buffer[MAXWIDTH];

    switch(msg)
    {
       case WM_INITDLG:
          if (mp2) {                    /* @C07A */
             hini = HINI_PROFILE;
             timeout = PrfQueryProfileInt(hini,(PSZ)"PM_ControlPanel", (PSZ)"LogoDisplayTime", ldefault);
             if (timeout == 0)
             {
               WinDismissDlg (hwnd,TRUE);
               break;
             }
             else WinStartTimer(hab, hwnd, 0, (ULONG) timeout);
                                  /* start a time of required duration */
          } /* endif */
          return ( (MRESULT) FALSE);

        case WM_COMMAND:
            break;

        case WM_TIMER:               /* @C07A */
            WinStopTimer(hab, hwnd, 0);
            WinDismissDlg(hwnd, TRUE);
            break;
    }
    return(WinDefDlgProc(hwnd,msg,mp1,mp2));
}
/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME:  SysErrorBox                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME: System Error Box                                       */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Pop up a window with the error information for a base        */
/*               system error.                                                */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          ULONG  SysErrorBox(HWND hwnd, ULONG  errorcd, PSZ message)        */
/*                                                                            */
/*   INPUT:                                                                   */
/*       hwndOwner  -  Owners window handle                                   */
/*       errorcd    -  Error code                                             */
/*       message    -  User provided message                                  */
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
/*       DosSleep                                                             */
/*       WinPostMsg                                                           */
/**************************** END OF SPECIFICATIONS ***************************/

void SysErrorBox(HWND hwnd,ULONG  errorcd,PUCHAR message)

{

   /***************************************************************************/
   /* This call may be issued from a non-PM thread. So we have to use Post    */
   /*   instead of Send. If the Post fails, give the window procedure a       */
   /*   chance to empty the message queue and try again.                      */
   /***************************************************************************/

  while (!WinPostMsg(hwnd, DOS_ERROR, message,
       MPFROM2SHORT(errorcd, MB_CANCEL|MB_ICONEXCLAMATION))) {
    DosSleep(0L);                      /* This give the window thread a chance*/
                                       /* to run                              */
  }                                    /* endwhile                            */
}

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: ProcessSaveQuit                                         */
/*                                                                            */
/*   DESCRIPTIVE NAME: Set up call to Save/Quit dialog.                       */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               Checks to see if the output in the MLE has changed since the */
/*               last save, and if so brings up the Save/Quit dialog.         */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE: ULONG  ProcessSaveQuit                                       */
/*                                                                            */
/*   INPUT:                                                                   */
/*      HWND hWnd - window handle                                             */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*      Returns TRUE if processed correctly                                   */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*      Returns FALSE otherwise                                               */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       SaveQuit - Bring up Save/Quit/Cancel Dialog                          */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       WinDestroyWindow                                                     */
/*       WinInvalidateRect                                                    */
/*       WinQueryWindowULong                                                  */
/*       WinSendMsg                                                           */
/*       WinWindowFromID                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

  ULONG  ProcessSaveQuit(HWND hwnd)
  {
    HWND hEdit;
    ULONG  usRetVal;

    hEdit = WinWindowFromID(hwnd, MLE_WNDW);

    if (WinSendMsg(hEdit, MLM_QUERYCHANGED, NULL, NULL)) {
      usRetVal = (ULONG )SaveQuit(hab, hwnd, pszFileName);
    }
    else {
      usRetVal = 1;
    }

    return (usRetVal);
  }
