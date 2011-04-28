/*  file/directory API implementations
 *  (on the server side)
 */
 
#include <l4/os3/gcc_os2def.h>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <l4/log/l4log.h>

#include <dice/dice.h>

extern l4_threadid_t os2srv;

APIRET
os2fs_dos_Read_component(CORBA_Object _dice_corba_obj,
                             HFILE hFile, void **pBuffer,
                             ULONG *pcbRead,
                             short *dice_reply,
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
  *pcbRead = nread;


  return 0; // NO_ERROR
}

APIRET
os2fs_dos_Write_component(CORBA_Object _dice_corba_obj,
                              HFILE hFile, PVOID pBuffer,
                              ULONG *pcbWrite,
                              short *dice_reply,
	 		      CORBA_Server_Environment *_dice_corba_env)
{
  char *s;
  int  nwritten;

  LOG("entered");
  nwritten = write(hFile, pBuffer, *pcbWrite);
  LOG("in the middle");
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

  LOG("nwritten=%u", nwritten);
  *pcbWrite = nwritten;
  LOG("exited");
  return 0/*NO_ERROR*/;
}


APIRET DICE_CV
os2fs_dos_ResetBuffer_component (CORBA_Object _dice_corba_obj,
                                     HFILE handle /* in */,
                                     CORBA_Server_Environment *_dice_corba_env)
{
  return 0; /* NO_ERROR */
}

#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2

APIRET DICE_CV
os2fs_dos_SetFilePtr_component (CORBA_Object _dice_corba_obj,
                                    HFILE handle /* in */,
                                    long ib /* in */,
                                    ULONG method /* in */,
                                    ULONG *ibActual /* out */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  long pos, ret;
  unsigned long len;
  struct stat stat;
  
  ret = fstat(handle, &stat);

  if (ret == -1)
    return 6; /* ERROR_INVALID_HANDLE */

  /* get length */
  len = stat.st_size;
  /* get current position */
  pos = lseek(handle, 0L, SEEK_CUR);

  switch (method)
  {
  case FILE_BEGIN:
    pos = ib;
    break;
  case FILE_CURRENT:
    pos += ib;
    break;
  case FILE_END:
    pos = len + ib;
    break;
  default:
    pos = ib;
  }

  if (pos < 0)
    return 131; /* ERROR_NEGATIVE_SEEK */

  *ibActual = pos;
  ret = lseek(handle, pos, SEEK_SET);

  if (ret == -1)
    return 132; /* ERROR_SEEK_ON_DEVICE */

  return 0; /* NO_ERROR */
}

APIRET DICE_CV
os2fs_dos_Close_component (CORBA_Object _dice_corba_obj,
                               HFILE handle /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  int ret;
  
  ret = close(handle);
  
  if (ret == - 1)
  {
    if (errno == EBADF)
      return 6; /* ERROR_INVALID_HANDLE */
      
    return 5; /* ERROR_ACCESS_DENIED */
  }
  
  return 0; /* NO_ERROR */
}


APIRET DICE_CV
os2fs_dos_QueryHType_component (CORBA_Object _dice_corba_obj,
                                    HFILE handle /* in */,
                                    ULONG *pType /* out */,
                                    ULONG *pAttr /* out */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  int ret;
  unsigned short m;
  struct stat stat;
  
  ret = fstat(handle, &stat);

  if (ret == -1)
    return 6; /* ERROR_INVALID_HANDLE */

  m = stat.st_mode;

  if (S_ISREG(m))
  {
    *pType = 0; // disk file
    *pAttr = 0;
    return 0;
  }

  if (S_ISCHR(m))
  {
    *pType = 1; // character device
    *pAttr = 0;
    return 0;
  }

  if (S_ISFIFO(m))
  {
    *pType = 2; // pipe
    *pAttr = 0;
    return 0;
  }
  
  return 0; /* NO_ERROR */
}
