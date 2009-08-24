#include <freepm.hpp>
#include <pmclient.h>
#include <habmgr.hpp>
//#include <window.hpp>

extern class _FreePM_HAB  _hab;

#define debug(...)

BOOL    APIENTRY WinRegisterClass(HAB ihab,
                                  PCSZ  pszClassName,
                                  PFNWP pfnWndProc,
                                  ULONG flStyle,
                                  ULONG cbWindowData)
{
//todo
 debug(3, 2)( __FUNCTION__ "is not yet implemented\n");
   return TRUE;
}

/* Create general Window */
HWND    APIENTRY WinCreateWindow(HWND hwndParent,
                                 PCSZ  pszClass,
                                 PCSZ  pszName,
                                 ULONG flStyle,
                                 LONG x,
                                 LONG y,
                                 LONG nx,
                                 LONG ny,
                                 HWND hwndOwner,
                                 HWND hwndInsertBehind,
                                 ULONG id,
                                 PVOID pCtlData,
                                 PVOID pPresParams)
{
  HAB iHab;
  HWND hwnd=NULL;
  FPM_Window *pw;
  int len,rc;

  iHab = _hab.GetCurrentHAB();
    if(iHab < 0)
    {  //_hab.SetError(ihab - bad! , PMERR_INVALID_HAB);
       debug(3, 0)("WARNING: __FUNCTION__: bad ihab %x\n",iHab);
       fatal("Thread don't initialized to FreePM");
       return NULL;
    }

//create hwndClient
    rc = _F_SendCmdToServer(F_CMD_WINCREATE_HWND, iHab);
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {      /* todo: attempt to reconnect till timeout */
       }
       debug(3, 0)("WARNING:__FUNCTION__:SendCmdToServer Error: %s\n",rc);
       fatal("SendCmdToServer Error\n");
    }

    rc = _F_RecvDataFromServer(&hwnd, &len, sizeof(HWND));
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {      /* todo: attempt to reconnect till timeout */
       }
       debug(3, 0)("WARNING:__FUNCTION__:RecvDataFromServer Error: %s\n",rc);
       fatal("RecvDataFromServer Error\n");
    }

    pw = new FPM_Window();
    pw->CreateFPM_Window(hwndParent,  /*  Parent-window handle. */
                        pszClass,    /*  Registered-class name. */
                        pszName,     /*  Window text. */
                        flStyle,     /*  Window style. */
                        x,           /*  x-coordinate of window position. */
                        y,           /*  y-coordinate of window position. */
                        nx,          /*  Width of window, in window coordinates. */
                        ny,          /*  Height of window, in window coordinates. */
                        hwndOwner,   /*  Owner-window handle. */
                        hwndInsertBehind, /*  Sibling-window handle. */
                        id,          /*  Window resource identifier. */
                        pCtlData,    /*  Pointer to control data. */
                        pPresParams);/*  Presentation parameters. */

//    _hab.AddHwnd(hwnd,iHab, pw);

    return hwnd;
}

/* Create standard Window */
HWND  APIENTRY WinCreateStdWindow(HWND hwndParent,
                                     ULONG flStyle,
                                     PULONG pflCreateFlags,
                                     PCSZ  pszClientClass,
                                     PCSZ  pszTitle,
                                     ULONG styleClient,
                                     HMODULE hmod,
                                     ULONG idResources,
                                     PHWND phwndClient)

{
  HWND hwndFrame=NULL, hwndClient=NULL;
  int rc,data,len;
  HAB iHab;
  STD_Window *pStdW;
  iHab = _hab.GetCurrentHAB();
    if(iHab < 0)
    {  //_hab.SetError(ihab - bad! , PMERR_INVALID_HAB);
       debug(3, 0)("WARNING: WinCreateStdWindow: bad ihab %x\n",iHab);
       fatal("Thread don't initialized to FreePM");
       return NULL;
    }

   pStdW = new STD_Window();
   pStdW->CreateSTD_Window(hwndParent,    /*  Parent-window handle. */
                          flStyle,        /*  Frame Window style. */
                          pflCreateFlags,
                          pszClientClass, /* Client Class */
                          pszTitle,       /* Title */
                          styleClient,    /* Client Window style. */
                          hmod,           /* resource module */
                          idResources,    /*  Window resource identifier. */
                          phwndClient);    /*  return: client window hwnd */

// debug(3, 2)( __FUNCTION__ "is not yet full implemented\n");


//create hwndFrame
  hwndFrame = pStdW->GetHandle();

////create hwndClient
//    rc = _F_SendCmdToServer(F_CMD_WINCREATE_HWND, iHab);
////todo check rc
//    rc = _F_RecvDataFromServer(&hwndFrame, &len, sizeof(HWND));
////todo check rc
////create hwndFrame
//    rc = _F_SendCmdToServer(F_CMD_WINCREATE_HWND, iHab);
////todo check rc
//    rc = _F_RecvDataFromServer(&hwndClient, &len, sizeof(HWND));
////todo check rc
//  *phwndClient = hwndClient;
////todo:
////make hwndFrame  frame window, hwndClient - client window
  return hwndFrame;
}

MRESULT APIENTRY WinDefWindowProc(HWND hwnd,
                                  ULONG msg,
                                  MPARAM mp1,
                                  MPARAM mp2)
{  //todo
 debug(3, 2)( __FUNCTION__ "is not yet implemented\n");
     return NULL;
}

BOOL    APIENTRY WinDestroyWindow(HWND hwnd)
{
   //todo
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
    return TRUE;
}

BOOL APIENTRY WinSetActiveWindow(HWND hwndDesktop,  HWND hwnd)
{
  return WinFocusChange(hwndDesktop, hwnd,0);  /* ??? is it really must be 0 ? */
}


