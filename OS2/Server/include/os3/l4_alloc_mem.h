#ifndef __L4_ALLOC_MEM_H__
#define __L4_ALLOC_MEM_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/dataspace.h>

void l4_test_mem_alloc(void);
void *l4_alloc_mem(unsigned long long area, int base, int size, int flags,
                   unsigned long PIC, l4_os3_dataspace_t *ds);
int l4_translate_os2_flags(int flags);

#ifdef __cplusplus
  }
#endif

#endif /* __L4_ALLOC_MEM_H__ */
