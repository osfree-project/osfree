/*
 *
 *
 *
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>                // From the "Developer Connection Device Driver Kit" version 2.0

//#include <ifs.h>

int far pascal FS_INIT(
                          char                  *szParm,
                          unsigned long         DevHelp,
                          unsigned long far *pMiniFSD
                         )
{
    return NO_ERROR;
}


int far pascal FS_READ(
                          struct sffsi   *psffsi,
                          union  sffsd   *psffsd,
                          char           *pData,
                          unsigned short *pLen,
                          unsigned short  IOflag
                         )
{
    return NO_ERROR;
}

int far pascal FS_CHGFILEPTR(
                            struct sffsi far *psffsi,
                            union  sffsd far *psffsd,
                            long                 offset,
                            unsigned short       type,
                            unsigned short       IOflag
                           )
{
    return NO_ERROR;
}

int far pascal FS_CLOSE(
                       unsigned short          type,
                       unsigned short          IOflag,
                       struct sffsi    far *psffsi,
                       union  sffsd    far *psffsd
                      )
{
    return NO_ERROR;
}

void far pascal FS_EXIT(
                       unsigned short uid,
                       unsigned short pid,
                       unsigned short pdb
                      )
{
    //printf("FS_EXIT( uid = %u pid = %u pdb = %u )", uid, pid, pdb);
}

int far pascal FS_IOCTL(
                           struct sffsi   far *psffsi,
                           union  sffsd   far *psffsd,
                           unsigned short         cat,
                           unsigned short         func,
                           char           far *pParm,
                           unsigned short         lenParm,
                           unsigned       far *pParmLenInOut,
                           char           far *pData,
                           unsigned short         lenData,
                           unsigned       far *pDataLenInOut
                          )
{
    return NO_ERROR;
}

int far pascal FS_MOUNT(
                           unsigned short         flag,
                           struct vpfsi   far *pvpfsi,
                           union  vpfsd   far *pvpfsd,
                           unsigned short        hVPB,
                           char           far *pBoot
                          )
{
    return NO_ERROR;
}

int far pascal FS_OPENCREATE(
                                struct cdfsi   far *pcdfsi,
                                struct cdfsd   far *pcdfsd,
                                char           far *pName,
                                unsigned short         iCurDirEnd,
                                struct sffsi   far *psffsi,
                                union  sffsd   far *psffsd,
                                unsigned long          ulOpenMode,
                                unsigned short         openflag,
                                unsigned short far *pAction,
                                unsigned short         attr,
                                char           far *pEABuf,
                                unsigned short far *pfgenFlag
                               )
{
    return NO_ERROR;
}

int far pascal FS_PROCESSNAME(
                                 char *pNameBuf
                                )
{
    return NO_ERROR;
}

void main (void)
{
}
