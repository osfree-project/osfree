#ifndef __OS3_IO_H__
#define __OS3_IO_H__

#include <l4/sys/types.h>

void io_printf(const char* chrFormat, ...);
int  io_load_file(const char *filename, l4_addr_t *addr, l4_size_t *size);
int  io_close_file(l4_addr_t address);


#endif
