/* OS/2 API includes */
#define INCL_ERRORS
#include <os2.h>

#ifndef ENEWHDR
  #define ENEWHDR 0x3c
#endif

/* osFree internal */
#include <os3/ne.h>

unsigned long NEIdentify(void * addr, unsigned long size)
{
  unsigned long ne_module_header_offset = 0;

  if (((*(char *)addr == 'M') && (*((char *)addr+1) == 'Z')) ||
      ((*(char *)addr == 'Z') && (*((char *)addr+1) == 'M')))
  {
    /* Found DOS stub. Get offset of NE module. */
    ne_module_header_offset = *(unsigned int *)((char *)addr+ENEWHDR);
  }

  if ((*((char *)addr + ne_module_header_offset) == 'N') &&
      (*((char *)addr + ne_module_header_offset+1) == 'E'))
  {
    return NO_ERROR;
  }

  return ERROR_BAD_FORMAT;
}

unsigned long NELoad(void * addr, unsigned long size, void * ixfModule)
{
  return ERROR_BAD_FORMAT;
}

unsigned long NEFixup(void * Module)
{
  return ERROR_BAD_FORMAT;
}
