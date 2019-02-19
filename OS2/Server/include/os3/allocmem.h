#ifndef __ALLOCMEM_H__
#define __ALLOCMEM_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/dataspace.h>

void *allocmem(unsigned long long area, int base, int size, int flags,
               unsigned long PIC, l4_os3_dataspace_t *ds);

int translate_os2_flags(int flags);

#ifdef __cplusplus
  }
#endif

#endif /* __ALLOCMEM_H__ */
