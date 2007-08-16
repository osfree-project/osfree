/*
 $Id: FreePM_win.cpp,v 1.8 2003/06/15 17:45:48 evgen2 Exp $
*/
/* FreePM_win.cpp */
/* DEBUG: section 3 client Window manager */
/* ver 0.00 5.09.2002       */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreePM.hpp"


#define F_INCL_DOSSEMAPHORES
#define F_INCL_DOSNMPIPES
#define F_INCL_DOSPROCESS
#define F_INCL_WINERRORS /* Or use INCL_WIN, INCL_PM, */
#define F_INCL_SHLERRORS
#define F_INCL_GPIERRORS
   #include "F_OS2.hpp"

#include "F_pipe.hpp"
#include "F_utils.hpp"
#include "F_win.hpp"
#include "F_hab.hpp"
#include "F_globals.hpp"
#include "FreePM_cmd.hpp"

class _FreePM_HAB  _hab;

/*+---------------------------------+*/
/*| External function prototypes.   |*/
/*+---------------------------------+*/
int InitServerConnection(char *remotemachineName);
int CloseServerConnection(void);
int _F_SendCmdToServer(int cmd, int data);
int _F_SendDataToServer(void *data, int len);
int _F_RecvDataFromServer(void *data, int *len, int maxlen);

/*+---------------------------------+*/
/*| Internal function prototypes.   |*/
/*+---------------------------------+*/
MRESULT EXPENTRY _F_DefaultWindowProc( HWND hwndWnd, ULONG ulMsg, MPARAM mpParm1,MPARAM mpParm2 );

/*** WinInitialize/WinTerminate Interface declarations ******************/

/* WinInitialize
Input:
  flOptions (ULONG) - input  Initialization options.

     0       The initial state for newly created windows is that all messages for the window are available for
             processing by the application.
             This is the only option available in PM.
Output:
  hab (HAB) - returns   Anchor-block handle.

     NULLHANDLE         An error occurred.
     Other              Anchor-block handle.
Errors:
    PMERR_ALREADY_INITIALIZED
    FPMERR_INITSERVER_CONNECTION
*/

HAB APIENTRY WinInitialize(ULONG flOptions)
{   int ordinal,tid,rc, iHAB,len,inf[2];
    ordinal = QueryThreadOrdinal(tid);
    rc = _hab.QueryOrdinalUsed(ordinal, Q_ORDINAL_HAB);
    if(rc != -1)
    {  _hab.SetError(rc, PMERR_ALREADY_INITIALIZED);
       return NULLHANDLE;
    }
/* Connect to server */
    rc =  InitServerConnection(NULL);
    if(rc)
    {  _hab.SetError(FPMERR_INITSERVER_CONNECTION);
       return NULLHANDLE;
    }

    rc = _F_SendCmdToServer(F_CMD_GET_IHAB,  0);
    rc = _F_RecvDataFromServer(inf, &len, sizeof(inf));
    if(rc)
    {  _hab.SetError(FPMERR_INITSERVER_CONNECTION);
        return NULLHANDLE;
    }
    rc =  _hab.AddHAB(ordinal,inf[0],inf[1]);
    if(rc) iHAB = NULLHANDLE;
    else   iHAB = inf[0];

    return iHAB;
}

BOOL APIENTRY WinTerminate(HAB ihab)
{   int rc;
    rc = _F_SendCmdToServer( F_CMD_CLIENT_EXIT,  ihab);
    rc = CloseServerConnection();
    _hab.DelHAB(ihab);
    return TRUE;
}

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

HMQ     APIENTRY WinCreateMsgQueue(HAB ihab, LONG cmsg)
{   int ordinal,tid,rc, iHAB;
    rc = _hab.QueryHABexist(ihab);
    if(rc < 1)
    {  return NULLHANDLE;
    }
    _hab.hab[ihab].pQueue = new FreePM_Queue(ihab);
    return ihab; /* ihab = hmq */
}

BOOL    APIENTRY WinDestroyMsgQueue(HMQ hmq)
{   if(_hab.hab[hmq].pQueue)
    {
         delete _hab.hab[hmq].pQueue;
    }
    _hab.hab[hmq].pQueue = NULL;

    return TRUE;
}

