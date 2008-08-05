/* static char *SCCSID = "@(#)pmrxsubs.c        12.7 89/10/13";               */
/*********************  START OF SPECIFICATIONS  ******************************/
/*                                                                            */
/*   SOURCE FILE NAME:  pmrxsubs.c                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  Miscellaneous subroutines to support the PMREXX       */
/*                      program.                                              */
/*                                                                            */
/*   COPYRIGHT:         IBM Corporation 1989                                  */
/*                                                                            */
/*   STATUS:            Version 1.20                                          */
/*                                                                            */
/*   FUNCTION:                                                                */
/*           This module provides several support subroutines used by the     */
/*      PMREXX program. The routines included are listed below.               */
/*                                                                            */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES:                                                         */
/*          This function has dependencies on the following                   */
/*      files for compilation.                                                */
/*          pmrexx.h   - Definitions necessary for the resource               */
/*                       file compilation.                                    */
/*          rhdtatyp.h - Macros, structures, typedefs and defines             */
/*                       local to and necessary for this program.             */
/*          The C runtime multi-threaded header files                         */
/*          See README.C in \TOOLKT12\C for a list and description of the     */
/*          system files that are needed.                                     */
/*                                                                            */
/*                                                                            */
/*   PROCEDURES:                                                              */
/*                                                                            */
/*     Add_Q_Element: Add an element to a queue                               */
/*     getfixedfont:  Get a fixed font for the default display                */
/*     getstring:     Retrieve a string resource                              */
/*     makepipe:      Create a pipe with specific read & write handles        */
/*     Remove_Q_Element: Remove a queue element, wait if none there           */
/*     RestoreWindow: Restore the window from the user profile                */
/*     RHWinErrorBox: Pops up a message box with PM error info                */
/*     SaveWindow:    Save the window position to the user profile            */
/*     SetCheckMark:  Check/uncheck a menu item                               */
/*     setinherit:    Set the inheritance flags for a file handle.            */
/*     SetOptions:    Enable/disable menu options                             */
/*     SysErrorBox:   Pops up an error box with the system error text         */
/*     SysErrorBoxM:  SysErrorBox with added user control                     */
/*                                                                            */
/***********************  END OF SPECIFICATIONS  ******************************/
/******************************************************************************/
/* Include relevant sections of the PM header file.                           */
/******************************************************************************/

#define  INCL_WINERRORS                /* Error code definitions              */
#define  INCL_GPILCIDS                 /* Physical and logical fonts with     */
                                       /* lcids                               */
#define  INCL_WINFRAMEMGR              /* Frame manager                       */
#define  INCL_WINMENUS                 /* Menu controls                       */
#define  INCL_WINSHELLDATA             /* Profile calls                       */
#define  INCL_WINWINDOWMGR
#define  INCL_DOSSEMAPHORES            /* OS/2 semaphore support              */
#define  INCL_WINSTDFONT               /* Standard Font dialog                */
#define  INCL_WINCOUNTRY               /* Code page support                   */
#define  INCL_DOSNLS                   /* NLS (code page) support             */

/******************************************************************************/
/* Include relevant sections of the BSE header file.                          */
/******************************************************************************/

#define  INCL_DOSMISC                  /* Miscellaneous doscalls              */
#define  INCL_DOSERRORS                /* OS/2 errors                         */
#define  INCL_DOSQUEUES                /* Queues                              */
#define  INCL_DOSPROCESS               /* Process and thread support          */
#define  INCL_RXSYSEXIT                /* REXX system exits                   */
#include <os2.h>
#include <rexxsaa.h>

/******************************************************************************/
/* Include the multi-threaded C header files needed.                          */
/******************************************************************************/

#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************/
/* Include application header files                                           */
/******************************************************************************/

