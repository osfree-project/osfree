/* DEBUG: section 3 client Window manager */
// This is message manager of client PM application

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <FreePM.hpp>

#include "habmgr.hpp"

#include <pmclient.h>

#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/time.h>

extern class _FreePM_HAB  _hab;


/* time variables */
time_t  _FreePM_curtime = 0;
struct timeval _FreePM_current_time;
struct timeval _FreePM_start;
double _FreePM_current_dtime = 0.;

time_t
getCurrentTime(void)
{
    struct _timeb timebuffer;

    _ftime(&timebuffer);
    _FreePM_current_time.tv_sec  = (long) timebuffer.time;
    _FreePM_current_time.tv_usec = (long) timebuffer.millitm * 1000;

    _FreePM_current_dtime = (double)timebuffer.time   +
       (double) timebuffer.millitm / 1000.0;
    return _FreePM_curtime = timebuffer.time;
}

int QueryThreadOrdinal(int &tid)
{
   PTIB   ptib;           /* Thread information block structure  */
   PPIB   ppib;           /* Process information block structure */
   APIRET rc;             /* Return code                         */
   PTIB2 pt2;
   int ordinal;

   ptib=NULL;
   ppib=NULL;
   rc=NO_ERROR;

    rc = DosGetInfoBlocks(&ptib, &ppib);
    if (rc != NO_ERROR)
    {  printf ("DosGetInfoBlocks error : rc = %u\n", rc);
          return 1;
    }
    ordinal = ptib->tib_ordinal;
    pt2 = ptib->tib_ptib2;
    tid = pt2->tib2_ultid;
    return ordinal;
}


HMQ     APIENTRY WinCreateMsgQueue(HAB ihab, LONG cmsg)
{
  int ordinal,tid,rc, iHAB;

  // Check for correct HAB
  rc = _hab.QueryHABexist(ihab);
  if(rc < 1)
  {
    _hab.SetError(PMERR_INVALID_HAB);
    debug(3, 0)("WARNING: "__FUNCTION__": bad ihab %x\n",ihab);
    return NULLHANDLE;
  }

  _hab.hab[ihab].pQueue = new FreePM_Queue(ihab);

  return ihab; /* ihab = hmq */
}

BOOL    APIENTRY WinDestroyMsgQueue(HMQ hmq)
{
  if(_hab.hab[hmq].pQueue)
  {
    delete _hab.hab[hmq].pQueue;
  }
  _hab.hab[hmq].pQueue = NULL;

  return TRUE;
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
    FPMQMSG fpmqmsg;

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
       {  rc =  _hab.hab[ihab].pQueue->Get(&fpmqmsg /*pqmsg*/);
          if(rc == 0)
          {  debug(3, 1)("WinGetMsg Getmsg: hwnd %x, msg %x, mp1 %x, mp2 %x\n",pqmsg->hwnd,pqmsg->msg,pqmsg->mp1,pqmsg->mp2);
            if(fpmqmsg.msg == WM_QUIT) brc = FALSE;
            // Здесь копируем кусок сообщения, т.к. мы используем расширенное
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
               debug(3, 0)("WinGetMsg Error: %s\n",rc);
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
                         debug(3, 0)("WinGetMsg Error(2): %s\n",rc);
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
    FPMQMSG msg;
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
       {
           if(_hab.hab[iHAB].pHwnd[indpw].pw == NULL)
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


#if 0

BOOL    APIENTRY WinQueryQueueInfo(HMQ hmq,
                                   PMQINFO pmqi,
                                   ULONG cbCopy)
{
 debug(3, 2)( __FUNCTION__ "is not yet implemented\n");
//todo
    return TRUE;
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


#endif