BOOL    APIENTRY WinQueryQueueInfo(HMQ hmq,
                                   PMQINFO pmqi,
                                   ULONG cbCopy)
{
 debug(3, 2)( __FUNCTION__ "is not yet implemented\n");
//todo
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
       debug(3, 0)("WARNING:__FUNCTION__:SendCmdToServer Error: %s\n",GetOS2ErrorMessage(rc));
       fatal("SendCmdToServer Error\n");
    }

    rc = _F_RecvDataFromServer(&hwnd, &len, sizeof(HWND));
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {      /* todo: attempt to reconnect till timeout */
       }
       debug(3, 0)("WARNING:__FUNCTION__:RecvDataFromServer Error: %s\n",GetOS2ErrorMessage(rc));
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
                                  PSZ pszClientClass,
                                  PSZ pszTitle,
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

/*********************************************************************************
 WinGetMsg
 #define INCL_WINMESSAGEMGR  Or use INCL_WIN, INCL_PM, Also in COMMON section

 This function gets, waiting if necessary, a message from the thread's message queue
 and returns when a  message conforming to the filtering criteria is available.

 rc = WinGetMsg(hab, pqmsgmsg, hwndFilter, ulFirst, ulLast);

Return: rc (BOOL) - returns  Continue message indicator.

     TRUE   Message returned is not a WM_QUIT message
     FALSE  Message returned is a WM_QUIT message.

ERRORS:
  PMERR_INVALID_HWND (0x1001)     An invalid window handle was specified.
  FPMERR_NULL_POINTER             pqmsg is NULL
***********************************************************************************/
//todo: filter implementation
//todo: semaphore implementation with client-server
BOOL    APIENTRY WinGetMsg(HAB ihab,             /* Anchor-block handle.         */
                           PQMSG pqmsg,          /* Pointer to Message structure */
                           HWND hwndFilter,      /* Window filter          */
                           ULONG msgFilterFirst, /* First message identity */
                           ULONG msgFilterLast)  /* Last message identity  */
{   BOOL brc = TRUE;
    int rc,rcs;

 debug(3, 2)("WinGetMsg call\n");

    if(pqmsg == NULL)
    {  _hab.SetError(ihab, FPMERR_NULL_POINTER);
       return brc;
    }
//test ihab
    rc =  _hab.QueryHABexist(ihab);
    if(rc != 1)
    {  //_hab.SetError(ihab - bad! , PMERR_INVALID_HAB);
       debug(3, 0)("WARNING: WinGetMsg: bad ihab %x\n",ihab);
       return brc;
    }
/***  Wait messages loop */
    do
    {
       rc = _hab.hab[ihab].pQueue->GetLength();
       if(rc)
       {  rc =  _hab.hab[ihab].pQueue->Get(pqmsg);
          if(rc == 0)
          {  debug(3, 1)("WinGetMsg Getmsg: hwnd %x, msg %x, mp1 %x, mp2 %x\n",pqmsg->hwnd,pqmsg->msg,pqmsg->mp1,pqmsg->mp2);
            if(pqmsg->msg == WM_QUIT) brc = FALSE;
            return brc;
          }
       } else {
            int len,nmsg;
//Query number of messages on server
            rc = _F_SendCmdToServer(F_CMD_WINQUERY_MSG, ihab);
            if(rc)
            {  if(rc == ERROR_BROKEN_PIPE)
               {      /* todo: attempt to reconnect till timeout */
               }
               debug(3, 0)("WinGetMsg Error: %s\n",GetOS2ErrorMessage(rc));
               fatal("WinGetMsg Error\n");
            }
//todo check rc
            rc = _F_RecvDataFromServer(&nmsg, &len, sizeof(int));
            if(rc == 0 && nmsg > 0)
            {     rc = _F_SendCmdToServer(F_CMD_WINGET_MSG, ihab);
                  rc = _F_RecvDataFromServer(&rcs, &len, sizeof(int));
                  if(rc == 0 && rcs == 1)
                  {   rc = _F_RecvDataFromServer(pqmsg, &len, sizeof(QMSG));
                      if(rc == 0)
                      {  if(pqmsg->msg == WM_QUIT) brc = FALSE;
                         return brc;
                      } else {
                         if(rc == ERROR_BROKEN_PIPE)
                         {      /* todo: attempt to reconnect till timeout */
                         }
                         debug(3, 0)("WinGetMsg Error(2): %s\n",GetOS2ErrorMessage(rc));
                         fatal("WinGetMsg Error(2)\n");
                      }
                  }
            }
       }
//No messages: Let's sleep
       debug(3, 9)("WinGetMsg Sleep\n");
       DosSleep(1);
    } while(1);

}

