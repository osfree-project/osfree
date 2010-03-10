/*
 *
 *
 */

#include <const.h>                // From the "Developer Connection Device Driver Kit" version 2.0

#include <mb_info.h>
#include "serial.h"
#include "struct.h"

//#include <string.h>
#include <ifs.h>

#define CHECKRC \
    if (rc) \
    {       \
      kprintf("MFSH_PHYSTOVIRT() failed, @%s, line #%lu\n", \
              __FILE__, __LINE__); \
      return 1; \
    }

int serial_init (long port, long speed,
                int word_len, int parity, int stop_bit_len);

extern char FS_NAME[12];
extern unsigned long FS_ATTRIBUTE;
extern struct dpb far *pdpb; // pointer to our DPB
extern unsigned short hVPB;
extern int open_files;

extern unsigned long mbi0;
unsigned long mbi;

unsigned short FlatR0CS;
unsigned short FlatR0DS;

unsigned long far *p_minifsd;

char debug = 0;
char remotefs = 0;
char ramdisk = 0;

char mount_name[]   = "FS_MOUNT";
char open_name[]    = "FS_OPENCREATE";
char mkdir_name[]   = "FS_MKDIR";
char attach_name[]  = "FS_ATTACH";
char write_name[]   = "FS_WRITE";
char close_name[]   = "FS_CLOSE";
char chgfileptr_name[] = "FS_CHGFILEPTR";

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

long dummy = 0;

struct mfsdata {
  char buf[128];
} mfsdata;

struct ripl {
  unsigned long long sig;
} ripl;

char fs_module[12];

char boot[0x800];
char save_map[0x80]; // open files indicate area
struct save_item save_area[0x80];
struct save_item *save_pos;
int save_index = 0;

extern struct devcaps devcaps;
extern char volchars[0x20];
extern struct vpfsi far *pVPfsi;
extern struct vpfsd far *pVPfsd;

#pragma pack(1)

struct p48
{
  unsigned long  off;
  unsigned short sel;
};

#pragma pack()

int _cdecl _loadds GetFlatSelectors(void);
int _cdecl _loadds GetProcAddr(char far *fpszModName,
                               unsigned short cchModName,
                               char far *fpszProcName,
                               unsigned char cchProcName,
                               struct p48 far *fp48EP);
int _cdecl _loadds GetModule(char far *fpszModName,
                             unsigned short cchModName,
                             struct p48 far *fpMte48);
int _cdecl _loadds GetDPBHead(void far * far *addr);

unsigned char drvletter = 0;
unsigned char cd_drvletter = 0;

struct multiboot_info far *mbi_far;
struct mod_list far *mods_addr;
unsigned long mods_count;
unsigned short sel;
unsigned short sel1;
unsigned short mods_sel;

unsigned long filemax;
unsigned long filepos;
unsigned long filebase;
char far *fileaddr;

// our command line
char cmdline[0x400];

#pragma aux mbi0             "*"
#pragma aux FS_NAME          "*"
#pragma aux FlatR0CS         "*"
#pragma aux FlatR0DS         "*"
#pragma aux GetFlatSelectors "*"
#pragma aux GetProcAddr      "*"
#pragma aux GetModule        "*"
#pragma aux GetDPBHead       "*"
#pragma aux FS_ATTRIBUTE     "*"

void far *fmemset (void far *start, int c, int len);
int fstrlen (const char far *str);
void far *fmemmove (void far *_to, const void far *_from, int _len);
char far *fstrcpy (char far *dest, const char far *src);
int toupper (int c);
int tolower (int c);
int isspace (int c);
char far *strip(char far *s);
char far *fstrstr (const char far *s1, const char far *s2);
char *strstr (const char *s1, const char *s2);
char *skip_to (int after_equal, char *cmdline);
int safe_parse_maxint (char **str_ptr, long *myint_ptr);
int strlen (const char *str);
int strcmp (const char *s1, const char *s2);
char *strcpy (char *dest, const char *src);
void *memmove (void *_to, const void *_from, int _len);
void *memset (void *start, int c, int len);

int far pascal MFSH_INTERR(char far *pcMsg, unsigned short cbMsg);
int far pascal MFSH_SETBOOTDRIVE(unsigned short usDrive);
int far pascal MFSH_PHYSTOVIRT(unsigned long ulAddr,
                               unsigned short usLen,
                               unsigned short far *pusSel);
int far pascal MFSH_UNPHYSTOVIRT(unsigned short usSel);
void far pascal FSH_GETVOLPARM(unsigned short hVPB,
                               struct vpfsi far * far *ppVPBfsi,
                               struct vpfsd far * far *ppVPBfsd);
int far pascal MFSH_SEGFREE(unsigned short usSel);

void far pascal FSH_INTERR(char far *pMsg,
                           unsigned short cbMsg);
int far pascal FSH_DOVOLIO(unsigned short operation,
                           unsigned short fAllowed,
                           unsigned short hVPB,
                           char far *pData,
                           unsigned short far *pcSec,
                           unsigned long iSec);
int far pascal FSH_FINDDUPHVPB(unsigned short hVPB,
                               unsigned short far *phVPB);

typedef int far pascal (*mount_t)(unsigned short flag,
                                  struct vpfsi far *pvpfsi,
                                  struct vpfsd far *pvpfsd,
                                  unsigned short hVPB,
                                  char far *pBoot);
typedef int far pascal (*open_t)(struct cdfsi far *pcdfsi,
                                 struct cdfsd far *pcdfsd,
                                 char far *pName,
                                 unsigned short iCurDirEnd,
                                 struct sffsi far *psffsi,
                                 struct sffsd far *psffsd,
                                 unsigned long ulOpenMode,
                                 unsigned short usOpenFlag,
                                 unsigned short far *pusAction,
                                 unsigned short usAttr,
                                 char far *pcEABuf,
                                 unsigned short far *pfgenflag);
