/*
 $Id: Fs_queue.hpp,v 1.1 2002/09/11 10:56:23 evgen2 Exp $
*/
/* Fs_queue.hpp */
/* ver 0.00 10.09.2002 */
#ifndef FREEPMS_QUEUE
  #define FREEPMS_QUEUE

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
