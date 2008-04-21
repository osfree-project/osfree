/*
 *
 *
 */

#include <lip.h>
#include <types.h>
#include <loader.h>
#include <bpb.h>

#include <shared.h>

#include "fsys.h"
#include "fsd.h"

extern lip2_t *l;
extern bios_parameters_block *bpb;
extern FileTable ft;

struct multiboot_info *m;
void create_lip_module(lip2_t **l);
void multi_boot(void);

char *
skip_to (int after_equal, char *cmdline);

//#pragma aux skip_to "*"

#pragma aux multi_boot     "*"
#pragma aux kernel_func    "*"
#pragma aux module_func    "*"
#pragma aux modaddr_func   "*"
#pragma aux lipmodule_func "*"
#pragma aux m              "*"
#pragma aux l              "*"

int kernel_func (char *arg, int flags);
int module_func (char *arg, int flags);
int modaddr_func (char *arg, int flags);
int lipmodule_func (char *arg, int flags);
int vbeset_func (char *arg, int flags);

void panic(char *msg, char *file);
int  abbrev(char *s1, char *s2, int n);
char *strip(char *s);
char *trim(char *s);

int process_cfg(char *cfg);

void init(lip2_t *l)
{

}

int process_cfg_line(char *line)
{
  int i;
  static int section = 0;
  static int insection = 0;
  static int sec_to_load;

  // delete CR and LF symbols at the end
  line = strip(trim(line));
  if (!*line && insection) insection = 0;
  // skip comments ";"
  i = grub_index(';', line);
  if (i) line[i - 1] = '\0';
  // skip comments "#"
  i = grub_index('#', line);
  if (i) line[i - 1] = '\0';
  // delete leading and trailing spaces
  line = strip(line);
  if (!*line) return 1;

  if (abbrev(line, "title", 5))   
  {
    section++;
    if (sec_to_load == section) 
      insection = 1;
    else 
      insection = 0;
  }
  else if (abbrev(line, "default", 7))
  {
    line = strip(skip_to(1, line));
    sec_to_load = grub_aton(line) + 1;
  }
  else if (insection && abbrev(line, "modaddr", 7)) 
  {
    line = strip(skip_to(1, line));
    if (modaddr_func(line, 0x2))
    {
      printf("An error occured during execution of modaddr_func\r\n");
      return 0;
    }
  }
  else if (insection && abbrev(line, "kernel", 6))
  {
    line = strip(skip_to(1, line));
    if (kernel_func(line, 0x2)) 
    {
      printf("An error occured during execution of kernel_func\r\n");
      return 0;
    }
  }
  else if (insection && abbrev(line, "module", 6))
  {
    line = strip(skip_to(1, line));
    if (abbrev(line, "--type", 6))
    {
      line = strip(skip_to(1, line));
      if (abbrev(line, "lip", 3))
      {
        if (lipmodule_func(line, 0x2))
        {
          printf("An error occured during execution of lipmod_func\r\n");
          return 0;
        }
      }
      else if (abbrev(line, "bootsector", 10))
      {
        //if (bootsecmodule_func(line, 0x2))
        //{
        //  printf("An error occured during execution of lipmod_func\r\n");
        //  return 0;
        //}
      }
    }
    else if (module_func(line, 0x2))
    {
      printf("An error occured during execution of module_func\r\n");
      return 0;
    }
  }
  else if (insection && abbrev(line, "vbeset", 6))
  {
    line = strip(skip_to(1, line));
    if (vbeset_func(line, 0x2)) 
    {
      printf("An error occured during execution of vbeset_func\r\n");
      return 0;
    }
  }
  else
  {
  }
  
  return 1;
}

void KernelLoader(void)
{
  char *cfg = "/boot/freeldr/freeldr.cfg";
  int rc;

  printf("Kernel loader started.\r\n");

  if (!(rc = process_cfg(cfg)))
  {
    printf("Error parsing loader config file!\r\n");
  }
  else if (rc == -1)
  {
    panic("Load error!", "");
  }
  else
  {
    //create_lip_module(&l);
    multi_boot();
  }
}

void cmain(void)
{
  /* Get mbi structure address from pre-loader */
  u_parm(PARM_MBI, ACT_GET, (unsigned int *)&m);
  KernelLoader();
}
