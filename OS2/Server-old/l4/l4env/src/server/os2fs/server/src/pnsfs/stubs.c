/*
 *
 *
 */

FSRET FSENTRY pns_ALLOCATEPAGESPACE(struct sffsi *psffsi,
                                   struct sffsd *psffsd,
                                   ULONG ulsize,
                                   USHORT ulWantContig)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_CANCELLOCKREQUEST(struct sffsi *psffsi,
                                   struct sffsd *psffsd,
                                   struct filelock *pLockRange)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_COMMIT(USHORT type,
                        USHORT IOflag,
                        struct sffsi *psffsi,
                        struct sffsd *psffsd)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_COPY(USHORT flag,
                      struct cdfsi *pcdfsi,
                      struct cdfsd *pcdfsd,
                      char *pSrc,
                      USHORT iSrcCurDirEnd,
                      char *pDst,
                      USHORT iDstCurDirEnd,
                      USHORT nameType)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_DOPAGEIO(struct sffsi *psffsi,
                          struct sffsd *psffsd,
                          struct PageCmdHeader *pList)
{
  return ERROR_NOT_SUPPORTED;
}

void  FSENTRY pns_EXIT(USHORT uid,
                      USHORT pid,
                      USHORT pdb)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FILEATTRIBUTE(USHORT flag,
                               struct cdfsi *pcdfsi,
                               struct cdfsd *pcdfsd,
                               char *pName,
                               USHORT iCurDirEnd,
                               USHORT *pAttr)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FILEINFO(USHORT flag,
                          struct sffsi *psffsi,
                          struct sffsd *psffsd,
                          USHORT level,
                          char *pData,
                          USHORT cbData,
                          USHORT IOflag)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FILEIO (struct sffsi *psffsi,
                         struct sffsd *psffsd,
                         char *pCmdList,
                         USHORT cbCmdList,
                         USHORT *poError,
                         USHORT IOflag)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FILELOCKS(struct sffsi *psffsi,
                           struct sffsd *psffsd,
                           struct filelock *pUnLockRange,
                           struct filelock *pLockRange,
                           ULONG timeout,
                           ULONG flags)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FINDCLOSE(struct fsfsi *pfsfsi,
                           struct fsfsd *pfsfsd)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FINDFIRST(struct cdfsi *pcdfsi,
                           struct cdfsd *pcdfsd,
                           char *pName,
                           USHORT iCurDirEnd,
                           USHORT attr,
                           struct fsfsi *pfsfsi,
                           struct fsfsd *pfsfsd,
                           char *pData,
                           USHORT cbData,
                           USHORT *pcMatch,
                           USHORT level,
                           USHORT flags)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FINDFROMNAME(struct fsfsi *pfsfsi,
                              struct fsfsd *pfsfsd,
                              char *pData,
                              USHORT cbData,
                              USHORT *pcMatch,
                              USHORT level,
                              ULONG position,
                              char *pName,
                              USHORT flags)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FINDNEXT(struct fsfsi *pfsfsi,
                          struct fsfsd *pfsfsd,
                          char *pData,
                          USHORT cbData,
                          USHORT *pcMatch,
                          USHORT level,
                          USHORT flags)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FINDNOTIFYCLOSE(USHORT handle)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FINDNOTIFYFIRST(struct cdfsi *pcdfsi,
                                 struct cdfsd *pcdfsd,
                                 char *pName,
                                 USHORT iCurDirEnd,
                                 USHORT attr,
                                 USHORT *pHandle,
                                 char *pData,
                                 USHORT cbData,
                                 USHORT *pMatch,
                                 USHORT level,
                                 ULONG timeout)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FINDNOTIFYNEXT(USHORT handle,
                                char *pData,
                                USHORT cbData,
                                USHORT *pcMatch,
                                USHORT level,
                                ULONG timeout)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FLUSHBUF(USHORT hVPB,
                          USHORT flag)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FSCTL(union argdat *pArgDat,
                       USHORT iArgType,
                       USHORT func,
                       char *pParm,
                       USHORT lenParm,
                       USHORT *plenParmIO,
                       char *pData,
                       USHORT lenData,
                       USHORT *plenDataIO)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_FSINFO(USHORT flag,
                        USHORT hVPB,
                        char *pData,
                        USHORT cbData,
                        USHORT level)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_IOCTL(struct sffsi *psffsi,
                       struct sffsd *psffsd,
                       USHORT cat,
                       USHORT func,
                       char *pParm,
                       USHORT lenMaxParm,
                       USHORT *plenInOutParm,
                       char *pData,
                       USHORT lenMaxData,
                       USHORT *plenInOutData)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_MOVE(struct cdfsi *pcdfsi,
                      struct cdfsd *pcdfsd,
                      char *pSrc,
                      USHORT iSrcCurDirEnd,
                      char *pDst,
                      USHORT iDstCurDirEnd,
                      USHORT flags)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_NEWSIZE(struct sffsi *psffsi,
                         struct sffsd *psffsd,
                         ULONG len,
                         USHORT IOflag)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_NMPIPE(struct sffsi *psffsi,
                        struct sffsd *psffsd,
                        USHORT OpType,
                        union npoper *pOpRec,
                        char pData,
                        char *pName)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_OPENPAGEFILE(unsigned long *pFlag,
                              unsigned long *pcMaxReq,
                              char *pName,
                              struct sffsi *psffsi,
                              struct sffsd *psffsi,
                              USHORT usOpenMode,
                              USHORT usOpenFlag,
                              USHORT usAttr,
                              ULONG Reserved)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_PATHINFO(unsigned short flag,
                          struct cdfsi *pcdfsi,
                          struct cdfsd *pcdfsd,
                          char *pName,
                          USHORT iCurDirEnd,
                          USHORT level,
                          char *pData,
                          USHORT cbData)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_PROCESSNAME(char *pNameBuf)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_SETSWAP(struct sffsi *psffsi,
                         struct sffsd *psffsd)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_SHUTDOWN(USHORT type,
                          ULONG reserved)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY pns_VERIFYUNCNAME(USHORT flag,
                               char *pName)
{
  return ERROR_NOT_SUPPORTED;
}
