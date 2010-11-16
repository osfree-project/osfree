/*  This multiboot kernel is intended for
 *  loading other loaders by chainloading
 */

#pragma aux m     "*"
#pragma aux l     "*"
#pragma aux stop  "*"
#pragma aux force_lba            "*"
#pragma aux start_kernel         "*"
#pragma aux boot_drive           "*"
#pragma aux ldr_drive            "*"
//#pragma aux printk               "*"
#pragma aux lip_module_present   "*"
#pragma aux start1               "*"
#pragma aux bss_end              "*"

//#include <string.h>
//#include <ctype.h>
#include <stdarg.h>
#include <lip.h>
#include "fsys.h"
#include "term.h"
#include "fsd.h"

// BootPart address. Here we copy
// ((current_partion & 0xff0000) >> 12)
#define BOOTPART_ADDR (0x7c00 + 0x200 - 0x44)

void start_kernel(void);
grub_error_t errnum;
extern void stop(void);
struct multiboot_info *m;
lip2_t *l;
struct term_entry *t;

unsigned char lip_module_present = 0;
//unsigned char force_lba = 0;

//void printk(const char *fmt,...);
//#define printk printf

unsigned long ldr_drive;
unsigned long boot_drive;
unsigned long boot_part;
unsigned long current_drive;
unsigned long current_partition;
unsigned long cdrom_drive;
unsigned long saved_drive;
unsigned long saved_partition;


unsigned long load_addr = 0;
unsigned long bootsec_segment = -1;
unsigned long load_segment = -1;
unsigned long load_offset = -1;
unsigned long skip_length = 0;
unsigned long __ebx = -1;
unsigned long __edx = -1;
int boot_cs = 0;
int boot_ip = 0x7c00;

extern unsigned long start1;
extern unsigned long bss_end;

void init(void)
{

}

/*

int check_lip(char *mods_addr, unsigned long mods_count)
{
  struct mod_list *mod;

  // last module in the list
  mod = (struct mod_list *)(mods_addr + (mods_count - 1) * sizeof(struct mod_list));
  if (grub_strstr((char *)(mod->cmdline), "*lip"))
  {
    // set LIP pointer
    l = (lip2_t *)mod->mod_start;
    // check if the LIP begins with a
    // magic number of 0x3badb002
    if (*((unsigned long *)l) == LIP2_MAGIC)
    {
      t = l->u_termctl(-1);
      if (lip_module_present) printf("boot_chain started\r\n");
      return 1;
    }
    else
      return 0;
  }

  return 0;
}

*/

void kernel_ldr(char *kernel, unsigned long kernel_len)
{
  if (lip_module_present)
  {
    printf("kernel_ldr() started\r\n");
    printf("kernel loaded @ 0x%x, len = %u\r\n", kernel, kernel_len);
  }

  // Get boot_drive value
  //u_parm(PARM_BOOT_DRIVE, ACT_GET, (unsigned int *)&boot_drive);
  //u_parm(PARM_CDROM_DRIVE, ACT_GET, (unsigned int *)&cdrom_drive);
  //u_parm(PARM_SAVED_DRIVE, ACT_GET, (unsigned int *)&saved_drive);
  //u_parm(PARM_SAVED_PARTITION, ACT_GET, (unsigned int *)&saved_partition);

  //saved_drive = boot_drive;
  //saved_partition = 0x00ffffff;

  start_kernel();
}

