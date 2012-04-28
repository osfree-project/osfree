#include <freepm.hpp>
#include <pmclient.h>
#include <string.h>
#include <F_hab.hpp>
#include "F_def.hpp"
#include "debug.h"

extern class _FreePM_HAB  _hab;

//#define debug(...)

extern "C" BOOL    APIENTRY Win32RegisterClass(HAB ihab,
                                  PCSZ  pszClassName,
                                  PFNWP pfnWndProc,
                                  ULONG flStyle,
                                  ULONG cbWindowData)
{
   int i;
   struct _FreePM_ClassInfo *ci;

   debug(3, 0)( __FUNCTION__ " called\n");
   i  = _hab.hab[ihab].numWinClasses;

   if (!_hab.hab[ihab].lAllocWinClasses)
     _hab.hab[ihab].lAllocWinClasses = 16;

   if (!i)
     _hab.hab[ihab].pWinClassList = (struct _FreePM_ClassInfo *)calloc(_hab.hab[ihab].lAllocWinClasses,
                                    sizeof(struct _FreePM_ClassInfo));
   else if (!(i % _hab.hab[ihab].lAllocWinClasses))
     _hab.hab[ihab].pWinClassList = (struct _FreePM_ClassInfo *)realloc((void *)_hab.hab[ihab].pWinClassList, 
                                    _hab.hab[ihab].lAllocWinClasses * sizeof(struct _FreePM_ClassInfo));

   // current class info
   ci = _hab.hab[ihab].pWinClassList;

   _hab.hab[ihab].numWinClasses++;
   ci += i;

   if (!pszClassName || !pfnWndProc)
     return FALSE;

   ci->ci.pfnWindowProc = pfnWndProc;
   ci->ci.flClassStyle  = flStyle;
   ci->ci.cbWindowData  = cbWindowData;

   strncpy(ci->classname, pszClassName, 128);
   ci->classname[strlen(pszClassName)] = '\0';
   debug(3, 0)("WinRegisterClass: successful\n");

   return TRUE;
}

/* Create general Window */
extern "C" HWND    APIENTRY Win32CreateWindow(HWND hwndParent,
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
       _fatal("Thread don't initialized to FreePM");
       return NULL;
    }

//create hwndClient
    rc = F_SendCmdToServer(client_obj, F_CMD_WINCREATE_HWND, iHab);
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {      /* todo: attempt to reconnect till timeout */
       }
       debug(3, 0)("WARNING:__FUNCTION__:SendCmdToServer Error: %s\n",rc);
       _fatal("SendCmdToServer Error\n");
    }

    rc = F_RecvDataFromServer(client_obj, &hwnd, &len, sizeof(HWND));
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {      /* todo: attempt to reconnect till timeout */
       }
       debug(3, 0)("WARNING:__FUNCTION__:RecvDataFromServer Error: %s\n",rc);
       _fatal("RecvDataFromServer Error\n");
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
extern "C" HWND  APIENTRY Win32CreateStdWindow(HWND hwndParent,
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
       _fatal("Thread don't initialized to FreePM");
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

 debug(3, 2)( __FUNCTION__ "is not yet full implemented\n");


//create hwndFrame
  hwndFrame = pStdW->GetHandle();

////create hwndClient
//    rc = F_SendCmdToServer(client_obj, F_CMD_WINCREATE_HWND, iHab);
////todo check rc
//    rc = F_RecvDataFromServer(client_obj, &hwndFrame, &len, sizeof(HWND));
////todo check rc
////create hwndFrame
//    rc = F_SendCmdToServer(client_obj, F_CMD_WINCREATE_HWND, iHab);
////todo check rc
//    rc = F_RecvDataFromServer(client_obj, &hwndClient, &len, sizeof(HWND));
////todo check rc
//  *phwndClient = hwndClient;
////todo:
////make hwndFrame  frame window, hwndClient - client window
  return hwndFrame;
}

extern "C" MRESULT APIENTRY Win32DefWindowProc(HWND hwnd,
                                  ULONG msg,
                                  MPARAM mp1,
                                  MPARAM mp2)
{  //todo
 debug(3, 2)( __FUNCTION__ "is not yet implemented\n");
     return NULL;
}

extern "C" BOOL    APIENTRY Win32DestroyWindow(HWND hwnd)
{
   //todo
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
    return TRUE;
}

extern "C" BOOL APIENTRY Win32SetActiveWindow(HWND hwndDesktop,  HWND hwnd)
{
  return WinFocusChange(hwndDesktop, hwnd,0);  /* ??? is it really must be 0 ? */
}


