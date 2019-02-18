/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal includes */
#include <os3/processmgr.h>
#include <os3/segment.h>
#include <os3/types.h>
#include <os3/kal.h>
#include <os3/exec.h>
#include <os3/fs.h>
#include <os3/cpi.h>
#include <os3/io.h>
#include <os3/stacksw.h>

/* private memory arena settings */
ULONG   private_memory_base = 0x10000;
ULONG   private_memory_size = 64*1024*1024;
ULONGLONG private_memory_area;

/* shared memory arena settings */
ULONG   shared_memory_base = 0x60000000;
ULONG   shared_memory_size = 1024*1024*1024;
ULONGLONG shared_memory_area;

/* previous stack (when switching between 
   task and os2app stacks)        */
ULONG __stack;

extern l4_os3_thread_t os2srv;
extern l4_os3_thread_t fs;
l4_os3_thread_t execsrv;

l4_os3_thread_t me;

char pszLoadError[260];
ULONG rcCode = 0;

struct options
{
  char  use_events;
  const char  *progname;
};

void test(void);
int init(struct options *opts);
void reserve_regions(void);
VOID CDECL Exit(ULONG action, ULONG result);

VOID CDECL Exit(ULONG action, ULONG result)
{
  STKIN
  // send OS/2 server a message that we want to terminate
  io_log("action=%lu\n", action);
  io_log("result=%lu\n", result);
  CPClientExit(action, result);
  // tell L4 task server that we want to terminate
  TaskExit(result);
  STKOUT
}

int init(struct options *opts)
{
  int rc;

  io_log("OS/2 application wrapper\n");

  if ( (rc = CPClientInit(&os2srv)) )
  {
    io_log("Can't find os2srv, exiting...\n");
    Exit(1, 1);
  }

  if ( (rc = FSClientInit(&fs)) )
  {
    io_log("Can't find os2fs, exiting...\n");
    Exit(1, 1);
  }

  if ( (rc = ExcClientInit(&execsrv)) )
  {
    io_log("Can't find os2exec, exiting...\n");
    Exit(1, 1);
  }

  /* Reserve private and shared regions */
  reserve_regions();

  CPClientTest();

  me = KalNativeID();

  io_log("calling KalStartApp...\n");
  KalStartApp(opts->progname, pszLoadError, sizeof(pszLoadError));

  return 0;
}

void done(void)
{
  FSClientDone();
  ExcClientDone();
  CPClientDone();
}
