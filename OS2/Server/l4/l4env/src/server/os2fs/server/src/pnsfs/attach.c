/*  FS volume Attach
 *
 *
 */

#include <fsd.h>

FSRET FSENTRY pns_ATTACH(USHORT flag,
                        char *pDev,
                        struct vpfsd *pvpfsd,
                        struct cdfsd *pcdfsd,
                        char *pParm,
                        USHORT *pLen)
{
  if (!pDev || !*pDev || pDev[1] != ':' || pDev[2])
    return ERROR_INVALID_PARAMETER;

  switch (flag)
  {
    case 0:  // attach
      
      break;
    case 1:  // detach
      break;
    case 2:  // query the attachment info
      break;
    default: // invalid param
      return ERROR_INVALID_PARAMETER;
  }

  return NO_ERROR;
}

/* not supported for remote FS's */
FSRET FSENTRY pns_MOUNT(USHORT flag,
                       struct vpfsi *pvpfsi,
                       struct vpfsd *pvpfsd,
                       USHORT hVPB,
                       char *pBoot)
{
  return ERROR_NOT_SUPPORTED;
}
