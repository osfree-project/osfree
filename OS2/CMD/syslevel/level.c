/***************************************************************************\
* level.c -- a syslevel files API                        940422
*
* Copyright (c) 1994 Martin Lafaix. All Rights Reserved.
\***************************************************************************/

#define INCL_DOSFILEMGR
#define INCL_NOPMAPI                               /* PM not needed        */
#include <os2.h>

#include "level.h"                                 /* API public header    */
#include "level.ph"                                /* API private header   */

/***************************************************************************/
/* functions body                                                          */
/***************************************************************************/
APIRET LvlOpenLevelFile(PSZ pszName, PHFILE phFile, ULONG ulOpenMode, PSZ pszCID)
{
  APIRET rc = 0;
  ULONG ulAction;
  CHAR achFileName[CCHMAXPATHCOMP];
 
  switch(ulOpenMode)
    {
    case OLF_OPEN:
    case OLF_OPEN | OLF_CHECKID:
      if(pszName || pszCID)
        {
        rc = _findsyslevelfile(pszName, pszCID, achFileName);

        if(rc == 18)
          rc = _searchsubdirs(pszName, pszCID, achFileName);

        if(rc == 0)
          rc = DosOpen(achFileName,
                       phFile,
                       &ulAction,
                       0,
                       FILE_NORMAL,
                       FILE_OPEN,
                       OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                       0);

        if(rc == 18)
          rc = 2;
        }
      else
        rc = ERROR_INVALID_PARAMETER;
      break;
    case OLF_SCANDISKS:
    case OLF_SCANDISKS | OLF_CHECKID:
    case OLF_SCANDISKS | OLF_OPEN:
    case OLF_SCANDISKS | OLF_OPEN | OLF_CHECKID:
      if(pszName || pszCID)
        {
        ULONG ulDrive, ulDriveMap, ulCurrent, ulFound = 0;
 
        rc = DosQueryCurrentDisk(&ulDrive, &ulDriveMap);
        ulCurrent = _firstdrive(ulDriveMap);

        while(ulCurrent && !ulFound)
          {
          rc = _findsyslevelfile(pszName, pszCID, achFileName);
          if(rc == 18)
            rc = _searchsubdirs(pszName, pszCID, achFileName);
 
          if(rc == 0)
            {
            ulFound = 1;

            rc = DosOpen(achFileName,
                         phFile,
                         &ulAction,
                         0,
                         FILE_NORMAL,
                         FILE_OPEN,
                         OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                         0);
            }
          else
            ulCurrent = _nextdrive(ulDriveMap, ulCurrent);
          }

        DosSetDefaultDisk(ulDrive);
        if(ulFound == 0)
          rc = 2;
        }
      else
        rc = ERROR_INVALID_PARAMETER;
      break;
    case OLF_CREATE:
      if(pszName && pszCID)
        {
        SYSLEVELHEADER slh;
        SYSLEVELDATA sld;

        strcat(strcpy(achFileName, "syslevel."), pszName);
      
        rc = DosOpen(achFileName,
                     phFile,
                     &ulAction,
                     0,
                     FILE_NORMAL,
                     FILE_CREATE,
                     OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                     0);

        slh.h_magic[0] = slh.h_magic[1] = 0xFF;
        strcpy(slh.h_name, "SYSLEVEL");
        for(ulAction = 0; ulAction < sizeof(slh.h_reserved1); ulAction++)
          slh.h_reserved1[ulAction] = 0;
        slh.h_updated = 0;
        for(ulAction = 0; ulAction < sizeof(slh.h_reserved2); ulAction++)
          slh.h_reserved2[ulAction] = 0;
        slh.h_data = sizeof(SYSLEVELHEADER);

        if(rc == 0)
          rc = _writesyslevelheader(*phFile, &slh);
 
        memset(&sld, 0, sizeof(sld));
        sld.d_kind[0] = SLK_STANDARD;
        sld.d_reserved3 = sld.d_reserved4 = 0x5F;
        memcpy(sld.d_cid, pszCID, sizeof(sld.d_cid));

        if(rc == 0)
          rc = DosWrite(*phFile, &sld, sizeof(SYSLEVELDATA), &ulAction);
        }
      else
        rc = ERROR_INVALID_PARAMETER;
      break;
    default: 
      rc = ERROR_INVALID_PARAMETER;
      break;
    }

  return rc;
}

