/*
 *
 *
 *
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>                // From the "Developer Connection Device Driver Kit" version 2.0

#include "ifs.h"

char hellomsg[] = "Hello stage2!\n";
unsigned long DevHlp;

int kprintf(const char *format, ...);
void advance_ptr(void);

int open_files = 0;
unsigned long volsize = 0;
unsigned long secsize = 0;
unsigned short hVPB   = 0;
extern unsigned long filemax;
extern unsigned long filepos;
extern unsigned long filebase;

char *strcpy (char *dest, const char *src);
void *memset (void *start, int c, int len);

int far pascal _loadds MFS_OPEN(char far *pszName, unsigned long far *pulSize);
int far pascal _loadds MFS_READ(char far *pcData,  unsigned short far *pusLength);

int  far pascal FSH_DOVOLIO    (unsigned short operation,
                               unsigned short fAllowed,
                               unsigned short hVPB,
                               char far *pData,
                               unsigned short far *pcSec,
                               unsigned long iSec);
void far pascal FSH_GETVOLPARM(unsigned short hVPB,
                               struct vpfsi far * far *ppVPBfsi,
                               struct vpfsd far * far *ppVPBfsd);
int  far pascal FSH_DEVIOCTL  (unsigned short flag,
                               unsigned long hDev,
                               unsigned short sfn,
                               unsigned short cat,
                               unsigned short func,
                               char far *pParm,
                               unsigned short cbParm,
                               char far *pData,
                               unsigned short cbData);
int  far pascal FSH_PROBEBUF  (unsigned short operation,
                               char far *pdata,
                               unsigned short cbData);

#pragma aux DevHlp          "*"
#pragma aux FS_NAME         "*"
#pragma aux FS_ATTRIBUTE    "*"
#pragma aux FS_MPSAFEFLAGS2 "*"

char FS_NAME[12];
unsigned long FS_ATTRIBUTE         = 0x0L;
unsigned long long FS_MPSAFEFLAGS2 = 0x0LL;

/*
FS_ATTRIBUTE dd 00000000000000000000000000101100b
;                                         | ||||
;                 large file support   ---+ ||||
;                 level 7 requests     -----+|||
;                 lock notifications   ------+||
;                 UNC support          -------+|
;                 remote FSD           --------+
FS_NAME                 db      'JFS',0
FS_MPSAFEFLAGS2         dd      41h, 0         ; 01h = don't get r0 spinlock
                                                 ; 40h = don't acquire subsys spinlock
                                            ; FS_MPSAFEFLAGS2 is an array of 64 bits
 */

int far pascal _loadds FS_INIT(
                          char              *szParm,
                          unsigned long     DevHelp,
                          unsigned long far *pMiniFSD
                         )
{
  kprintf("**** FS_INIT\n");
  kprintf(hellomsg);

  DevHlp = DevHelp;

  return NO_ERROR;
}


int far pascal _loadds FS_READ(
                          struct sffsi   far *psffsi,
                          struct sffsd   far *psffsd,
                          char           far *pData,
                          unsigned short far *pLen,
                          unsigned short     IOflag
                         )
{
  int rc;

  kprintf("**** FS_READ\n");

  filemax = psffsi->sfi_size;
  filepos = psffsi->sfi_position;

  kprintf("sfi_size = %lu, sfi_position = %lu\n", filemax, filepos);
  kprintf("buf = 0x%08lx, size = %u\n", pData, *pLen);
  //kprintf("1\n");

  if (*((unsigned long far *)(psffsd + 1))) // direct read flag
  {
    //kprintf("2\n");
    if (!FSH_PROBEBUF(1, pData, *pLen))
    {
      unsigned short cbSec = *pLen / secsize;
      if (rc = FSH_DOVOLIO(8,                    // return errors directly, don't use harderr daemon
                           1 + 2 + 4,            // ABORT | RETRY | FAIL
                           hVPB,
                           pData,
                           &cbSec,
                           filepos / secsize))
        kprintf("FSH_DOVOLIO failed, rc = %u\n", rc);
      else
      {
        //kprintf("3\n");
        kprintf("FSH_DOVOLIO(buf = 0x%08lx, len = %u, start = %lu sectors) succeeded\n",
                pData,
                cbSec,
                filepos / secsize);
        *pLen = cbSec * secsize;
        kprintf("%u bytes read\n", *pLen);
        filepos += *pLen;
        psffsi->sfi_position = filepos;
        rc = NO_ERROR;
      }
    }
    else
    {
      kprintf("FS_READ: FSH_PROBEBUF failed!\n");
      rc = 1;
    }
  }
  else
  {
    //kprintf("4\n");
    filebase = *((unsigned long far *)psffsd);
    advance_ptr();
    if (rc = MFS_READ(pData, pLen))
      kprintf("MFS_READ failed, rc = %u\n", rc);
  }
  //kprintf("5\n");

  return rc;
}

int far pascal _loadds FS_CHGFILEPTR(
                            struct sffsi far *psffsi,
                            struct sffsd far *psffsd,
                            long             offset,
                            unsigned short   type,
                            unsigned short   IOflag
                           )
{
  long off;

  kprintf("**** FS_CHGFILEPTR - offset=%ld, type=%u\n", offset, type);

  switch (type)
  {
  case 0: // from the beginning
    off = offset;
    break;
  case 1: // relative to current file pointer
    off = psffsi->sfi_position + offset;
    break;
  case 2: // relative to the end of file
    off = psffsi->sfi_size + offset;
    break;
  default:
    return ERROR_INVALID_PARAMETER;
  }

  psffsi->sfi_position = off;

  filemax = psffsi->sfi_size;
  filepos = psffsi->sfi_position;
  if (!*((unsigned long far *)(psffsd + 1))) // direct read flag
  {
    filebase = *((unsigned long far *)psffsd);
    advance_ptr();
  }

  kprintf("FS_CHGFILEPTR: sfi_size = %lu\n", psffsi->sfi_size);
  kprintf("FS_CHGFILEPTR: sfi_position = %lu\n", psffsi->sfi_position);

  return NO_ERROR;
}

