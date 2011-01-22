/*  MBI info specific functions
 *
 */

#include <shared.h>

#include "fsd.h"

int kprintf(const char *format, ...);

extern int filemax;
extern int filepos;
extern int fileaddr;

/* multiboot structure pointer */
extern struct multiboot_info *m;

#pragma aux m            "*"
#pragma aux filemax      "*"
#pragma aux filepos      "*"
#pragma aux ufs_open     "*"
#pragma aux ufs_read     "*"
#pragma aux ufs_seek     "*"
#pragma aux ufs_close    "*"
#pragma aux ufs_term     "*"

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

char *lastpos(const char *s1, const char *s2)
{
  const char *s = s1 + strlen(s1) - strlen(s2);

  while (s >= s1)
    {
      const char *ptr, *tmp;

      ptr = s;
      tmp = s2;

      while (*tmp && *ptr == *tmp)
        ptr++, tmp++;

      if (tmp > s2 && ! *tmp)
        return (char *) s;

      s--;
    }

  return 0;
}

int
ufs_open (char *filename)
{
  int  mods_count;
  char *mods_addr;
  struct mod_list *mod;
  char buf1[0x100];
  char buf2[0x100];
  char *p, *q, *l;
  int  n;

  kprintf("**** ufs_open(\"%s\") = ", filename);

  memset(buf1, 0, sizeof(buf1));
  memset(buf2, 0, sizeof(buf2));

  if (m->flags & MB_INFO_MODS) // If there are modules
  {
    mods_count = (int)m->mods_count;
    mods_addr  = (char *)m->mods_addr;
    mod = (struct mod_list *)mods_addr;

    // search for a given filename
    for (n = 0; n < mods_count; n++)
    {
      // copy to buffers
      strcpy(buf1, (char *)mod->cmdline);
      strcpy(buf2, filename);

      // translate '/' to '\' in command line
      for (p = buf1; *p; p++) if (*p == '/') *p = '\\';

      // make it uppercase
      for (p = buf1; *p; p++) *p = grub_toupper(*p);
      for (p = buf2; *p; p++) *p = grub_toupper(*p);

      p = strip(buf1); q = strip(buf2);

      // skip a disk/partition
      if (*p == '(')
      {
        while (*p && *p != ')') p++;
        p++;
      }

      // skip a driveletter
      if (q[1] == ':') q += 2;
      if (*q == '\\')  q++;
      if (*p == '\\')  p++;

      l = lastpos(p, q);

      if (l && ((p + strlen(p)) == (l + strlen(q))) && (l == p || l[-1] == ' '))
          break;

      mod++;
    };

    // we have gone through all mods, and no given filename
    if (n == mods_count)
      return 0;

    // filename found
    filepos  = 0;
    filemax  = mod->mod_end - mod->mod_start;
    fileaddr = mod->mod_start;

    return 1;
  }

  return 0;
}

int
ufs_read (char *buf, int len)
{
  kprintf("**** ufs_read(0x%08x, %ld) = ", buf, len);

  if (fileaddr && buf && len)
  {
    if (len == -1) len = filemax;

    memmove(buf, (char *)fileaddr + filepos, len);

    kprintf("%lu\n", len);

    return len;
  }

  return 0;
}

int
ufs_seek (int offset)
{
  kprintf("**** ufs_seek(\"%ld\")\n", offset);

  if (offset > filemax || offset < 0)
    return -1;

  filepos = offset;

  return offset;
}

void
ufs_close (void)
{
  kprintf("**** ufs_close()\n");
}

void
ufs_term (void)
{
  kprintf("**** ufs_term()\n");
}
