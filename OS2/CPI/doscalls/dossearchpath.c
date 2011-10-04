/*
    (c) osFree project
    author: Valery V. Sedletski

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define  INCL_DOSMISC
#define  INCL_DOSERRORS
#include <os2.h>

#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <stdio.h>

#include "token.h"
#include "strnlen.h"
#include "strlcpy.h"

void log(const char *fmt, ...);

/*!
  @brief     Searches for a file on a path. Path is explicitly specified,
             or by path name

  @param  flag   includes:
                   SEARCH_IGNORENETERRS: Ignore network errors, while searching
                   SEARCH_ENVIRONMENT:   Search for the env. var., in pszPathOrName, in ENVIRONMENT
                   SEARCH_CUR_DIRECTORY: search in a current directory first
  @param  pszPathOrName   path name (if SEARCH_ENVIRONMENT is specified), or the path itself, othrewise
  @param  pszFilename     filename to search for
  @param  pBuf            user buffer for a resulting filename
  @param  cbBuf           user buffer size

  @return
    NO_ERROR                        if successful
    ERROR_INVALID_PARAMETER         input parameters are incorrect
    ERROR_FILE_NOT_FOUND            file not found on the path
    ERROR_BUFFER_OVERFLOW           input buffer is too small
    ERROR_ENVVAR_NOT_FOUND          env. variable not found
    ERROR_INVAID_FUNCTION           incorrect flag value

  API:
    DosAllocMem
    DosFreeMem
    DosOpen
    DosClose
    DosScanEnv

  note: StrTok* functions are not multithread-aware, probably
*/

APIRET APIENTRY  DosSearchPath(ULONG flag,
                               PCSZ  pszPathOrName,
                               PCSZ  pszFilename,
                               PBYTE pBuf,
                               ULONG cbBuf)
{
  char   *psep = ";";
  char   *sep  = "\\";
  STR_SAVED_TOKENS st;
  int    pathlen;
  char   *pathtmp;
  char   *path;
  char   *p;
  ULONG  ulAction;
  HFILE  hf;
  APIRET rc;

  log("flag=%08x\n", flag);
  log("pszPathOrName=%s\n", pszPathOrName);
  log("pszFilename=%s\n", pszFilename);
  log("pBuf=%08x\n", pBuf);
  log("cbBuf=%lu\n", cbBuf);

  if (!pszPathOrName || !*pszPathOrName ||
      !pszFilename   || !*pszFilename   ||
      !pBuf || !cbBuf)
    return ERROR_INVALID_PARAMETER;

  // if incorrect flag is specified
  if (flag & ~(SEARCH_ENVIRONMENT | SEARCH_CUR_DIRECTORY | SEARCH_IGNORENETERRS))
    return ERROR_INVALID_FUNCTION;

  // need to find 'path' value first
  // search for path on the environment
  if (flag & SEARCH_ENVIRONMENT)
  {
    rc = DosScanEnv(pszPathOrName, &pathtmp);

    pathlen = strlen(pathtmp) + 1;
  }
  else
  {
    pathtmp = (char *)pszPathOrName;
    pathlen = strlen(pszPathOrName) + 1;
  }

  if (flag & SEARCH_CUR_DIRECTORY)
    pathlen += 2;

  // create an area with read-write access
  rc = DosAllocMem((void **)&path, pathlen,
                   PAG_READ | PAG_WRITE | PAG_COMMIT);

  p = path;

  if (flag & SEARCH_CUR_DIRECTORY)
  {
    // add current dir in front of path
    p[0] = '.';
    p[1] = *psep;
    p += 2;
  }

  strcpy(p, pathtmp);
  StrTokSave(&st);

  if (p = StrTokenize(path, psep))
    do if (*p)
    {
      pBuf[0] = '\0';

      strlcat(pBuf, p, cbBuf);

      if (p[strnlen(p, cbBuf) - 1] != *sep)
        strlcat(pBuf, sep, cbBuf);

      strlcat(pBuf, pszFilename, cbBuf);

      if (strnlen(pBuf, cbBuf) == cbBuf)
      {
        StrTokStop();
        DosFreeMem(path);
        return ERROR_BUFFER_OVERFLOW;
      }

      // try to DosOpen it
      if (rc = DosOpen(pBuf,
                       &hf,
                       &ulAction,
                       0,
                       0,
                       OPEN_ACTION_FAIL_IF_NEW |
                       OPEN_ACTION_OPEN_IF_EXISTS,
                       OPEN_SHARE_DENYNONE |
                       OPEN_ACCESS_READONLY,
                       NULL))
        continue;

      // file found, return
      DosClose(hf);
      StrTokStop();
      DosFreeMem(path);
      log("pBuf=%s\n", pBuf);

      return NO_ERROR;
    } while (p = StrTokenize(0, psep));

  StrTokRestore(&st);
  DosFreeMem(path);
  *pBuf = '\0';


  return ERROR_FILE_NOT_FOUND;
}
