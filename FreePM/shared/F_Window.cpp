/*
 $Id: F_Window.cpp,v 1.7 2003/07/17 20:21:38 evgen2 Exp $
*/
/* F_Window.cpp */
/* functions of class FPM_Window */
/* ver 0.01 3.09.2002       */

#include <stdio.h>
#include <string.h>

#define F_INCL_DOSSEMAPHORES
#define F_INCL_DOSNMPIPES
#define F_INCL_DOSPROCESS
#define F_INCL_WIN
  #include "FreePM.hpp"
  #include "F_OS2.hpp"
  #include "F_GPI.hpp"
#include "F_pipe.hpp"
#include "F_hab.hpp"
#include "F_globals.hpp"
#include "FreePM_cmd.hpp"
//#include "FreePM_win.hpp"
//???#include "PM_FPM.h"
#include <F_win.hpp>

#include "F_utils.hpp"

#include <malloc.h>

#ifndef FREPM_SERVER
extern class _FreePM_HAB  _hab;
#endif

/*+---------------------------------+*/
/*| External function prototypes.   |*/
/*+---------------------------------+*/
int _F_SendCmdToServer(int cmd, int data);
int _F_SendGenCmdToServer(int cmd, int par);
int _F_SendGenCmdDataToServer(int cmd, int par, void *data, int datalen);
int _F_SendDataToServer(void *data, int len);
int _F_RecvDataFromServer(void *data, int *len, int maxlen);
//////////////// CreateFPM_Window/ WinCreateWindow /////////////////////////////////////////////
/*
 The appearance and behavior of a window are determined by its style, which is the combination of the
 style established by pszClass and flStyle ORed together. Any of the standard styles WS_* can be used in
 addition to any class-specific styles that may be defined.

 A window is usually created enabled and invisible.  For more information on the initial state of a created
 window, see the list of the standard window styles.

 Messages may be received from other processes or threads when this function is called.

 This function sends the WM_CREATE message to the window procedure of the window being created.

 This function sends the WM_ADJUSTWINDOWPOS message after the WM_CREATE message, and
 before the window is displayed (if applicable).  The values passed are those given to the
 WinCreateWindow function. If the window has style WS_VISIBLE, the window is created visible.

 The WM_SIZE message is not sent by the WinCreateWindow function while the window is being created.
 Any required size processing can be performed during the processing of the WM_CREATE message.

 Because windows are often created with zero height or width and sized later, it is good practice not to
 perform any size-related processing if the size of the window is zero.

 If the WinCreateWindow function is called for a window of class WC_FRAME, the controls specified by are
 created but not formatted. The frame is formatted when a WM_FORMATFRAME message is received but
 this is not sent during window creation. To cause the frame to format, either a WM_FORMATFRAME
 message must be sent, or the window position adjusted using the WinSetWindowPos function call which
 sends a WM_SIZE message if the position or size is changed.

 The only limitation to the size and position specified for a window is the number range allowed for the
 size and position parameters; that is, an application can create windows that are larger than the screen or
 that are positioned partially or totally off the screen. However, the user interface for manipulation of window
 sizes and positions is affected if part or all of the window is off the screen.

 It is recommended that part of the title bar be left on the screen, if the window has one, to enable the
 user to move the window around.

 When a WC_MENU window is created with this call, pCtlData is assumed to be a menu template, which
 is used to create the menu. If pCtlData is NULL, an empty menu is created.
*/
/*
 A window can have a combination of styles; an application can combine styles by using the bitwise inclusive OR operator.  An application usually sets the
 window styles when it creates the window.  The OS/2 operating system provides several standard window styles that apply to all windows.  It also
 provides many styles for the predefined frame and control windows.  The frame and control styles are unique to each predefined window class and can
 be used only for windows of the corresponding class.

 Initially, the styles of the window class used to create the window determine the styles of the new window.  For example, if the window class has the style
 CS_SYNCPAINT, all windows created using that class, by default, will have the window style WS_SYNCPAINT. The OS/2 operating system has the
 following standard window styles:

 ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³Style Name               ³Description                                  ³
 ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 ³WS_CLIPCHILDREN          ³Prevents a window from painting over its     ³
 ³                         ³child windows. This style increases the time ³
 ³                         ³necessary to calculate the visible region.   ³
 ³                         ³This style is usually not necessary because  ³
 ³                         ³if the parent and child windows overlap and  ³
 ³                         ³both are invalidated, the system draws the   ³
 ³                         ³parent window before drawing the child       ³
 ³                         ³window. If the child window is invalidated   ³
 ³                         ³independently of the parent window, the      ³
 ³                         ³system redraws only the child window. If the ³
 ³                         ³update region of the parent window does not  ³
 ³                         ³intersect the child window, drawing the      ³
 ³                         ³parent window causes the child window to be  ³
 ³                         ³redrawn. This style is useful to prevent a   ³
 ³                         ³child window that contains a complex graphic ³
 ³                         ³from being redrawn unnecessarily.            ³
 ³                         ³WS_CLIPCHILDREN is an absolute requirement if³
 ³                         ³a window with children ever performs output  ³
 ³                         ³in response to any message other than        ³
 ³                         ³WM_PAINT. Only WM_PAINT processing is        ³
 ³                         ³synchronized such that the children will get ³
 ³                         ³their messages after the parent.             ³
 ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 ³WS_CLIPSIBLINGS          ³Prevents a window from painting over its     ³
 ³                         ³sibling windows. This style protects sibling ³
 ³                         ³windows but increases the time necessary to  ³
 ³                         ³calculate the visible region. This style is  ³
 ³                         ³appropriate for windows that overlap and that³
 ³                         ³have the same parent window.                 ³
 ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 ³WS_DISABLED              ³Used by an application to disable a window.  ³
 ³                         ³It is up to the window to recognize this     ³
 ³                         ³style and reject input.                      ³
 ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 ³WS_GROUP                 ³Specifies the first control of a group of    ³
 ³                         ³controls in which the user can move from one ³
 ³                         ³control to the next by using the ARROW keys. ³
 ³                         ³All controls defined after the control with  ³
 ³                         ³the WS_GROUP style belong to the same group. ³
 ³                         ³The next control with the WS_GROUP style ends³
 ³                         ³the first group and starts a new group.      ³
 ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 ³WS_MAXIMIZED             ³Enlarges a window to the maximum size.       ³
 ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 ³WS_MINIMIZED             ³Reduces a window to the size of an icon.     ³
 ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 ³WS_PARENTCLIP            ³Extends a window's visible region to include ³
 ³                         ³that of its parent window. This style        ³
 ³                         ³simplifies the calculation of the child      ³
 ³                         ³window's visible region but is potentially   ³
 ³                         ³dangerous because the parent window's visible³
 ³                         ³region is usually larger than the child      ³
 ³                         ³window.                                      ³
 ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 ³WS_SAVEBITS              ³Saves the screen area under a window as a bit³
 ³                         ³map. When the user hides or moves the window,³
 ³                         ³the system restores the image by copying the ³
 ³                         ³bits; there is no need to add the area to the³
 ³                         ³uncovered window's update region. The style  ³
 ³                         ³can improve system performance but also can  ³
 ³                         ³consume a great deal of memory. It is        ³
 ³                         ³recommended only for transient windows, such ³
 ³                         ³as menus and dialog windows, not for main    ³
 ³                         ³application windows.                         ³
 ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 ³WS_SYNCPAINT             ³Causes a window to receive WM_PAINT messages ³
 ³                         ³immediately after a part of the window       ³
 ³                         ³becomes invalid. Without this style, the     ³
 ³                         ³window receives WM_PAINT messages only if no ³
 ³                         ³other message is waiting to be processed.    ³
 ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 ³WS_TABSTOP               ³Specifies one of any number of controls      ³
 ³                         ³through which the user can move by tabbing.  ³
 ³                         ³Pressing the TAB key moves the keyboard focus³
 ³                         ³to the next control that has the WS_TABSTOP  ³
 ³                         ³style.                                       ³
 ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 ³WS_VISIBLE               ³Makes a window visible. The operating system ³
 ³                         ³draws the window on the screen unless        ³
 ³                         ³overlapping windows completely obscure it.   ³
 ³                         ³Windows without this style are hidden. If    ³
 ³                         ³overlapping windows completely obscure the   ³
 ³                         ³window, the window is still considered       ³
 ³                         ³visible.  (Visibility means that the         ³
 ³                         ³operating system draws the window if it can.)³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
///////////////////////////////////////////////////////////////
/* á®§¤ ­¨¥ ®ª­  */
int FPM_Window::CreateFPM_Window
                  (HWND hwndParent,  /*  Parent-window handle. */
                   PCSZ  _pszClass,   /*  Registered-class name. */
                   PCSZ  _pszName,    /*  Window text. */
                   ULONG _flStyle,    /*  Window style. */
                   LONG _x,       /*  x-coordinate of window position relative to the origin of the parent window */
                   LONG _y,       /*  y-coordinate of window position relative to the origin of the parent window */
                   LONG _nx,      /*  Width of window, in window coordinates. */
                   LONG _ny,      /*  Height of window, in window coordinates. */
                   HWND hwndOwner,   /*  Owner-window handle. */
                   HWND hwndInsertBehind, /*  Sibling-window handle. */
                   ULONG _rid,       /*  Window resource identifier. */
                   PVOID pCtlData,   /*  Pointer to control data. */
                   PVOID pPresParams)/*  Presentation parameters. */
{  int i,len,rc,fl;
   HAB iHab;
   HWND hwnd;
   CREATESTRUCT crStruct;
//   class_type = 0;
   pszClass = _pszClass;
   pszName  = _pszName;
   rid = _rid;
#ifndef FREPM_SERVER

  debug(3, 0)(__FUNCTION__"\n");

    iHab = _hab.GetCurrentHAB();
    if(iHab < 0)
    {  //_hab.SetError(ihab - bad! , PMERR_INVALID_HAB);
       debug(3, 0)("WARNING:"__FUNCTION__": bad ihab %x\n",iHab);
       fatal("Thread don't initialized to FreePM");
    }
//   x = _x;
//   y = _y;
//   nx = _nx;
//   ny = _ny;
//create hwnd
    rc = _F_SendCmdToServer(F_CMD_WINCREATE_HWND, iHab);
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {      /* todo: attempt to reconnect till timeout */
       }
       debug(3, 0)("WARNING:"__FUNCTION__":SendCmdToServer Error: %s\n",GetOS2ErrorMessage(rc));
       fatal("SendCmdToServer Error\n");
    }

    rc = _F_RecvDataFromServer(&hwnd, &len, sizeof(HWND));
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {      /* todo: attempt to reconnect till timeout */
       }
       debug(3, 0)("WARNING:"__FUNCTION__":RecvDataFromServer Error: %s\n",GetOS2ErrorMessage(rc));
       fatal("RecvDataFromServer Error\n");
    }
    handle = hwnd;
    _hab.AddHwnd(hwnd,iHab, this);

    if(hwndParent)
    {
       rc = _F_SendCmdToServer(F_CMD_WINSET_PARENT_HWND, handle);
       if(rc)
       {  if(rc == ERROR_BROKEN_PIPE)
          {      /* todo: attempt to reconnect till timeout */
          }
          debug(3, 0)("WARNING:"__FUNCTION__":SendCmdToServer Error: %s\n",GetOS2ErrorMessage(rc));
          fatal("SendCmdToServer Error\n");
       }
       rc =  _F_SendDataToServer((void *)&hwndParent, sizeof(HWND));
       if(rc)
       {  if(rc == ERROR_BROKEN_PIPE)
          {      /* todo: attempt to reconnect till timeout */
          }
          debug(3, 0)("WARNING:__FUNCTION__:_F_SendDataToServer Error: %s\n",GetOS2ErrorMessage(rc));
          fatal("_F_SendDataToServer Error\n");
       }
    }
