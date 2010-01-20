/*  Config.sys preprocessor
 *  and editor
 *
 */

#include <shared.h>
#include <term.h>

#include <cfgparsr.h>

#pragma aux callback     "*"
#pragma aux u_msg        "*"
#pragma aux m            "*"
#pragma aux exec_cmd     "*"

#define NO_ERROR 0

struct term_entry *t;
/* multiboot structure pointer */
extern struct multiboot_info *m;
extern char   cfged;
extern char   autopreload;

int (*exec_cmd)(char *cmd) = 0;

/* first available free address */
extern unsigned long cur_addr;
/* changed config.sys length    */
extern int    len;

void editor (char *cfg, int len, char force);
int kprintf(const char *format, ...);
void mbi_reloc(void);

int getkey (void)
{
  return t->getkey();
}

int checkkey (void)
{
  return t->checkkey();
}

void gotoxy(int x, int y)
{
  t->gotoxy(x, y);
}

int getxy(void)
{
  return t->getxy();
}

int setcursor(int n)
{
  return t->setcursor(n);
}

void setcolorstate(int n)
{
  t->setcolorstate(n);
}

void setcolor(int x, int y)
{
  t->setcolor(x, y);
}

void cls(void)
{
  t->cls();
}

/* a dummy for minilibc */
void u_msg (char *s)
{
}


int preload_module(char *module, int three_dirs)
{
  char str[0x200];
  char *s = module;
  char *t;
  int  i, rc;
  char **p;
  char *dirs[] = {"/", "/os2", "/os2/boot", 0};
  struct mod_list *mod;

  /* skip drive letter */
  if (s[1] == ':') s += 2;
  /* skip leading slash */
  if (*s == '/' || *s == '\\') s++;

  /* if a file with such name already is loaded, skip loading */
  for (i = 0, mod = (struct mod_list *)m->mods_addr;
       i < m->mods_count; i++, mod++)
    if (strstr((char *)mod->cmdline, module))
      return 1;

  /* find 1st space in the command line */
  for (t = s; *t && !isspace(*t); t++) ;
  /* delete command line params */
  if (*t) *t = 0;
  /* make a filename lowercase */
  for (t = s; *t; t++) *t = tolower(*t);

  if (three_dirs)
  {
    for (p = dirs; *p; p++)
    {
      sprintf(str, "module ()%s/%s", *p, s);
      if (rc = exec_cmd(str))
        break;
    }
  }
  else
  {
    sprintf(str, "module ()/%s", s);
    rc = exec_cmd(str);
  }

  return rc;
}

int preload_library(const char *name)
{
  int  i, rc;
  char buf[0x200];
  char *p, *dir, *s, *t;

  p = options.libpath;
  for (;;)
  {
    /* copy libpath entry to buffer */
    for (i = 0, t = p; *t && *t != ';'; t++, i++) buf[i] = *t;
    /* skip '.' in libpath */
    if (i == 1 && *buf == '.')
    {
      p += 2;
      continue;
    }
    if (buf[i - 1] != '\\') buf[i++] = '\\';

    strcpy(buf + i, name);

    /* if a library is found on the libpath */
    if (rc = preload_module(buf, 0))
      break;

    /* if libpath ended */
    if (!*t)
      break;

    /* next dir */
    t++; p = t;
  }

  return rc;
}

int load_type (const char *tp)
{
  int  i, j;

  for (i = 0; i < 8; i++)
  {
    if (!strcmp(type[i].name, tp))
    {
      for (j = 0; j < type[i].ip; j++)
      {
        if (!preload_module(type[i].sp[j].string, 1))
          return 1;
      }
    }
  }

  return 0;
}

