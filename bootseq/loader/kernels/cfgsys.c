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
#pragma aux mfsd_start   "*"
#pragma aux mfsd_size    "*"

#define NO_ERROR 0

struct term_entry *t;
/* multiboot structure pointer */
extern struct multiboot_info *m;
extern char   cfged;
extern char   autopreload;
extern char   remotefs;

/* mFSD start                  */
extern unsigned long mfsd_start;
/* mFSD size                   */
extern unsigned long mfsd_size;

int (*exec_cmd)(char *cmd) = 0;

/* first available free address */
extern unsigned long cur_addr;
/* changed config.sys length    */
extern int    len;

char drv;

void editor (char *cfg, int len, char force);
int kprintf(const char *format, ...);
int toupper (int c);
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
  char *dirs[] = {"/os2/boot", "/os2", "/", 0};
  struct mod_list *mod;

  for (t = s; *t; t++)
  {
    /* change all '\\' to '/' */
    if (*t == '\\') *t = '/';
    /* make a filename lowercase */
    *t = tolower(*t);
  }
  /* skip drive letter */
  if (s[1] == ':') s += 2;
  /* skip leading slash */
  if (*s == '/') s++;

  /* if a file with such name already is loaded, skip loading */
  for (i = 0, mod = (struct mod_list *)m->mods_addr;
       i < m->mods_count; i++, mod++)
    if (strstr((char *)mod->cmdline, s))
      return 1;

  /* find 1st space in the command line */
  for (t = s; *t && !isspace(*t); t++) ;
  /* delete command line params */
  if (*t) *t = 0;

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
  char *p, *t;

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

int load_type_bootdrv (const char *tp)
{
  int  i, j;
  char *str;

  for (i = 0; i < 8; i++)
  {
    if (!strcmp(type[i].name, tp))
    {
      for (j = 0; j < type[i].ip; j++)
      {
        str = type[i].sp[j].string;
        /* if it is booting from boot drive */
        if ((str[1] == ':' && toupper(str[0]) == toupper(drv)) ||
             str[0] == '\\')
        {
          if (!preload_module(str, 0))
            return 1;
        }
      }
    }
  }

  return 0;
}

int load_modules (char *cfg, int len)
{
  int  rc;
  int  last, first;
  int  i, j, l;
  char *p, *t;
  char *ifs, *device;
  int  ifs_line, line;
  char buf[0x400];
  char str[0x400];
  struct mod_list *mod;

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

  /* exec modaddr command to skip saved config.sys */
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

  /* preload doscall1 and forwarders */
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

  if (remotefs) /* remote fs boot -- additional modules are needed */
  {
    /* load all IFS'es from the boot drive */
    if (load_type_bootdrv("IFS")    ||
    /* load all 'device' drivers from the boot drive */
        load_type_bootdrv("DEVICE") ||
    /* load all 'run' daemons from the boot drive */
        load_type_bootdrv("RUN")    ||
    /* load all 'call' programs from the boot drive */
        load_type_bootdrv("CALL"))
      return 1;
    /* the same for protshell */
    device = options.protshell;
    if ((device[1] == ':' && toupper(device[0]) == toupper(drv)) || device[0] == '\\')
    {
      if (!preload_module(device, 0))
        return 1;
    }
  }
  else /* ordinary boot */
  {
    /* 1st ifs command line */
    ifs = type[3].sp[0].string;
    /* its line number      */
    ifs_line = type[3].sp[0].line;

    /* load all 'device' drivers before 1st ifs */
    for (i = 0; i < type[2].ip; i++)
    {
      line = type[2].sp[i].line;
      device = type[2].sp[i].string;
      if (line < ifs_line)
      {
        if (!preload_module(device, 0))
          return 1;
      }
      else
        break;
    }

    /* load 1st ifs */
    if (!preload_module(ifs, 0))
      return 1;
  }

  /* load video DLL's, like 'bvhvga' and 'bvhsvga' */
  if (CfgGetenv("VIDEO_DEVICES", buf) == NO_ERROR)
  {
    if (CfgGetenv(buf, buf) == NO_ERROR)
    {
      /* find '(' */
      for (p = buf; *p && *p != '('; p++) ;
      if (*p)
      {
        p++; /* skip '(' */
        for (;;)
        {
          /* load next DLL name into str[] buffer */
          for (i = 0, t = p; *t && *t != ',' && *t != ')'; i++, t++) str[i] = tolower(*t);
          if (*t) /* if the list not reached its end */
          {
            strcpy(str + i, ".dll");

            if (!preload_library(str))
              return 1;

            if (*t == ')')
              break;

            t++; p = t; /* skip ',' */
          }
        }
      }
    }
  }

  /* preload codepage files for unicode.sys */
  for (i = 0, mod = (struct mod_list *)m->mods_addr;
       i < m->mods_count; i++, mod++)
  {
    if (strstr((char *)mod->cmdline, "unicode.sys"))
    {
      if (!preload_module("/language/codepage/ucstbl.lst", 0))
        return 1;

      p = options.codepage;
      for (l = 0; l < 2; l++)
      {
        strcpy(buf, "/language/codepage/ibm");
        for (j = strlen(buf), t = p; *t && *t != ','; j++, t++) buf[j] = *t;
        buf[j] = '\0';

        if (!preload_module(buf, 0))
          return 1;

        t++; p = t; /* skip comma */
      }
    }
  }

  /* load oso001.msg */
  if (CfgGetenv("DPATH", buf) == NO_ERROR)
  {
    t = buf;
    for (;;)
    {
      /* copy next DPATH element into str[] buffer */
      for (i = 0, p = t; *p && *p != ';'; i++, p++) str[i] = tolower(*p);

      /* skip '.' DPATH entry */
      if (i == 1 && *str == '.')
      {
        t += 2;
        continue;
      }

      /* if DPATH ended */
      if (!*str)
        break;

      /* add trailing backslash */
      if (str[i - 1] != '\\') str[i++] = '\\';
      /* copy filename */
      strcpy(str + i, "oso001.msg");

      if (preload_module(str, 0))
        break;

      p++; t = p;
    }
  }

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
  char *cfg, *p;
  struct mod_list *mod;

  t = term;
  drv = drvletter;
  cfg = (char *)addr;

  mod = (struct mod_list *)m->mods_addr;
  // find the address of modules end
  // pointer to the last module in the list
  mod += m->mods_count - 1;
  // last module end
  p = (char *)mod->mod_end;
  // skip a string after a module (cmdline for FreeLdr, none for GRUB)
  while (*p++) ;
  cur_addr = ((unsigned long)(p + 0xfff)) & 0xfffff000;

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

  // copy mFSD at 0x7c0
  memmove((char *)0x7c0, (char *)&mfsd_start, mfsd_size);

  cls ();

  return m;
}