//send WM_CREATE
    crStruct.pPresParams = pPresParams;
    crStruct.pCtlData    = pCtlData;
    crStruct.id = rid;
    crStruct.hwndInsertBehind = hwndInsertBehind;
    crStruct.hwndOwner = hwndOwner;
    crStruct.cy = _nx;
    crStruct.cx = _ny;
    crStruct.y  = _y;
    crStruct.x  = _x;
    crStruct.flStyle = _flStyle;
    crStruct.pszText = (char *) pszName;
    crStruct.pszClass = (char *)pszClass;
    crStruct.hwndParent = hwndParent;

    SendMsg_to_proc(WM_CREATE, MPARAM (&crStruct), pCtlData);

    fl = 0;
    if(WS_VISIBLE & (_flStyle))
              fl |= SWP_SHOW;
    if(WS_MAXIMIZED & (_flStyle))
              fl |= SWP_MAXIMIZE;
    else
      if(WS_MINIMIZED & (_flStyle))
              fl |= SWP_MINIMIZE;
    fl |= WM_SIZE|WM_MOVE;


    rc = WinSetWindowPos(hwndInsertBehind, _x,_y, _nx, _ny, fl);

    flStyle = _flStyle;

#else
/* ??????  */
//todo
#endif
 /* FREPM_SERVER */


    return 0;
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
****************************************************************
  rc (BOOL) - returns
     Repositioning indicator.

     TRUE
             Window successfully repositioned
     FALSE
             Window not successfully repositioned.

