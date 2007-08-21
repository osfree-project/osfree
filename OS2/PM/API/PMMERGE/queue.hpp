#include <osfree.h>

#define MAX_QMSG_SIZE  64
#define MAX_SQMSG_SIZE 256

#define LOCKED    1
#define UNLOCKED  0

/* message queue. */
class  FreePM_Queue
{
 HAB     hab;         /* Anchor-block handle.   */
 int     length;      /* queue length, messages */
 int     start;       /* start index of circular queue */
 LONG    lQueuesize;  /* Maximum queue size.    */
 HMQ     hmq;         /* Message-queue handle.  */
 volatile int Access; /* Fast semaphore         */

 QMSG queue[MAX_QMSG_SIZE];

public:
 FreePM_Queue(void)
 {  lQueuesize   = MAX_QMSG_SIZE;
    length       = 0;
    hmq          = NULLHANDLE;
    start        = 0;
    Access       = 0;
    hab          = NULL;
 }
 FreePM_Queue(int iHab)
 {  lQueuesize   = MAX_QMSG_SIZE;
    length       = 0;
    hmq          = NULLHANDLE;
    start        = 0;
    Access       = 0;
    hab          = iHab;
 }
 int Add(PQMSG pmsg);
 int Del(int id);
 int DelFirst(void);
 int Get(PQMSG pmsg);

 int GetLength(void)
 {  return length;
 }

};
