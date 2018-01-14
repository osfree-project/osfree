/*  configuration options passing
 *
 *
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/cfgparser.h>
//#include <os3/ipc.h>

/* libc includes */
#include <string.h>

/* local includes */
#include "api.h"

#define NO_ERROR 0

extern cfg_opts options;

/* offset of structure field */
#define off(p, f) (&(((p)(0))->f)) - &((p)(0)))

/* 'name' structure member offset */
#define O(name) ((int)(&options) + off(cfg_opts *, name))

APIRET CPCfgGetenv(PCSZ name,
                   char **value)
{
  APIRET rc;

  io_log("name=%s\n", name);
  rc = CfgGetenv((char *)name, value);
  io_log("path=%s\n", *value);

  return rc;
}

APIRET CPCfgGetopt(PCSZ name,
                   int *is_int,
                   int *value_int,
                   char **value_str)
{
  int  i, rc = 1;
  char f;
  char ch;
  cfg_opts *o = &options;
  struct tab
  {
    char *name;
    void *addr;
  };

  struct tab intlist[] = 
  {{"autofail",      &(o->autofail)}, 
   {"buffers",       &(o->buffers)},
   {"clockscale",    &(o->clockscale)}, 
   {"closefiles",    &(o->closefiles)}, 
   {"dllbasing",     &(o->dllbasing)},
   {"earlymeminit",  &(o->earlymeminit)},
   {"fakeiss",       &(o->fakeiss)},
   {"i13pages",      &(o->i13pages)},
   {"javanofpk",     &(o->javanofpk)},
   {"ldrstackopt",   &(o->ldrstackopt)},
   {"maxwait",       &(o->maxwait)},
   {"ndwtimer",      &(o->ndwtimer)},
   {"pauseonerror",  &(o->pauseonerror)},
   {"priority",      &(o->priority)},
   {"priority_disk_io", &(o->priority_disk_io)},
   {"protectonly",   &(o->protectonly)},
   {"raskdata",      &(o->raskdata)},
   {"reipl",         &(o->reipl)},
   {"sxfakehwfpu",   &(o->sxfakehwfpu)},
   {"threads",       &(o->threads)},
   {"tracebuf",      &(o->tracebuf)},
   {"truemode",      &(o->truemode)},
   {"vme",           &(o->vme)},
   {"workplace_native", &(o->workplace_native)},
   {"workplace_primary_cp", &(o->workplace_primary_cp)},
   {"workplace_process", &(o->workplace_process)},
   {"wp_objhandle",  &(o->wp_objhandle)},
   {"debugmodmgr",   &(o->debugmodmgr)},
   {"debugixfmgr",   &(o->debugixfmgr)},
   {"debugprcmgr",   &(o->debugprcmgr)}, 
   {"debugmemmgr",   &(o->debugmemmgr)}};
  struct tab chrlist[] = 
  {{"dumpprocess",   &(o->dumpprocess)},
   {"lastdrive",     &(o->lastdrive)},
   {"remote_install_state", &(o->remote_install_state)},
   {"reservedriveletter", &(o->reservedriveletter)},
   {"suppresspopups", &(o->suppresspopups)}};
  struct tab arrlist[] = 
  {{"codepage",      &(o->codepage)}};
  struct tab strlist[] = 
  {{"country",       &(o->country)},
   {"devinfo_kbd",   &(o->devinfo_kbd)},
   {"devinfo_vio",   &(o->devinfo_vio)},
   {"diskcache",     &(o->diskcache)},
   {"ibm_java_options", &(o->ibm_java_options)},
   {"iopl",          &(o->iopl)},
   {"iropt",         &(o->iropt)},
   {"libpath",       &(o->libpath)},
   {"memman",        &(o->memman)},
   {"printmonbufsize", &(o->printmonbufsize)},
   {"protshell",     &(o->protshell)},
   {"sourcepath",    &(o->sourcepath)},
   {"strace",        &(o->strace)},
   {"swapppath",     &(o->swappath)},
   {"sysdump",       &(o->sysdump)},
   {"timeslice",     &(o->timeslice)},
   {"trace",         &(o->trace)},
   {"trapdump",      &(o->trapdump)},
   {"traplog",       &(o->traplog)},
   {"configfile",    &(o->configfile)},
   {"bootdrive",     &(o->bootdrive)}};

  for (f = 0, i = 0; !f && i < sizeof(intlist) / sizeof(struct tab); 
       f = f || !strcmp(name, intlist[i].name), i++) ;
  /* if it is integer field */

  if (f)
  {
    *is_int = 1;
    *value_int = *(short int *)(intlist[i - 1].addr);
    return 0;
  }

  for (f = 0, i = 0; !f && i < sizeof(chrlist) / sizeof(struct tab); 
       f = f || !strcmp(name, chrlist[i].name), i++) ;

  if (f)
  {
    *is_int = 0;
    ch = *(char *)(chrlist[i - 1].addr);
    **value_str = ch;
    *(*value_str + 1) = '\0';
    return 0;
  }

  for (f = 0, i = 0; !f && i < sizeof(arrlist) / sizeof(struct tab);
       f = f || !strcmp(name, arrlist[i].name), i++) ;

  if (f)
  {
    *is_int = 0;
    strcpy(*value_str, (char *)&(arrlist[i - 1].addr));
    return 0;
  }

  for (f = 0, i = 0; !f && i < sizeof(strlist) / sizeof(struct tab);
       f = f || !strcmp(name, strlist[i].name), i++) ;

  if (f)
  {
    *is_int = 0;
    strcpy(*value_str, *(char **)(strlist[i - 1].addr));
    return 0;
  }

  return rc;
}