int load_modules (char *cfg, int len)
{
  int  rc;
  int  i, j;
  char *p, *t;
  char buf[0x80];

  // Initialize initial values from CONFIG.SYS
  rc = CfgInitOptions();
  if (rc != NO_ERROR)
  {
    kprintf("Can't initialize CONFIG.SYS parser\n");
    return rc;
  }

  // Parse CONFIG.SYS in memory
  rc = CfgParseConfig(cfg, len);
  if (rc != NO_ERROR)
  {
    kprintf("Error parse CONFIG.SYS\n");
    return rc;
  }

  printf ("Auto preloading modules: \r\n");

  /* exec modaddr command to skip a saved config.sys */
  sprintf(buf, "modaddr 0x%x", cur_addr);
  exec_cmd(buf);
  /* now config.sys is just below cur_addr */

  if (!preload_module("os2ldr", 0)       ||
      !preload_module("os2ldr.msg", 0)   ||
      !preload_module("os2krnl", 0))
    return 1;

  preload_module("os2dump", 0);
  preload_module("os2dbcs", 0);
  preload_module("os2dbcs.fnt", 0);
  preload_module("os2ldr.fnt", 0);
  preload_module("os2ldr.ini", 0);

  preload_module("kdb.ini", 0);
  preload_module("altf2on.$$$", 0);
  preload_module("os2norev.$$$", 0);
  preload_module("os2logo", 0);

  /* load all psd's, according to config.sys file */
  if (load_type("PSD"))
    return 1;

  /* preload implicit basedev's */
  if (!preload_module("resource.sys", 1) ||
      !preload_module("clock01.sys", 1)  ||
      !preload_module("screen01.sys", 1) ||
      !preload_module("kbdbase.sys", 1)  ||
      !preload_module("isapnp.snp", 1)   ||
      !preload_module("pnp.sys", 1))
    return 1;

  /* load all basedev's according to config.sys file */
  if (load_type("BASEDEV"))
    return 1;

  /* load country.sys according to config.sys file */
  p = options.country;
  /* search ',' */
  for (t = p; *t != ','; t++) ;
  /* skip it */
  t++;

  if (!preload_module(t, 0))
    return 1;

  /* load DCP files according to config.sys */
  p = options.devinfo_kbd;
  /* find 1st ',' */
  for (t = p; *t != ','; t++) ;
  /* skip it */
  t++;
  /* find 2nd ',' */
  for (p = t; *p != ','; p++) ;
  /* skip */
  p++;

  if (!preload_module(p, 0))
    return 1;

  p = options.devinfo_vio;
  /* find 1st ',' */
  for (t = p; *t != ','; t++) ;
  t++;
  /* find 2nd ',' */
  for (p = t; *p != ','; p++) ;
  p++;

  if (!preload_module(p, 0))
    return 1;

  if (!preload_library("doscall1.dll") ||
      !preload_library("sesmgr.dll")   ||
      !preload_library("msg.dll")      ||
      !preload_library("nls.dll")      ||
      !preload_library("os2char.dll")  ||
      !preload_library("bvscalls.dll") ||
      !preload_library("bmscalls.dll") ||
      !preload_library("bkscalls.dll") ||
      !preload_library("kbdcalls.dll") ||
      !preload_library("moucalls.dll") ||
      !preload_library("viocalls.dll"))
    return 1;

  return 0;
}

/* a callback function which is called by a microfsd
   for config.sys preprocessing/patching/editing    */
struct multiboot_info *
callback(unsigned long addr,
         unsigned long size,
         char drvletter,
         struct term_entry *term)
{
  int i;
  char *cfg;
  struct mod_list *mod;

  t = term;
  cfg = (char *)addr;

  /* Patch a boot drive letter */
  for (i = 0; i < size; i++)
  {
    if (cfg[i] == '@') /* change '@' symbol to a boot drive letter */
      cfg[i] = drvletter;
  }

  /* Start config.sys editor */
  printf ("Hello editor!\r\n");
  editor (cfg, size, cfged);

  if (len) /* if config.sys changed */
  {
    /* a new cfg address */
    cfg = (char *)cur_addr;
    /* find an original config.sys in mods */
    for (i = 0, mod = (struct mod_list *)m->mods_addr;
         i < m->mods_count; i++, mod++)
    {
      if (strstr((char *)mod->cmdline, "config.sys"))
      {
        /* modify addresses to point to a new copy */
        mod->mod_start = cur_addr;
        cur_addr += len;
        mod->mod_end   = cur_addr;
        break;
      }
    }
  }
  else
    len = size;

  if (autopreload)
  {
    /* Preload needed files */
    if (load_modules (cfg, len))
    {
      kprintf("Error preloading modules!\n");
      printf("Error preloading modules!\r\n");
    }
  }

  // relocate mbi info after all modules
  kprintf("Relocating MBI info...\n");
  mbi_reloc();
  kprintf("done.\n");

  return m;
}
