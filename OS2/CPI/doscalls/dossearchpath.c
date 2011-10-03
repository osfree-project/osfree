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
  STR_SAVED_TOKENS st;
  char   *psep = ";";
  char   *sep  = "\\";
  char   *path;
  char   *pathtmp, *pathval;
  char   *p;
  ULONG  ulAction;
  HFILE  hf;
  APIRET rc;

  if (!pszPathOrName || !*pszPathOrName ||
      !pszFilename   || !*pszFilename   ||
      !pBuf || !cbBuf)
    return ERROR_INVALID_PARAMETER;

  // if incorrect flag is specified
  if (flag & ~(SEARCH_ENVIRONMENT | SEARCH_CUR_DIRECTORY | SEARCH_IGNORENETERRS))
    return ERROR_INVALID_FUNCTION;

  // need to find 'path' value first
  if (flag & SEARCH_ENVIRONMENT)
  {
    // search for path on the environment
    rc = DosScanEnv(pszPathOrName, &pathtmp);

    if (flag & SEARCH_CUR_DIRECTORY)
    {
      rc = DosAllocMem((void **)&pathval, strlen(pathtmp) + 3,
                       PAG_READ | PAG_WRITE | PAG_COMMIT);
      pathval[0] = '.';
      pathval[1] = *psep;
      strcpy(pathval + 2, pathtmp);
      path = pathval;
    }
    else
      path = pathtmp;
  }
  else // path is specified immediately as pszPathOrName
    path = (char *)pszPathOrName;

  StrTokSave(&st);

  if (p = StrTokenize(path, psep))
    do if (*p)
    {
      pBuf[0] = '\0';

      strlcat(pBuf, p, cbBuf);

      if (p[strnlen(p, CCHMAXPATH) - 1] != *sep)
        strlcat(pBuf, sep, cbBuf);

      strlcat(pBuf, pszFilename, cbBuf);

      if (strnlen(pBuf, cbBuf) == cbBuf)
      {
        StrTokStop();
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

      if (flag & SEARCH_CUR_DIRECTORY)
        DosFreeMem(pathval);

      return NO_ERROR;
    } while ((p = StrTokenize(0, psep)) != 0);

  if (flag & SEARCH_CUR_DIRECTORY)
    DosFreeMem(pathval);

  StrTokRestore(&st);
  *pBuf = '\0';


  return ERROR_FILE_NOT_FOUND;
}
