/* Fs_queue.hpp */
/* ver 0.00 10.09.2002 */
#ifndef FREEPMS_QUEUE
  #define FREEPMS_QUEUE

   /* QMSG structure */
   typedef struct _FPMQMSG    /* qmsg */
   {
      HWND    hwnd;
      ULONG   msg;
      MPARAM  mp1;
      MPARAM  mp2;
      ULONG   time;   /* compatibility time of message in msec since system boot */
      POINTL  ptl;
      ULONG   uid;    /* user id - reserved in old PM _QMSG */
/* =======> end of old PM _QMSG structure <======= */
      ULONG   remoteId; /* 0- local message, else remote connection id */
      double  dtime;  /* time of message in sec */
   } FPMQMSG;
   typedef FPMQMSG *PFPMQMSG;

   /* SQMSG structure */
   typedef struct _SQMSG    /* sqmsg */
   {
      QMSG    qmsg;         /* message */
      HAB     ihto;         /* iHABto, this field is server writeble */
      HAB     ihfrom;       /* iHABfrom, this field is client writeble */
   } SQMSG;
   typedef SQMSG *PSQMSG;


/* server message queue. */
class  Fs_Queue
{
 HAB     hab;         /* Anchor-block handle.   */
 int     length;      /* queue length, messages */
 int     start;       /* start index of circular queue */
 LONG    lQueuesize;  /* Maximum queue size.    */
 HMQ     hmq;         /* Message-queue handle.  */
 volatile int Access; /* Fast semaphore         */

 SQMSG queue[MAX_SQMSG_SIZE];

public:
 Fs_Queue(void)
 {  lQueuesize   = MAX_SQMSG_SIZE;
    length       = 0;
    hmq          = NULLHANDLE;
    start        = 0;
    Access       = 0;
    hab          = NULL;
 }
 Fs_Queue(int iHab)
 {  lQueuesize   = MAX_SQMSG_SIZE;
    length       = 0;
    hmq          = NULLHANDLE;
    start        = 0;
    Access       = 0;
    hab          = iHab;
 }
 int Add(PSQMSG pmsg);
 int Del(int id);
 int DelFirst(void);
 int Get(PSQMSG pmsg);
 int GetForIhab(PSQMSG pmsg, int iHab);

 int QueryNmsg(int ihabto);

 int GetLength(void)
 {  return length;
 }

};


#endif
   /* FREEPMS_QUEUE  */
