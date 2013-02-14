/*  (c) osFree project,
 *  by valerius, Jun 2010, 18
 */

#define  INCL_BSE
/* #define  INCL_KBD */

#include <os2.h>
#include <stdio.h>

/* Copy declaration so we don't need INCL_KBD */
#define  DO_KBD
#include <copied_decl.h>
/* Watcoms declaration:
 USHORT APIENTRY16 KbdStringIn(PCH,PSTRINGINBUF,USHORT,HKBD); */





USHORT APIENTRY
KbdStringIn (PCH pch,
             PSTRINGINBUF pchin,
             USHORT flag,
             HKBD hkbd)
{
  ULONG  nread;

  if (hkbd)
    return 439; //ERROR_KBD_INVALID_HANDLE;

  if (flag)
    return 373; // ERROR_KBD_PARAMETER;

  if (!pch || !pchin || !pchin->cb)
    return 373; //ERROR_KBD_PARAMETER

  if (DosRead (0, pch, pchin->cb, &nread) == 0 /* NO_ERROR */)
  {
    if (nread)
    {
      nread--; // skip '\n'
      pch[nread] = '\0';
      pchin->cchIn = nread;
    }
    else
      return 232; //ERROR_NO_DATA;
  }
  else
    return 232; //ERROR_NO_DATA

  return 0; //NO_ERROR;
}
