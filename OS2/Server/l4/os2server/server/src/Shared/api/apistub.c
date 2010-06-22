/*
 *  Some minimal API
 *  functions stubs.
 */

#include <gcc_os2def.h>
#include "../apistub.h"
#include <dice/dice.h>
#include <l4/os2server/os2server-client.h>
#include <l4/log/l4log.h>

extern l4_threadid_t os2srv;

APIRET CDECL
api_DosRead (HFILE hFile, PVOID pBuffer,
            ULONG cbRead, PULONG pcbActual)
{
  CORBA_Environment env = dice_default_environment;
  char    *buf;
  APIRET  rc;

  rc = os2server_dos_Read_call(&os2srv, hFile, &buf, &cbRead, &env);
  memmove(pBuffer, buf, cbRead);
  *pcbActual = cbRead;
  return rc;
}


APIRET CDECL
api_DosWrite (HFILE hFile, PVOID pBuffer,
              ULONG cbWrite, PULONG pcbActual)
{
  CORBA_Environment env = dice_default_environment;
  APIRET  rc;

  rc = os2server_dos_Write_call(&os2srv, hFile, pBuffer, &cbWrite, &env);
  *pcbActual = cbWrite;
  return rc;
}


VOID CDECL
api_DosExit(ULONG action, ULONG result)
{
  CORBA_Environment env = dice_default_environment;

  os2server_dos_Exit_send(&os2srv, action, result, &env);
}


APIRET CDECL
api_DosQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical)
{
  CORBA_Environment env = dice_default_environment;

  return os2server_dos_QueryCurrentDisk_call(&os2srv, pdisknum, plogical, &env);
}


APIRET CDECL
api_DosQueryCurrentDir(ULONG disknum,
                       PBYTE pBuf,
                       PULONG pcbBuf)
{
  CORBA_Environment env = dice_default_environment;

  return os2server_dos_QueryCurrentDir_call(&os2srv, disknum, pBuf, pcbBuf, &env);
}
