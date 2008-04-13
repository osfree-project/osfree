/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <lip.h>
#include <filesys.h>

#include "fsys.h"

/* The type of kernel loaded.  */
kernel_t kernel_type;
grub_error_t errnum;

/* The address for Multiboot command-line buffer.  */
static char *mb_cmdline;

#pragma aux l "*"

#pragma aux kernel_func    "*"
#pragma aux module_func    "*"
#pragma aux modaddr_func   "*"
#pragma aux lipmodule_func "*"

#pragma aux skip_to "*"

extern lip2_t *l;
void create_lip_module(lip2_t **l);

/* kernel */
int
kernel_func (char *arg, int flags)
{
  int len;
  kernel_t suggested_type = KERNEL_TYPE_NONE;
  unsigned long load_flags = 0;

#ifndef AUTO_LINUX_MEM_OPT
  load_flags |= KERNEL_LOAD_NO_MEM_OPTION;
#endif

  /* Deal with GNU-style long options.  */
  while (1)
    {
      /* If the option `--type=TYPE' is specified, convert the string to
         a kernel type.  */
      if (grub_memcmp (arg, "--type=", 7) == 0)
        {
          arg += 7;

          if (grub_memcmp (arg, "netbsd", 6) == 0)
            suggested_type = KERNEL_TYPE_NETBSD;
          else if (grub_memcmp (arg, "freebsd", 7) == 0)
            suggested_type = KERNEL_TYPE_FREEBSD;
          else if (grub_memcmp (arg, "openbsd", 7) == 0)
            /* XXX: For now, OpenBSD is identical to NetBSD, from GRUB's
               point of view.  */
            suggested_type = KERNEL_TYPE_NETBSD;
          else if (grub_memcmp (arg, "linux", 5) == 0)
            suggested_type = KERNEL_TYPE_LINUX;
          else if (grub_memcmp (arg, "biglinux", 8) == 0)
            suggested_type = KERNEL_TYPE_BIG_LINUX;
          else if (grub_memcmp (arg, "multiboot", 9) == 0)
            suggested_type = KERNEL_TYPE_MULTIBOOT;
          else
            {
              errnum = ERR_BAD_ARGUMENT;
              u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
              return 1;
            }
        }
      /* If the `--no-mem-option' is specified, don't pass a Linux's mem
         option automatically. If the kernel is another type, this flag
         has no effect.  */
      else if (grub_memcmp (arg, "--no-mem-option", 15) == 0)
        load_flags |= KERNEL_LOAD_NO_MEM_OPTION;
      else
        break;

      /* Try the next.  */
      arg = skip_to (0, arg);
    }

  len = grub_strlen (arg);

  /* Reset MB_CMDLINE.  */
  mb_cmdline = (char *) MB_CMDLINE_BUF;
  if (len + 1 > MB_CMDLINE_BUFLEN)
    {
      errnum = ERR_WONT_FIT;
      u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      return 1;
    }

  /* Copy the command-line to MB_CMDLINE.  */
  grub_memmove (mb_cmdline, arg, len + 1);
  kernel_type = load_image (arg, mb_cmdline, suggested_type, load_flags);
  if (kernel_type == KERNEL_TYPE_NONE)
    return 1;

  mb_cmdline += len + 1;
  return 0;
}

/* module */
int
module_func (char *arg, int flags)
{
  int len = grub_strlen (arg);

  switch (kernel_type)
    {
    case KERNEL_TYPE_MULTIBOOT:
      if (mb_cmdline + len + 1 > (char *) MB_CMDLINE_BUF + MB_CMDLINE_BUFLEN)
        {
          errnum = ERR_WONT_FIT;
          u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
          return 1;
        }
      grub_memmove (mb_cmdline, arg, len + 1);
      if (! load_module (arg, mb_cmdline))
        return 1;
      mb_cmdline += len + 1;
      break;

    //case KERNEL_TYPE_LINUX:
    //case KERNEL_TYPE_BIG_LINUX:
    //  if (! load_initrd (arg))
    //    return 1;
    //  break;

    default:
      errnum = ERR_NEED_MB_KERNEL;
      u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      return 1;
    }

  return 0;
}

/* modaddr */
int
modaddr_func (char *arg, int flags)
{
  int addr;

  switch (kernel_type)
    {
    case KERNEL_TYPE_MULTIBOOT:
      if (safe_parse_maxint(&arg, &addr))
        {
          set_load_addr(addr);
          break;
        }

      /* else fallthrough */

    default:
      errnum = ERR_NEED_MB_KERNEL;
      u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      return 1;
    }

  return 0;
}

/* lipmod */
int
lipmodule_func (char *arg, int flags)
{
  create_lip_module(&l);
  return 0;
}

/* vbeset MODE */
int
vbeset_func (char *arg, int flags)
{
#ifndef GRUB_UTIL
  int mode_number;
  int pmif_segoff, pmif_len;
  struct vbe_controller controller;
  struct vbe_mode mode;

  if (kernel_type != KERNEL_TYPE_MULTIBOOT)
    {
      grub_printf("Multiboot kernel must be loaded before vbeset command\n");
      errnum = MAX_ERR_NUM;
      return 1;
    }

  safe_parse_maxint (&arg, &mode_number);

  if (u_vbectl(&controller, mode_number, 
               &mode, (unsigned int *)&pmif_segoff, 
               (unsigned int *)&pmif_len) == -1)
    return 1;

  create_vbe_module(&controller, sizeof(struct vbe_controller),
		    &mode, sizeof(struct vbe_mode),
		    mode_number, pmif_segoff, pmif_len, controller.version);

  /* mode setting was successful */
  return 0;
#else
  errnum = ERR_BAD_ARGUMENT;
  return 1;
#endif
}
