/*   minifsd FS_*-entry points
 *   (c) osFree, 2008-2011,
 *   author valerius,
 *   _valerius (dog) mail (dot) ru
 *
 */

#include <const.h>
#include <mb_info.h>

#include "ifs.h"
#include "struct.h"

extern char boot[0x800];
extern unsigned long mbi;

char hellomsg[] = "Hello stage2!\n";
unsigned long DevHlp;
char oncecntr = 0;
char drvflag  = 0;

int tolower (int c);
int kprintf(const char *format, ...);
void advance_ptr(void);

int open_files = 0;
unsigned long volsize = 0;
unsigned long secsize = 0;
unsigned short my_hVPB   = 0;
struct dpb far *pdpb  = 0; // pointer to our DPB

#define CCHMAXPATHCOMP 256

#define CHECKRC \
    if (rc) \
    {       \
      kprintf("MFSH_PHYSTOVIRT() failed, @%s, line #%lu\n", \
              __FILE__, __LINE__); \
      return 1; \
    }

#pragma pack(1)

struct save_item
{
  struct cdfsi far *pcdfsi;
  struct cdfsd far *pcdfsd;
  struct sffsi far *psffsi;
  struct sffsd far *psffsd;
  unsigned long ulOpenMode;
  unsigned short usOpenFlag;
  char pName[0x3a];
};

typedef struct _FILEFINDBUF2 {
  unsigned short fdateCreation;            /*  Date of file creation. */
  unsigned short ftimeCreation;            /*  Time of file creation. */
  unsigned short fdateLastAccess;          /*  Date of last access.   */
  unsigned short ftimeLastAccess;          /*  Time of last access.   */
  unsigned short fdateLastWrite;           /*  Date of last write.    */
  unsigned short ftimeLastWrite;           /*  Time of last write.    */
  unsigned long  cbFile;                   /*  Size of file.          */
  unsigned long  cbFileAlloc;              /*  Allocated size.        */
  unsigned long  attrFile;                 /*  File attributes.       */
  unsigned long  cbList;                   /*  EA list size           */
  unsigned char  cchName;                  /*  Length of file name.   */
  char achName[CCHMAXPATHCOMP];       /*  File name including null terminator. */
} FILEFINDBUF2;

#pragma pack()

extern unsigned char drvletter;
extern unsigned char cd_drvletter;

extern char save_map[0x80]; // open files indicate area
extern struct save_item save_area[0x80];
extern struct save_item *save_pos;
extern int save_index;

extern unsigned long filemax;
extern unsigned long filepos;
extern unsigned long filebase;

char *strstr (const char *s1, const char *s2);
void *memmove (void *_to, const void *_from, int _len);
char *strcpy (char *dest, const char *src);
int strcmp (const char *s1, const char *s2);
void *memset (void *start, int c, int len);
int strlen (const char *str);

#pragma aux mbi             "*"
#pragma aux DevHlp          "*"
#pragma aux FS_NAME         "*"
#pragma aux FS_ATTRIBUTE    "*"
#pragma aux FS_MPSAFEFLAGS2 "*"

char FS_NAME[12];
unsigned long FS_ATTRIBUTE         = 0x0;
unsigned long long FS_MPSAFEFLAGS2 = 0x0LL;

char read_panic[] = "Error reading boot sector from the boot drive!\n";
struct devcaps devcaps;
char volchars[0x20];