/*********************************************************************************
WinPeekMsg

Errors:
  PMERR_INVALID_HWND (0x1001)      An invalid window handle was specified.

  PMERR_INVALID_FLAG (0x1019)      An invalid bit was set for a parameter. Use constants defined
                                   by PM for options, and do not set any  reserved bits.

***********************************************************************************/

BOOL    APIENTRY WinPeekMsg(HAB ihab,
                            PQMSG pqmsg,
                            HWND hwndFilter,
                            ULONG msgFilterFirst,
                            ULONG msgFilterLast,
                            ULONG fl)
{   BOOL brc;
    int rc;
//test ihab
    rc =  _hab.QueryHABexist(ihab);
    if(rc != 1)
    {  //_hab.SetError(ihab - bad!, PMERR_INVALID_HAB);
       debug(3, 0)("WARNING: WinPeekMsg: bad ihab %x\n",ihab);
       return TRUE;
    }
//todo
    brc = TRUE;
    return brc;
}

/*

 The message contains hwnd, ulMsgid, mpParam1, mpParam2, and the time and pointer position when this
 function is called.

 WinPostMsg returns immediately, while WinSendMsg waits for the receiver to return.

 A thread which does not have a message queue can still call WinPostMsg but cannot call WinSendMsg.

FreePM extra: A thread wich even does not call WinInitialize can still call  WinPostMsg

Errors:
//PMERR_INVALID_HAB
PMERR_NO_MSG_QUEUE

*/
BOOL    APIENTRY WinPostMsg(HWND hwnd,
                               ULONG umsg,
                               MPARAM mp1,
                               MPARAM mp2)
{   int rc,i;
    QMSG msg;
    BOOL brc;
    msg.hwnd = hwnd;
    msg.msg = umsg;
    msg.mp1 = mp1;
    msg.mp2 = mp2;
    msg.time =  getCurrentTime(); /* В предпложении time_t = int, todo */

//todo    msg.ptl =
    msg.uid = 0;
    msg.remoteId = 0;
    msg.dtime = _FreePM_curtime;

/* проверим все окна, принадлежащие  hab на предмет принадлежности hwnd */
  rc = _hab.QueryHwnd(hwnd);
  if(rc > 0) /* внутреннее сообщение */
  {    int  iHABto;
       iHABto = rc -1;
       rc =  _hab.QueryHABexist(iHABto);
       if(rc != 1)
       {  _hab.SetError(PMERR_INVALID_HAB);
          debug(3, 0)("WARNING: WinPostMsg: bad iHABto %x for hwnd %x\n",iHABto,hwnd);

          return NULL;
       }

       if(_hab.hab[iHABto].pQueue == NULL)
       {  _hab.SetError(PMERR_NO_MSG_QUEUE);
          return NULL;
       }
       _hab.hab[iHABto].pQueue->Add(&msg);

  } else {      /* внешнее сообщение */
     SQMSG sqmsg;
     sqmsg.qmsg = msg;
     sqmsg.ihfrom = _hab.GetCurrentHAB();
     rc =  _F_SendCmdToServer(F_CMD_WINPOSTMSG, sizeof(SQMSG));
     rc =  _F_SendDataToServer((void *)&sqmsg, sizeof(SQMSG));
  }

    brc = TRUE;
    return brc;
}

