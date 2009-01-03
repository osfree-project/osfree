#define INCL_DOS
#define INCL_BSEDOS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include <os2.h>

#include <os2errcodes.h>
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
    #ifdef __OS2__
    // This is check for internal relocations support. Specific for OS/2 host because
    // it is hard to manage virtual memory and processes without kernel driver support.
    // We don't want to loss forces for such driver so debug only code with internal relocations
    // support
    #endif
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
  int rc=NO_ERROR;
  do_fixup_code_data_lx((struct LX_module *)lx_exe_mod, &rc);
  /* Apply fixups. */
  return rc; /*NO_ERROR;*/
}
