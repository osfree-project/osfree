#ifndef __OS3_IO_H__
#define __OS3_IO_H__

#ifdef __cplusplus
  extern "C" {
#endif

#include <l4/sys/types.h>

void io_log(const char* format, ...);

int  io_load_file(const char *filename, l4_addr_t *addr, l4_size_t *size);
int  io_close_file(l4_addr_t address);

#ifdef __cplusplus
  }
#endif

#endif
