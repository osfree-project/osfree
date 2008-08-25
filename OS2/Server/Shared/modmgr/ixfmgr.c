/*! @file ixfmgr.c

    @brief Instalable eXecutable Format Manager. Provides support for
           different executable file formats.

*/

#define INCL_ERRORS
#include <os2.h>

#include <ixfmgr.h>
#include <modlx.h>

#ifndef ENEWHDR
  #define ENEWHDR 0x3c
#endif

unsigned long NEIdentify(void * addr, unsigned long size)
{
  unsigned long ne_module_header_offset=0;

  if (((*(char *)addr == 'M') && (*(char *)((unsigned long)addr+1) == 'Z')) ||
      ((*(char *)addr == 'Z') && (*(char *)((unsigned long)addr+1) == 'M')))
  {
    /* Found DOS stub. Get offset of NE module. */
    ne_module_header_offset=*(unsigned long *)((unsigned long)addr+ENEWHDR);
  }

  if ((*(char *)((unsigned long)addr+ne_module_header_offset) == 'N') &&
      (*(char *)((unsigned long)addr+ne_module_header_offset+1) == 'E'))
  {
    return NO_ERROR;
  };

  return ERROR_BAD_FORMAT;
}

unsigned long NELoad(void * addr, unsigned long size, IXFModule ** ixfModule)
{
  return ERROR_BAD_FORMAT;
}

unsigned long NEFixup(IXFModule * ixfModule)
{
  return ERROR_BAD_FORMAT;
}

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

unsigned long LXLoad(void * addr, unsigned long size, struct LX_module ** lx_exe_mod)
{
  *lx_exe_mod = (struct LX_module *) malloc(sizeof(struct LX_module));

  if(load_lx_stream((char*)addr, size, *lx_exe_mod)) /* A file from a buffer.*/
  {
    load_dll_code_obj_lx(*lx_exe_mod); /* Load all objects in dll.*/
  }
  return NO_ERROR;
}

unsigned long LXFixup(struct LX_module * lx_exe_mod)
{
  do_fixup_code_data_lx(lx_exe_mod);
  /* Apply fixups. */
  return NO_ERROR;
}

typedef
  struct
  {
    fnIdentify * Identify;
    fnLoad * Load;
    fnFixup * Fixup;
  } IXFHandler;

IXFHandler IXFHandlers[2] =
  {
    NEIdentify,  NELoad,  NEFixup,
    LXIdentify,  LXLoad,  LXFixup
  };

#define maxFormats (sizeof(IXFHandlers)/sizeof(IXFHandler))

unsigned long IXFIdentifyModule(void * addr, unsigned long size, IXFModule * ixfModule)
{
  int i;
  unsigned int rc;

  for(i=0; i<maxFormats; i++)
  {
    rc=IXFHandlers[i].Identify(addr, size);
    if (rc==NO_ERROR)
    {
      ixfModule->Load=IXFHandlers[i].Load;
      ixfModule->Fixup=IXFHandlers[i].Fixup;
      break;
    }
  }
  return rc;
}

unsigned long IXFLoadModule(void * addr, unsigned long size, IXFModule * ixfModule)
{
  return ixfModule->Load(addr, size, &ixfModule->FormatStruct);
}

unsigned long IXFFixupModule(IXFModule * ixfModule)
{
  return ixfModule->Fixup(ixfModule->FormatStruct);
}
