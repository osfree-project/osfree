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

#include <l4/os3/processmgr.h>
#include <l4/os3/execlx.h>

l4_threadid_t    loader_id;
l4_threadid_t    fprov_id;
l4_threadid_t    dm_id;

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

  if (!names_register("os2srv.pshell"))
    LOG("error registering on the name server");

  rc = PrcExecuteModule(NULL, 0, 0, "", "", NULL, options->protshell, 0);
  if (rc != NO_ERROR) 
    LOG("Error execute: %d ('%s')", rc, options->protshell);

  // Clean up config data
  rc = CfgCleanup();
  if (rc != NO_ERROR)
  {
    LOG("CONFIG.SYS parser cleanup error.");
    //return rc;
  }

  LOG("OS/2 Server ended");

  l4_ipc_sleep(L4_IPC_NEVER);
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

	srv     = getcmd (skipto(0, strstr(s, "-LOOKFOR")));
	strcpy (server, srv);
	to      = getcmd (skipto(0, strstr(s, "-TIMEOUT")));
	timeout = atoi (to);

        LOG("LOOKFOR:%s, TIMEOUT:%d", server, timeout);
	if (*server && names_waitfor_name(server, &tid, timeout))
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

int sysinit (void)
{
  CORBA_Server_Environment env = dice_default_server_environment;

  /* Wait for servers to be started */
  LOG("waiting for LOADER server to be started...");
  if (!names_waitfor_name("LOADER", &loader_id, 30000))
    {
      LOG("Dynamic loader LOADER not found -- terminating");
      while (1) l4_sleep(0.1);
    }
  LOG("loader id: %x.%x", loader_id.id.task, loader_id.id.lthread);
  
  if (!names_waitfor_name("fprov_proxy_fs", &fprov_id, 30000))
    {
      LOG("File provider \"fprov_proxy_fs\" not found, fatal!");
      while (1) l4_sleep(0.1);
    }
  LOG("fprov_id=%x.%x", fprov_id.id.task, fprov_id.id.lthread);

  if (!names_waitfor_name("DM_PHYS", &dm_id, 30000))
    {
      LOG("Dataspace manager \"dm_phys\" not found, fatal!");
      while (1) l4_sleep(0.1);
    }
  LOG("dm_id=%x.%x", dm_id.id.task, dm_id.id.lthread);

  /* Start servers */
  exec_runserver();
 
  // Check PROTSHELL statement value
  if (!options.protshell||(strlen(options.protshell)==0))
  {
    io_printf("No PROTSHELL statement in CONFIG.SYS");
    return ERROR_INVALID_PARAMETER; /*ERROR_INVALID_PARAMETER 87; Not defined for Windows*/
  } else {
    l4thread_create(executeprotshell, (void *)&options, L4THREAD_CREATE_ASYNC);

  }

  // server loop
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2server_server_loop(&env);
  
  return 0;  
}
