/*
 $Id: F_win1.cpp,v 1.4 2003/06/15 17:45:48 evgen2 Exp $
*/
/* F_win1.cpp */
/* General functions for Create/Destroy/Show/Move windows */
/* DEBUG: section 6  General functions for Create/Destroy/Show/Move/Set Owner/Child/Parent windows */
/* ver 0.00 7.09.2002       */

#include <stdio.h>
#include <string.h>
#include "FreePM.hpp"
#define F_INCL_DOSPROCESS
#define F_INCL_DOSNMPIPES
   #include "F_OS2.hpp"
#include "F_pipe.hpp"
#include "FreePM_cmd.hpp"
#include "F_win.hpp"
#include "F_hab.hpp"
#include "F_globals.hpp"

/*+---------------------------------+*/
/*| External function prototypes.   |*/
/*+---------------------------------+*/
int _F_SendCmdToServer(int cmd, int data);
int _F_SendGenCmdToServer(int cmd, int par);
int _F_SendGenCmdDataToServer(int cmd, int par, void *data, int datalen);
int _F_SendDataToServer(void *data, int len);
int _F_RecvDataFromServer(void *data, int *len, int maxlen);

/* for use debug - include stdio.h and F_globals.hpp */
#include "F_globals.hpp"


BOOL    APIENTRY WinDestroyWindow(HWND hwnd)
{
   //todo
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
    return TRUE;
}

BOOL    APIENTRY WinShowWindow(HWND hwnd, BOOL fShow)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}

BOOL    APIENTRY WinQueryWindowRect(HWND hwnd, PRECTL prclDest)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}

HPS     APIENTRY F_WinGetPS(HWND hwnd)
{   HPS hps;
    hps = (HPS) _F_SendGenCmdToServer(F_CMD_GET_HPS, hwnd);
    return hps;
}

BOOL    APIENTRY WinReleasePS(HPS hps)
{   int rc;
    rc =  _F_SendGenCmdToServer(F_CMD_RELEASE_HPS, hps);
    return rc;
}

BOOL    APIENTRY WinEndPaint(HPS hps)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}

BOOL    APIENTRY WinEnableWindow(HWND hwnd,  BOOL fEnable)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}

BOOL    APIENTRY WinIsWindowEnabled(HWND hwnd)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}

BOOL    APIENTRY WinEnableWindowUpdate(HWND hwnd, BOOL fEnable)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}

BOOL    APIENTRY WinIsWindowVisible(HWND hwnd)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}


LONG    APIENTRY WinQueryWindowText(HWND hwnd,
                                    LONG cchBufferMax,
                                    PCH pchBuffer)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return 0;
}


BOOL    APIENTRY WinSetWindowText(HWND hwnd,
                                  PCSZ  pszText)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}


LONG    APIENTRY WinQueryWindowTextLength(HWND hwnd)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return 0;
}

HWND    APIENTRY WinWindowFromID(HWND hwndParent,
                                 ULONG id)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return NULL;
}

HWND    APIENTRY WinWindowFromDC(HDC hdc)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return NULL;
}

 
/*
 This function returns the device context for a given window. 
 
 HWND    hwnd;   Window handle. 
 HDC     hdc;    Device-context handle.
*/
HDC     APIENTRY WinQueryWindowDC(HWND hwnd)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return NULL;
}


BOOL    APIENTRY WinIsWindow(HAB hab,
                             HWND hwnd)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}

HWND    APIENTRY WinQueryWindow(HWND hwnd,
                                LONG cmd)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return NULL;
}


LONG    APIENTRY WinMultWindowFromIDs(HWND hwndParent,
                                      PHWND prghwnd,
                                      ULONG idFirst,
                                      ULONG idLast)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return 0;
}

BOOL   APIENTRY WinSetParent(HWND hwnd,
                             HWND hwndNewParent,
                             BOOL fRedraw)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}


BOOL   APIENTRY WinIsChild(HWND hwnd,
                           HWND hwndParent)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}

BOOL   APIENTRY WinSetOwner(HWND hwnd,
                            HWND hwndNewOwner)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}