*/
///////////////////////////////////////////////////////////////////
int FPM_Window::
WinSetWindowPos(HWND hwndInsertBehind,
                LONG _x,
                LONG _y,
                LONG _cx,
                LONG _cy,
                ULONG fl)
{   int rc,rc1,rc0=FALSE,len;
    int data[7];
    SWP swp;
    MPARAM mp1,mp2;
    swp.fl = fl;
    swp.cy = _cy;
    swp.cx = _cx;
    swp.y = _y;
    swp.x = _x;
    swp.hwndInsertBehind = hwndInsertBehind;
    swp.hwnd = handle;
    swp.ulReserved1 = 0;
    swp.ulReserved2 = 0;

  debug(3, 1)(__FUNCTION__"%x %i %i %i %i %x  %x\n",hwndInsertBehind,_x,_y,_cx,_cy,fl);

#ifndef FREPM_SERVER

    if(fl & (SWP_SIZE|SWP_MOVE)) /* Change the window size or move */
    {  if(!(fl & SWP_NOADJUST)) /* Not Do not send a WM_ADJUSTWINDOWPOS message before moving or sizing. */
             SendMsg_to_proc(WM_ADJUSTWINDOWPOS, MPARAM (&swp), NULL);
       if(fl & SWP_SIZE) /* Change the window size  */
       {     mp1 = (MPARAM) MAKELONG((short)nx,(short)ny);  /* Warning: ®£à ­¨ç¥­¨¥ à §¬¥à®¢ ¢ 32k */
             mp2 = (MPARAM) MAKELONG((short)_cx,(short)_cy);
             nx = _cx;
             ny = _cy;
             data[0] = nx;
             data[1] = ny;
             SendMsg_to_proc(WM_SIZE, mp1, mp2);
             _F_SendGenCmdDataToServer(F_CMD_WIN_SET_WND_SIZE, handle, data, 2);
       }
       if(fl & SWP_MOVE) /* Change the window x,y position */
       {     x = _x;
             y = _y;
             data[0] = x;
             data[1] = y;
             SendMsg_to_proc(WM_MOVE, NULL, NULL);
             _F_SendGenCmdDataToServer(F_CMD_WIN_SET_WND_SIZE, handle, data, 2);
       }
    }


    if(fl & SWP_ZORDER) /* Change the relative window placement. */
    {
  debug(3, 1)(__FUNCTION__" call for SWP_ZORDER is not implemented yet\n");
    }
    if(fl & (SWP_SHOW|SWP_HIDE))
    {   mp1 = (MPARAM)TRUE; /* Show the window. */
        if(fl & SWP_HIDE) /* Hide the window. */
                 mp1 = (MPARAM)FALSE;
        data[0] = (int) mp1;
        rc = _F_SendGenCmdDataToServer(F_CMD_WIN_SHOW, handle, data, 1);
        SendMsg_to_proc(WM_SHOW, mp1, NULL);
    }

    if(fl & SWP_NOREDRAW) /* Changes are not redrawn. */
    {
  debug(3, 1)(__FUNCTION__" call for SWP_NOREDRAW is not implemented yet\n");
    }

/* Activate the hwnd window if it is a frame window.
   This indicator has no effect on other windows.
   The frame window is made the topmost window, unless SWP_ZORDER is specified also in
    which instance the hwndInsertBehind window is used.
*/
    if(fl & SWP_ACTIVATE)
    {
  debug(3, 1)(__FUNCTION__" call for SWP_ACTIVATE is not implemented yet\n");
    }
/*  Deactivate the hwnd window if it is a frame window. This indicator has no effect on other windows.
    The frame window is made the bottommost window, unless SWP_ZORDER is specified, in
    which instance the hwndInsertBehind window is used.
*/
    if(fl & SWP_DEACTIVATE)
    {
  debug(3, 1)(__FUNCTION__" call for SWP_DEACTIVATE is not implemented yet\n");
    }

/*
   Minimize the window. This indicator has no effect if the window is in a minimized state, and is
   also mutually exclusive with SWP_MAXIMIZE and SWP_RESTORE.
*/
    if(fl & SWP_MINIMIZE)
    {
  debug(3, 1)(__FUNCTION__" call for SWP_MINIMIZE is not implemented yet\n");
    } else if(fl & SWP_MAXIMIZE) {
/*
   Maximize the window. This indicator has no effect if the window is in a maximized state, and is
   also mutually exclusive with SWP_MINIMIZE and SWP_RESTORE.
*/
  debug(3, 1)(__FUNCTION__" call for SWP_MAXIMIZE is not implemented yet\n");

    } else if(fl & SWP_RESTORE) {
/*
   Restore the window. This indicator has no effect if the window is in its normal state, and is
   also mutually exclusive with SWP_MINIMIZE and SWP_MAXIMIZE.

   The position and size of the window in its normal state is remembered in its window words
   when it is first maximized or minimized, although these values can be altered by use of the
   WinSetWindowUShort function.

   The window is restored to the position and size remembered in its window words, unless the
   SWP_MOVE or SWP_SIZE indicators are set.  These indicators cause the position and size
   values specified in this function to be used.
*/
  debug(3, 1)(__FUNCTION__" call for SWP_RESTORE is not implemented yet\n");
    }
#else  /* FREPM_SERVER */

#endif /* FREPM_SERVER */

    return rc0;
}



