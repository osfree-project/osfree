/*  This multiboot kernel is intended for
 *  loading other loaders by chainloading
 */

#pragma aux m     "*"
#pragma aux l     "*"
#pragma aux stop  "*"
#pragma aux errnum               "*"
#pragma aux start_kernel         "*"
#pragma aux boot_drive           "*"

#include <lip.h>
#include "fsys.h"
#include "term.h"

void start_kernel(void);
grub_error_t errnum;
extern void stop(void);
struct multiboot_info *m;
lip2_t *l;
struct term_entry *t;

unsigned long boot_drive;
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
      printf("boot_chain started\r\n");
      return 1;
    }
    else
      return 0;
  }

  return 0;
}

int kernel_ldr(char *kernel, unsigned long kernel_len)
{
  printf("kernel_ldr() started\r\n");
  printf("kernel loaded @ 0x%x, len = %u\r\n", kernel, kernel_len);

  // Copy kernel
  grub_memmove((char *)BOOTSEC_LOCATION, kernel, kernel_len);

  // Get boot_drive value
  u_parm(PARM_BOOT_DRIVE, ACT_GET, (unsigned int *)&boot_drive);
  u_parm(PARM_CDROM_DRIVE, ACT_GET, (unsigned int *)&cdrom_drive);
  u_parm(PARM_SAVED_DRIVE, ACT_GET, (unsigned int *)&saved_drive);
  u_parm(PARM_SAVED_PARTITION, ACT_GET, (unsigned int *)&saved_partition);

  start_kernel();

  return 0;
}

void cmain(void)
{
  char *mods_addr;
  int mods_count;
  struct mod_list *mod; 

  char *cmdline;
  char *kernel;
  unsigned long kernel_len;
  char *s, *kernel_cmdline;
  int force = 0;

  mods_addr  = (char *)m->mods_addr;
  mods_count = m->mods_count;

  cmdline = (char *)m->cmdline;

  // kernel is the first module in the list,
  // and initrd is the second 
  mod = (struct mod_list *)mods_addr;

  if (mods_count < 2)
    stop();
  else
  {
    if (!check_lip(mods_addr, mods_count))
      stop();

    kernel = (char *)mod->mod_start;
    kernel_len = mod->mod_end - mod->mod_start;
    kernel_cmdline = (char *)mod->cmdline;

    printf("cmdline=%s\r\n", cmdline);

    if (grub_strstr(cmdline, "-force"))
      force = 1;

    if (s = grub_strstr(cmdline, "-bootdev"))
    {
      s = skip_to(1, s);
      if (!set_device(s) || (current_partition != 0xffffff))
      {
        printf("bootdev is incorrect!\r\n");
        stop();
      }
      boot_drive = current_drive;
      printf("boot_drive=%u\r\n", boot_drive);
    }

    if (!kernel_ldr(kernel, kernel_len))
    {
      printf("Error chainloading!\r\n");
      stop();
    }
  }
}
