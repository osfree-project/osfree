/*
 *
 */

#pragma aux m     "*"
#pragma aux l     "*"
#pragma aux stop  "*"
#pragma aux linux_boot     "*"
#pragma aux big_linux_boot "*"
#pragma aux linux_data_real_addr "*"
#pragma aux linux_data_tmp_addr  "*"
#pragma aux linux_text_len       "*"
#pragma aux errnum               "*"
#pragma aux start_kernel         "*"
#pragma aux boot_drive           "*"

#include <lip.h>
#include "fsys.h"

unsigned long boot_drive;
void start_kernel(void);
grub_error_t errnum;
extern void stop(void);
struct multiboot_info *m;
lip2_t *l;

void init(void)
{

}

/* Find the next word from CMDLINE and return the pointer. If
   AFTER_EQUAL is non-zero, assume that the character `=' is treated as
   a space. Caution: this assumption is for backward compatibility.  */
char *
skip_to (int after_equal, char *cmdline)
{
  /* Skip until we hit whitespace, or maybe an equal sign. */
  while (*cmdline && *cmdline != ' ' && *cmdline != '\t' &&
         ! (after_equal && *cmdline == '='))
    cmdline ++;

  /* Skip whitespace, and maybe equal signs. */
  while (*cmdline == ' ' || *cmdline == '\t' ||
         (after_equal && *cmdline == '='))
    cmdline ++;

  return cmdline;
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
  start_kernel();

  return 0;
}

void cmain(void)
{
  char *mods_addr;
  int mods_count;
  struct mod_list *mod; 

  char *kernel;
  unsigned long kernel_len;

  mods_addr  = (char *)m->mods_addr;
  mods_count = m->mods_count;

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

    if (!kernel_ldr(kernel, kernel_len))
    {
      printf("Error chainloading!\r\n");
      stop();
    }
  }
}
