/*  osFree Exec server
 *
 *
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/cfgparser.h>
#include <os3/memmgr.h>
#include <os3/dataspace.h>
#include <os3/thread.h>
#include <os3/types.h>
#include <os3/cpi.h>
#include <os3/fs.h>
#include <os3/io.h>

/* libc includes */
#include <string.h>

/* local includes */
#include "api.h"

l4_os3_thread_t thread;

l4_os3_thread_t execsrv;
l4_os3_thread_t os2srv;
l4_os3_thread_t fs;

l4_os3_thread_t fprov_id;

/* shared memory arena settings */
extern void      *shared_memory_base;
extern ULONG     shared_memory_size;
extern ULONGLONG shared_memory_area;

struct options
{
  char use_events;
  char *configfile;
  char *kal_map;
};

void reserve_regions(void);
int init(struct options *opts);
void done(void);

extern cfg_opts options;

/* Root mem area for memmgr */
struct t_mem_area root_area;

int init(struct options *opts)
{
  os2exec_module_t s = {0};
  char szLoadError[260];
  void *addr;
  unsigned long size;
  int  rc;

  io_log("osFree Exec server\n");

  //init_globals();

  memset (&options, 0, sizeof(options));

  if (opts->kal_map)
  {
    options.kal_map = opts->kal_map;
  }

  if (opts->configfile)
  {
    options.configfile = opts->configfile;
  }

  if ( CPClientInit(&os2srv) )
    {
      io_log("os2srv not found on name server!\n");
      return -1;
    }

  if ( FSClientInit(&fs) )
    {
      io_log("os2fs not found on name server!\n");
      return -1;
    }

  fprov_id = fs;

  reserve_regions();

#if 0
  if (! CfgGetopt("debugmodmgr", &is_int, &value_int, (char **)&p) )
  {
    if (is_int)
      options.debugmodmgr = value_int;
  }

  if (! CfgGetopt("debugixfmgr", &is_int, &value_int, (char **)&p) )
  {
    if (is_int)
      options.debugixfmgr = value_int;
  }

  if (! CfgGetopt("libpath", &is_int, &value_int, (char **)&p) )
    if (! is_int)
    {
      options.libpath = (char *)malloc(strlen(p) + 1);
      strcpy(options.libpath, p);
    }

  io_log("debugmodmgr=%d\n", options.debugmodmgr);
  io_log("debugixfmgr=%d\n", options.debugixfmgr);
  io_log("libpath=%s\n", options.libpath);
#endif
  //options.libpath = (char *)malloc(4);
  //strcpy(options.libpath, "c:\\");

  // Initialize initial values from config.sys
  rc = CfgInitOptions();

  if (rc)
  {
    io_log("Cannot initialize CONFIG.SYS parser!\n");
    return -1;
  }

  io_log("configfile=%s\n", opts->configfile);

  // Load CONFIG.SYS into memory
  rc = io_load_file(opts->configfile, &addr, &size);

  if (rc)
  {
    io_log("Can't load CONFIG.SYS!\n");
    return -1;
  }

  // Parse CONFIG.SYS in memory
  rc = CfgParseConfig((char *)addr, size);

  if (rc)
  {
    io_log("Error parsing CONFIG.SYS!\n");
    return -1;
  }

  // Release all memory allocated by parser
  CfgCleanup();

  // Remove CONFIG.SYS from memory
  io_close_file(addr);

#if 0
  /* load shared libs */
  rc = loadso();
  if (rc)
  {
    io_log("Error loading shared libraries\n");
    return -1;
  }
#endif

  init_memmgr(&root_area);

  /* load IXF's */
  rc = load_ixfs();

  if (rc)
  {
    io_log("Error loading IXF driver\n");
    return -1;
  }

  /* Initializes the module list. Keeps info about which dlls an process have loaded and
     has linked to it (Only support for LX dlls so far). The head of the linked list is
     declared as a global inside dynlink.c */
  rc = ModInitialize();

  if (rc)
  {
    io_log("Can't initialize module manager\n");
    return -1;
  }

  // notify os2srv about successful startup
  CPClientAppNotify2(&s, "os2exec", &thread,
                     szLoadError, sizeof(szLoadError), rc);

  return 0;
}

void done(void)
{
  FSClientDone();
  CPClientDone();
}
