/* API implementations
 * (on the server side)
 */
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#include <gcc_os2def.h>

#include <io.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <l4/sys/types.h>
#include <l4/sys/syscalls.h>
#include <l4/log/l4log.h>
#include <l4/semaphore/semaphore.h>
#include <l4/thread/thread.h>

#include <dice/dice.h>

char buf[0x100];
void exe_end(void);

extern l4_threadid_t os2srv;
extern l4semaphore_t sem;

APIRET
os2server_dos_Read_component(CORBA_Object _dice_corba_obj,
                             HFILE hFile, void **pBuffer,
                             ULONG *pcbRead,
                             CORBA_Server_Environment *_dice_corba_env)
{
  int  c;
  int  nread = 0;
  int  total = 1;

  nread = read(hFile, (char *)*pBuffer, *pcbRead);
  if (nread == -1)
  {
    LOG("read() error, errno=%d", errno);
    switch (errno)
    {
      // @todo: more accurate error handling
      default:
        return 232; //ERROR_NO_DATA
    }
  }
  LOG("DosRead returned %u bytes", nread);
  *pcbRead = nread;

  //io_printf("hFile=%d, pBuffer=%x, cbRead=%u, cbActual=%u\n", hFile, pBuffer, cbRead, *pcbActual);

  return 0; // NO_ERROR
}

APIRET
os2server_dos_Write_component(CORBA_Object _dice_corba_obj,
                              HFILE hFile, PVOID pBuffer,
                              ULONG *pcbWrite,
	 		      CORBA_Server_Environment *_dice_corba_env)
{
  char *s;
  int  nwritten;

  nwritten = write(hFile, pBuffer, *pcbWrite);
  if (nwritten == -1)
  {
    LOG("write() error, errno=%d", errno);
    switch (errno)
    {
      // @todo: more accurate error handling
      default:
        return 232; //ERROR_NO_DATA
    }
  }

  *pcbWrite = nwritten;
  return 0/*NO_ERROR*/;
}

APIRET
os2server_dos_Exit_component(CORBA_Object _dice_corba_obj,
                             ULONG action, ULONG result,
                             CORBA_Server_Environment *_dice_corba_env)
{
  //struct t_os2process *proc;
  //l4semaphore_t sem;

  //if (action) // EXIT_PROCESS
  //{
  //    proc = PrcGetProc(l4thread_id(*_dice_corba_obj));
  //    sem = proc->term_sem;
  //    proc->term_code = result;
    l4semaphore_up(&sem);
  //}
  //else        // EXIT_THREAD
  //    l4thread_shutdown(l4thread_id(*_dice_corba_obj));
}

APIRET
os2server_dos_QueryCurrentDisk_component(CORBA_Object _dice_corba_obj,
                                         PULONG pdisknum, PULONG plogical,
                                         CORBA_Server_Environment *_dice_corba_env)
{
}

APIRET
os2server_dos_QueryCurrentDir_component(CORBA_Object _dice_corba_obj,
                                        ULONG disknum, PBYTE pBuf, PULONG pcbBuf,
                                        CORBA_Server_Environment *_dice_corba_env)
{
}
