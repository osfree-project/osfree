/*  Sysinit - system initialisation process
 *
 *
 *
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/cfgparser.h>
#include <os3/processmgr.h>
#include <os3/loader.h>
#include <os3/thread.h>
#include <os3/io.h>

/* libc includes */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "api.h"

extern l4_os3_thread_t sysinit_id;

void exec_runserver(int ppid);
void exec_protshell(cfg_opts *options);
int exec_run_call(int ppid);
int sysinit (cfg_opts *options);

void create_env(char **pEnv);
void destroy_env(char *pEnv);
void exit_notify(void);

char *basename(char *cmdline);
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

void create_env(char **pEnv)
{
  int  i, j, cntVars = 0;
  char **envp = NULL;
  int  bufsize = 0;

  // create global environment array
  for (i = 0; i < 5; i++)
  {
    if (! strcmp(type[i].name, "SET"))
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
  struct _RESULTCODES res;

  rc = PrcExecuteModule(NULL,
                        0,
                        EXEC_SYNC,
                        NULL,                 // pArgs
                        NULL,                 // pEnv
                        &res,                 // pRes
                        options->protshell,
                        0);

  if (rc != NO_ERROR) 
    io_log("Error execute: %d ('%s')\n", rc, options->protshell);

  // Clean up config data
  rc = CfgCleanup();

  if (rc != NO_ERROR)
    io_log("CONFIG.SYS parser cleanup error.\n");
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
  int time, delta = 10;
  server_t *serv;
  l4_os3_task_t tid;
  struct t_os2process *proc; // server PTDA/proc

  for (i = 0; i < 5; i++)
  {
    name = (char *)type[i].name;

    io_log("name=%s\n", name);
    if (! strcmp(name, "RUNSERVER"))
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

        LoaderExec (p, params, "/dev/vc0", &tid);

        /* set task number */
        proc->task = tid;

	srv     = getcmd (skipto(0, strstr(s, "-LOOKFOR")));

        /* skip spaces and quotes */
        for (p = srv; *p == '"' || *p == ' '; p++) ;
	srv = p;

	for (p = srv + strlen(srv) - 1; *p == '"' || *p == ' '; p--) *p = '\0';
	strcpy (server, srv);

	to      = getcmd (skipto(0, strstr(s, "-TIMEOUT")));
	timeout = atoi (to);

	time = 0;
	for (;;)
	{
	    // wait until timeout expires, or a server starts
	    ThreadSleep(delta);

	    time += delta;

	    if (time > timeout)
	    {
		io_log("Timeout waiting for %s\n", server);
		return;
	    }

	    serv = server_query(server, 0);

	    if (serv)
	    {
		if (serv->ret)
		{
		    io_log("Error %u starting %s!", serv->ret, server);
		    io_log("Error message: %.*s", serv->cbLoadError, serv->szLoadError);
		    return;
		}

		io_log("Server %s started successfully.\n", server);
		break;
	    }
	}
      }
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
  sysinit_id = CPNativeID();
  proc->task = sysinit_id;

  /* Start servers */
  exec_runserver(proc->pid);

  /* Start run=/call= */
  exec_run_call(proc->pid);

  // Check PROTSHELL statement value
  if (! options->protshell || ! *(options->protshell))
  {
    io_log("No PROTSHELL statement in CONFIG.SYS\n");
    rc = ERROR_INVALID_PARAMETER; /*ERROR_INVALID_PARAMETER 87; Not defined for Windows*/
  }
  else
  {
    proc->exec_sync = 1;
    exec_protshell(options);
    rc = NO_ERROR;
  }

  io_log("sem wait\n");

  if (! rc) // wait until child process (protshell) terminates (this will unblock us)
    SemaphoreDown(&proc->term_sem);

  io_log("done waiting\n");

  exit_notify();

  io_log("event notification sent\n");

  // destroy proc/PTDA
  //PrcDestroy(proc); // it is destroyed in CPExit()

  // destroy global environment
  destroy_env(env);

  io_log("OS/2 Server ended\n");
  // terminate OS/2 Server
  exit(rc);

  return rc;
}
