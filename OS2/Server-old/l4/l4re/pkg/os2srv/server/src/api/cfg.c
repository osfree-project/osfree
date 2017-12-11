/*  configuration options passing
 *
 *
 */
 
#include <string.h>

#include <l4/log/l4log.h>
#include <l4/os3/cfgparser.h>
#include <l4/os3/gcc_os2def.h>
#include <os2server-server.h>

#include <dice/dice.h>

#define NO_ERROR 0

extern cfg_opts options;

/* offset of structure field */
#define off(p, f) (&(((p)(0))->f)) - &((p)(0)))
/* 'name' structure member offset */
#define O(name) ((int)(&options) + off(cfg_opts *, name))

APIRET DICE_CV
os2server_cfg_getenv_component (CORBA_Object _dice_corba_obj,
                                const char* name /* in */,
                                char* *value /* out */,
                                CORBA_Server_Environment *_dice_corba_env)
{
  APIRET rc;
  LOG("name=%s", name);
  rc = CfgGetenv(name, value);
  LOG("path=%s", *value);
  return rc;
}


APIRET DICE_CV
os2server_cfg_getopt_component (CORBA_Object _dice_corba_obj,
                                const char* name,
                                int *is_int,
                                int *value_int,
                                char* *value_str,
                                CORBA_Server_Environment *_dice_corba_env)
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