#include "rhdtatyp.h"

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: RHWinErrorBox                                           */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Display information about the last PM error in a message     */
/*               box.                                                         */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               When a PM error occurs, this routine can be called to        */
/*               obtain the text of the error information from PM and place   */
/*               it into a message box for display to the user.               */
/*   NOTES:                                                                   */
/*          The code that is commented out allows the error box to contain    */
/*          additional information returned from WinGetErrorInfo.  However,   */
/*          this information is of a technical nature, and not of use to      */
/*          the average user.                                                 */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          ULONG  RHWinErrorBox(HAB hab, HWND hwnd, HMODULE, handle,         */
/*                               ULONG title, ULONG style)                    */
/*                                                                            */
/*   INPUT:                                                                   */
/*       hwnd           - The handle of the window that received the error    */
/*       title          - The resource ID of a string to use as a title for   */
/*                        the message box.                                    */
/*       style          - The style bits to use on the message box.           */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*       returns the value returned from the WinMessageBox call.              */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       getstring      - To obtain the title bar name and window name.       */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       WinFreeErrorInfo                                                     */
/*       WinGetErrorInfo                                                      */
/*       WinMessageBox                                                        */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

ULONG  RHWinErrorBox(HAB hab, HWND hwnd, HMODULE handle, ULONG  title,
    ULONG  style)
{

  ULONG  rc = 0;                       /* return code                         */
  PUCHAR header;                       /* Holds message box title             */
                                       /* Obtain the message box title.       */
  header = getstring(hab, handle, title);

      /* Now, display the message, save the button press for a return code    */

  rc = WinMessageBox(HWND_DESKTOP, hwnd, header, NULL, (USHORT)0, style);
  free(header);                        /* get rid of header                   */

  return  rc;
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
/*      LINKAGE:  getstring(HAB hab, HMODULE handle, ULONG  string_name)      */
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
/*          MAXWIDTH is defined in file RHDTATYP.H.                           */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       WinLoadString                                                        */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

PUCHAR getstring(HAB hab, HMODULE handle, ULONG string_name)
{
  UCHAR buffer[MAXWIDTH];
  ULONG  chars;

  buffer[0] = '\0';
   /* Get the message into our local buffer, then make a copy to return.      */

  chars = WinLoadString(hab, (HMODULE)handle,
      (ULONG)string_name, MAXWIDTH, buffer);
  return  strdup((const char *)buffer);
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: getfixedfont                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Attempt to get a fixed width font for display in the MLE     */
/*               output window.                                               */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This routine will attempt to get a fixed width font for      */
/*               display in the output window.  This is necessary to so       */
/*               that VIO type commands (like DIR) look normal.               */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:  PFATTRS getfixedfont(HWND hwnd)                             */
/*                                                                            */
/*   INPUT:                                                                   */
/*       hwnd           - A window handle to use to obtain a presentation     */
/*                        space with which the font calls to PM may be made.  */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*      Returns a pointer to the FATTRS structure identifying the fixed width */
/*      font. This pointer is then passed to the MLE to tell it what font to  */
/*      use.                                                                  */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*      A null pointer is returned.                                           */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*       DevQueryCaps              PrfQueryProfileData                        */
/*       GpiCreateLogFont          WinGetPS                                   */
/*       GpiQueryDevice            WinReleasePS                               */
/*       GpiQueryFonts                                                        */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

void    getfixedfont(HWND hwnd, PFATTRS fat)
{
  HPS hps = WinGetPS(hwnd);            /* Presentation space handle           */
  HDC hdc = GpiQueryDevice(hps);       /* Device context handle               */
  PFONTMETRICS pfm;                    /* struc with info on font             */
  LONG lHorzRes,                       /* Display horizontal res              */

      lVertRes,                        /* Display vertical res                */
      lRequestFonts,                   /* Number of fonts returned            */
      lNumberFonts;                    /* Number of fonts available           */
  LONG  sIndex;                        /* Loop variable                       */
  LONG alMatch = 0;                    /* TRUE if we found a font             */
  ULONG bytes;                         /* Byte count for Prf call             */

   /* Get the horizontal and vertical resolution of the display.              */

  DevQueryCaps(hdc, CAPS_HORIZONTAL_FONT_RES, 1L, &lHorzRes);
  DevQueryCaps(hdc, CAPS_VERTICAL_FONT_RES, 1L, &lVertRes);

  bytes = sizeof(*fat);

      /* Check the profile for any saved font information                     */

  if (PrfQueryProfileData(HINI_USERPROFILE, "PMREXX_Interface", "UserFont",
      fat, &bytes) && bytes == sizeof(*fat)) {

         /* We successfully read in the saved font information                */
    GpiCreateLogFont(hps, NULL, 1L, fat);
  }

  else {

         /* No profile information found, so look for a font                  */

    lRequestFonts = 0;
    memset(fat, '\0', sizeof(*fat));   /* Clear out fat struct                */

         /* The system is distributed with the Courier fixed width font.      */
         /*   So ask for information about the Courier fonts available.       */

    if (lNumberFonts = GpiQueryFonts(hps, QF_PUBLIC, "Courier",
        &lRequestFonts, 0L, NULL)) {   /* If some number of fonts was         */
                                       /* indicated, make sure the info can be*/
                                       /* contained in a single segment.      */

      if (lNumberFonts *sizeof(FONTMETRICS) < 65536L) {/* Allocate the space  */
                                       /* for the font info                   */

                                       /* and get the information             */
        if (pfm = malloc((LONG )lNumberFonts *sizeof(FONTMETRICS))) {
          GpiQueryFonts(hps, QF_PUBLIC, "Courier", &lNumberFonts,
                        (LONG)sizeof(FONTMETRICS), pfm);
                                       /* Look through the information        */
                                       /* returned for an appropriate font.   */

          for (sIndex = 0; sIndex < (LONG )lNumberFonts; sIndex++) {/* Look   */
                                       /* only at fixed fonts (fsDefn &       */
                                       /* FM_TYPE_FIXED) that match the       */
                                       /* display resolution.                 */

            if (pfm[sIndex].sXDeviceRes == (SHORT)lHorzRes &&
                pfm[sIndex]. sYDeviceRes == (SHORT)lVertRes &&
                (pfm[sIndex].fsDefn & FM_TYPE_FIXED) == 0) {
                                       /* Then look for an 8 point font.      */


              if (pfm[sIndex].sNominalPointSize == 80) {
                alMatch = pfm[sIndex].lMatch;
              }
            }
          }
          free(pfm);                   /* Free the information buffer, now    */
                                       /* that it is no longer needed.        */

          if (alMatch) {               /* Obtain information for a font       */
                                       /* attribute structure.                */
            fat->usRecordLength = sizeof *fat;/* with the info                */
            fat->fsSelection = FATTR_SEL_BOLD;/* for PM to                    */
            fat->lMatch = alMatch;     /* define it.                          */
            strcpy(fat->szFacename, "Courier");
            GpiCreateLogFont(hps, NULL, 1L, fat);
          }
        }
      }
    }
  }

  WinReleasePS(hps);
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
/*          void SetOptions(HWND hwndFrame, ULONG  item, BOOL option )        */
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

void SetOptions(HWND hwndFrame, ULONG item, BOOL option)
{

   /* This call may be issued from a non-PM thread. So we have to use Post    */
   /*   instead of Send. If the Post fails, give the window procedure a       */
   /*   chance to empty the message queue and try again.                      */

  while (!WinPostMsg(WinWindowFromID(hwndFrame, (USHORT)FID_MENU),
      MM_SETITEMATTR, MPFROM2SHORT((USHORT)item, TRUE),
      MPFROM2SHORT(MIA_DISABLED, option?~ MIA_DISABLED:MIA_DISABLED))) {
    DosSleep(0L);                      /* This give the window thread a chance*/
                                       /* to run                              */
  }                                    /* end of while                        */
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
/*          void SetCheckMark(HWND hwndFrame, ULONG item, BOOL option )       */
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

void SetCheckMark(HWND hwndFrame, ULONG item, BOOL option)
{

   /* This call may be issued from a non-PM thread. So we have to use Post    */
   /*   instead of Send. If the Post fails, give the window procedure a       */
   /*   chance to empty the message queue and try again.                      */

  while (!WinPostMsg(WinWindowFromID(hwndFrame, (USHORT)FID_MENU),
      MM_SETITEMATTR, MPFROM2SHORT(item, TRUE),
      MPFROM2SHORT(MIA_CHECKED, option?MIA_CHECKED:~MIA_CHECKED))) {
    DosSleep(0L);                      /* This give the window thread a chance*/
                                       /* to run                              */
  }                                    /* end of while                        */
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: Add_Q_Element                                           */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Add q queue element.                                         */
/*                                                                            */
/*   FUNCTION:                                                                */
/*               This function adds an element to a queue.                    */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void  Add_Q_Element(PLIST q, PLIST_ENTRY q_entry)                 */
/*                                                                            */
/*   INPUT:                                                                   */
/*       q              -  queue                                              */
/*       q_entry        -  queue entry                                        */
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
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

void  Add_Q_Element(PLIST q,PLIST_ENTRY q_entry)
{
  PLIST_ENTRY qptr;                    /* Pointer to an element in the queue  */

  /****************************************************************************/
  /* Get the q semaphore                                                      */
  /****************************************************************************/

  if (!DosRequestMutexSem(q->q_sem, SEM_INDEFINITE_WAIT)) {

    if (qptr = q->q_1st)

     /*************************************************************************/
     /* There are entries in the queue                                        */
     /*************************************************************************/

      {

        /**********************************************************************/
        /* Add entry onto end of queue                                        */
        /**********************************************************************/

      while (qptr->next)
        qptr = qptr->next;
      qptr->next = q_entry;
    }

    else {

        /**********************************************************************/
        /* There are no entries in the queue, so put it in front              */
        /**********************************************************************/

      q->q_1st = q_entry;
    }                                  /* endif                               */
    q_entry->next = NULL;
    DosPostEventSem(q->q_data);
    DosReleaseMutexSem(q->q_sem);
  }
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: Remove_Q_Element                                        */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Remove a queue element.                                      */
/*                                                                            */
/*   FUCNTION:    This function removes an element from a queue and           */
/*                returns it to the caller. If none are available, it         */
/*                waits for one.                                              */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          PLIST_ENTRY  Remove_Q_Element( PLIST q )                          */
/*                                                                            */
/*   INPUT:                                                                   */
/*       q              -  queue                                              */
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
/*       DosEnterCritSec                                                      */
/*       DosExitCritSec                                                       */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

PLIST_ENTRY Remove_Q_Element(PLIST q)
{
  PLIST_ENTRY qptr = NULL;             /* pointer to an element in the queue  */
  ULONG       postcount;               /* number of posted events             */

  /****************************************************************************/
  /* Wait for an element if the queue is empty                                */
  /****************************************************************************/

  while (NULL == qptr) {

     /*************************************************************************/
     /* Get the semaphores to update the queue (and wait for data)            */
     /*************************************************************************/

    if (!DosWaitEventSem(q->q_data, SEM_INDEFINITE_WAIT) &&
        !DosRequestMutexSem(q->q_sem, SEM_INDEFINITE_WAIT)) {

        /**********************************************************************/
        /* Update the queue                                                   */
        /**********************************************************************/

      qptr = q->q_1st;

      if (NULL != qptr)
        q->q_1st = qptr->next;
      DosReleaseMutexSem(q->q_sem);
    }
  }

  DosEnterCritSec();

  /****************************************************************************/
  /* If the queue is empty, set the data semaphore                            */
  /****************************************************************************/

  if (NULL == q->q_1st)
    DosResetEventSem(q->q_data, &postcount);
  DosExitCritSec();
  return  qptr;
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME: makepipe                                                */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*               Make a pipe.                                                 */
/*                                                                            */
/*   FUNCTION:    This function creates a pipe with the read and write        */
/*                handles at the requested handle locations.                  */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void makepipe(PHFILE read, HFILE rspot,                           */
/*                        PHFILE write, HFILE wspot, ULONG  psize )           */
/*                                                                            */
/*   INPUT:                                                                   */
/*       read       -    read file ptr                                        */
/*       rspot      -    read handle                                          */
/*       write      -    write file ptr                                       */
/*       wspot      -    write handle                                         */
/*       psize      -    size of pipe buffer                                  */
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
/*       DosClose                                                             */
/*       DosDupHandle                                                         */
/*       DosCreatePipe                                                        */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

void makepipe(PHFILE read,HFILE rspot,PHFILE write,HFILE wspot,ULONG  psize)
{
  HFILE rh,wh;                         /* read and write handles for pipe     */
  HFILE newh = 0xFFFF;                 /* we want to get a new handle         */
  ULONG  rc;                           /* holds return code                   */

  /****************************************************************************/
  /* Create the pipe                                                          */
  /****************************************************************************/

  if (rc = DosCreatePipe(&rh, &wh, psize)) {

     /* We haven't initialized yet, so we can't display an error              */

    exit(rc);
  }

  if (rh != rspot)

  /****************************************************************************/
  /* We didn't get the read handle we wanted                                  */
  /****************************************************************************/

    {

    if (wh == rspot)

     /*************************************************************************/
     /* The write handle we got is the read handle we wanted                  */
     /*************************************************************************/

      {

        /**********************************************************************/
        /* If the read handle we got isn't the write handle requested, dup    */
        /*   the handle received to what was requested, otherwise get a new   */
        /*   handle.                                                          */
        /**********************************************************************/

      if (rh != wspot)
        newh = wspot;

      if (rc = DosDupHandle(wh, &newh)) {
        exit(rc);
      }
      wh = newh;
    }

     /*************************************************************************/
     /* Dup requested read handle to the read handle we got                   */
     /*************************************************************************/

    if (rc = DosDupHandle(rh, &rspot)) {
      exit(rc);
    }

     /*************************************************************************/
     /* Close the original read handle we received                            */
     /*************************************************************************/

    DosClose(rh);
    rh = rspot;
  }

  if (wh != wspot)

  /****************************************************************************/
  /* We didn't get the write handle we wanted                                 */
  /****************************************************************************/

    {

     /*************************************************************************/
     /* From above we are assured that we can dup to the handle we wanted     */
     /*************************************************************************/

    if (rc = DosDupHandle(wh, &wspot)) {
      exit(rc);
    }

     /*************************************************************************/
     /* Close original write handle we received                               */
     /*************************************************************************/

    DosClose(wh);
    wh = wspot;
  }

  *read = rh;
  *write = wh;
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME:  setinherit                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*              Set the inheritance of the given file handle                  */
/*                                                                            */
/*   FUNCTION:  This function will set the inheritance state of the given     */
/*              file handle.                                                  */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void setinherit(HFILE handle, BOOL inh)                           */
/*                                                                            */
/*   INPUT:                                                                   */
/*       handle     -    handle of given file                                 */
/*       inh        -    inherit or not                                       */
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
/*       DosQueryFHState                                                      */
/*       DosSetFHandState                                                     */
/**************************** END OF SPECIFICATIONS ***************************/

void setinherit(HFILE handle,BOOL inh)
{
  ULONG  state;                        /* Variable to change current state of */
                                       /* the file handle                     */
  DosQueryFHState(handle,
                   &state);

  if (inh) {
    state |= INHERIT;
  }

  else {
    state &= ~INHERIT;
  }
  DosSetFHState(handle,
                state);
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME:  RestoreWindow                                          */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*              Restore the window from the ini file                          */
/*                                                                            */
/*   FUNCTION:  This routine will attempt to restore the window to its        */
/*              previous position saved in the user profile.                  */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void RestoreWindow(HWND hwnd, PRHWINDATA pWinData)                */
/*                                                                            */
/*   INPUT:                                                                   */
/*       hwnd  - window handle                                                */
/*       pWinData - pointer to WinData structure                              */
/*                                                                            */
/*   EXIT-NORMAL:                                                             */
/*                                                                            */
/*   EXIT-ERROR:                                                              */
/*                                                                            */
/*   EFFECTS:                                                                 */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        SizeWindow                                                          */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*        PrfQueryProfileData                                                 */
/*        PrfWriteProfileData                                                 */
/*        WinSetWindowPos                                                     */
/*        WinSetWindowUShort                                                  */
/**************************** END OF SPECIFICATIONS ***************************/

void RestoreWindow(HWND hwnd, PRHWINDATA pWinData)
{
  SAVEDWINDOWPOS windowpos;            /* structure to hold info from profile */
  ULONG bytes;                         /* byte counter                        */

   /* Get Window position from user profile                                   */

  bytes = sizeof(windowpos);

  if (PrfQueryProfileData(HINI_USERPROFILE, "PMREXX_Interface", "WindowState",
      &windowpos, &bytes) && bytes == sizeof(windowpos)) {

      /* Delete the entry from the profile so if the user starts another      */
      /* copy of PMREXX before this one ends, it won't be right on top of us. */

    PrfWriteProfileData(HINI_USERPROFILE, "PMREXX_Interface", "WindowState",
        NULL, 0);

      /* Everything was read in OK, so set window to previous position        */

    WinSetWindowPos(pWinData->frame,
                    HWND_TOP,
                    windowpos.x,
                    windowpos.y,
                    windowpos.cx,
                    windowpos.cy,
                    windowpos.flags);

      /* If window is maximized or minimized, set the position and size to    */
      /* restore it to.                                                       */

    if (windowpos.flags&SWP_MINIMIZE || windowpos.flags&SWP_MAXIMIZE) {
      WinSetWindowUShort(pWinData->frame, QWS_CYRESTORE, windowpos.restorecy);
      WinSetWindowUShort(pWinData->frame, QWS_CXRESTORE, windowpos.restorecx);
      WinSetWindowUShort(pWinData->frame, QWS_YRESTORE, windowpos.restorey);
      WinSetWindowUShort(pWinData->frame, QWS_XRESTORE, windowpos.restorex);
    }
  }

   /* If the window isn't minimized, size the windows inside it               */

  if (!(windowpos.flags&SWP_MINIMIZE)) {
    SizeWindow(hwnd, pWinData);
  }
}


/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SUBROUTINE NAME:  SaveWindow                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:                                                        */
/*              Save information about the current window state.              */
/*                                                                            */
/*   FUNCTION:  This routine will save the window position and the current    */
/*              font in the output window to the user profile.                */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*      LINKAGE:                                                              */
/*          void SaveWindow(PRHWINDATA pWinData)                              */
/*                                                                            */
/*   INPUT:                                                                   */
/*       pWinData - pointer to WinData structure                              */
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
/*        PrfWriteProfileData                                                 */
/*        WinQueryWindowPos                                                   */
/*        WinQueryWindowUShort                                                */
/**************************** END OF SPECIFICATIONS ***************************/

void SaveWindow(PRHWINDATA pWinData)
{
  SAVEDWINDOWPOS savewindowpos;        /* holds window position info          */
  SWP swp;                             /* window poition structure            */

   /* Don't bother saving the data to the profile if we aren't visible        */

  if (pWinData->visible) {

      /* Save the font for the output window to the user profile              */

    PrfWriteProfileData(HINI_USERPROFILE, "PMREXX_Interface", "UserFont",
        &(pWinData->MleFontAttrs.fAttrs), (ULONG)sizeof(FATTRS));

      /* Get Window positions                                                 */

    WinQueryWindowPos(pWinData->frame, &swp);

      /* Set the fields in the savewindowpos structure                        */

    savewindowpos.flags = swp.fl;
    savewindowpos.cy = swp.cy;
    savewindowpos.cx = swp.cx;
    savewindowpos.y = swp.y;
    savewindowpos.x = swp.x;

      /* Get the restore positions, also                                      */

    savewindowpos.restorecy = WinQueryWindowUShort(pWinData->frame,
                                                   QWS_CYRESTORE);
    savewindowpos.restorecx = WinQueryWindowUShort(pWinData->frame,
                                                   QWS_CXRESTORE);
    savewindowpos.restorey = WinQueryWindowUShort(pWinData->frame,
                                                  QWS_YRESTORE);
    savewindowpos.restorex = WinQueryWindowUShort(pWinData->frame,
                                                  QWS_XRESTORE);

      /* Write the window data to the profile                                 */

    PrfWriteProfileData(HINI_USERPROFILE, "PMREXX_Interface", "WindowState",
        &savewindowpos, (ULONG)sizeof(savewindowpos));
  }
}
