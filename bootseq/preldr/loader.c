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

#pragma aux skip_to "*"

#define BUFSIZE 0x200
char linebuf[BUFSIZE];
char buf[BUFSIZE];

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

void init(lip2_t *l)
{

}

void panic(char *msg, char *file)
{
  printf("\r\nFatal error: \r\n");
  printf(msg);
  printf(file);

  __asm {
    cli
    hlt
  }
}

int abbrev(char *s1, char *s2, int n)
{
  char *p = s1;
  char *q = s2;
  int  i = 1;

  while (*p++ == *q++ && *p != '\0') i++;
  if (i >= n) return 1;

  return 0;
}

/*  Strip leading and trailing
 *  spaces
 */
char *strip(char *s)
{
  char *p = s;
  int  i;

  i = grub_strlen(p) - 1;
  while (grub_isspace(p[i])) p[i--] = '\0'; // strip trailing spaces
  while (grub_isspace(*p)) p++;             // strip leading spaces

  return p;
}

/*  Delete ending CR
 *  and LF
 */
char *trim(char *s)
{
  int l, i;

  l = grub_index('\r', s);
  if (l) s[l - 1] = '\0';
  l = grub_index('\n', s);
  if (l) s[l - 1] = '\0';
  
  return s;
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

/*  Returns a next line from a buffer in memory
 *  and changes current position (*p)
 */
char *getline(char **p, int n)
{
  int  i = 0;
  char *q = *p;
  char *s;

  if (!q)
    panic("getline(): zero pointer: ", "*p");

  while (*q != '\r' && *q != '\n' && *q != '\0' && q - buf < n)
    linebuf[i++] = *q++;

  if (*q == '\r') linebuf[i++] = *q++;
  if (*q == '\n') linebuf[i++] = *q++;

  linebuf[i] = '\0';
  *p = q;

  s = linebuf;

  return s;
}

int process_cfg(char *cfg)
{
  // buffer for config file reading
  char str[BUFSIZE];
  char s[BUFSIZE];
  char *p, *line;
  char f, g;
  char cont = 0; // line continuation indication
  unsigned int size, sz;
  unsigned int rd;
  unsigned int lineno = 0;
  int n, m, k, i;
  int bytes_read = 0;

  if (u_open(cfg, &size)) {
    printf("Cannot open config file!\r\n");
    return 0;
  }
  u_close(); 

  grub_memset(s, 0, sizeof(s));

  sz = size;
  while (sz) 
  {
    if (u_open(cfg, &size)) {
      printf("Cannot open config file!\r\n");
      return 0;
    }

    u_seek(bytes_read);

    rd = u_read(buf, sizeof(buf));
    sz -= rd;
    bytes_read += rd;

    u_close();

    if (sz && !rd)
    {
      printf("Can't read from config file!\r\n");
      return 0;
    }      

    f = 1;
    p = buf;
    while (*p && f) 
    {
      line = getline(&p, rd);
      grub_strcat(str, s, line);
      grub_strcpy(s, str);
      f = (p - buf < rd);
      g = f || (!f && (buf[rd - 1] == '\n'));
      if (g)
      {
        if (!process_cfg_line(s)) return -1;
        s[0] = '\0';
      }
    }
  }

  return 1;
}

void KernelLoader(void)
{
  char *cfg = "/boot/freeldr/freeldr.cfg";
  int rc;

  printf("Kernel loader started.\r\n");

  if(!(rc = process_cfg(cfg)))
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