/**********************************************************

 mresReply is the value returned by the window procedure that is invoked. For standard window
 classes, the values of mresReply are documented with the message definitions.

 This function does not complete until the message has been processed by the window procedure
 whose  return value is returned in mresReply.

 If the window receiving the message belongs to the same thread, the window function is called
 immediately as a subroutine. If the window is of another thread or process, the operating
 system switches to the appropriate thread that enters the necessary window procedure recursively.
 The message is not  placed in the queue  of the destination thread.

 If a message is sent from one process to another and the message contains a pointer, the receiving
 process may not have read/write access to the memory referenced by the pointer. If the receiving
 process is expected to update that memory, this must be done using shared memory. For more
 information about Dynamic Data Exchange (DDE) and shared memory, see "Dynamic Data Exchange"
 section of the Presentation Manager Programming Guide - Advanced Topics.

************************************************************/
/*
Errors:
PMERR_INVALID_HAB
*/

MRESULT APIENTRY WinSendMsg(HWND hwnd,
                            ULONG umsg,
                            MPARAM mp1,
                            MPARAM mp2)
{   int rc,rcf,i,iHAB;
    QMSG msg;
    msg.hwnd = hwnd;
    msg.msg = umsg;
    msg.mp1 = mp1;
    msg.mp2 = mp2;
    msg.time =  getCurrentTime(); /* В предпложении time_t = int, todo */

//todo    msg.ptl =
    msg.uid = 0;
    msg.remoteId = 0;
    msg.dtime = _FreePM_curtime;

/* проверим все окна, принадлежащие  hab на предмет принадлежности hwnd */
  rc = _hab.QueryHwnd(hwnd);
  if(rc > 0) /* внутреннее сообщение */
  {    int ordinal, iHab, tid,iHABto, indpw;
       iHABto = rc -1;
       ordinal = QueryThreadOrdinal(tid);
       rc = _hab.QueryOrdinalUsed(ordinal,Q_ORDINAL_HAB);
       if(rc == -1)
       {  _hab.SetError(PMERR_INVALID_HAB);
          debug(3, 0)("WARNING: WinSendMsg: error PMERR_INVALID_HAB\n");

         return NULL;
       }

       iHAB = rc;
       indpw = _hab.QueryHwnd(hwnd, iHABto);
       if(indpw == 0) /* May be if from QueryHwnd(hwnd) hwnd was deleted, todo: semafor or smth */
        {  _hab.SetError(iHAB, PMERR_INVALID_HWND);
          return NULL;
       }
       indpw--; /* indexes start from zero */
       if(iHAB == iHABto)
       {   if(_hab.hab[iHAB].pHwnd[indpw].pw == NULL)
           {    _hab.SetError(iHAB, FPMERR_NULL_WINDPROC);
                return NULL;
           }
           rcf = _hab.hab[iHAB].pHwnd[indpw].pw->proc(&msg);
       } else {
//todo: how to call proc from thread iHABto ?

//       _hab.hab[iHAB].pQueue->Add(&msg);
///* цикл ожидания */
       }
  } else {      /* внешнее сообщение */
     SQMSG sqmsg;
     sqmsg.qmsg = msg;
     sqmsg.ihfrom = _hab.GetCurrentHAB();
     rc =  _F_SendCmdToServer(F_CMD_WINSENDMSG, sizeof(SQMSG));
     rc =  _F_SendDataToServer((void *)&sqmsg, sizeof(SQMSG));
/* цикл ожидания */
//todo

  }

    return NULL;
}


BOOL    APIENTRY WinRegisterUserMsg(HAB hab,
                                    ULONG msgid,
                                    LONG datatype1,
                                    LONG dir1,
                                    LONG datatype2,
                                    LONG dir2,
                                    LONG datatyper)
{
//todo
 debug(3, 2)( __FUNCTION__ "is not yet implemented\n");
    return TRUE;
}

MRESULT APIENTRY WinDispatchMsg(HAB hab,
                                   PQMSG pqmsg)
{ int ordinal, iHab, tid,iHABto, indiHabto, indpw;
  int rc,rcf;

   ordinal = QueryThreadOrdinal(tid);
   rc = _hab.QueryOrdinalUsed(ordinal,Q_ORDINAL_HAB);
   if(rc == -1)
       {  _hab.SetError(PMERR_INVALID_HAB);
          debug(3, 0)("WARNING: WinDispatchMsg: error PMERR_INVALID_HAB\n");
         return NULL;
       }
   iHab = rc;

//  pqmMsg->hwnd;
/* проверим все окна, принадлежащие  hab на предмет принадлежности hwnd */
  rc = _hab.QueryIndexesHwnd(pqmsg->hwnd,indpw);
  if(rc > 0) /* внутреннее сообщение */
  {   indiHabto = rc -1;
       if(_hab.hab[indiHabto].pHwnd[indpw].pw == NULL)
       {    _hab.SetError(iHab, FPMERR_NULL_WINDPROC);
                return NULL;
       }
       rcf = _hab.hab[indiHabto].pHwnd[indpw].pw->proc(pqmsg);

  } else {
       debug(3, 0)( "WARNING:" __FUNCTION__ "is called for external HWND ?");
  }

  return NULL;
}