void cmain(void)
{
  char *mods_addr;
  int mods_count;
  struct mod_list *mod, *bootblk;
  struct geometry geom;

  char *cmdline;
  char *kernel;
  unsigned long kernel_len;
  char *s, *kernel_cmdline;
  int force = 0;
  unsigned char ch, *p;
  int n, i;

  boot_part  = (boot_drive >> 8) << 8;
  boot_drive = boot_drive & 0xff;

  mods_addr  = (char *)m->mods_addr;
  mods_count = m->mods_count;

  cmdline = (char *)m->cmdline;

  mod = (struct mod_list *)mods_addr;

  if (mods_count < 1)
    stop();
  else
  {
    //if (check_lip(mods_addr, mods_count))
    //   lip_module_present = 1;
    lip_module_present = 0;

    kernel = (char *)mod->mod_start;
    kernel_len = mod->mod_end - mod->mod_start;
    kernel_cmdline = (char *)mod->cmdline;

    if (s = grub_strstr(cmdline, "--load-segment"))
    {
      s = skip_to(1, s);
      safe_parse_maxint(&s, (long *)&load_segment);
    }

    if (s = grub_strstr(cmdline, "--load-offset"))
    {
      s = skip_to(1, s);
      safe_parse_maxint(&s, (long *)&load_offset);
    }

    if (s = grub_strstr(cmdline, "--boot-cs"))
    {
      s = skip_to(1, s);
      safe_parse_maxint(&s, (long *)&boot_cs);
    }

    if (s = grub_strstr(cmdline, "--boot-ip"))
    {
      s = skip_to(1, s);
      safe_parse_maxint(&s, (long *)&boot_ip);
    }

    if (s = grub_strstr(cmdline, "--skip-length"))
    {
      s = skip_to(1, s);
      safe_parse_maxint(&s, (long *)&skip_length);
    }

    if (s = grub_strstr(cmdline, "--ebx"))
    {
      s = skip_to(1, s);
      safe_parse_maxint(&s, (long *)&__ebx);
    }

    if (s = grub_strstr(cmdline, "--edx"))
    {
      s = skip_to(1, s);
      safe_parse_maxint(&s, (long *)&__edx);
    }

    if (s = grub_strstr(cmdline, "--bootsec-segment"))
    {
      s = skip_to(1, s);
      safe_parse_maxint(&s, (long *)&bootsec_segment);
    }

    if (bootsec_segment == -1)
      bootsec_segment = 0x7c0;

    if (skip_length)
      kernel_len = skip_length;

    // Copy myself to conventional memory
    grub_memmove((char *)REL_BASE, (char *)KERN_BASE, (int)&bss_end - (int)KERN_BASE);

    if (lip_module_present) printf("cmdline=%s\r\n", cmdline);

    if (grub_strstr(cmdline, "--force"))
      force = 1;

    if (lip_module_present) printf("boot_drive=%u\r\n", boot_drive);

    if (get_diskinfo (boot_drive, &geom)
        || ! (geom.flags & BIOSDISK_FLAG_CDROM))
      cdrom_drive = GRUB_INVALID_DRIVE;
    else
      cdrom_drive = boot_drive;

    // set I13X flag, if appropriate
    if (s = grub_strstr(cmdline, "--i13x"))
    {
      if (geom.flags & BIOSDISK_FLAG_LBA_EXTENSION)
      {
        char *p;
        p = (char *)(0x30000);
        /* clear space after I13X flag place to make happy
           IBM's bootsectors, which don't clear unused parts
           of LBA packet, which is located here */
        memset(p, 0, 0x20);
        grub_strcpy(p, "I13X"); // I13X
      }
    }

    if (s = grub_strstr(cmdline, "--bootdev"))
    {
      s = skip_to(1, s);
      if (!set_device(s))
      {
        if (lip_module_present) printf("bootdev is incorrect!\r\n");
        stop();
      }
      ldr_drive = current_drive;
      if (lip_module_present) printf("ldr_drive=%u\r\n",  current_drive);
    }

    // Copy BootPart value
    ch = (unsigned char)((current_partition >> 8) & 0xff);
    if (ch == 0xff) ch = 0;
    p  = (unsigned char *)BOOTPART_ADDR;
    *p = ch;
    ldr_drive = current_partition | (ldr_drive << 24);

    if (load_segment != -1 && load_offset != -1)
      load_addr = (load_segment << 4) + load_offset;

    if (!load_addr) load_addr = BOOTSEC_LOCATION;

    if (mods_count > 1)
    {
      for (i = 0, mod = (struct mod_list *)mods_addr; i < mods_count; i++, mod++)
      if (strstr((char *)mod->cmdline, "*bootsec*"))
      {
        /* if "*bootsec*" line is in the command line */
        memmove((char *)(bootsec_segment << 4), (char *)mod->mod_start, mod->mod_end - mod->mod_start);
        p = (unsigned char *)0x7c24;
        *p = 0x80;
        p = (unsigned char *)0x7c15;
        *p = 0xF8;
        break;
      }
    }

    // Copy kernel
    grub_memmove((char *)load_addr, kernel, kernel_len);

    kernel_ldr(kernel, kernel_len);
  }
}