typedef int far pascal (*attach_t)(unsigned short flag,
                                 char far *pDev,
                                 struct vpfsd far *pvpfsd,
                                 struct cdfsd far *pcdfsd,
                                 char far *pParm,
                                 unsigned short far *p);
typedef int far pascal (*write_t)(struct sffsi far *psffsi,
                                 struct sffsd far *psffsd,
                                 char far *pData,
                                 unsigned short far *pLen,
                                 unsigned short IOfl€);
typedef int far pascal (*close_t)(unsigned short type,
                                  unsigned short IOflag,
                                  struct sffsi far *psffsi,
                                  struct sffsd far *psffsd);
typedef int far pascal (*mkdir_t)(struct cdfsi far * pcdfsi,
                                  struct cdfsd far *pcdfsd,
                                  char far *pName,
                                  unsigned short iCurDirEnd,
                                  char far *pEABuf,
                                  unsigned short fl);
typedef int far pascal (*chgfileptr_t)(struct sffsi far * psffsi,
                                  struct sffsd far * psffsd,
                                  long offset,
                                  unsigned short type,
                                  unsigned short IOfl);

void serout(char *s);
int kprintf(const char *format, ...);

char *lastpos(const char *s1, const char *s2)
{
  const char *s = s1 + strlen(s1) - strlen(s2);

  while (s >= s1)
    {
      const char *ptr, *tmp;

      ptr = s;
      tmp = s2;

      while (*tmp && *ptr == *tmp)
        ptr++, tmp++;

      if (tmp > s2 && ! *tmp)
        return (char *) s;

      s--;
    }

  return 0;
}

// refresh fileaddr value with
// filepos change
void advance_ptr(void)
{
    MFSH_UNPHYSTOVIRT(sel1);

    // convert file read point phys addr to char far *
    MFSH_PHYSTOVIRT(filebase + filepos, 0xffff, &sel1);
    fileaddr = (char far *)MAKEP(sel1, 0);
}

int far pascal _loadds MFS_CHGFILEPTR(
    long  offset,                       /* offset       */
    unsigned short type                 /* type         */
)
{
  kprintf("**** MFS_CHGFILEPTR\n");

  switch (type)
  {
  case 0:
    filepos = offset;
    break;
  case 1:
    filepos += offset;
    break;
  case 2:
    filepos = filemax + offset;
    break;
  default:
    return 1;
  }
  advance_ptr();

  return NO_ERROR;
}

