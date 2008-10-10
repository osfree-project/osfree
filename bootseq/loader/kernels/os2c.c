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
#pragma aux boot_drive           "*"
#pragma aux start_realmode_part  "*"

#include <lip.h>

grub_error_t errnum;
extern void stop(void);
struct multiboot_info *m;
lip2_t *l;
struct term_entry *t;
// lip present flag
char lip = 0;
unsigned long boot_drive;


void start_realmode_part (void);

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
      t = l->u_termctl(-1);
      printf("boot_os2 started\r\n");
      return 1;
    }
    else
      return 0;
  }

  return 0;
}

void cmain(void)
{
  char *cmdline;         // multiboot kernel command line
  char *mods_addr;
  int mods_count;
  struct mod_list *mod;

  mods_addr  = (char *)m->mods_addr;
  mods_count = m->mods_count;

  mod = (struct mod_list *)mods_addr;

  cmdline = (char *)m->cmdline;

  // if lip module present
  if (grub_strstr(cmdline, "-lip")) lip = 1;

  if (lip && !check_lip(mods_addr, mods_count))
      stop();

  //
  // place for additional code
  //
  start_realmode_part();
}