int far pascal _loadds FS_INIT(
                          char              *szParm,
                          unsigned long     DevHelp,
                          unsigned long far *pMiniFSD
                         )
{
  unsigned short selector;
  char far *p;
  int i;

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

  if (*((unsigned long far *)psffsd + 1)) // direct read flag
  {
    if (!(rc = FSH_PROBEBUF(1, pData, *pLen)))
    {
      unsigned short cbSec = *pLen / secsize;
      if (rc = FSH_DOVOLIO(8,                    // return errors directly, don't use harderr daemon
                           1 + 2 + 4,            // ABORT | RETRY | FAIL
                           psffsi->sfi_hVPB,
                           pData,
                           &cbSec,
                           filepos / secsize))
        kprintf("FSH_DOVOLIO failed, rc = %u\n", rc);
      else
      {
        kprintf("FSH_DOVOLIO(buf = 0x%08lx, len = %u, start = %lu sectors) succeeded\n",
                pData,
                cbSec,
                filepos / secsize);
        *pLen = cbSec * secsize;
        kprintf("%u butes read\n", *pLen);
        filepos += *pLen;
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
    filebase = *((unsigned long far *)psffsd);
    advance_ptr();
    if (rc = MFS_READ(pData, pLen))
    {
      kprintf("MFS_READ failed, rc = %u\n", rc);
    }
  }
  psffsi->sfi_position = filepos;

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

  if (!*((unsigned long far *)psffsd + 1)) // direct read flag
  {
    filebase = *((unsigned long far *)psffsd);
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

  save_index = *((unsigned long *)psffsd + 2);
  save_map[save_index] = 0;
  memset(psffsd, 0, sizeof(struct sffsd));
  open_files--;

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
      kprintf("FS_IOCTL: FSH_DEVIOCTL returned %u\n", rc);
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
  unsigned short cbSec = 1;
  unsigned short old_hVPB;
  unsigned short selector;
  char far *p;
  int rc;
  int i;

  kprintf("**** FS_MOUNT\n");

  if (flag)
    return ERROR_NOT_SUPPORTED;

  memset(pvpfsd, 0, sizeof(struct vpfsd));

  secsize = pvpfsi->vpi_bsize;
  volsize = pvpfsi->vpi_totsec * secsize;
  kprintf("volume size: %lu bytes, sector size: 0x%x\n", volsize, secsize);
  pdpb = (struct dpb far *)pvpfsi->vpi_hDEV;
  kprintf("DPB addr: pvpfsi->vpi_hDEV = 0x%08lx\n", pdpb);

  kprintf("vpi_pDCS = 0x%08lx, vpi_pVCS = 0x%08lx\n",
          pvpfsi->vpi_pDCS,
          pvpfsi->vpi_pVCS);
  kprintf("hVPB: 0x%04x\n", hVPB);

  if (drvletter == 'A' + pvpfsi->vpi_drive)
  {
    my_hVPB = hVPB;
    // ...
    memset(boot, 0, sizeof(boot));
    rc = FSH_DOVOLIO(8,               // return errors directly, not through harderr daemon
                     1 + 2 + 4 + 8,   // ABORT | RETRY | FAIL | IGNORE
                     hVPB,
                     boot,
                     &cbSec,
                     0);

    kprintf("FSH_DOVOLIO: rc = 0x%x\n", rc);
    if (rc) FSH_INTERR(read_panic, strlen(read_panic));
    kprintf("read %u sectors\n", cbSec);
    // ...
    // save devcaps structure to the safe place
    memmove(&devcaps, pvpfsi->vpi_pDCS, sizeof(struct devcaps));
    // save volume characteristics
    memmove(&volchars, pvpfsi->vpi_pVCS, sizeof(volchars));
    // volume label
    memmove(pvpfsi->vpi_text, "MBI", 4);
    // volume serial number

    pvpfsi->vpi_vid = 0x11111111;

    memset(save_map, 0, sizeof(save_map));

    return NO_ERROR; // volume is recognized
  }

  return NO_ERROR + 1; // volume is not recognized
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
  unsigned long  size;
  unsigned short hVPB;
  char msg_toomany[] = "Open file limit exceeded!\n";
  char far *p;
  int rc, i;

  kprintf("**** FS_OPENCREATE(\"%s\")\n", pName);
  kprintf("iCurDirEnd=%d, curdir=%s, cdi_end=%d\n", iCurDirEnd, pcdfsi->cdi_curdir, pcdfsi->cdi_end);

  my_hVPB = psffsi->sfi_hVPB;

  if (ulOpenMode & OPEN_FLAGS_DASD) // opening whole drive for a direct read
  {
    kprintf("OPEN_FLAGS_DASD set\n");
    if (pName[1] == ':' && pName[2] == '\\' && pName[3] == '\0') // like "C:\"
    {
      *((unsigned long far *)psffsd + 1) = 1; // direct read flag
      psffsi->sfi_size = volsize;
      psffsi->sfi_position = 0;
      hVPB = my_hVPB;
      open_files++;
      kprintf("sfi_size = %lu\n", volsize);
      kprintf("hVPB: 0x%04x\n", hVPB);

      rc = NO_ERROR;
    }
  }
  else // ordinary opening
  {
    rc = file_open(pName, &size, psffsi, psffsd);
    *((unsigned long far *)psffsd + 1) = 0; // direct read flag
  }

  if (!rc) // if a file is successfully opened
  {
    // search for a 1st unused file save area
    for (save_index = 0; save_map[save_index]; save_index++) ;

    // if too many open files -- panic
    if (save_index > 0x80)
      FSH_INTERR(msg_toomany, strlen(msg_toomany));

    save_pos = save_area + save_index;

    // save open file info
    save_map[save_index] = 1;
    save_pos->pcdfsi = pcdfsi;
    save_pos->pcdfsd = pcdfsd;
    save_pos->psffsi = psffsi;
    save_pos->psffsd = psffsd;
    save_pos->ulOpenMode = ulOpenMode;
    save_pos->usOpenFlag = openflag;
    memmove(save_pos->pName, pName, 0x3a);
    // make lowercase
    for (p = save_pos->pName; *p; p++) *p = tolower(*p);

    *((unsigned long *)psffsd + 2) = save_index;
  }


  return rc;
}

int far pascal _loadds FS_PROCESSNAME(
                                 char far *pNameBuf
                                )
{
  int rc;
  kprintf("**** FS_PROCESSNAME(\"%s\")\n", pNameBuf);

  return NO_ERROR;
}

int far pascal _loadds FS_ATTACH(
                         unsigned short     flag,        /* flag                */
                         char           far *pDev,       /* pDev                */
                         struct vpfsd   far *pvpfsd,     /* if remote drive
                                                            struct vpfsd far *
                                                            else if remote device
                                                               null ptr (0L)    */
                         struct cdfsd   far *pcdfsd,     /* if remote drive
                                                            struct cdfsd far *
                                                            else
                                                            struct devfsd far * */
                         char           far *pParm,      /* pParm               */
                         unsigned short far *pLen        /* pLen                */
                        )
{
  int rc;
  kprintf("**** FS_ATTACH(\"%s\")\n", pDev);

  // zero out open files indicate area
  memset(save_map, 0, sizeof(save_map));
  oncecntr++;

  return NO_ERROR;
}

int     far pascal _loadds FS_FINDFIRST(
                               struct cdfsi   far *pcdfsi,
                               struct cdfsd   far *pcdfsd,
                               char           far *pName,
                               unsigned short iCurDirEnd,
                               unsigned short attr,
                               struct fsfsi   far *pfsfsi,
                               struct fsfsd   far *pfsfsd,
                               char           far *pData,
                               unsigned short cbData,
                               unsigned short far *pcMatch,
                               unsigned short level,
                               unsigned short flags
                              )
{
  int    rc;
  unsigned long size;
  char far *p;
  FILEFINDBUF2 far *ff;
  char buf[0x100];

  kprintf("**** FS_FINDFIRST(\"%s\"), level=%u, flags=%u, curdir=%s, iCurDirEnd=%d\n",
          pName, level, flags, pcdfsi->cdi_curdir, iCurDirEnd);

  p = pName;
  // skip a drive letter
  if (p[1] == ':' && p[2] == '\\') p += 2;

  /* find by trying to open */
  *pcMatch = 0; rc = 2; // file not found
  if (!MFS_OPEN(p, &size))
  {
    // file found
    switch (level)
    {
    case 1:
      *pcMatch = 1;
      break;
    case 2: // query EA size, specially for *.cmd
      // query write access to the buffer
      if (!FSH_PROBEBUF(1, pData, cbData))
      {
        // fill in the position field
        *((unsigned long far *)pData) = 0; // sizeof(FILEFINDBUF2) - CCHMAXPATHCOMP;
        pData += sizeof(unsigned long);
        ff = (FILEFINDBUF2 far *)pData;
        ff->cbFile = size;
        ff->cbFileAlloc = size;
        ff->cbList = 0; // sizeof(unsigned long);

        // for files in root dir. or files in first-level dirs, return the
        // full pathname; for deeper files, return a filename with last dir
        // i.e.: s:\tools\netdrive\ndctl.exe -> netdrive\ndctl.exe
        for (p = pName + strlen(pName) - 1; p > pName && *p != '\\'; p--) ;
        if (p - pName > 2)
        {
          p--;
          for (; p > pName && *p != '\\'; p--) ;
        }
        if (p - pName > 2)
          p++;
        else
          p = pName;

        ff->cchName = strlen(p);
        if (attr & 0x0040)
        {
          strcpy(ff->achName, p);
          attr &= ~0x0040;
        }
        else
          FSH_UPPERCASE(p, ff->cchName, ff->achName);

        kprintf("achName=%s, curdir=%s, cdi_end=%d\n", ff->achName, pcdfsi->cdi_curdir, pcdfsi->cdi_end);
      }
      else
        kprintf("FSH_PROBEBUF failed!\n");
      break;
    default:
      break;
    }

    rc = 0;
  }

  return rc;
}

int     far pascal _loadds FS_FINDCLOSE(
                               struct fsfsi far *pfsfsi,
                               struct fsfsd far *pfsfsd
                              )
{
    kprintf("**** FS_FINDCLOSE\n");

    return NO_ERROR;
}
