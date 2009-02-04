/*************************************************************************
*
*  File Name   : STY_EDIT.C
*
*  Description : This module contains the code for the WM_COMMAND
*                messages posted by the standard edit menu.
*
*  Concepts    : Demonstrates the cut, paste, copy, undo, and
*                clear features of an MLE control.
*
*  API's       : WinSendMsg
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

#define INCL_WINMLE

#include <os2.h>
#include "sty_main.h"
#include "sty_xtrn.h"

extern LONG lClrForeground;                         /* color for window text */
extern LONG lClrBackground;                          /* color for background */

/*********************************************************************
 *  Name: EditUndo
 *
 *  Description : Processes the Edit menu's Undo item.
 *
 *  Concepts : Called whenever Undo from the Edit menu is selected
 *             Sends a MLM_UNDO message to the MLE control.
 *
 *  API's : WinSendMsg
 *
 *  Parameters : mp2 - Message parameter
 *
 *  Returns: Void
 *
 ****************************************************************/
VOID EditUndo(MPARAM mp2)
{
 ULONG undoFlag;
  /*
   *   Tell MLE to undo last operation
   */
   if(!WinSendMsg(hwndMLE, MLM_UNDO, NULL, NULL))
        MessageBox(hwndMLE, IDMSG_UNDOFAILED, MB_OK | MB_ERROR, FALSE);

  /*
   *   Get the MLE's colors in case he just changed them back
   *   We're just using the hi-ushort (operation) regardless of the
   *   value of the lo-ushort (boolean) undo-redo flag.
   */
   undoFlag=(ULONG)WinSendMsg(hwndMLE, MLM_QUERYUNDO, NULL, NULL) ;
   if (undoFlag)
   {
      switch (HIUSHORT(undoFlag))
      {
         case MLM_SETTEXTCOLOR:
         case MLM_SETBACKCOLOR:
            lClrForeground =
               (ULONG)WinSendMsg(hwndMLE,MLM_QUERYTEXTCOLOR, NULL, NULL);
            lClrBackground =
               (ULONG)WinSendMsg(hwndMLE,MLM_QUERYBACKCOLOR, NULL, NULL);
            WinInvalidateRect(hwndMLE, NULL, TRUE);
            break;
         default:
            break;
      }    /* endswitch HIUSHORT(undoFlag) */

   } /* end if (UndoFlag) */

  /*
   * This routine currently doesn't use the mp2 parameter but
   * it is referenced here to prevent an 'Unreferenced Parameter'
   * warning at compile time.
   */
   mp2;

}   /*  End of EditUndo()                                               */

/*********************************************************************
 *  Name: EditCut
 *
 *  Description : Processes the Edit menu's Cut item.
 *
 *  Concepts : Called whenever Cut from the Edit menu is selected
 *             Sends a MLM_CUT message to the MLE control.
 *
 *  API's : WinSendMsg
 *
 *  Parameters : mp2 - Message parameter
 *
 *  Returns: Void
 *
 ****************************************************************/
VOID EditCut(MPARAM mp2)
{
   WinSendMsg(hwndMLE, MLM_CUT, NULL, NULL);
 /*
  * This routine currently doesn't use the mp2 parameter but
  * it is referenced here to prevent an 'Unreferenced Parameter'
  * warning at compile time.
  */
   mp2;
}   /*  End of EditCut()                                                */

/*********************************************************************
 *  Name: EditCopy
 *
 *  Description : Processes the Edit menu's Copy item.
 *
 *  Concepts : Called whenever Copy from the Edit menu is selected
 *             Sends a MLM_COPY message to the MLE control.
 *
 *  API's : WinSendMsg
 *
 *  Parameters : mp2 - Message parameter
 *
 *  Returns: Void
 *
 ****************************************************************/
VOID EditCopy(MPARAM mp2)
{
   WinSendMsg(hwndMLE, MLM_COPY, NULL, NULL);
 /*
  * This routine currently doesn't use the mp2 parameter but
  * it is referenced here to prevent an 'Unreferenced Parameter'
  * warning at compile time.
  */
   mp2;
}   /*   End of EditCopy()                                              */

/*********************************************************************
 *  Name: EditPaste
 *
 *  Description : Processes the Edit menu's Paste item.
 *
 *  Concepts : Called whenever Paste from the Edit menu is selected
 *             Sends a MLM_PASTE message to the MLE control.
 *
 *  API's : WinSendMsg
 *
 *  Parameters : mp2 - Message parameter
 *
 *  Returns: Void
 *
 ****************************************************************/
VOID EditPaste(MPARAM mp2)
{
   WinSendMsg(hwndMLE, MLM_PASTE, NULL, NULL);
 /*
  * This routine currently doesn't use the mp2 parameter but
  * it is referenced here to prevent an 'Unreferenced Parameter'
  * warning at compile time.
  */
   mp2;
}   /* End of EditPaste()                                               */

/*********************************************************************
 *  Name: EditClear
 *
 *  Description : Processes the Edit menu's Clear item.
 *
 *  Concepts : Called whenever Clear from the Edit menu is selected
 *             Sends a MLM_CLEAR message to the MLE control.
 *
 *  API's : WinSendMsg
 *
 *  Parameters : mp2 - Message parameter
 *
 *  Returns: Void
 *
 ****************************************************************/
VOID EditClear(MPARAM mp2)
{
   WinSendMsg(hwndMLE, MLM_CLEAR, NULL, NULL);
 /*
  * This routine currently doesn't use the mp2 parameter but
  * it is referenced here to prevent an 'Unreferenced Parameter'
  * warning at compile time.
  */
   mp2;
}   /* End of EditClear()                                               */