APIRET LvlQueryLevelFile(PSZ pszName, PSZ pszCID, PVOID pBuffer, ULONG ulBufSize)
{
  ULONG ulDrive, ulDriveMap, ulCurrent;
  CHAR achFileName[CCHMAXPATHCOMP];
  APIRET rc = DosQueryCurrentDisk(&ulDrive, &ulDriveMap);
 
  strcat(strcpy(achFileName, "syslevel."),
         (pszName == NULL) ? (PSZ)"???" : pszName);
  memset(pBuffer, 0, ulBufSize);
 
  ulCurrent = _firstdrive(ulDriveMap);

  while(ulCurrent && (rc == 0))
    {
    rc = _findallsyslevelfiles(achFileName, pszCID, &pBuffer, &ulBufSize);

    if(rc == 0)
      ulCurrent = _nextdrive(ulDriveMap, ulCurrent);
    }

  if(rc == 0)
    rc = DosSetDefaultDisk(ulDrive);
 
  return rc;
}

APIRET LvlQueryLevelFileData(HFILE hFile, ULONG ulWhat,
                             PVOID pBuffer, ULONG ulBufSize, PULONG pulSize)
{
  APIRET rc = _locatesysleveldata(hFile);
  PSYSLEVELDATA psld = _allocsysleveldata(hFile);

  if(rc == 0)
    rc = pBuffer ? rc : ERROR_INVALID_PARAMETER;
  if(rc == 0)
    rc = _readsysleveldata(hFile, psld);

  if(rc == 0)
    switch(ulWhat)
      {
      case QLD_MAJORVERSION:
        if(ulBufSize >= 1)
          _achBuffer[0] = '0' + (psld->d_version[0] >> 4);
        else
          rc = ERROR_INSUFFICIENT_BUFFER;
        _setsize(1);
        break;
      case QLD_MINORVERSION:
        if(ulBufSize >= 2)
          {
          _achBuffer[0] = '0' + (psld->d_version[0] & 0x0F);
          _achBuffer[1] = '0' + psld->d_version[1];
          }
        else
          rc = ERROR_INSUFFICIENT_BUFFER;
        _setsize(2);
        break;
      case QLD_REVISION:
        if(ulBufSize >= 1)
          _achBuffer[0] = '0' + psld->d_revision[0];
        else
          rc = ERROR_INSUFFICIENT_BUFFER;
        _setsize(1);
        break;
      case QLD_KIND:
        _getfield(d_kind);
        break;
      case QLD_CURRENTCSD:
        _getfield(d_clevel);
        break;
      case QLD_PREVIOUSCSD:
        _getfield(d_plevel);
        break;
      case QLD_TITLE:
        _getfield(d_title);
        break;
      case QLD_ID:
        _getfield(d_cid);
        break;
      case QLD_TYPE:
        if(ulBufSize >= strlen(psld->d_type))
          memcpy(pBuffer, psld->d_type, strlen(psld->d_type));
        else
          rc = ERROR_INSUFFICIENT_BUFFER;
        _setsize(strlen(psld->d_type));
        break;
      default: 
        rc = ERROR_INVALID_PARAMETER;
        break;
      }

  _freesysleveldata(psld);
 
  return rc;
}

APIRET LvlWriteLevelFileData(HFILE hFile, ULONG ulWhat,
                             PVOID pBuffer, ULONG ulBufSize, PULONG pulSize)
{
  APIRET rc = _locatesysleveldata(hFile);
  PSYSLEVELDATA psld = _allocsysleveldata(hFile);

  if(rc == 0)
    rc = pBuffer ? rc : ERROR_INVALID_PARAMETER;
  if(rc == 0)
    rc = _readsysleveldata(hFile, psld);

  if(rc == 0)
    switch(ulWhat)
      {
      case QLD_MAJORVERSION:
        if(ulBufSize >= 1)
          {
          psld->d_version[0] &= 0x0F;
          psld->d_version[0] |= (_achBuffer[0] - '0') << 4;
          }
        else
          rc = ERROR_INSUFFICIENT_BUFFER;
        _setsize(1);
        break;
      case QLD_MINORVERSION:
        if(ulBufSize >= 2)
          {
          psld->d_version[0] &= 0xF0;
          psld->d_version[0] |= (_achBuffer[0] - '0') && 0x0F;
          psld->d_version[1] = _achBuffer[1] - '0';
          }
        else
          rc = ERROR_INSUFFICIENT_BUFFER;
        _setsize(2);
        break;
      case QLD_REVISION:
        _setfield(d_revision);
        break;
      case QLD_KIND:
        _setfield(d_kind);
        break;
      case QLD_CURRENTCSD:
        memcpy(psld->d_plevel, psld->d_clevel, sizeof(psld->d_plevel));
        _setfield(d_clevel);
        break;
      case QLD_TITLE:
        memcpy(psld->d_title, pBuffer, min(ulBufSize, sizeof(psld->d_title)));
        _setsize(min(ulBufSize, sizeof(psld->d_title)));
        break;
      case QLD_ID:
        _setfield(d_cid);
        break;
      case QLD_TYPE:
        break;
      default: 
        rc = ERROR_INVALID_PARAMETER;
        break;
      }

  if(rc == 0)
    rc = _locatesysleveldata(hFile);
  if(rc == 0)
    rc = _writesysleveldata(hFile, psld);
 
  _freesysleveldata(psld);
 
  return rc;
}

