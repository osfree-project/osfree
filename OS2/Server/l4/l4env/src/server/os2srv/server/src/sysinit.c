/*  Sysinit - system initialisation process
 *
 *
 *
 */

// libc includes
#include <ctype.h>

// L4 includes
//#include <l4/events/events.h>
#include <l4/names/libnames.h>

// osFree includes
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/cfgparser.h>
#include <l4/os3/processmgr.h>
#include <l4/os3/execlx.h>
#include <l4/os3/io.h>

// libc includes
#include <string.h>

extern l4_threadid_t fs;
extern l4_threadid_t sysinit_id;

// use events server
extern char use_events;

void exec_runserver(int ppid);
void exec_protshell(cfg_opts *options);
int exec_run_call(int ppid);
int sysinit (cfg_opts *options);

void create_env(char **pEnv);
void destroy_env(char *pEnv);

char *basename(char *cmdline);
char *skipto (int flag, char *s);
char *getcmd (char *s);

void l4os3_exec(char *cmd, char *params, l4_taskid_t *taskid);

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

void create_env(char **pEnv)
{
  int  i, j, cntVars = 0;
  char **envp = NULL;
  int  bufsize = 0;

  // create global environment array
  for (i = 0; i < 5; i++)
  {
    if (!strcmp(type[i].name, "SET"))
    {
      cntVars = type[i].ip;
      envp = (char **)malloc((cntVars + 1) * sizeof(char *));

      for (j = 0; j < cntVars; j++)
      {
        envp[j] = type[i].sp[j].string;
        bufsize += strlen(envp[j]) + 1;
      }

      envp[cntVars] = NULL;
    }
  }

  // create environment buffer
  *pEnv = (char *)malloc(bufsize);
  bufsize = 0;

  // copy the global environment
  for (j = 0; j < cntVars; j++)
  {
    strcpy(*pEnv + bufsize, envp[j]);
    bufsize += strlen(envp[j]) + 1;
  }

  (*pEnv)[bufsize] = '\0';
  free(envp);
}

void destroy_env(char *pEnv)
{
  free(pEnv);
}

char *basename(char *cmdline)
{
  char *p;

  if ((p = strchr(cmdline, ' ')))
    cmdline[p - cmdline] = '\0';

  return cmdline;
}

void exec_protshell(cfg_opts *options)
{
  int  rc;

  rc = PrcExecuteModule(NULL,
                        0,
                        EXEC_SYNC,
                        NULL,                 // pArgs
                        NULL,                 // pEnv
                        NULL,
                        options->protshell,
                        0);

  if (rc != NO_ERROR) 
    io_log("Error execute: %d ('%s')", rc, options->protshell);

  // Clean up config data
  rc = CfgCleanup();
  if (rc != NO_ERROR)
    io_log("CONFIG.SYS parser cleanup error.");
}

void
exec_runserver(int ppid)
{
  int  i, j;
  char *s, *p, *q, *name;
  char params[] = "";
  char server[0x20];
  char *srv, *to;
  int  timeout = 30000;
  l4_threadid_t tid;
  struct t_os2process *proc; // server PTDA/proc
  
  for (i = 0; i < 5; i++)
  {
    name = (char *)type[i].name;
    io_log("name=%s", name);
    if (!strcmp(name, "RUNSERVER"))
    {
      for (j = 0; j < type[i].ip; j++)
      {
        s = type[i].sp[j].string;
	p = getcmd (s);
	s = skipto(0, s);

        q = strdup(p);

        // create proc/PTDA structure
        proc = PrcCreate(ppid,      // ppid
                         basename(q), // pPrg
                         NULL,      // pArg
                         NULL);     // pEnv

        free(q);

	l4os3_exec (p, params, &tid);
        io_log("started task: %x.%x", tid.id.task, tid.id.lthread);

        /* set task number */
        proc->task = tid;

        if (strstr(p, "os2fs"))
	{
	  io_log("os2fs started");
	  if (!names_waitfor_name("os2fs", &fs, 30000))
	  {
	    io_log("Can't find os2fs on name server!");
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

        io_log("LOOKFOR:%s, TIMEOUT:%d", server, timeout);
	if (*server && !names_waitfor_name(server, &tid, timeout))
	{
	  io_log("Timeout waiting for %s", server);
	  return;
	}
      }
      io_log("Server %s started", server);
    }  
  }
    
  return;
}

int exec_run_call(int ppid)
{
  return 0;
}

int sysinit (cfg_opts *options)
{
  struct t_os2process *proc; // sysinit's PTDA/proc
  //l4events_ch_t event_ch = L4EVENTS_EXIT_CHANNEL;
  //l4events_nr_t event_nr = L4EVENTS_NO_NR;
  //l4events_event_t event;
  char   *env;
  APIRET rc;

  // create global environment
  create_env(&env);

  // Create the sysinit process PTDA structure (pid == ppid == 0)
  proc = PrcCreate(0,         // ppid
                   "sysinit", // pPrg
                   NULL,      // pArg
                   env);      // pEnv

  /* set task number */
  sysinit_id = l4_myself();
  proc->task = sysinit_id;

  if (!names_register("os2srv.sysinit"))
    io_log("error registering on the name server\n");

  /* Start servers */
  exec_runserver(proc->pid);

  /* Start run=/call= */
  exec_run_call(proc->pid);
 
  // Check PROTSHELL statement value
  if (!options->protshell || !*(options->protshell))
  {
    io_log("No PROTSHELL statement in CONFIG.SYS\n");
    rc = ERROR_INVALID_PARAMETER; /*ERROR_INVALID_PARAMETER 87; Not defined for Windows*/
  } else {
    exec_protshell(options);
    rc = 0; /* NO_ERROR */
  }

  io_log("sem wait\n");

  if (!rc) // wait until child process (protshell) terminates (this will unblock us)
    l4semaphore_down(&proc->term_sem);

  io_log("done waiting\n");

  /* if (use_events) // use events server
  {
    // terminate by sending an exit event
    event.len = sizeof(l4_threadid_t);
    *(l4_threadid_t*)event.str = l4_myself();
    // send exit event
    l4events_send(event_ch, &event, &event_nr, L4EVENTS_SEND_ACK);
    // get acknowledge
    l4events_get_ack(&event_nr, L4_IPC_NEVER);
  } */

  io_log("event notification sent\n");

  // unregister at names
  if (!names_unregister_task(sysinit_id))
      io_log("Cannot unregister at name server!\n");

  // destroy proc/PTDA
  PrcDestroy(proc);

  // destroy global environment
  destroy_env(env);

  io_log("OS/2 Server ended\n");
  // terminate OS/2 Server
  exit(rc);

  return rc;
}