int far pascal _loadds FS_CLOSE(
                       unsigned short    type,
                       unsigned short    IOflag,
                       struct sffsi  far *psffsi,
                       struct sffsd  far *psffsd
                      )
{
    kprintf("**** FS_CLOSE\n");
    open_files--;
    memset(psffsd, 0, sizeof(struct sffsd));

    return NO_ERROR;
}

void far pascal _loadds FS_EXIT(
                       unsigned short uid,
                       unsigned short pid,
                       unsigned short pdb
                      )
{
    kprintf("**** FS_EXIT\n");
}

int far pascal _loadds FS_IOCTL(
                           struct sffsi   far *psffsi,
                           struct sffsd   far *psffsd,
                           unsigned short     cat,
                           unsigned short     func,
                           char           far *pParm,
                           unsigned short     lenParm,
                           unsigned       far *pParmLenInOut,
                           char           far *pData,
                           unsigned short     lenData,
                           unsigned       far *pDataLenInOut
                          )
{
  int           rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  kprintf("**** FS_IOCTL: cat = 0x%x, func = 0x%x\n", cat, func);

  FSH_GETVOLPARM(psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
  if ((rc = FSH_DEVIOCTL(
                         0,
                         pvpfsi->vpi_hDEV,
                         psffsi->sfi_selfsfn,
                         cat,
                         func,
                         pParm,
                         lenParm,
                         pData,
                         lenData
                        )) == NO_ERROR) {
  // nothing
  } else {
      kprintf("FS_IOCTL: FSH_DEVIOCTL returned %d", rc);
  }

  if (pDataLenInOut) {
    *pDataLenInOut = lenData;
  }
  if (pParmLenInOut) {
    *pParmLenInOut = lenParm;
  }

  return rc;
}

int far pascal _loadds FS_MOUNT(
                           unsigned short     flag,
                           struct vpfsi   far *pvpfsi,
                           struct vpfsd   far *pvpfsd,
                           unsigned short     hVPB,
                           char           far *pBoot
                          )
{
  kprintf("**** FS_MOUNT\n");

  if (flag)
    return ERROR_NOT_SUPPORTED;

  memset(pvpfsd, 0, sizeof(struct vpfsd));
  pvpfsi->vpi_vid = 0x12345678;
  strcpy(pvpfsi->vpi_text, "MBI_TEST");
  secsize = pvpfsi->vpi_bsize;
  volsize = pvpfsi->vpi_totsec * secsize;
  kprintf("volume size: %lu bytes, sector size: 0x%x\n", volsize, secsize);

  return NO_ERROR;
}

int far pascal _loadds file_open(char far *name,
             unsigned long far *size,
             struct sffsi far *psffsi,
             struct sffsd far *psffsd)
{
  char far *p = name;

  if (p[1] == ':' && p[2] == '\\') p = p + 2;

  if (!MFS_OPEN(p, size))
  {
    psffsi->sfi_tstamp = 3; // ST_SCREAT | ST_PCREAT
    psffsi->sfi_cdate  = 0x1e21; // от балды
    psffsi->sfi_adate  = 0x1e21; //
    psffsi->sfi_mdate  = 0x1e21; //
    *((unsigned long far *)psffsd) = filebase; // save base physical address of the file in memory
    psffsi->sfi_size = *size;
    psffsi->sfi_position = 0;
    psffsi->sfi_DOSattr = FILE_ARCHIVED | FILE_READONLY;
    open_files++;
  }
  else
    return ERROR_FILE_NOT_FOUND;

  return NO_ERROR;
}

int far pascal _loadds FS_OPENCREATE(
                                struct cdfsi   far *pcdfsi,
                                struct cdfsd   far *pcdfsd,
                                char           far *pName,
                                unsigned short     iCurDirEnd,
                                struct sffsi   far *psffsi,
                                struct sffsd   far *psffsd,
                                unsigned long      ulOpenMode,
                                unsigned short     openflag,
                                unsigned short far *pAction,
                                unsigned short     attr,
                                char           far *pEABuf,
                                unsigned short far *pfgenFlag
                               )
{
  unsigned long size;
  int rc;

  kprintf("**** FS_OPENCREATE(\"%s\")\n", pName);

  if (ulOpenMode & OPEN_FLAGS_DASD) // opening whole drive for a direct read
  {
    kprintf("OPEN_FLAGS_DASD set\n");
    if (pName[1] == ':' && pName[2] == '\\' && pName[3] == '\0') // like "C:\"
    {
      *((unsigned long far *)(psffsd + 1)) = 1; // direct read flag
      psffsi->sfi_size = volsize;
      psffsi->sfi_position = 0;
      hVPB = psffsi->sfi_hVPB;
      open_files++;
      //rc = file_open("*bootsec*", &size, psffsi, psffsd);
      kprintf("sfi_size = %lu\n", volsize);
      kprintf("hVPB: 0x%04x\n", hVPB);
      rc = NO_ERROR;
    }
  }
  else // ordinary opening
  {
    rc = file_open(pName, &size, psffsi, psffsd);
    *((unsigned long far *)(psffsd + 1)) = 0; // direct read flag
  }

  return rc;
}

int far pascal _loadds FS_PROCESSNAME(
                                 char far *pNameBuf
                                )
{
  kprintf("**** FS_PROCESSNAME(\"%s\")\n", pNameBuf);

  return NO_ERROR;
}