/***************************************************************************/
/* local functions body                                                    */
/***************************************************************************/
static APIRET _readsysleveldata(HFILE hFile, PSYSLEVELDATA psld)
{
  ULONG ulSize;
 
  return DosRead(hFile, psld, sizeof(SYSLEVELDATA), &ulSize);
}

static APIRET _writesysleveldata(HFILE hFile, PSYSLEVELDATA psld)
{
  ULONG ulSize;
 
  return DosWrite(hFile, psld, sizeof(SYSLEVELDATA), &ulSize);
}

static APIRET _locatesysleveldata(HFILE hFile)
{
  ULONG ulPos, ulData;
  APIRET rc = DosSetFilePtr(hFile,
                            FIELDOFFSET(SYSLEVELHEADER, h_data),
                            FILE_BEGIN,
                            &ulPos);
 
  rc = rc ? rc : DosRead(hFile, &ulData, sizeof(ulData), &ulPos);
 
  return rc ? rc : DosSetFilePtr(hFile, ulData, FILE_BEGIN, &ulPos);
}

static PSYSLEVELDATA _allocsysleveldata(HFILE hFile)
{
  return (PSYSLEVELDATA) malloc(sizeof(SYSLEVELDATA));
}

static VOID _freesysleveldata(PSYSLEVELDATA psld)
{
  if(psld)
    free(psld);
}

static APIRET _writesyslevelheader(HFILE hFile, PSYSLEVELHEADER pslh)
{
  ULONG ulSize;
 
  return DosWrite(hFile, pslh, sizeof(SYSLEVELHEADER), &ulSize);
}

static BOOL _isasyslevelfile(PSZ pszFile, PSZ pszCID)
{
  HFILE hFile;
  ULONG ulAction;
  SYSLEVELHEADER slh;
  APIRET rc = DosOpen(pszFile,
                      &hFile,
                      &ulAction,
                      0,
                      FILE_NORMAL,
                      FILE_OPEN,
                      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                      0);

  if(rc == 0)
    rc = DosRead(hFile, &slh, sizeof(slh), &ulAction);

  ulAction = (slh.h_magic[0] == 0xFF) && (slh.h_magic[1] == 0xFF);
  if(ulAction)
    ulAction = strcmp(slh.h_name, "SYSLEVEL") == 0;
  if(ulAction)
    if(pszCID)
      {
      CHAR achCID[9];
      ULONG ulCID;
 
      rc = LvlQueryLevelFileData(hFile, QLD_ID, achCID, 9, &ulCID);
      ulAction = strncmp(achCID, pszCID) == 0;
      }

  rc = DosClose(hFile);

  return ulAction;
}

static APIRET _getfullname(PVOID pBuffer, PSZ pszFileName)
{
  ULONG ulDrive, ulDriveMap, ulBufSize = CCHMAXPATHCOMP;
  APIRET rc = DosQueryCurrentDisk(&ulDrive, &ulDriveMap);
 
  _achBuffer[0] = '@'+ulDrive; _achBuffer[1] = ':'; _achBuffer[2] = '\\';
  rc = DosQueryCurrentDir(0, _achBuffer+3, &ulBufSize);
  strcat(strcat(pBuffer, "\\"), pszFileName);

  return rc;
}

static APIRET _findsyslevelfile(PSZ pszName, PSZ pszCID, PVOID pBuffer)
{
  HDIR hDirHandle = HDIR_CREATE;
  FILEFINDBUF3 ffb;
  ULONG ulCount = 1, ulFound = 0;
  CHAR achFileName[CCHMAXPATHCOMP] = "syslevel."; 
  APIRET rc = 0;

  if(pszName)
    strcat(achFileName, pszName);
  else
    strcat(achFileName, "???");

  rc = DosFindFirst(achFileName,
                    &hDirHandle,
                    FILE_NORMAL,
                    (PVOID)&ffb,
                    sizeof(ffb),
                    &ulCount,
                    FIL_STANDARD);

  while((!ulFound) && (rc == 0))
    {
    if(_isasyslevelfile(ffb.achName, pszCID))
      {
      _getfullname(pBuffer, ffb.achName);
      ulFound = 1; break;
      }

    rc = DosFindNext(hDirHandle, (PVOID)&ffb, sizeof(ffb), &ulCount);
    }

  if((rc == 0) || (rc == 18))
    rc = DosFindClose(hDirHandle);

  return ulFound ? 0 : 18;
}