/*
WinGetLastError
WinGetLastError2
 This function returns the error code set by the failure of a Presentation Manager function.

 In multiple thread applications where there are multiple anchor blocks, errors are stored in the anchor
 block created by the WinInitialize function of the thread invoking a call. The last error for the process and
 thread on which this function call is made will be returned.

 Returns
 -1 = hab not exist or not used, or
  the last nonzero error code, and sets the error code to zero.
*/
ERRORID     APIENTRY    WinGetLastError(HAB iHAB)
{   int rc;
    rc = _hab.QueryHABexist(iHAB);
    if(rc != 1)
       return -1;
    rc = _hab.hab[iHAB].lastError;
    _hab.hab[iHAB].lastError = 0;
    return rc;

}

ERRORID     APIENTRY    WinGetLastError2(void)
{   int ordinal,tid,rc, ind;
    ordinal = QueryThreadOrdinal(tid);
    ind = _hab.QueryOrdinalUsed(ordinal,Q_ORDINAL_INDEX);
    if(ind == -1)
       return -1;

    rc = _hab.hab[ind].lastError;
    _hab.hab[ind].lastError = 0;
    return rc;
}


/*
 This function allocates a single private segment to contain the ERRINFO structure. All the pointers to string
 fields within the ERRINFO structure are offsets to memory within that segment.

 The memory allocated by this function is not released until the returned pointer is passed to the
 WinFreeErrorInfo function.

 Returns:

    NULL     No error information available
    Other    pointer to structure contains information about the previous error code for the current thread:

   { ERRINFO,       sizeof(ERRINFO)
     char *pstr,    sizeof(char *)
     NULL,          sizeof(char *)
     char str[]     strlen(errorinfo)+1
   }
*/
//todo: handle "thread not initialized to FreePM (PMERR_INVALID_HAB)"

PERRINFO    APIENTRY    WinGetErrorInfo(HAB iHAB)
{  int rc, errid,l;
   ERRINFO *perrInf;
   char *perrstr, *perrInfStr, * *ptr;

   errid = WinGetLastError(iHAB);
   if(errid == -1) return NULL;
   perrstr = GetFreePMErrorMessage(errid);
   l = strlen(perrstr);

   perrInf = (PERRINFO) calloc(sizeof(ERRINFO)+ l+1 + 2 * sizeof(char *)+1, 1);
   perrInf->cbFixedErrInfo = sizeof(ERRINFO);
   perrInf->idError =  errid;
   perrInf->offaoffszMsg = sizeof(ERRINFO);
   perrInf->offBinaryData = 0;
   ptr = (char * *)( ((char *)perrInf) + sizeof(ERRINFO));
   perrInfStr = ((char *)perrInf) + sizeof(ERRINFO) + 2 * sizeof(char *);
   strcpy(perrInfStr,perrstr);
   *ptr =  perrInfStr;
   return perrInf;
}

BOOL        APIENTRY    WinFreeErrorInfo(PERRINFO perrinfo)
{  if(perrinfo == NULL) return FALSE;
   free(perrinfo);
   return TRUE;
}


/***************************/


MRESULT EXPENTRY _F_DefaultWindowProc ( HWND hwndWnd, ULONG ulMsg, MPARAM mpParm1,MPARAM mpParm2 )
{
//todo
 debug(3, 2)( __FUNCTION__ "is not yet implemented\n");
  return NULL;
}

MRESULT APIENTRY WinDefWindowProc(HWND hwnd,
                                  ULONG msg,
                                  MPARAM mp1,
                                  MPARAM mp2)
{  //todo
 debug(3, 2)( __FUNCTION__ "is not yet implemented\n");
     return NULL;
}