/* Helper for sending msg to window */
/* constructed msg from umsg, mp1, mp2, add current time and local id */
int FPM_Window::SendMsg_to_proc(ULONG umsg, MPARAM mp1, MPARAM mp2)
{   QMSG msg;
    msg.hwnd = handle;
    msg.msg  = umsg;
    msg.mp1  = mp1;
    msg.mp2  = mp2;
    msg.time = getCurrentTime(); /* ‚ ¯à¥¤¯®«®¦¥­¨¨ time_t = int, todo */
    msg.ptl.x = 0; //todo
    msg.ptl.y = 0; //todo
    msg.uid = 0;
    msg.remoteId = 0;
    msg.dtime = _FreePM_curtime;

    return proc(&msg);
}

int FPM_Window::proc( PQMSG pqmsg)
{
  debug(3, 1)(__FUNCTION__"call with message %x\n",pqmsg->msg);
 //todo
  return NULL;
}

//à¨á®¢ ­¨¥ ®ª­ : ­ã«¥¢®¥ ¯à¨¡«¨¦¥­¨¥
//todo
int FPM_Window::Draw(HPS hps)
{ int rc;
  int windColor = 0xff8040;
  POINTL Point;
debug(3, 0)(__FUNCTION__"call with hps %x\n",hps);
   rc = F_GpiSetColor(hps,windColor);
   Point.x =  Point.y = 0;
   rc = F_GpiMove(hps, &Point);
   Point.x =  nx-1;
   rc = F_GpiLine(hps, &Point);
   Point.y =  ny-1;
   rc = F_GpiLine(hps, &Point);
   Point.x =  0;
   rc = F_GpiLine(hps, &Point);
   Point.y =  0;
   rc = F_GpiLine(hps, &Point);
 return NULL;
}