/////////////////////////////WinSetWindowPos//////////////////////////////////////
/*
 If a window created with the CS_SAVEBITS style is reduced, the screen image saved is used to redraw
 the area uncovered when the window size changes, if those bits are still valid.

 If the CS_SIZEREDRAW style is present, the entire window area is assumed invalid if sized. Otherwise,
 WM_CALCVALIDRECTS is sent to the window to inform the window manager which bits it may be
 possible to preserve.

 Messages sent from WinSetWindowPos and WinSetMultWindowPos have specific orderings within the
 window-positioning process.  The process begins with redundancy checks and precalculations on every
 window for each requested operation. For example, if SWP_SHOW is present but the window is already
 visible, SWP_SHOW is turned off. If SWP_SIZE is present, and the new size is equal to the old size,
 SWP_SIZE is turned off.

 If the operations create new results, the information is calculated and stored (for instance, when sizing or
 moving, the new window rectangle is stored for later use).  It is at this point that the
 WM_ADJUSTWINDOWPOS message is sent to any window that is sizing or moving.  It is also at this
 point that the WM_CALCVALIDRECTS message is sent to any window that is sizing and does not have
 the CS_SIZEREDRAW window style.

 When all the new window states are calculated, the window-management process begins.  Window areas
 that can be preserved are moved from the old to the new positions, window areas that are invalidated by
 these operations are calculated and distributed as update regions. When this is finished, and before any
 synchronous-paint windows are repainted, the WM_SIZE message is sent to any windows that have
 changed size. Next, all the synchronous-paint windows that can be are repainted, and the process is
 complete.

 If a synchronous-paint parent window has a size-sensitive area displayed that includes synchronous-paint
 child windows, the parent needs to reposition those windows when it receives the WM_SIZE message.
 Their invalid regions are added to the parent's invalid region, resulting in one update after the parent's
 WM_SIZE message, rather than many independent (and later, duplicated) updates.

 Note:  Some windows will not be positioned precisely to the parameters of this function, but according to
        the behavior of their window procedure.  For example, frame windows without a style creation flag
        of FCF_NOBYTEALIGN will not position to any specific screen coordinate. Similarly, frame windows
        with zero size and position are created by the WinCreateStdWindow function and therefore these
        values are treated as a special case by the frame window procedure.

  Messages sent by this function are:

  WM_ACTIVATE             Sent if a different window becomes the active window. See also
                          WinSetActiveWindow.

  WM_ADJUSTWINDOWPOS Not sent if SWP_NOADJUST is specified.  The message contains an SWP
                          structure that has been filled in by this function with the proposed move/size data.
                          The window can adjust this new position by changing the contents of the SWP
                          structure. If hwnd specifies a frame window, this function recalculates the sizes and
                          positions of the frame controls. If the new window rectangle for any frame control is
                          empty, instead of resizing or repositioning that control, it is hidden if SWP_HIDE is
                          specified. This eliminates needless processing of windows that are not visible. The
                          window rectangle of the control in question is left in its original state.  For example,
                          if WinSetWindowPos is issued to change the size of a standard-frame window to
                          an empty rectangle, and WinQueryWindowRect is issued against the client window,
                          the rectangle returned is not an empty rectangle, but the original client rectangle
                          before WinSetWindowPos was issued.

  WM_CALCVALIDRECTS Sent to determine the area of a window that may be possible to preserve as the
                          window is sized.

  WM_SIZE                 Sent if the size of the window has changed, after the change has been made.

  WM_MOVE                 Sent when a window with CS_MOVENOTIFY class style moves its absolute
                          position.
*/
///////////////////////////////////////////////////////////////////


BOOL   APIENTRY WinSetWindowPos(HWND hwnd,
                                HWND hwndInsertBehind,
                                LONG x,
                                LONG y,
                                LONG cx,
                                LONG cy,
                                ULONG fl)
{   int rc,rc1,rc0=FALSE,indwind,len;
    int data[7];

/* проверим все окна, принадлежащие  _hab на предмет принадлежности hwnd текущей задаче */
  rc = _hab.QueryIndexesHwnd(hwnd, indwind);
  if(rc > 0) /* внутреннее окно вызовем FPM_Window::WinSetWindowPos */
  {   rc0 = _hab.hab[rc-1].pHwnd[indwind].pw->WinSetWindowPos(hwndInsertBehind, x, y, cx, cy, fl);
  } else {  /* внешнее окно */
    debug(6, 2)( __FUNCTION__ " is not yet implemented for extrnal hwnd\n");

  }
/*

    rc = _F_SendCmdToServer(F_CMD_WIN_SET_WND_POS, hwnd);
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {      / * todo: attempt to reconnect till timeout * /
       }
       debug(3, 0)("WARNING:__FUNCTION__:SendCmdToServer Error: %s\n",GetOS2ErrorMessage(rc));
       fatal("SendCmdToServer Error\n");
    }
    data[0] = hwndInsertBehind;
    data[1] = (int) x;
    data[2] = (int) y;
    data[3] = (int) cx;
    data[4] = (int) cy;
    data[5] = (int) fl;
    data[6] = 0;    / * z * /

    rc =  _F_SendDataToServer((void *)&data, sizeof(data));
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {      / * todo: attempt to reconnect till timeout * /
       }
       debug(3, 0)("WARNING:__FUNCTION__:_F_SendDataToServer Error: %s\n",GetOS2ErrorMessage(rc));
       fatal("_F_SendDataToServer Error\n");
    }
    rc = _F_RecvDataFromServer(&rc1, &len, sizeof(HPS));
    if(!rc && rc1 == TRUE)
          rc0 = TRUE;
*/

    return rc0;
}

BOOL   APIENTRY WinSetMultWindowPos(HAB hab,
                                    PSWP pswp,
                                    ULONG cswp)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}

BOOL   APIENTRY WinQueryWindowPos(HWND hwnd,
                                  PSWP pswp)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
   //todo
    return TRUE;
}

BOOL APIENTRY WinSetActiveWindow(HWND hwndDesktop,  HWND hwnd)
{  return WinFocusChange(hwndDesktop, hwnd,0);  /* ??? точно 0 ? */
}


/*** Keyboard and mouse input COMMON subsection *************************/
BOOL  APIENTRY WinSetFocus(HWND hwndDesktop,
                            HWND hwndSetFocus)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
 return 0;
}

BOOL  APIENTRY WinFocusChange(HWND hwndDesktop,
                              HWND hwndSetFocus,
                              ULONG flFocusChange)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
 return 0;
}

