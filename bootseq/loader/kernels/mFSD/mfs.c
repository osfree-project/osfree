/*
 *
 *
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>                // From the "Developer Connection Device Driver Kit" version 2.0

#include <mb_info.h>
#include "serial.h"

//#include <string.h>
//#include <ifs.h>

#define CHECKRC \
    if (rc) \
    {       \
      kprintf("MFSH_PHYSTOVIRT() failed, @%s, line #%lu\n", \
              __FILE__, __LINE__); \
      return 1; \
    }

int serial_init (unsigned short port, unsigned int speed,
                int word_len, int parity, int stop_bit_len);

extern unsigned long mbi;
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

#pragma aux mbi "*"

void far *fmemset (void far *start, int c, int len);
int fstrlen (const char far *str);
void far *fmemmove (void far *_to, const void far *_from, int _len);
char far *fstrcpy (char far *dest, const char far *src);
int toupper (int c);
int isspace (int c);
char far *strip(char far *s);
char far *fstrstr (const char far *s1, const char far *s2);
char *strstr (const char *s1, const char *s2);
char *skip_to (int after_equal, char *cmdline);
int safe_parse_maxint (char **str_ptr, int *myint_ptr);

int far pascal MFSH_INTERR(char far *pcMsg, unsigned short cbMsg);
int far pascal MFSH_PHYSTOVIRT(unsigned long ulAddr,
                               unsigned short usLen,
                               unsigned short far *pusSel);
int far pascal MFSH_UNPHYSTOVIRT(unsigned short usSel);

void serout(char *s);
int kprintf(const char *format, ...);

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
    unsigned long  offset,              /* offset       */
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
      filepos = filemax - offset;
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
    long far *vectorripl,               /* vectorripl   */
    void far *bpb,                      /* bpb          */
    unsigned long far *pMiniFSD,        /* pMiniFSD     */
    unsigned long far *dump             /* dump address */
) {
    int i, rc;
    unsigned long  mbi;
    struct mod_list far *mod;
    unsigned short sl, sl1;
    char far *p, far *q;
    int port = 0;
    char *pp;

    // mbi physical address
    mbi = *((unsigned long far *)bootdata);
    // get GDT selector to mbi structure
    rc = MFSH_PHYSTOVIRT(mbi, sizeof(struct multiboot_info), &sel);
    CHECKRC
    mbi_far = (struct multiboot_info far *)MAKEP(sel, 0);

    rc = MFSH_PHYSTOVIRT(mbi_far->cmdline, 0xffff, &sl1);
    CHECKRC
    q = (char far *)MAKEP(sl1, 0);
    fmemmove(cmdline, q, fstrlen(q));
    MFSH_UNPHYSTOVIRT(sl1);

    // if "--serial=..." specified on the command line
    if ((mbi_far->flags & MB_INFO_CMDLINE) && (pp = strstr(cmdline, "--serial")))
    {
      pp = skip_to(1, pp);
      safe_parse_maxint(&pp, &port);
    }

    // init serial port
    serial_init(port, 9600, UART_8BITS_WORD, UART_NO_PARITY, UART_1_STOP_BIT);

    kprintf("**** MFS_INIT\n");
    kprintf("Hello MBI minifsd!\n");
    kprintf("comport = 0x%x\n", port);
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


    return NO_ERROR;
}

int far pascal _loadds MFS_OPEN(
    char far *name,                     /* name         */
    unsigned long far *size             /* size         */
)
{
    char far buf1[0x100];
    char far buf2[0x100];
    struct mod_list far *mod;
    char far *p, far *q;
    char far *s;
    int  n, rc;

    kprintf("**** MFS_OPEN(\"%s\")", name);

    fmemset(buf1, 0, sizeof(buf1));
    fmemset(buf2, 0, sizeof(buf2));

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
        fstrcpy(buf1, s);
        fstrcpy(buf2, name);

        // deallocate a selector
        MFSH_UNPHYSTOVIRT(sel1);

        // make it uppercase
        for (p = buf1; *p; p++) *p = toupper(*p);
        for (p = buf2; *p; p++) *p = toupper(*p);

        p = strip(buf1); q = strip(buf2);

        if (fstrstr(p, q))
          break;

        mod++;
      };

      // we have gone through all mods, and no given filename
      if (n == mods_count)
      {
        kprintf(" failed!\n");
        return -1;
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
  return -1;
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
    fmemmove(data, fileaddr, *length);
    filepos += *length;
  }
  advance_ptr();

  return NO_ERROR;
}

int far pascal _loadds MFS_CLOSE(void) {
    kprintf("**** MFS_CLOSE\n");

    if (sel1)
    {
      MFSH_UNPHYSTOVIRT(sel1);
      sel1 = 0;
    }

    return NO_ERROR;
}

int far pascal _loadds MFS_TERM(void) {

    kprintf("**** MFS_TERM\n");
    MFSH_UNPHYSTOVIRT(mods_sel);
    MFSH_UNPHYSTOVIRT(sel);

    return NO_ERROR;
}
