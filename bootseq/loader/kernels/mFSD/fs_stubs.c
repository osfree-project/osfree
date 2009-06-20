/*
 *
 *
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>                // From the "Developer Connection Device Driver Kit" version 2.0

//#include <ifs.h>

#if 0
int far pascal FS_ALLOCATEPAGESPACE(
                                    struct sffsi far *psffsi,       /* ptr to fs independent SFT */
                                    struct sffsd far *psffsd,       /* ptr to fs dependent SFT         */
                                    unsigned long        ulSize,       /* new size                         */
                                    unsigned long        ulWantContig  /* contiguous chunk size         */
                                   )
{
    return ERROR_NOT_SUPPORTED;
}


int far pascal FS_DOPAGEIO(
                              struct sffsi         far *psffsi,
                              struct sffsd         far *psffsd,
                              struct PageCmdHeader far *pPageCmdList
                             )
{
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_OPENPAGEFILE(
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
    return ERROR_NOT_SUPPORTED;
}
#endif

int far pascal FS_SETSWAP(
                             struct sffsi far *psffsi,
                             struct sffsd far *psffsd
                            )
{
    return ERROR_NOT_SUPPORTED;
}

#if 0
int far pascal FS_VERIFYUNCNAME(
                                   unsigned short     flag,
                                   char           far *pName
                                  )
{
    return ERROR_NOT_SUPPORTED;
}
#endif

int far pascal FS_CANCELLOCKREQUEST(
                                   struct sffsi    far *psffsi,        /* psffsi        */
                                   struct sffsd    far *psffsd,        /* psffsd        */
                                   struct filelock far *pLockRange        /* pLockRang        */
                                  )
{
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_FILELOCKS(
                               struct sffsi    far *psffsi,
                               struct sffsd    far *psffsd,
                               struct filelock far *pUnLockRange,
                               struct filelock far *pLockRange,
                               unsigned long           timeout,
                               unsigned long           flags
                              )
{
    return ERROR_NOT_SUPPORTED;
}


int far pascal FS_FILEIO(
                            struct sffsi   far *psffsi,
                            struct sffsd   far *psffsd,
                            char           far *cbCmdList,
                            unsigned short         pCmdLen,
                            unsigned short far *poError,
                            unsigned short         IOflag
                           )
{
    return ERROR_NOT_SUPPORTED;
}


int far pascal FS_ATTACH(
                         unsigned short       flag,        /* flag                */
                         char           far *pDev,        /* pDev                */
                         struct vpfsd   far *pvpfsd, /* if remote drive
                                                           struct vpfsd far *
                                                           else if remote device
                                                                  null ptr (0L)    */
                         struct cdfsd   far *pcdfsd,        /* if remote drive
                                                           struct cdfsd far *
                                                          else
                                                          struct devfsd far * */
                         char           far *pParm,        /* pParm        */
                         unsigned short far *pLen        /* pLen                */
                        )
{
    return ERROR_NOT_SUPPORTED;
}


int far pascal FS_NMPIPE(
                            struct sffsi   far *psffsi,
                            struct sffsd   far *psffsd,
                            unsigned short         OpType,
                            union npoper   far *pOpRec,
                            char           far *pData,
                            char           far *pName
                           )
{
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_FINDNOTIFYCLOSE(
                                     unsigned short handle
                                    )
{
    return ERROR_NOT_SUPPORTED;
}


int far pascal FS_FINDNOTIFYFIRST(
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
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_FINDNOTIFYNEXT(
                                    unsigned short         handle,
                                    char           far *pData,
                                    unsigned short         cbData,
                                    unsigned short far *pcMatch,
                                    unsigned short         infolevel,
                                    unsigned long          timeout
                                   )
{
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_COMMIT(
                            unsigned short         type,
                            unsigned short         IOflag,
                            struct sffsi   far *psffsi,
                            struct sffsd   far *psffsd
)
{
    return NO_ERROR;
}

int far pascal FS_COPY(
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
    return ERROR_NOT_SUPPORTED;
}




int far pascal FS_DELETE(
                            struct cdfsi   far *pcdfsi,
                            struct cdfsd   far *pcdfsd,
                            char           far *pFile,
                            unsigned short         iCurDirEnd
                           )
{
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_FLUSHBUF(
                              unsigned short hVPB,
                              unsigned short flag
                             )
{
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_MOVE(
                          struct cdfsi   far *pcdfsi,
                          struct cdfsd   far *pcdfsd,
                          char           far *pSrc,
                          unsigned short     iSrcCurDirEnd,
                          char           far *pDst,
                          unsigned short         iDstCurDirEnd,
                          unsigned short         flags
                         )
{
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_NEWSIZE(
                             struct sffsi   far *psffsi,
                             struct sffsd   far *psffsd,
                             unsigned long          len,
                             unsigned short         IOflag
                            )
{
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_SHUTDOWN(
                              unsigned short usType,
                              unsigned long ulReserved
                             )
{
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_WRITE(
                           struct sffsi   far *psffsi,
                           struct sffsd   far *psffsd,
                           char           far *pData,
                           unsigned short far *pLen,
                           unsigned short         IOflag
                          )
{
    return ERROR_NOT_SUPPORTED;
}


int far pascal FS_MKDIR(
                           struct cdfsi   far *pcdfsi,
                           struct cdfsd   far *pcdfsd,
                           char           far *pName,
                           unsigned short         iCurDirEnd,
                           char           far *pEABuf,
                           unsigned short         flags
                          )
{
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_RMDIR(
                           struct cdfsi   far *pcdfsi,
                          struct cdfsd   far *pcdfsd,
                           char           far *pName,
                           unsigned short         iCurDirEnd
                          )
{
    return ERROR_NOT_SUPPORTED;
}


int     far pascal FS_FINDFIRST(
                               struct cdfsi   far *pcdfsi,
                               struct cdfsd   far *pcdfsd,
                               char           far *pName,
                               unsigned short         iCurDirEnd,
                               unsigned short         attr,
                               struct fsfsi   far *pfsfsi,
                               struct fsfsd   far *pfsfsd,
                               char           far *pData,
                               unsigned short         cbData,
                               unsigned short far *pcMatch,
                               unsigned short         level,
                               unsigned short         flags
                              )
{
    return ERROR_NOT_SUPPORTED;
}

int     far pascal FS_FINDCLOSE(
                               struct fsfsi far *pfsfsi,
                               struct fsfsd far *pfsfsd
                              )
{
    return ERROR_NOT_SUPPORTED;
}


int far pascal FS_FINDFROMNAME(
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
    return ERROR_NOT_SUPPORTED;
}

int     far pascal  FS_FINDNEXT(
                               struct fsfsi   far *pfsfsi,
                               struct fsfsd   far *pfsfsd,
                               char           far *pData,
                               unsigned short         cbData,
                               unsigned short far *pcMatch,
                               unsigned short         level,
                               unsigned short         flags
                              )
{
    return ERROR_NOT_SUPPORTED;
}



int far pascal FS_FILEINFO(
                              unsigned short         flag,
                              struct sffsi   far *psffsi,
                              struct sffsd   far *psffsd,
                              unsigned short         level,
                              char           far *pData,
                              unsigned short         cbData,
                              unsigned short         IOflag
                             )
{
    return ERROR_NOT_SUPPORTED;
}




int far pascal FS_PATHINFO(
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
    return ERROR_NOT_SUPPORTED;
}


int far pascal FS_FSCTL(
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
    return ERROR_NOT_SUPPORTED;
}


int far pascal FS_FILEATTRIBUTE(
                                   unsigned short         flag,
                                   struct cdfsi   far *pcdfsi,
                                   struct cdfsd   far *pcdfsd,
                                   char           far *pName,
                                   unsigned short         iCurDirEnd,
                                   unsigned short far *pAttr
                                  )
{
    return ERROR_NOT_SUPPORTED;
}


int far pascal FS_FSINFO(
                            unsigned short         flag,
                            unsigned short         hVPB,
                            char           far *pData,
                            unsigned short         cbData,
                            unsigned short         level
                           )
{
    return ERROR_NOT_SUPPORTED;
}

int far pascal FS_CHDIR(
                       unsigned short         flag,
                       struct cdfsi   far *pcdfsi,
                       struct cdfsd   far *pcdfsd,
                       char                  *pDir,
                       unsigned short         iCurDirEnd
                      )
{
    return ERROR_NOT_SUPPORTED;
}
