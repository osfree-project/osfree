#define  INCL_DOSMODULEMGR
#define  INCL_DOSPROCESS
#define  INCL_DOSSEMAPHORES
#define  INCL_DOSQUEUES
#define  INCL_DOSERRORS

#undef   INCL_REXXSAA

#include <os2.h>

#define  INCL_REXXSAA

// worker thread stack size
#define  THREAD_STACK 0x400
// DosSubSetMem pool size
#define  POOL_SIZE 0x10000

// commands received in REQUESTDATA structure
#define CMD_INIT  0xfff8
#define CMD_DONE  0xfff9
#define CMD_FREE  0xfffa
#define CMD_CLOSE 0xfffb
#define CMD_OPEN  0xfffc
#define CMD_LOAD  0xfffd
#define CMD_QUIT  0xffff

/* DLL entry points */
APIRET APIENTRY dllinit (ULONG param);
APIRET APIENTRY RxApiInit (ULONG param);
void   APIENTRY RxInitThread (void *param);

/* forward declaration */
typedef struct quebuf quebuf_t;

/* structure for the REXX queue implementation data */
typedef struct quebuf
{
  quebuf_t      *next;         /* next queue pointer              */
  char          *heapptr;      /* pointer to the heap containing the queue elements */
  ULONG         count;         /* count of elements in the queue  */
  ULONG         timestp;       /* timestamp of last queue access  */
  HEV           hev;           /* event semaphore for some events */
  HMTX          hmtx;          /* mutex protecting the queue      */
  quebuf_t      *first;        /* first queue in the list         */
  quebuf_t      *last;         /* last queue in the list          */
  char          *quename;      /* this queue name                 */
  int           session;       /* "SESSION" queue number          */
} quebuf_t;
