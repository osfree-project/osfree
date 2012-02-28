#define  INCL_DOSMODULEMGR
#define  INCL_DOSPROCESS
#define  INCL_DOSSEMAPHORES
#define  INCL_DOSQUEUES
#define  INCL_DOSERRORS

#undef   INCL_REXXSAA

#include <os2.h>

#define  INCL_REXXSAA

#define  THREAD_STACK 0x400

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

/* structure passed through the queue */
typedef struct
{
  ULONG         something[4];
  HEV           hev;
  HMTX          hmtx;
} buf_t;