static APIRET _findallsyslevelfiles(PSZ achFileName, PSZ pszCID,
                                    PPVOID ppBuffer, PULONG pulBufSize)
{
  HDIR hDirHandle = HDIR_CREATE;
  FILEFINDBUF3 ffb;
  ULONG ulCount = 1, ulSize;
  APIRET rc = DosFindFirst(achFileName,
                           &hDirHandle,
                           FILE_NORMAL,
                           (PVOID)&ffb,
                           sizeof(ffb),
                           &ulCount,
                           FIL_STANDARD);

  while(rc == 0)
    {
    if(_isasyslevelfile(ffb.achName, pszCID))
      {
      CHAR achBuf[CCHMAXPATHCOMP];
 
      _getfullname(achBuf, ffb.achName);
      ulSize = strlen(achBuf);
      if((*pulBufSize) > ulSize)
        {
        *pulBufSize -= ulSize+1;
        memcpy(*ppBuffer, achBuf, ulSize);
        *ppBuffer = ((CHAR *)(*ppBuffer))+ulSize+1;
        }
      else
        rc = ERROR_INSUFFICIENT_BUFFER;
      }

    if(rc == 0)
      rc = DosFindNext(hDirHandle, (PVOID)&ffb, sizeof(ffb), &ulCount);
    }

  if((rc == 0) || (rc == 18))
    {
    rc = DosFindClose(hDirHandle);
    if(rc == 6)
      rc = 0;
    }

  hDirHandle = HDIR_CREATE;
  if(rc == 0)
    rc = DosFindFirst("*",
                      &hDirHandle,
                      MUST_HAVE_DIRECTORY,
                      (PVOID)&ffb,
                      sizeof(ffb),
                      &ulCount,
                      FIL_STANDARD);

  while(rc == 0)
    {
    if(ffb.achName[0] == '.' &&
       (ffb.achName[1] == 0 || (ffb.achName[1] == '.' && ffb.achName[2] == 0)))
      {
      }
    else
      {
      rc = DosSetCurrentDir(ffb.achName);

      if(rc == 0)
        rc = _findallsyslevelfiles(achFileName, pszCID, ppBuffer, pulBufSize);

      if((rc == 0) || (rc == 18))
        rc = DosSetCurrentDir("..");
      }
 
    if(rc == 0)
      rc = DosFindNext(hDirHandle, (PVOID)&ffb, sizeof(ffb), &ulCount);
    }

  if((rc == 0) || (rc == 18))
    {
    rc = DosFindClose(hDirHandle);
    if(rc == 6)
      rc = 0;
    }

  return rc;
}

static APIRET _searchsubdirs(PSZ pszName, PSZ pszCID, PVOID pBuffer)
{
  HDIR hDirHandle = HDIR_CREATE;
  FILEFINDBUF3 ffb;
  ULONG ulCount = 1, ulFound = 0;
  APIRET rc = DosFindFirst("*",
                           &hDirHandle,
                           MUST_HAVE_DIRECTORY,
                           (PVOID)&ffb,
                           sizeof(ffb),
                           &ulCount,
                           FIL_STANDARD);

  while((!ulFound) && (rc == 0))
    {
    if(ffb.achName[0] == '.' &&
       (ffb.achName[1] == 0 || (ffb.achName[1] == '.' && ffb.achName[2] == 0)))
      {
      }
    else
      {
      rc = DosSetCurrentDir(ffb.achName);

      if(rc == 0)
        rc = _findsyslevelfile(pszName, pszCID, pBuffer);

      if(rc == 18)
        rc = _searchsubdirs(pszName, pszCID, pBuffer);
      
      if(rc == 0)
        ulFound = 1;

      if((rc == 0) || (rc == 18))
        rc = DosSetCurrentDir("..");
      }
 
    if((!ulFound) && (rc == 0))
      rc = DosFindNext(hDirHandle, (PVOID)&ffb, sizeof(ffb), &ulCount);
    }

  rc = DosFindClose(hDirHandle);
 
  return ulFound ? 0 : 18;
}

static ULONG _firstdrive(ULONG ulDriveMap)
{
  return _nextdrive(ulDriveMap, 2);
}

static ULONG _nextdrive(ULONG ulDriveMap, ULONG ulCurrent)
{
  ULONG ulNext = ulCurrent+1;
 
  while(ulNext<27)
    if(ulDriveMap & (0x1 << (ulNext - 1)))
      break;
    else
      ulNext++;
 
  if(ulNext < 27)
    {
    DosSetDefaultDisk(ulNext);
    DosSetCurrentDir("\\");
    }
 
  return (ulNext == 27) ? 0 : ulNext;
}
