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

//#include <string.h>
//#include <ctype.h>
#include <stdarg.h>
#include <lip.h>
#include "fsys.h"
#include "term.h"

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


void init(void)
{

}

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

void kernel_ldr(char *kernel, unsigned long kernel_len)
{
  if (lip_module_present)
  {
    printf("kernel_ldr() started\r\n");
    printf("kernel loaded @ 0x%x, len = %u\r\n", kernel, kernel_len);
  }

  // Copy kernel
  grub_memmove((char *)BOOTSEC_LOCATION, kernel, kernel_len);

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
  struct mod_list *mod;
  struct geometry geom;

  char *cmdline;
  char *kernel;
  unsigned long kernel_len;
  char *s, *kernel_cmdline;
  int force = 0;
  unsigned char ch, *p;

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
    if (check_lip(mods_addr, mods_count))
       lip_module_present = 1;

    kernel = (char *)mod->mod_start;
    kernel_len = mod->mod_end - mod->mod_start;
    kernel_cmdline = (char *)mod->cmdline;

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
  
    kernel_ldr(kernel, kernel_len);
  }
}