int far pascal _loadds MFS_INIT(
    void far *bootdata,                 /* bootdata     */
    char far *number,                   /* number io    */
    struct ripl far * far *vectorripl,  /* vectorripl   */
    void far *bpb,                      /* bpb          */
    unsigned long far *pMiniFSD,        /* pMiniFSD     */
    unsigned long far *dump             /* dump address */
) {
    int i, rc;
    struct mod_list far *mod;
    unsigned short sl, sl1;
    char far *p, far *q;
    long port = 0x3f8;
    long speed = 9600;
    char *pp, *r;
    char panic_msg[] = "MBI:mbi uninitialized, panic!\n";
    unsigned short selector;

    //__asm {
    //  int 3
    //}

    //if (!bpb) MFSH_INTERR("bpb\n", 4);

    // mbi as RIPL data
    mbi = *((unsigned long far *)bootdata);
    //mbi = *((unsigned long far *)((char far *)bpb + 0x20));
    //mbi = mbi0;

    // free a segment used for RIPL data
    if (bootdata) MFSH_SEGFREE(SELECTOROF(bootdata));

    if (!mbi)
      MFSH_INTERR(panic_msg, strlen(panic_msg));

    // get GDT selector to mbi structure
    rc = MFSH_PHYSTOVIRT(mbi, sizeof(struct multiboot_info), &sel);
    CHECKRC
    mbi_far = (struct multiboot_info far *)MAKEP(sel, 0);

    if (!mbi_far->cmdline) MFSH_INTERR(panic_msg, strlen(panic_msg));

    rc = MFSH_PHYSTOVIRT(mbi_far->cmdline, 0xffff, &sl1);
    CHECKRC
    q = (char far *)MAKEP(sl1, 0);
    memmove(cmdline, q, strlen(q));
    MFSH_UNPHYSTOVIRT(sl1);

    if (mbi_far->flags & MB_INFO_CMDLINE)
    {
      // if we're booting from a ramdisk
      if (pp = strstr(cmdline, "--ramdisk-boot"))
      {
        ramdisk  = 1;
        remotefs = 1;
      }
    // if we're booting from remote fs
      if (pp = strstr(cmdline, "--remote-fs"))
      {
        remotefs = 1;
      }

      // if debug output is on
      if (pp = strstr(cmdline, "--debug"))
      {
        debug = 1;
      }

      // if "--serial=..." specified on the command line
      if (pp = strstr(cmdline, "--port"))
      {
        pp = skip_to(1, pp);
        safe_parse_maxint(&pp, &port);
      }

      if (pp = strstr(cmdline, "--speed"))
      {
        pp = skip_to(1, pp);
        safe_parse_maxint(&pp, &speed);
      }

      if (pp = strstr(cmdline, "--fs"))
      {
        pp = skip_to(1, pp);
        // find name end
        for (r = pp; *r && *r != ' '; r++) ;
        memmove(FS_NAME, pp, r - pp);
        FS_NAME[r - pp] = '\0';
      }

      if (pp = strstr(cmdline, "--module"))
      {
        pp = skip_to(1, pp);
        // find name end
        for (r = pp; *r && *r != ' '; r++) ;
        memmove(fs_module, pp, r - pp);
        fs_module[r - pp] = '\0';
        // make name uppercase
        r = fs_module;
        while (*r) *r++ = toupper(*r);
      }

      if (pp = strstr(cmdline, "--drv"))
      {
        pp = skip_to(1, pp);
        drvletter = toupper(pp[0]);
      }

      //if (pp = strstr(cmdline, "--cd"))
      //{
      //  pp = skip_to(1, pp);
      //  cd_drvletter = toupper(pp[0]);
      //}
    }

    //debug = 1;
    //port = 0x3f8;
    //speed = 115200;
    //strcpy(FS_NAME, "CDFS");
    //strcpy(fs_module, "CDFSD");
    //drvletter = 'C';

    memset(&mfsdata, 0, sizeof(struct mfsdata));
    *pMiniFSD = (unsigned long)&mfsdata;

    if (!strcmp(FS_NAME, "JFS") ||
        !strcmp(FS_NAME, "FAT32"))
      **((unsigned long far * far *)pMiniFSD) = 0x1961;

    if (remotefs)
    {
      // if booting from a remote drive, use remote boot
      // attach one remote drive
      *number = 1;
      FS_ATTRIBUTE  |= 0x1;
    }

    // init serial port
    serial_init(port, speed, UART_8BITS_WORD, UART_NO_PARITY, UART_1_STOP_BIT);

    kprintf("**** MFS_INIT\n");
    kprintf("Hello MBI minifsd!\n");

    __asm {
      mov  ax, cs
      mov  selector, ax
    }
    p = (char far *)MAKEP(selector, 0);
    for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *(p + 0x1386 + i));
    //for (i = 0; i < 0x2b36; i++) kprintf("0x%02x,", *(p + i));
    kprintf("\n");

    kprintf("comport = 0x%x\n", port);
    kprintf("ifs: %s\n", FS_NAME);
    kprintf("drive letter: %c\n", drvletter);
    kprintf("RIPL data address: 0x%04x:0x%04x\n",
            ((unsigned long)bootdata >> 16),
            ((unsigned long)bootdata & 0xffff));
    kprintf("mbi phys. address: 0x%08lx\n", mbi);
    kprintf("allocated mbi selector: 0x%04x\n", sel);
    kprintf("mem_lower = %lu Kb, mem_upper = %lu Mb\n",
            mbi_far->mem_lower, mbi_far->mem_upper >> 10);
    kprintf("boot_device = 0x%08lx\n", mbi_far->boot_device);
    kprintf("kernel cmdline @0x%08lx\n", mbi_far->cmdline);
    kprintf("cmdline = %s\n", cmdline);
    kprintf("mods_count = %lu, mods_addr = 0x%08lx\n",
            mbi_far->mods_count, mbi_far->mods_addr);
    kprintf("syms = 0x%lx\n", mbi_far->syms);
    kprintf("mmap_length = 0x%lu, mmap_addr = 0x%08lx\n",
            mbi_far->mmap_length, mbi_far->mmap_addr);
    kprintf("drives_length = 0x%lu, drives_addr = 0x%08lx\n",
            mbi_far->drives_length, mbi_far->drives_addr);
    kprintf("config_table = 0x%08lx\n", mbi_far->config_table);
    kprintf("boot_loader_name @0x%08lx\n",
            mbi_far->boot_loader_name);

    rc = MFSH_PHYSTOVIRT(mbi_far->mods_addr, sizeof(struct mod_list) * mbi_far->mods_count, &mods_sel);
    CHECKRC
    kprintf("allocated mods selector: 0x%04x\n", mods_sel);
    mods_addr  = MAKEP(mods_sel, 0);
    mods_count = mbi_far->mods_count;
    mod = (struct mod_list far *)mods_addr;
    kprintf("mods cmd lines: \n");

    for (i = 0; i < mods_count; i++, mod++)
    {
      rc = MFSH_PHYSTOVIRT(mod->cmdline, 0xffff, &sl);
      CHECKRC
      p = MAKEP(sl, 0);
      kprintf("%u: %s\n", i, p);
      MFSH_UNPHYSTOVIRT(sl);
    }

    rc = MFSH_SETBOOTDRIVE(drvletter - 'A'); // t:
    kprintf("MFSH_SETBOOTDRIVE() returned: 0x%x\n", rc);

    //*FS_NAME = 0;

    __asm {
      mov  ax, cs
      mov  selector, ax
    }
    p = (char far *)MAKEP(selector, 0);
    for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *(p + 0x1386 + i));
    kprintf("\n");

    return NO_ERROR;
}

int far pascal _loadds MFS_OPEN(
    char far *name,                     /* name         */
    long far *size             /* size         */
)
{
    char far buf1[0x100];
    char far buf2[0x100];
    struct mod_list far *mod;
    char far *p, far *q, far *l;
    char far *s;
    int  n, rc;

    kprintf("**** MFS_OPEN(\"%s\")", name);

    memset(buf1, 0, sizeof(buf1));
    memset(buf2, 0, sizeof(buf2));

    // get GDT selector to mbi structure
    rc = MFSH_PHYSTOVIRT(mbi, sizeof(struct multiboot_info), &sel);
    CHECKRC
    mbi_far = (struct multiboot_info far *)MAKEP(sel, 0);

    // get mods selector
    rc = MFSH_PHYSTOVIRT(mbi_far->mods_addr, sizeof(struct mod_list) * mbi_far->mods_count, &mods_sel);
    CHECKRC
    mods_addr  = MAKEP(mods_sel, 0);

    if (mbi_far->flags & MB_INFO_MODS) // If there are modules
    {
      mod = mods_addr;

      // search for a given filename
      for (n = 0; n < mods_count; n++)
      {
        // convert module cmdline phys addr to char far *
        rc = MFSH_PHYSTOVIRT(mod->cmdline, 0xffff, &sel1);
        CHECKRC
        s = (char far *)MAKEP(sel1, 0);

        // copy to buffers
        strcpy(buf1, s);
        strcpy(buf2, name);

        // deallocate a selector
        MFSH_UNPHYSTOVIRT(sel1);

        // translate '/' to '\' in command line
        for (p = buf1; *p; p++) if (*p == '/') *p = '\\';

        // make it uppercase
        for (p = buf1; *p; p++) *p = toupper(*p);
        for (p = buf2; *p; p++) *p = toupper(*p);

        p = strip(buf1); q = strip(buf2);

        // skip a disk/partition
        if (*p == '(')
        {
          while (*p && *p != ')') p++;
          p++;
        }

        // skip a driveletter
        if (q[1] == ':') q += 2;
        if (*q == '\\')  q++;
        if (*p == '\\')  p++;

        l = lastpos(p, q);
        if (l && ((p + strlen(p)) == (l + strlen(q))) && (l == p || l[-1] == ' '))
            break;

        mod++;
      };

      // we have gone through all mods, and no given filename
      if (n == mods_count)
      {
        kprintf(" failed!\n");
        return ERROR_FILE_NOT_FOUND;
      }

      // filename found
      filepos  = 0;
      filemax  = mod->mod_end - mod->mod_start;

      // convert module cmdline phys addr to char far *
      rc = MFSH_PHYSTOVIRT(mod->mod_start, 0xffff, &sel1);
      CHECKRC
      s = (char far *)MAKEP(sel1, 0);

      fileaddr = s;
      filebase = mod->mod_start;

      *size = filemax;
      kprintf(" succeeded\n");
      return NO_ERROR;
    }

  kprintf(" failed!\n");
  return ERROR_FILE_NOT_FOUND;
}

