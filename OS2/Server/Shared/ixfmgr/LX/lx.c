#define INCL_ERRORS
#include <os2.h>

#include <lx.h>

#ifndef ENEWHDR
  #define ENEWHDR 0x3c
#endif

#include "modlx.h"
#include "loadobjlx.h"

unsigned long LXIdentify(void * addr, unsigned long size)
{
  unsigned long lx_module_header_offset=0;

  if (((*(char *)addr == 'M') && (*(char *)((unsigned long)addr+1) == 'Z')) ||
      ((*(char *)addr == 'Z') && (*(char *)((unsigned long)addr+1) == 'M')))
  {
    /* Found DOS stub. Get offset of LX module. */
    lx_module_header_offset=*(unsigned long *)((unsigned long)addr+ENEWHDR);
  }

  if ((*(char *)((unsigned long)addr+lx_module_header_offset) == 'L') &&
      (*(char *)((unsigned long)addr+lx_module_header_offset+1) == 'X'))
  {
    return NO_ERROR;
  };

  return ERROR_BAD_FORMAT;
}

unsigned long LXLoad(void * addr, unsigned long size, void ** lx_exe_mod)
{
  *lx_exe_mod = malloc(sizeof(struct LX_module));

  if(load_lx_stream((char*)addr, size, (struct LX_module *)(*lx_exe_mod))) /* A file from a buffer.*/
  {
    load_dll_code_obj_lx((struct LX_module *)(*lx_exe_mod)); /* Load all objects in dll.*/
  }
  return NO_ERROR;
}

unsigned long LXFixup(void * lx_exe_mod)
{
  do_fixup_code_data_lx((struct LX_module *)lx_exe_mod);
  /* Apply fixups. */
  return NO_ERROR;
}
