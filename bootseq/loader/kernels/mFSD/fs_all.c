/*
 *
 *
 *
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>                // From the "Developer Connection Device Driver Kit" version 2.0

#include "ifs.h"
#include "struct.h"

extern char boot[0x800];

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
unsigned short hVPB   = 0;
struct dpb far *pdpb  = 0; // pointer to our DPB

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

int far pascal _loadds MFS_OPEN(char far *pszName, unsigned long far *pulSize);
int far pascal _loadds MFS_READ(char far *pcData,  unsigned short far *pusLength);

int far pascal MFSH_SETBOOTDRIVE(unsigned short usDrive);

int  far pascal FSH_DOVOLIO    (unsigned short operation,
                               unsigned short fAllowed,
                               unsigned short hVPB,
                               char far *pData,
                               unsigned short far *pcSec,
                               unsigned long iSec);
int far pascal FSH_FINDDUPHVPB(unsigned short hVPB,
                               unsigned short far *phVPB);
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
void far pascal FSH_INTERR(char far *pMsg,
                           unsigned short cbMsg);

#pragma aux DevHlp          "*"
#pragma aux FS_NAME         "*"
#pragma aux FS_ATTRIBUTE    "*"
#pragma aux FS_MPSAFEFLAGS2 "*"

char FS_NAME[12];
unsigned long FS_ATTRIBUTE         = 0x0L;
unsigned long long FS_MPSAFEFLAGS2 = 0x0LL;

char read_panic[] = "Error reading boot sector from the boot drive!\n";
struct devcaps devcaps;
char volchars[0x20];
struct vpfsi far *pVPfsi;
struct vpfsd far *pVPfsd;

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

  __asm {
    mov  ax, cs
    mov  selector, ax
  }
  p = (char far *)MAKEP(selector, 0);
  for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *(p + 0x1386 + i));
  //for (i = 0; i < 0x2b36; i++) kprintf("0x%02x,", p[i]);
  kprintf("\n");

  //__asm {
  //  int 3
  //}

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

  if (*((unsigned long far *)psffsd + 1)) // direct read flag
  {
    //kprintf("2\n");
    if (!(rc = FSH_PROBEBUF(1, pData, *pLen)))
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
    //kprintf("4\n");
    filebase = *((unsigned long far *)psffsd);
    advance_ptr();
    if (rc = MFS_READ(pData, pLen))
    {
      kprintf("MFS_READ failed, rc = %u\n", rc);
    }
  }
  //kprintf("5\n");
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

  //filemax = psffsi->sfi_size;
  //filepos = psffsi->sfi_position;
  if (!*((unsigned long far *)psffsd + 1)) // direct read flag
  {
    filebase = *((unsigned long far *)psffsd);
    //advance_ptr();
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

  //__asm {
  //  int 3
  //}

  kprintf("**** FS_MOUNT\n");

  __asm {
    mov  ax, cs
    mov  selector, ax
  }
  p = (char far *)MAKEP(selector, 0);
  for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *(p + 0x1386 + i));
  kprintf("\n");

  if (flag)
    return ERROR_NOT_SUPPORTED;

  if (!oncecntr)
    kprintf("mounting a DASD partition...\n");
  else
    kprintf("the drive is remounted.\nmounting a real boot partition...\n");

  kprintf("hVPB = 0x%04x\n", hVPB);
  if (!FSH_FINDDUPHVPB(hVPB, &old_hVPB))
  {
    //hVPB = old_hVPB;
    //FSH_GETVOLPARM(hVPB, &pvpfsi, &pvpfsd);
    kprintf("found a dup hVPB = 0x%04x\n", old_hVPB);
    return NO_ERROR;
  }

  memset(pvpfsd, 0, sizeof(struct vpfsd));

  //if (!oncecntr)
  //{
  //  pvpfsi->vpi_vid = 0x12345678;
  //  strcpy(pvpfsi->vpi_text, "MBI_TEST");
  //}

  secsize = pvpfsi->vpi_bsize;
  volsize = pvpfsi->vpi_totsec * secsize;
  kprintf("volume size: %lu bytes, sector size: 0x%x\n", volsize, secsize);
  pdpb = (struct dpb far *)pvpfsi->vpi_hDEV;
  kprintf("DPB addr: pvpfsi->vpi_hDEV = 0x%08lx\n", pdpb);
  //boot = pBoot;

  //FSH_GETVOLPARM(hVPB, &pvpfsi, &pvpfsd);
  pdpb = (struct dpb far *)pvpfsi->vpi_hDEV;
  kprintf("DPB addr: pvpfsi->vpi_hDEV = 0x%08lx\n", pdpb);
  kprintf("vpi_pDCS = 0x%08lx, vpi_pVCS = 0x%08lx\n",
          pvpfsi->vpi_pDCS,
          pvpfsi->vpi_pVCS);
  kprintf("hVPB: 0x%04x\n", hVPB);

  if (!strcmp(FS_NAME, "CDFS") && pvpfsi->vpi_bsize != 0x800)
    return ERROR_NOT_SUPPORTED;

  if (!oncecntr)
  {
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

    if (drvletter == 'A' + pdpb->dpb_drive)
    {
      // save devcaps structure to the safe place
      memmove(&devcaps, pvpfsi->vpi_pDCS, sizeof(struct devcaps));
      // save volume characteristics
      memmove(&volchars, pvpfsi->vpi_pVCS, sizeof(volchars));

      //boot[25] = 0x80; boot[26] = 0x80; // fixme!!!
      //boot[31] = 0x12; boot[30] = 0x34; boot[29] = 0x56; boot[28] = 0x78;
      //strcpy(boot + 32, "MBI_TEST");
      //strcpy(boot + 43, FS_NAME);
    }

    // zero out open files indicate area
    memset(save_map, 0, sizeof(save_map));
  }

  //pVPfsi = pvpfsi; pVPfsd = pvpfsd;
  oncecntr++; // for this block of code to execute only once

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
  char msg_toomany[] = "Open file limit exceeded!\n";
  char far *p;
  int rc, i;

  kprintf("**** FS_OPENCREATE(\"%s\")\n", pName);

  if (ulOpenMode & OPEN_FLAGS_DASD) // opening whole drive for a direct read
  {
    kprintf("OPEN_FLAGS_DASD set\n");
    if (pName[1] == ':' && pName[2] == '\\' && pName[3] == '\0') // like "C:\"
    {
      *((unsigned long far *)psffsd + 1) = 1; // direct read flag
      psffsi->sfi_size = volsize;
      psffsi->sfi_position = 0;
      hVPB = psffsi->sfi_hVPB;
      open_files++;
      kprintf("sfi_size = %lu\n", volsize);
      kprintf("hVPB: 0x%04x\n", hVPB);

      if (!strcmp(FS_NAME, "CDFS"))
      {
        FSH_GETVOLPARM(hVPB, &pVPfsi, &pVPfsd);
        memset(pVPfsd, 0, sizeof(struct vpfsd));
        secsize = pVPfsi->vpi_bsize = 0x800;
        volsize = pVPfsi->vpi_totsec = 0 ;
        pdpb = (struct dpb far *)pVPfsi->vpi_hDEV;
        kprintf("DPB addr: pvpfsi->vpi_hDEV = 0x%08lx\n", pdpb);
      }

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

  // If volume is remounted as a CD, then change a driveletter to a CD's one
  //if (oncecntr == 2 && pNameBuf[0] == drvletter)
  if (strstr(pNameBuf, "msg.dll") ||
      strstr(pNameBuf, "MSG.DLL"))
  {
    // msg.dll is loading after CDFS.IFS. At this point
    // we need to access a CD drive letter. So, we 'transform'
    // to the 'local' IFS by changing FS_ATTRIBUTE
    drvflag = 1;
    FS_ATTRIBUTE &= ~0x1; // clear 'remote fs' bit

    kprintf("FS_PROCESSNAME: %s\n", pNameBuf);
  }
  //if (drvflag) pNameBuf[0] = 'V';

  return NO_ERROR;
}

int far pascal _loadds FS_ATTACH(
                         unsigned short     flag,        /* flag                */
                         char           far *pDev,       /* pDev                */
                         struct vpfsd   far *pvpfsd,     /* if remote drive
                                                           struct vpfsd far *
                                                           else if remote device
                                                                  null ptr (0L)    */
                         struct cdfsd   far *pcdfsd,        /* if remote drive
                                                           struct cdfsd far *
                                                          else
                                                          struct devfsd far * */
                         char           far *pParm,      /* pParm        */
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