int far pascal _loadds MFS_READ(
    char far *data,             /* data         */
    unsigned short far *length   /* length       */
)
{
  kprintf("**** MFS_READ(0x%04x:0x%04x, ",
          (unsigned long)data >> 16,
          (unsigned long)data & 0xffff);
  kprintf("%u)\n", *length);

  if (fileaddr && data && *length)
  {
    memmove(data, fileaddr, *length);
    filepos += *length;
  }
  else
    return 1;

  advance_ptr();

  return NO_ERROR;
}

int far pascal _loadds MFS_CLOSE(void) {
    kprintf("**** MFS_CLOSE\n");

    if (sel1)     MFSH_UNPHYSTOVIRT(sel1);
    if (sel)      MFSH_UNPHYSTOVIRT(sel);

    return NO_ERROR;
}

int getaddr (char *module,
             char *funcname,
             unsigned long *funcaddr)
{
  struct p48 p48;
  unsigned short sel;

  if (!GetProcAddr(module,
                   strlen(module),
                   funcname,
                   strlen(funcname),
                   &p48))
  {
    sel = p48.sel;

    //if (sel != FlatR0CS) // 16:16
    //{
      *funcaddr = (unsigned short)p48.off | ((unsigned long)sel << 16);
    //}
    //else
    //{
    //  funcaddr->sel = sel;
    //  funcaddr->off = p48.off;
    //}

    kprintf("IFS %s addr: 0x%04x:0x%08lx\n",
            funcname,
            p48.sel,
            p48.off);

    return 0;
  }

  kprintf("IFS %s addr not found.\n", funcname);
  return 1;
}


