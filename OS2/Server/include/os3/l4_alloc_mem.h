#ifndef _L4_ALLOM_MEM_H_
#define _L4_ALLOM_MEM_H_

#ifdef __cplusplus
  extern "C" {
#endif

void l4_test_mem_alloc(void);
void * l4_alloc_mem(unsigned long long area, int base, int size, int flags, unsigned long PIC, void *ds);
int l4_translate_os2_flags(int flags);

#ifdef __cplusplus
  }
#endif

#endif