/* HPS     APIENTRY WinGetPS(HWND hwnd); */

HPS FPM_Window::GetPS(void)
{ HPS hps;
debug(3, 0)(__FUNCTION__"call\n");
//#ifdef FREPM_SERVER
  hps = ::F_WinGetPS(handle);
//#else
//    hps = ::WinGetPS(handle);
//#endif
  return hps;
}

#ifdef __cplusplus
      extern "C" {
#endif
   BOOL    APIENTRY /*F_*/ WinReleasePS(HPS hps);

#ifdef __cplusplus
 }
#endif

BOOL FPM_Window::ReleasePS(HPS hps)
{
debug(3, 0)(__FUNCTION__"call with hps %x\n",hps);
   return /*F_*/ WinReleasePS(hps);
}

int FRAME_Window::proc( PQMSG pqmsg)
{
  debug(3, 1)(__FUNCTION__"call with message %x\n",pqmsg->msg);
  switch(pqmsg->msg)
  {
     case WM_PAINT:
        HPS   hps;
        hps = GetPS();
//draw frame
        Draw(hps);
        ReleasePS(hps);
        break;
  }
 //todo
  return NULL;
}
/*
int FPM_Window::GetPS(F_PS *pps)
{
   if(pps == NULL) return -1;
   pps->dev_class = dev_class;
   pps->x = x;
   pps->y = y;
   pps->nx = nx;
   pps->ny = ny;
   pps->bytesPerPixel = bytesPerPixel;
   return 0;
}
*/