int far pascal _loadds MFS_TERM(void)
{
  open_t       p_open;
  mount_t      p_mount;
  attach_t     p_attach;
  write_t      p_write;
  close_t      p_close;
  mkdir_t      p_mkdir;
  chgfileptr_t p_chgfileptr;
  //unsigned char drv;
  struct vpfsi far *pvpfsi = 0;
  struct vpfsd far *pvpfsd = 0;
  struct cdfsd far *pcdfsd = 0;
  struct cdfsi far *pcdfsi = 0;
  struct sffsi far *psffsi = 0;
  struct sffsd far *psffsd = 0;
  int rc, i, j;
  unsigned long  ulOpenMode;
  unsigned short usOpenFlag;
  unsigned short hVPB1;
  unsigned short usAction;
  unsigned short flags;
  unsigned short l;
  unsigned long addr, len, written, length;
  unsigned short sel, msel;
  char far *data;
  char msg_errmount[] = "Error mounting/attaching the disk!\n";
  char msg_erropen[] = "Error reopening file!\n";
  char msg_errwrite[] = "Error writing to a file!\n";
  char msg_errclose[] = "Error closing file!\n";
  char msg_errptr[] = "Error changing file ptr!\n";
  struct vpfsi vpfsi;
  struct vpfsd vpfsd;
  struct cdfsi cdfsi;
  struct cdfsd cdfsd;
  struct sffsi sffsi;
  struct sffsd sffsd;
  char far *pBoot = 0;
  char str[0x3a];
  char buf[0x3a];
  char fs_name[12];
  char far *r, far *p, far *s, far *q;
  void far *dpbhead;
  unsigned short new_hVPB;
  char opened;
  struct dpb far *pdpb1 = 0;
  struct mod_list far *mod;

  kprintf("**** MFS_TERM\n");
  kprintf("hello stage3!\n");

  //rc = MFSH_SETBOOTDRIVE('U' - 'A'); // u:
  //kprintf("MFSH_SETBOOTDRIVE() returned: 0x%x\n", rc);

  // Get FLAT selectors
  if (!GetFlatSelectors())
    kprintf("FlatR0CS = 0x%x, FlatR0DS = 0x%x\n", FlatR0CS, FlatR0DS);
  else
    kprintf("Flat selectors not found.\n");

  if (remotefs) // booting from remote fs
  {
    // Get FS_ATTACH address
    if (getaddr(fs_module,
                attach_name,
                (unsigned long *)(&p_attach))    ||
        getaddr(fs_module,
                close_name,
                (unsigned long *)(&p_close))     ||
        getaddr(fs_module,
                mkdir_name,
                (unsigned long *)(&p_mkdir))     ||
        getaddr(fs_module,
                write_name,
                (unsigned long *)(&p_write))     ||
        getaddr(fs_module,
                chgfileptr_name,
                (unsigned long *)(&p_chgfileptr)))
      return 1;

  }
  else
  {
    // Get FS_MOUNT address
    if (getaddr(fs_module,
                mount_name,
                (unsigned long *)(&p_mount)))
      return 1;
  }
  // Get FS_OPENCREATE address
  if (getaddr(fs_module,
              open_name,
              (unsigned long *)(&p_open)))
    return 1;

  kprintf("p_mount = 0x%08lx\n",  p_mount);
  kprintf("p_open  = 0x%08lx\n",  p_open);
  kprintf("p_attach = 0x%08lx\n", p_attach);
  kprintf("p_write  = 0x%08lx\n", p_write);
  kprintf("p_close  = 0x%08lx\n", p_close);
  kprintf("p_mkdir  = 0x%08lx\n", p_mkdir);
  kprintf("p_chgfileptr = 0x%08lx\n", p_chgfileptr);

  kprintf("hard disk partition hVPB: 0x%04x\n", hVPB);

  hVPB1 = hVPB;
  FSH_GETVOLPARM(hVPB, &pvpfsi, &pvpfsd);

  GetDPBHead(&dpbhead);
  kprintf("dpbhead = 0x%08lx\n", dpbhead);

  pdpb = (struct dpb far *)dpbhead;

  while (1)
  {
    kprintf("drive: %c:, pdpb = 0x%08lx, ", 'a' + pdpb->dpb_drive, pdpb);
    kprintf("hVPB: 0x%04x\n", pdpb->dpb_hVPB);
    if (pdpb->dpb_drive == drvletter - 'A')  pdpb1 = pdpb;
    //if (pdpb->dpb_drive == 'v' - 'a')  pdpb1 = pdpb;
    if ((int)pdpb->dpb_next_dpb == -1) break;
    pdpb = pdpb->dpb_next_dpb;
  }

  pdpb = pdpb1;
  if (pdpb)
  {
    kprintf("pdpb = 0x%08lx\n", pdpb);
    hVPB = pdpb->dpb_hVPB;
    cd_drvletter = 'A' + pdpb->dpb_drive;
  }
  else
    cd_drvletter = drvletter;

  kprintf("boot drive: %c:, its hVPB: 0x%04x\n", cd_drvletter, hVPB);

  //if (!FSH_FINDDUPHVPB(hVPB, &new_hVPB)) hVPB = new_hVPB;
  //kprintf("hVPB: 0x%04x\n", hVPB);

  FSH_GETVOLPARM(hVPB, &pvpfsi, &pvpfsd);
  if (drvletter == cd_drvletter)  // if booting from a harddrive
  {
    pvpfsi->vpi_pDCS = &devcaps;
    pvpfsi->vpi_pVCS = &volchars;
    pBoot = boot;
  }

  kprintf("vpi_pDCS = 0x%08lx, vpi_pVCS = 0x%08lx\n",
          pvpfsi->vpi_pDCS,
          pvpfsi->vpi_pVCS);

  memset(pvpfsd, 0, sizeof(struct vpfsd));

  //FS_ATTRIBUTE = 1;
  //c = pvpfsi1->vpi_unit;
  //d = pvpfsi1->vpi_drive;
  //memmove(pvpfsi1, pvpfsi, sizeof(struct vpfsi));
  //pvpfsi1->vpi_unit  = c;
  //pvpfsi1->vpi_drive = d;
  //pdpb->dpb_unit  = c;
  //pdpb->dpb_drive = d;
  //pdpb->dpb_hVPB  = hVPB1;
  //strcpy(pvpfsi1->vpi_text, pvpfsi->vpi_text);

  ////pdpb->dpb_drive   = 'c' - 'a';
  ////pvpfsi->vpi_drive = 'c' - 'a';

  kprintf("vpi_vid    = 0x%08lx\n", pvpfsi->vpi_vid);
  kprintf("vpi_hDEV   = 0x%08lx\n", pvpfsi->vpi_hDEV);
  kprintf("vpi_bsize  = 0x%04x\n", pvpfsi->vpi_bsize);
  kprintf("vpi_totsec = 0x%08lx\n", pvpfsi->vpi_totsec);
  kprintf("vpi_trksec = 0x%04x\n", pvpfsi->vpi_trksec);
  kprintf("vpi_nhead  = 0x%04x\n", pvpfsi->vpi_nhead);
  kprintf("vpi_text   = %s\n", pvpfsi->vpi_text);
  kprintf("vpi_pDCS   = 0x%08lx\n", pvpfsi->vpi_pDCS);
  kprintf("vpi_pVCS   = 0x%08lx\n", pvpfsi->vpi_pVCS);
  kprintf("vpi_drive  = %c:\n", 'a' + pvpfsi->vpi_drive);
  kprintf("vpi_unit   = %01u\n", pvpfsi->vpi_unit);

  if (remotefs)
  {
    // call the FS_ATTACH entry point of the IFS
    kprintf("ifs FS_ATTACH()");
    str[0] = drvletter; str[1] = ':'; str[2] = '\0';
    rc = (*p_attach)(0, str, pvpfsd, pcdfsd, 0, 0);

/*
    __asm {
    .386p
      push  ds
      push  si
      push  bx

      xor   ax, ax
      push  ax            // flag
      lds   si, p
      push  ds            // str seg
      push  si            // str offset
      lds   si, pvpfsd
      push  ds            // pvpfsd seg
      push  si            // pvpfsd offset
      lds   si, pcdfsd
      push  ds            // pcdfsd seg
      push  si            // pcdfsd offset
      push  ax
      push  ax
      push  ax
      push  ax

      lea   si, p_attach
      mov   bx, word ptr ss:[si + 4]
      test  bx, bx
      jz    @@skip
      call  fword ptr ss:[si]
      jmp   @@exit
    @@skip:
      call  dword ptr ss:[si]

    @@exit:
      mov   rc, ax

      pop   bx
      pop   si
      pop   ds
    }
*/

    //rc = (*p_attach)(0, str, pvpfsd, pcdfsd, 0, 0);
    if (rc)
    {
      kprintf(" failed, rc = %u\n", rc);
      FSH_INTERR(msg_errmount, strlen(msg_errmount));
    }
    else
      kprintf(" = %u\n", rc);
  }
  else
  {
    // call the FS_MOUNT entry point of the IFS:
    kprintf("ifs FS_MOUNT()");

    rc = (*p_mount)(0, pvpfsi, pvpfsd, hVPB, boot);

/*
    __asm {
    .386p
      push  ds
      push  si
      push  bx

      xor   ax, ax
      push  ax            // flag
      lds   si, pvpfsi
      push  ds            // pvpfsi seg
      push  si            // pvpfsi offset
      lds   si, pvpfsd
      push  ds            // pvpfsd seg
      push  si            // pvpfsd offset
      mov   bx, hVPB
      push  bx            // hVPB
      lds   si, boot
      push  ds            // boot seg
      push  si            // boot offset

      lea   si, p_mount
      mov   bx, word ptr ss:[si + 4]
      test  bx, bx
      jz    @@skip
      call  fword ptr ss:[si]
      jmp   @@exit
    @@skip:
      call  dword ptr ss:[si]

    @@exit:
      mov   rc, ax

      pop   bx
      pop   si
      pop   ds
    }
*/

    //rc = (*p_mount)(0, pvpfsi, pvpfsd, hVPB, boot);
    if (rc)
    {
      kprintf(" failed, rc = %u\n", rc);
      FSH_INTERR(msg_errmount, strlen(msg_errmount));
    }
    else
      kprintf(" = %u\n", rc);
  }


  if (ramdisk)
  {
    for (i = 0, mod = (struct mod_list far *)mods_addr;
         i < mods_count; i++, mod++)
    {
      addr = mod->mod_start;
      len  = mod->mod_end - mod->mod_start;

      rc = MFSH_PHYSTOVIRT(addr, 0xffff, &msel);
      CHECKRC
      data = MAKEP(msel, 0);

      rc = MFSH_PHYSTOVIRT(mod->cmdline, 0xffff, &sel);
      CHECKRC
      s = MAKEP(sel, 0);

      strcpy(str, s);
      s = str;

      /* skip (...,...) */
      if (*s == '(')
      {
        s++;
        for (; *s && *s != ')'; s++) ;
        if (*s) s++;
      }

      /* change '/' to '\\' and make a file name lowercase */
      for (r = s; *r; r++)
      {
        if (*r == '/') *r = '\\';
        *r = tolower(*r);
      }

      /* search for a space in command line */
      for (r = s; *r && !isspace(*r); r++) ;

      if (*r) /* space found */
      {
        r++; /* skip it */
        s = r;
      }

      p = strstr(s, "\\");
      if (!p || p - s > 2)
      {
        /* no slash or no slash at the beginning */
        s -= 3;
        s[0] = tolower(drvletter);
        s[1] = ':';
        s[2] = '\\';
      }
      else if (p[-1] != ':')
      {
        s = p - 2;
        s[0] = tolower(drvletter);
        s[1] = ':';
      }
      else
        s = p - 2;

      // zero out cdfsi buffer
      memset(&cdfsi, 0, sizeof(struct cdfsi));
      memset(&cdfsd, 0, sizeof(struct cdfsd));
      cdfsi.cdi_hVPB = hVPB;

      memset(buf, 0, sizeof(buf));
      /* create parent dirs for s */
      for (j = 0, r = s; *r; j++, r++)
      {
        /* get 1st level dir */
        for (; *r && *r != '\\'; j++, r++) buf[j] = *r;
        if (!*r)
          break;
        else /* ended with slash */
        {
          buf[j] = '\0';

          /* skip "d:\" */
          if (r[-1] == ':')
          {
            buf[j] = *r;
            continue;
          }

          kprintf("ifs FS_MKDIR(\"%s\")", buf);
          p = (char far *)buf;
          pcdfsi = &cdfsi;
          pcdfsd = &cdfsd;

          rc = (*p_mkdir)(pcdfsi, pcdfsd, buf, -1, 0, 0);

/*
          __asm {
         .386p
            push  ds
            push  si
            push  bx

            xor   ax, ax
            lds   si, pcdfsi
            push  ds          // pcdfsi seg
            push  si          // pcdfsi offset
            lds   si, pcdfsd
            push  ds          // pcdfsd seg
            push  si          // pcdfsd offset
            lds   si, p
            push  ds          // buf seg
            push  si          // buf offset
            mov   bx, -1
            push  bx
            push  ax
            push  ax
            push  ax

            lea   si, p_mkdir
            mov   bx, word ptr ss:[si + 4]
            test  bx, bx
            jz    @@skip
            call  fword ptr ss:[si]
            jmp   @@exit
          @@skip:
            call  dword ptr ss:[si]

          @@exit:
            mov   rc, ax

            pop   bx
            pop   si
            pop   ds
          }
*/

          //rc = (*p_mkdir)(&cdfsi, &cdfsd, buf, -1, 0, 0);
          if (rc)
            kprintf(" failed, rc = %u\n", rc);
          else
            kprintf(" = %u\n", rc);

          buf[j] = *r;
        }
      }

      // zero out cdfsi buffer
      memset(&cdfsi, 0, sizeof(struct cdfsi));
      memset(&cdfsd, 0, sizeof(struct cdfsd));
      memset(&sffsi, 0, sizeof(struct sffsi));
      memset(&sffsd, 0, sizeof(struct sffsd));
      cdfsi.cdi_hVPB = hVPB;
      sffsi.sfi_hVPB = hVPB;

      /* determine whether this file is opened by minifsd */
      opened = 0;
      for (save_index = 0; save_index < 0x80; save_index++)
      {
        if (save_map[save_index])
        {
          save_pos = save_area + save_index;
          // find the first difference
          for (p = save_pos->pName, r = s; *p && *r && *p == *r; p++, r++) ;
          // if they're equal
          if (!*p && !*r)
          {
            opened = 1;
            break;
          }
        }
      }

      // reopen by an IFS all files open by minifsd
      if (opened)
      {
        // zero out cdfsi buffer
        // change hVPB of 'fake bootdrive' to hVPB of a real one
        save_pos->psffsi->sfi_hVPB = hVPB;

        strcpy(str, save_pos->pName);
        // change a 'fake bootdrive' drv letter to a CD drv letter
        if (cd_drvletter && *str == drvletter) *str = cd_drvletter;
        kprintf("ulOpenMode: 0x%08lx, usOpenFlag: 0x%04x\n",
                save_pos->ulOpenMode, // & ~(OPEN_ACCESS_READWRITE | OPEN_ACCESS_WRITEONLY),
                save_pos->usOpenFlag);

        p = (char far *)str;
        psffsi = save_pos->psffsi;
        psffsd = save_pos->psffsd;
        ulOpenMode = save_pos->ulOpenMode;
      }
      else
      {
        sffsi.sfi_mode = OPEN_ACCESS_WRITEONLY | OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE | OPEN_FLAGS_NOINHERIT;

        p = (char far *)s;
        psffsi = &sffsi;
        psffsd = &sffsd;
        ulOpenMode = sffsi.sfi_mode;
      }

      kprintf("ifs FS_OPENCREATE(\"%s\")", p);

      rc = (*p_open)(&cdfsi,
                     &cdfsd,
                     p,
                     -1, // not 0 -- needed by ext2_os2.ifs
                     psffsi,
                     psffsd,
                     ulOpenMode,
                     OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                     &usAction,             //dummy address
                     0,
                     0,
                     &flags);

/*
      __asm {
      .386p
        push  ds
        push  si
        push  ebx

        xor   ax,  ax

        lds   si,  pcdfsi
        push  ds                  // pcdfsi seg
        push  si                  // pcdfsi offset
        lds   si,  pcdfsd
        push  ds                  // pcdfsd seg
        push  si                  // pcdfsd offset
        lds   si,  p
        push  ds                  // str seg
        push  si                  // str offset
        mov   bx,  -1
        push  bx                  // iCurDirEnd
        lds   si,  psffsi
        push  ds                  // savepos->psffsi seg
        push  si                  // savepos->psffsi offset
        lds   si,  psffsd
        push  ds                  // savepos->psffsd seg
        push  si                  // savepos->psffsd offset

        mov   ebx, ulOpenMode
        push  ebx                 // ulOpenMode
        mov   bx,  usOpenFlag
        push  bx                  // usOpenFlag
        lds   si,  r
        push  ds                  // pusAction seg
        push  si                  // pusAction offset
        push  ax                  // 0
        push  ax                  // 0
        push  ax                  // 0
        lds   si,  q
        push  ds                  // &flags seg
        push  si                  // &flags offset

        lea   si, p_open
        mov   bx, word ptr ss:[si + 4]
        test  bx, bx
        jz    @@skip
        call  fword ptr ss:[si]
        jmp   @@exit
      @@skip:
        call  dword ptr ss:[si]

      @@exit:
        mov   rc, ax

        pop   ebx
        pop   si
        pop   ds
      }
*/

      if (!rc)
        kprintf(" = %u\n", rc);
      else
      {
        kprintf(" failed, rc = %u\n", rc);
        FSH_INTERR(msg_erropen, strlen(msg_erropen));
      }

      kprintf("ifs FS_CHGFILEPTR()");

      if (opened)
      {
        psffsi = save_pos->psffsi;
        psffsd = save_pos->psffsd;
      }
      else
      {
        psffsi = &sffsi;
        psffsd = &sffsd;
      }

      rc = (*p_chgfileptr)(psffsi, psffsd, 0, 0, 0);

/*
      __asm {
      .386p
        push  ds
        push  si
        push  bx

        xor   ax,  ax

        lds   si, psffsi
        push  ds                  // psffsi seg
        push  si                  // psffsi offset
        lds   si, psffsd
        push  ds                  // psffsd seg
        push  si                  // psffsd offset

        push  ax
        push  ax
        push  ax
        push  ax

        lea   si, p_chgfileptr
        mov   bx, word ptr ss:[si + 4]
        test  bx, bx
        jz    @@skip
        call  fword ptr ss:[si]
        jmp   @@exit
      @@skip:
        call  dword ptr ss:[si]

      @@exit:
        mov   rc, ax

        pop   bx
        pop   si
        pop   ds
      }
*/

      if (rc)
      {
        kprintf(" failed, rc = %u\n", rc);
        FSH_INTERR(msg_errptr, strlen(msg_errptr));
      }
      kprintf(" = %u\n", rc);

      length = len;
      for (;;)
      {
        l = 0x8000;   /* write portion */
        if (length < l)
          l = length;
        kprintf("ifs FS_WRITE()");

        if (opened)
        {
          psffsi = save_pos->psffsi;
          psffsd = save_pos->psffsd;
        }
        else
        {
          psffsi = &sffsi;
          psffsd = &sffsd;
        }
        p = (char far *)&l;

        rc = (*p_write)(psffsi, psffsd, data, &l, 0);

/*
        __asm {
        .386p
          push  ds
          push  si
          push  bx

          xor   ax,  ax

          lds   si, psffsi
          push  ds                  // psffsi seg
          push  si                  // psffsi offset
          lds   si, psffsd
          push  ds                  // psffsd seg
          push  si                  // psffsd offset
          lds   si, data
          push  ds                  // data seg
          push  si                  // data offset
          lds   si, p
          push  ds                  // &l seg
          push  si                  // &l offset
          push  ax

          lea   si, p_write
          mov   bx, word ptr ss:[si + 4]
          test  bx, bx
          jz    @@skip
          call  fword ptr ss:[si]
          jmp   @@exit
        @@skip:
          call  dword ptr ss:[si]

        @@exit:
          mov   rc, ax

          pop   bx
          pop   si
          pop   ds
        }
*/

        if (rc)
        {
          kprintf(" failed, rc = %u\n", rc);
          FSH_INTERR(msg_errwrite, strlen(msg_errwrite));
        }
        kprintf(" = %u; %u bytes written\n", rc, l);
        addr    += l;
        length  -= l;
        if (length)
        {
          rc = MFSH_UNPHYSTOVIRT(msel);
          CHECKRC
          rc = MFSH_PHYSTOVIRT(addr, 0xffff, &msel);
          CHECKRC
          data = MAKEP(msel, 0);
        }
        else
          break;
      }

      /* if current file is not opened by minifsd, then close it */
      if (!opened)
      {
        kprintf("ifs FS_CLOSE()");
        //rc = (*p_close)(0, 0, &sffsi, &sffsd);

        psffsi = &sffsi;
        psffsd = &sffsd;

        rc = (*p_close)(0, 0, psffsi, psffsd);

/*
        __asm {
        .386p
          push  ds
          push  si
          push  bx

          xor   ax,  ax

          push  ax
          push  ax
          lds   si, psffsi
          push  ds                  // &sffsi seg
          push  si                  // &sffsi offset
          lds   si, psffsd
          push  ds                  // &sffsd seg
          push  si                  // &sffsd offset

          lea   si, p_close
          mov   bx, word ptr ss:[si + 4]
          test  bx, bx
          jz    @@skip
          call  fword ptr ss:[si]
          jmp   @@exit
        @@skip:
          call  dword ptr ss:[si]

        @@exit:
          mov   rc, ax

          pop   bx
          pop   si
          pop   ds
        }
*/

        if (rc)
        {
          kprintf(" failed, rc = %u\n", rc);
          FSH_INTERR(msg_errclose, strlen(msg_errclose));
        }
        kprintf(" = %u\n", rc);
      }

      MFSH_UNPHYSTOVIRT(msel);
      MFSH_UNPHYSTOVIRT(sel);
    }
  }
  else
  {
    kprintf("open_files = %u\n", open_files);

    // reopen by an IFS all files open by minifsd
    for (save_index = 0; save_index < 0x80; save_index++)
    {
      if (save_map[save_index]) // if a file is open
      {
        save_pos = save_area + save_index;
        // zero out cdfsi buffer
        memset(&cdfsi, 0, sizeof(struct cdfsi));
        cdfsi.cdi_hVPB = hVPB;
        // change hVPB of 'fake bootdrive' to hVPB of a real one
        save_pos->psffsi->sfi_hVPB = hVPB;

        strcpy(str, save_pos->pName);
        // change a 'fake bootdrive' drv letter to a CD drv letter
        if (cd_drvletter && *str == drvletter) *str = cd_drvletter;

        kprintf("ulOpenMode: 0x%08lx, usOpenFlag: 0x%04x\n",
                save_pos->ulOpenMode, // & ~(OPEN_ACCESS_READWRITE | OPEN_ACCESS_WRITEONLY),
                save_pos->usOpenFlag);

        kprintf("ifs FS_OPENCREATE(\"%s\")", str);

        rc = (*p_open)(&cdfsi,
                       &cdfsd,
                       str,
                       -1, // not 0 -- needed by ext2_os2.ifs
                       save_pos->psffsi,
                       save_pos->psffsd,
                       save_pos->ulOpenMode,  // & ~(OPEN_ACCESS_READWRITE | OPEN_ACCESS_WRITEONLY),
                       save_pos->usOpenFlag,
                       &usAction,             //dummy address
                       0,
                       0,
                       &flags);

/*
        __asm {
        .386p
          push  ds
          push  si
          push  ebx

          xor   ax,  ax

          lds   si,  pcdfsi
          push  ds                  // pcdfsi seg
          push  si                  // pcdfsi offset
          lds   si,  pcdfsd
          push  ds                  // pcdfsd seg
          push  si                  // pcdfsd offset

          lds   si,  p
          push  ds                  // str seg
          push  si                  // str offset
          mov   bx,  -1
          push  bx                  // iCurDirEnd
          lds   si,  psffsi
          push  ds                  // savepos->psffsi seg
          push  si                  // savepos->psffsi offset
          lds   si,  psffsd
          push  ds                  // savepos->psffsd seg
          push  si                  // savepos->psffsd offset

          mov   ebx, ulOpenMode
          push  ebx                 // ulOpenMode
          mov   bx, usOpenFlag
          push  bx                  // usOpenFlag
          lds   si, r
          push  ds                  // pusAction seg
          push  si                  // pusAction offset
          push  ax                  // 0
          push  ax                  // 0
          push  ax                  // 0
          lds   si, q
          push  ds                  // &flags seg
          push  si                  // &flags offset

          lea   si, p_open
          mov   bx, word ptr ss:[si + 4]
          test  bx, bx
          jz    @@skip
          call  fword ptr ss:[si]
          jmp   @@exit
        @@skip:
          call  dword ptr ss:[si]

        @@exit:
          mov   rc, ax

          pop   ebx
          pop   si
          pop   ds
        }
*/

        if (!rc)
          kprintf(" = %u\n", rc);
        else
        {
          kprintf(" failed, rc = %u\n", rc);
          FSH_INTERR(msg_erropen, strlen(msg_erropen));
        }
      }
    }
  }

  if (mods_sel) MFSH_UNPHYSTOVIRT(mods_sel);

  return NO_ERROR;
}
