/*  Filesystem router
 *
 *
 */

#include <gcc_os2def.h>
#include <handlemgr.h>
#include <fsd.h>
#include <cds.h>
#include <vpb.h>
#include <sft.h>

/* VPB handle table */
HANDLE_TABLE *htVPB;
/* CDS handle table */
HANDLE_TABLE *htCDS;
/* SFT handle table */
HANDLE_TABLE *htSFT;

/* attach a (remote) file system */
int FSR_FSATTACH(PSZ pszDevice,
                 PSZ pszFilesystem,
	         PVOID pData,
	         ULONG cbData)
{
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
  struct cdfsi *pvpfsi;
  struct cdfsd *pcdfsd;
  PVPB   vpb;
  USHORT hVPB;
  PCDS   cds;
  USHORT hCDS;
  APIRET rc;

  if (!pszDevice || !*pszDevice || pszDevice[1] != ':' || pszDevice[2] ||
      !pszFilesystem || !*pszFilesystem)
    return ERROR_INVALID_PARAMETER;

  /* allocate a VPB for a new volume */
  if (rc = HndAllocateHandle(htVPB, &hVPB, vpb))
    return rc;

  pvpfsi = &vpb->vpb_vpfsi;
  pvpfsd = &vpb->vpb_vpfsd;

  /* alloocate a new CDS */
  if (rc = HndAllocateHandle(htCDS, &hCDS, cds))
  {
    // free the VPB
    HndFreeHandle(htVPB, vpb);
    return rc;  
  }

  pcdfsi = &cds->cds_cdfsi;
  pcdfsd = &cds->cds_cdfsd;
  
  pcdfsi->cdi_hVPB = hVPB;

  if (!strlcmp(pszFilesystem, "pnsfs", CCHMAXPATH))
    // built-in FS
    rc = pns_ATTACH(FS_ATTACH, pszDevice,
                    pvpfsd, pcdfsd,
                    pData, &cbData);
  else
  {
    // external FS
    //rc = FS_ATTACH(...);
  }

  if (rc)
  {
    // free the VPB
    HndFreeHandle(htVPB, vpb);
    return rc;
  }

  return NO_ERROR;
}

/* detach a (remote) file system */
int FSR_FSDETACH(PSZ pszDevice,
                 PSZ pszFilesystem)
{
  return NO_ERROR;
}

/* fs router initialization */
int FSR_INIT(void)
{
  /* allocate a handle table for VPB's */
  htVPB = (HANDLE_TABLE *)malloc(sizeof(HANDLE_TABLE));
  HndInitializeHandleTable(MAX_VPBS, sizeof(VPB), htVPB);
  /* allocate a handle table for CDS's */
  htCDS = (HANDLE_TABLE *)malloc(sizeof(HANDLE_TABLE));
  HndInitializeHandleTable(MAX_CDS, sizeof(CDS), htCDS);
  /* allocate a handle table for SFT   */
  htSFT = (HANDLE_TABLE *)malloc(sizeof(HANDLE_TABLE));
  HndInitializeHandleTable(MAX_FILES, sizeof(SFT), htSFT);

  /* hardcode for the 1st time */
  FSR_FSATTACH("c:", "pnsfs", "/file/system", 13);
  FSR_FSATTACH("d:", "pnsfs", "/", 2);
  FSR_FSATTACH("f:", "pnsfs", "/file", 6);
}

/* fs router deinit */
int FSR_DONE(void)
{
  /* hardcode for the 1st time */
  FSR_FSDETACH("f:", "pnsfs");
  FSR_FSDETACH("d:", "pnsfs");
  FSR_FSDETACH("c:", "pnsfs");

  return NO_ERROR;
}
