/*
 *
 *
 */

#include <const.h>                // From the "Developer Connection Device Driver Kit" version 2.0

int kprintf(const char *format, ...);

#if 1
int far pascal _loadds FS_ALLOCATEPAGESPACE(
                                    struct sffsi far *psffsi,       /* ptr to fs independent SFT */
                                    struct sffsd far *psffsd,       /* ptr to fs dependent SFT         */
                                    unsigned long        ulSize,       /* new size                         */
                                    unsigned long        ulWantContig  /* contiguous chunk size         */
                                   )
{
    kprintf("**** FS_ALLOCATEPAGESPACE\n");

    return ERROR_NOT_SUPPORTED;
}


int far pascal _loadds FS_DOPAGEIO(
                              struct sffsi         far *psffsi,
                              struct sffsd         far *psffsd,
                              struct PageCmdHeader far *pPageCmdList
                             )
{
    kprintf("**** FS_DOPAGEIO\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_OPENPAGEFILE(
                                  unsigned long far *pFlags,
                                  unsigned long far *pcMaxReq,
                                  char          far *pName,
                                  struct sffsi  far *psffsi,
                                  struct sffsd  far *psffsd,
                                  unsigned short        usOpenMode,
                                  unsigned short        usOpenFlag,
                                  unsigned short        usAttr,
                                  unsigned long         Reserved
                                 )
{
    kprintf("**** FS_OPENPAGEFILE\n");

    return ERROR_NOT_SUPPORTED;
}
#endif

int far pascal _loadds FS_SETSWAP(
                             struct sffsi far *psffsi,
                             struct sffsd far *psffsd
                            )
{
    kprintf("**** FS_SETSWAP\n");

    return ERROR_NOT_SUPPORTED;
}

#if 1
int far pascal _loadds FS_VERIFYUNCNAME(
                                   unsigned short     flag,
                                   char           far *pName
                                  )
{
    kprintf("**** FS_VERIFYUNCNAME\n");

    return ERROR_NOT_SUPPORTED;
}
#endif

int far pascal _loadds FS_CANCELLOCKREQUEST(
                                   struct sffsi    far *psffsi,        /* psffsi        */
                                   struct sffsd    far *psffsd,        /* psffsd        */
                                   struct filelock far *pLockRange        /* pLockRang        */
                                  )
{
    kprintf("**** FS_CANCELLOCKREQUEST\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_FILELOCKS(
                               struct sffsi    far *psffsi,
                               struct sffsd    far *psffsd,
                               struct filelock far *pUnLockRange,
                               struct filelock far *pLockRange,
                               unsigned long           timeout,
                               unsigned long           flags
                              )
{
    kprintf("**** FS_FILELOCKS\n");

    return ERROR_NOT_SUPPORTED;
}


int far pascal _loadds FS_FILEIO(
                            struct sffsi   far *psffsi,
                            struct sffsd   far *psffsd,
                            char           far *cbCmdList,
                            unsigned short         pCmdLen,
                            unsigned short far *poError,
                            unsigned short         IOflag
                           )
{
    kprintf("**** FS_FILEIO\n");

    return ERROR_NOT_SUPPORTED;
}


int far pascal _loadds FS_NMPIPE(
                            struct sffsi   far *psffsi,
                            struct sffsd   far *psffsd,
                            unsigned short         OpType,
                            union npoper   far *pOpRec,
                            char           far *pData,
                            char           far *pName
                           )
{
    kprintf("**** FS_NMPIPE\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_FINDNOTIFYCLOSE(
                                     unsigned short handle
                                    )
{
    kprintf("**** FS_FINDNOTIFYCLOSE\n");

    return ERROR_NOT_SUPPORTED;
}


int far pascal _loadds FS_FINDNOTIFYFIRST(
                                     struct cdfsi   far *pcdfsi,
                                     struct cdfsd   far *pcdfsd,
                                     char           far *pName,
                                     unsigned short         iCurDirEnd,
                                     unsigned short         attr,
                                     unsigned short far *pHandle,
                                     char           far *pData,
                                     unsigned short         cbData,
                                     unsigned short far *pcMatch,
                                     unsigned short         level,
                                     unsigned long          timeout
                                    )
{
    kprintf("**** FS_FINDNOTIFYFIRST\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_FINDNOTIFYNEXT(
                                    unsigned short         handle,
                                    char           far *pData,
                                    unsigned short         cbData,
                                    unsigned short far *pcMatch,
                                    unsigned short         infolevel,
                                    unsigned long          timeout
                                   )
{
    kprintf("**** FS_FINDNOTIFYNEXT\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_COMMIT(
                            unsigned short         type,
                            unsigned short         IOflag,
                            struct sffsi   far *psffsi,
                            struct sffsd   far *psffsd
)
{
    kprintf("**** FS_COMMIT\n");

    return NO_ERROR;
}

int far pascal _loadds FS_COPY(
                          unsigned short         flag ,
                          struct cdfsi   far *pcdfsi,
                          struct cdfsd   far *pcdfsd,
                          char           far *pSrc,
                          unsigned short         iSrcCurrDirEnd,
                          char           far *pDst,
                          unsigned short         iDstCurrDirEnd,
                          unsigned short         nameType
)
{
    kprintf("**** FS_COPY\n");

    return ERROR_NOT_SUPPORTED;
}



int far pascal _loadds FS_DELETE(
                            struct cdfsi   far *pcdfsi,
                            struct cdfsd   far *pcdfsd,
                            char           far *pFile,
                            unsigned short         iCurDirEnd
                           )
{
    kprintf("**** FS_DELETE\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_FLUSHBUF(
                              unsigned short hVPB,
                              unsigned short flag
                             )
{
    kprintf("**** FS_FLUSHBUF\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_MOVE(
                          struct cdfsi   far *pcdfsi,
                          struct cdfsd   far *pcdfsd,
                          char           far *pSrc,
                          unsigned short     iSrcCurDirEnd,
                          char           far *pDst,
                          unsigned short         iDstCurDirEnd,
                          unsigned short         flags
                         )
{
    kprintf("**** FS_MOVE\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_NEWSIZE(
                             struct sffsi   far *psffsi,
                             struct sffsd   far *psffsd,
                             unsigned long          len,
                             unsigned short         IOflag
                            )
{
    kprintf("**** FS_NEWSIZE\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_SHUTDOWN(
                              unsigned short usType,
                              unsigned long ulReserved
                             )
{
    kprintf("**** FS_SHUTDOWN\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_WRITE(
                           struct sffsi   far *psffsi,
                           struct sffsd   far *psffsd,
                           char           far *pData,
                           unsigned short far *pLen,
                           unsigned short         IOflag
                          )
{
    kprintf("**** FS_WRITE\n");

    return ERROR_NOT_SUPPORTED;
}


int far pascal _loadds FS_MKDIR(
                           struct cdfsi   far *pcdfsi,
                           struct cdfsd   far *pcdfsd,
                           char           far *pName,
                           unsigned short         iCurDirEnd,
                           char           far *pEABuf,
                           unsigned short         flags
                          )
{
    kprintf("**** FS_MKDIR\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_RMDIR(
                           struct cdfsi   far *pcdfsi,
                          struct cdfsd   far *pcdfsd,
                           char           far *pName,
                           unsigned short         iCurDirEnd
                          )
{
    kprintf("**** FS_RMDIR\n");

    return ERROR_NOT_SUPPORTED;
}

/***FS_FINDFIRST***/

/***FS_FINDCLOSE***/

int far pascal _loadds FS_FINDFROMNAME(
                                  struct fsfsi   far *pfsfsi,
                                  struct fsfsd   far *pfsfsd,
                                  char           far *pData,
                                  unsigned short         cbData,
                                  unsigned short far *pcMatch,
                                  unsigned short         level,
                                  unsigned long          position,
                                  char           far *pNameFrom,
                                  unsigned short         flags
                                 )
{
    kprintf("**** FS_FINDFROMNAME\n");

    return ERROR_NOT_SUPPORTED;
}

int     far pascal _loadds  FS_FINDNEXT(
                               struct fsfsi   far *pfsfsi,
                               struct fsfsd   far *pfsfsd,
                               char           far *pData,
                               unsigned short         cbData,
                               unsigned short far *pcMatch,
                               unsigned short         level,
                               unsigned short         flags
                              )
{
    kprintf("**** FS_FINDNEXT\n");

    return ERROR_NOT_SUPPORTED;
}



int far pascal _loadds FS_FILEINFO(
                              unsigned short         flag,
                              struct sffsi   far *psffsi,
                              struct sffsd   far *psffsd,
                              unsigned short         level,
                              char           far *pData,
                              unsigned short         cbData,
                              unsigned short         IOflag
                             )
{
    kprintf("**** FS_FILEINFO\n");

    return ERROR_NOT_SUPPORTED;
}




int far pascal _loadds FS_PATHINFO(
                              unsigned short         flag,
                              struct cdfsi   far *pcdfsi,
                              struct cdfsd   far *pcdfsd,
                              char           far *pName,
                              unsigned short         iCurDirEnd,
                              unsigned short         level,
                              char           far *pData,
                              unsigned short         cbData
                             )
{
    kprintf("**** FS_PATHINFO\n");

    return ERROR_NOT_SUPPORTED;
}


int far pascal _loadds FS_FSCTL(
                           union argdat   far *pArgdat,
                           unsigned short         iArgType,
                           unsigned short         func,
                           char           far *pParm,
                           unsigned short         lenParm,
                           unsigned short far *plenParmOut,
                           char           far *pData,
                           unsigned short         lenData,
                           unsigned short far *plenDataOut
                          )
{
    kprintf("**** FS_FSCTL\n");

    return ERROR_NOT_SUPPORTED;
}


int far pascal _loadds FS_FILEATTRIBUTE(
                                   unsigned short         flag,
                                   struct cdfsi   far *pcdfsi,
                                   struct cdfsd   far *pcdfsd,
                                   char           far *pName,
                                   unsigned short         iCurDirEnd,
                                   unsigned short far *pAttr
                                  )
{
    kprintf("**** FS_FILEATTRIBUTE\n");

    return ERROR_NOT_SUPPORTED;
}


int far pascal _loadds FS_FSINFO(
                            unsigned short         flag,
                            unsigned short         hVPB,
                            char           far *pData,
                            unsigned short         cbData,
                            unsigned short         level
                           )
{
    kprintf("**** FS_FSINFO\n");

    return ERROR_NOT_SUPPORTED;
}

int far pascal _loadds FS_CHDIR(
                       unsigned short         flag,
                       struct cdfsi   far *pcdfsi,
                       struct cdfsd   far *pcdfsd,
                       char                  *pDir,
                       unsigned short         iCurDirEnd
                      )
{
    kprintf("**** FS_CHDIR\n");

    return ERROR_NOT_SUPPORTED;
}
