/*
 *
 *
 */

#include <shared.h>
#include "serial.h"
#include "fsd.h"

#pragma aux m          "*"
#pragma aux rel_start  "*"
#pragma aux ufsd_start "*"
#pragma aux ufsd_size  "*"
#pragma aux mfsd_start "*"
#pragma aux mfsd_size  "*"
#pragma aux uFSD_base  "*"

/* multiboot structure pointer */
extern struct multiboot_info *m;
/* uFSD relocation info start  */
extern unsigned long rel_start;
/* uFSD start                  */
extern unsigned long ufsd_start;
/* uFSD size                   */
extern unsigned long ufsd_size;
/* mFSD start                  */
extern unsigned long mfsd_start;
/* mFSD size                   */
extern unsigned long mfsd_size;

char ramdisk = 0;
char autopreload = 0;
char cfged = 0;
char debug = 0;
unsigned long cur_addr;

int kprintf(const char *format, ...);
int serial_init (long port, long speed,
                int word_len, int parity, int stop_bit_len);

void init (void)
{

}

/*  Relocate a file in memory using its
 *  .rel file.
 *  base is a file base, rel_start is .rel
 *  file in memory pointer and shift is the
 *  relocation shift
 */
void reloc(char *base, char *rel_start, unsigned long shift)
{
  int  i, n, rc;

  typedef _Packed struct {
    unsigned short addr;
    unsigned char  shift;
  } rel_item;

  rel_item *p;
  unsigned long *addr;

  /* number of reloc items */
  n = *((unsigned short *)(rel_start)) / 3;
  p = (rel_item *)(rel_start + 2);

  for (i = 0; i < n; i++) {
    addr  = (unsigned long *)(base + p[i].addr);
    *addr += shift >> p[i].shift;
  }
}

/*  Relocate mbi structure to
 *  the safe place
 */
void mbi_reloc(void)
{
  int                   i;
  unsigned long         size;
  struct mod_list       *mod;
  struct multiboot_info *mbi_new;
  struct mod_list       *mod_new;
  char                  *p;

  mod = (struct mod_list *)m->mods_addr;

  // find the address of modules end

  // pointer to the last module in the list
  mod += m->mods_count - 1;

  // last module end
  p = (char *)mod->mod_end;

  // skip a string after a module (cmdline for FreeLdr, none for GRUB)
  while (*p++) ;

  cur_addr = ((unsigned long)(p + 0xfff)) & 0xfffff000;
  kprintf("mods end: 0x%08lx\n", cur_addr);
  size = sizeof(struct multiboot_info);
  memmove((char *)cur_addr, m, size);
  m = (struct multiboot_info *)cur_addr;
  kprintf("mbi new: 0x%08lx\n", m);

  // relocate a kernel command line
  cur_addr = (((unsigned long)(cur_addr + size)) + 0xfff) & 0xfffff000;
  size = strlen((char *)m->cmdline) + 1;
  memmove((char *)cur_addr, (char *)m->cmdline, size);
  m->cmdline = cur_addr;
  kprintf("cmdline: 0x%08lx\n", cur_addr);

  // relocate mods after mbi
  if (m->flags & MB_INFO_MODS)
  {
    cur_addr = (((unsigned long)(cur_addr + size)) + 0xfff) & 0xfffff000;
    size = sizeof(struct mod_list) * m->mods_count;
    memmove((char *)cur_addr, (char *)m->mods_addr, size);
    m->mods_addr = cur_addr;
    kprintf("mods_addr: 0x%08lx\n", cur_addr);

    // relocate mods command lines
    cur_addr = (((unsigned long)((char *)cur_addr + size)) + 0xfff) & 0xfffff000;
    for (i = 0, mod = (struct mod_list *)m->mods_addr;
         i < m->mods_count; i++, cur_addr += size, mod++)
    {
      size = strlen((char *)mod->cmdline) + 1;
      memmove((char *)cur_addr, (char *)mod->cmdline, size);
      mod->cmdline = cur_addr;
      kprintf("mods[%u] = %s\n", i, cur_addr);
    }
  }

  // relocate memmap
  if (m->flags & MB_INFO_MEM_MAP)
  {
    cur_addr = (cur_addr + 0xfff) & 0xfffff000;
    size = m->mmap_length;
    memmove((char *)cur_addr, (char *)m->mmap_addr, size);
    m->mmap_addr = cur_addr;
    kprintf("mmap_addr: 0x%08lx\n", cur_addr);
  }

  // relocate drives info
  if (m->flags & MB_INFO_DRIVE_INFO)
  {
    cur_addr = (((unsigned long)((char *)cur_addr + size)) + 0xfff) & 0xfffff000;
    size = m->drives_length;
    memmove((char *)cur_addr, (char *)m->drives_addr, size);
    m->drives_addr = cur_addr;
    kprintf("drives_addr: 0x%08lx\n", cur_addr);
  }

  // relocate the boot loader name
  if (m->flags & MB_INFO_BOOT_LOADER_NAME)
  {
    p = (char *)m->boot_loader_name;
    cur_addr = (((unsigned long)((char *)cur_addr + size)) + 0xfff) & 0xfffff000;
    size = strlen(p);
    strcpy((char *)cur_addr, p);
    m->boot_loader_name = cur_addr;
    kprintf("boot_loader_name: %s\n", cur_addr);
  }
  cur_addr += size;
}

