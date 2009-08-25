#define INCL_ERRORS
//#include <os2.h>


#ifndef ENEWHDR
  #define ENEWHDR 0x3c
#endif

//#include <ne.h>

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
    return 0/*NO_ERROR*/;
  };

  return 11/*ERROR_BAD_FORMAT*/;
}

unsigned long NELoad(void * addr, unsigned long size, void * ixfModule)
{
  return 11/*ERROR_BAD_FORMAT*/;
}

unsigned long NEFixup(void * Module)
{
  return /*ERROR_BAD_FORMAT*/;
}
