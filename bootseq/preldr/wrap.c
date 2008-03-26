/*
 *   LIP functions wrappers
 */

#include <lip.h>
#include "fsys.h"
#include "fsd.h"

#pragma aux l "*"
extern lip2_t *l;

unsigned int __cdecl
u_open (char *name, unsigned int *size)
{
  return l->u_open(name, size);
}

unsigned int __cdecl
u_read (char *buf, unsigned int count)
{
  return l->u_read(buf, count);
}

unsigned int __cdecl
u_seek (int loffseek)
{
  return l->u_seek(loffseek);
}

void __cdecl
u_close (void)
{
  l->u_close();
}

void __cdecl
u_terminate (void)
{
  l->u_terminate();
}

int __cdecl
u_diskctl (int func, int drive, struct geometry *geometry, int sector, int nsec, int addr)
{
  return l->u_diskctl(func, drive, geometry, sector, nsec, addr);
}

int __cdecl
u_boot (int type)
{
  return l->u_boot(type);
}

int __cdecl
u_load (char *image, unsigned long size, char *load_addr, struct exe_params *p)
{
  return l->u_load(image, size, load_addr, p);
}

int __cdecl
u_parm (int parm, int action, unsigned long *val)
{
  return l->u_parm(parm, action, val);
}

void __cdecl
u_msg (char *s)
{
  l->u_msg(s);
}