int cmain(void)
{
  int relshift;
  char *p;
  unsigned short *d;
  unsigned long  *e;
  unsigned ufs_base = 0;
  long port = 0x3f8;
  long speed = 9600;
  struct mod_list *mod;

  if (p = strstr((char *)m->cmdline, "--auto-preload"))
  {
    autopreload = 1;
  }

  if (p = strstr((char *)m->cmdline, "--ramdisk-boot"))
  {
    ramdisk = 1;
  }

  if (p = strstr((char *)m->cmdline, "--cfged"))
  {
    cfged = 1;
  }

  if (p = strstr((char *)m->cmdline, "--debug"))
  {
    debug = 1;
  }

  if (p = strstr((char *)m->cmdline, "--port"))
  {
    p = skip_to(1, p);
    safe_parse_maxint(&p, &port);
  }

  if (p = strstr((char *)m->cmdline, "--speed"))
  {
    p = skip_to(1, p);
    safe_parse_maxint(&p, &speed);
  }

  // init serial port
  serial_init(port, speed, UART_8BITS_WORD, UART_NO_PARITY, UART_1_STOP_BIT);

  kprintf("Hello MBI OS/2 booter!\n");
  kprintf("comport = 0x%x\n", port);

  // where to copy uFSD
  ufs_base = ((m->mem_lower << 10) - 0x10000 - ufsd_size - 0x3000 - 0x200) & 0xfffc000;
  kprintf("uFSD base: 0x%08lx\n", ufs_base);

  // a shift relative to REL1_BASE
  relshift  = ufs_base - REL1_BASE;
  kprintf("relshift: %ld\n", relshift);

  // copy uFSD where needed
  memmove((char *)ufs_base, (char *)&ufsd_start, ufsd_size);
  kprintf("uFSD is moved...\n");

  // save port value in 16-bit area in uFSD header
  e  = (unsigned long *)(ufs_base + 0x1c);
  *e = (unsigned long)&mfsd_start;

  // save port value in 16-bit area in uFSD header
  d  = (unsigned short *)(ufs_base + 0x24);
  *d = port;

  // save debug flag into the same area
  d = (unsigned short *)(ufs_base + 0x26);
  *((char *)d) = debug;

  // fixup uFSD
  reloc((char *)ufs_base, (char *)&rel_start, relshift);
  kprintf("uFSD is relocated...\n");

  return ufs_base;
}
