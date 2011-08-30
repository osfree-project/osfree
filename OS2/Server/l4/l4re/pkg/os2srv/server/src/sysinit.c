/*  Sysinit - system initialisation process
 *
 *
 *
 */

#include <ctype.h>

#include <l4/os3/cfgparser.h>
#include <dice/dice.h>
#include <l4/names/libnames.h>
#include <l4/thread/thread.h>
#include <l4/os3/gcc_os2def.h>
#include <l4/log/l4log.h>
#include <l4/events/events.h>
#include <l4/generic_ts/generic_ts.h>

#include <l4/os3/processmgr.h>
#include <l4/os3/execlx.h>

extern l4_threadid_t fs;
extern l4_threadid_t sysinit_id;

// use events server
extern char use_events;

void exec_runserver(void);
void executeprotshell(cfg_opts *options);
char *skipto (int flag, char *s);
char *getcmd (char *s);

char cmdbuf[1024];

char *
getcmd (char *s)
{
  char *p;
  int  i;
  char *str = (char *)cmdbuf;
  
  for (p = s, i = 0; *p && !isspace(*p); p++, i++)
    str[i] = *p;
    
  str[i++] = '\0';
  
  return str;
}

char *
skipto (int flag, char *s)
{
  while (*s && *s != ' ' && *s != '\t' &&
         ! (flag && *s == '='))
    s ++;

  while (*s == ' ' || *s == '\t' ||
         (flag && *s == '='))
    s ++;

  return s;
}

void executeprotshell(cfg_opts *options)
{
  int rc;

  rc = PrcExecuteModule(NULL, 0, EXEC_SYNC, "", "", NULL, options->protshell, 0);
  if (rc != NO_ERROR) 
    LOG("Error execute: %d ('%s')", rc, options->protshell);

  // Clean up config data
  rc = CfgCleanup();
  if (rc != NO_ERROR)
  {
    LOG("CONFIG.SYS parser cleanup error.");
    //return rc;
  }


  //l4_ipc_sleep(L4_IPC_NEVER);
}

void
exec_runserver(void)
{
  int  i, j;
  char *s, *p, *name;
  char params[] = "";
  char server[0x20];
  char *srv, *to;
  int  timeout = 30000;
  l4_threadid_t tid;
  
  for (i = 0; i < 5; i++)
  {
    name = type[i].name;
    LOG("name=%s", name);
    if (!strcmp(name, "RUNSERVER"))
    {
      for (j = 0; j < type[i].ip; j++)
      {
        s = type[i].sp[j].string;
	p = getcmd (s);
	s = skipto(0, s);

	l4_exec (p, params, &tid);
        LOG("started task: %x.%x", tid.id.task, tid.id.lthread);

        if (strstr(p, "os2fs"))
	{
	  LOG("os2fs started");
	  if (!names_waitfor_name("os2fs", &fs, 30000))
	  {
	    LOG("Can't find os2fs on name server!");
	    return;
	  }
	}

	srv     = getcmd (skipto(0, strstr(s, "-LOOKFOR")));

        /* skip spaces and quotes */
        for (p = srv; *p == '"' || *p == ' '; p++) ;
	srv = p;
	for (p = srv + strlen(srv) - 1; *p == '"' || *p == ' '; p--) *p = '\0';
	strcpy (server, srv);

	to      = getcmd (skipto(0, strstr(s, "-TIMEOUT")));
	timeout = atoi (to);

        LOG("LOOKFOR:%s, TIMEOUT:%d", server, timeout);
	if (*server && !names_waitfor_name(server, &tid, timeout))
	{
	  LOG("Timeout waiting for %s", server);
	  return;
	}
      }
      LOG("Server %s started", server);
    }  
  }
    
  return;
}

int exec_run_call(void)
{
  return 0;
}

int sysinit (cfg_opts *options)
{
  struct t_os2process *proc; // sysinit's PTDA/proc
  l4events_ch_t event_ch = L4EVENTS_EXIT_CHANNEL;
  l4events_nr_t event_nr = L4EVENTS_NO_NR;
  l4events_event_t event;
  APIRET rc;

  // Create the sysinit process PTDA structure (pid == ppid == 0)
  proc = PrcCreate(0);
  /* set task number */
  sysinit_id = l4_myself();
  proc->task = sysinit_id;
  /* assign params and environment */
  PrcSetArgsEnv("sysinit", "", "", proc);

  if (!names_register("os2srv.sysinit"))
    LOG("error registering on the name server");

  /* Start servers */
  exec_runserver();

  /* Start run=/call= */
  exec_run_call();
 
  // Check PROTSHELL statement value
  if (!options->protshell || !*(options->protshell))
  {
    io_printf("No PROTSHELL statement in CONFIG.SYS");
    rc = ERROR_INVALID_PARAMETER; /*ERROR_INVALID_PARAMETER 87; Not defined for Windows*/
  } else {
    executeprotshell(options);
    rc = 0; /* NO_ERROR */
  }

  if (!rc) // wait until child process (protshell) terminates (this will unblock us)
    l4semaphore_down(&proc->term_sem);

  if (use_events) // use events server
  {
    // terminate by sending an exit event
    event.len = sizeof(l4_threadid_t);
    *(l4_threadid_t*)event.str = l4_myself();
    // send exit event
    l4events_send(event_ch, &event, &event_nr, L4EVENTS_SEND_ACK);
    // get acknowledge
    l4events_get_ack(&event_nr, L4_IPC_NEVER);
  }

  // unregister at names
  if (!names_unregister_task(sysinit_id))
      LOG("Cannot unregister at name server!");

  LOG("OS/2 Server ended");
  // terminate OS/2 Server
  //l4ts_exit(); // this will send an exit event through events server too
  exit(rc);

  return rc;  
}
