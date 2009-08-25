#ifndef _L4_ALLOM_MEM_H_
#define _L4_ALLOM_MEM_H_

void l4_test_mem_alloc(void);
void * l4_alloc_mem(int base, int size, int flags, unsigned long PIC);
int l4_translate_os2_flags(int flags);

#endif
