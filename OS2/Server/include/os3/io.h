#ifndef __OS3_IO_H__
#define __OS3_IO_H__

#ifdef __cplusplus
  extern "C" {
#endif

void io_log(const char* format, ...);

int  fileprov_init(void);
int  fileprov_done(void);

int  io_load_file(const char *filename, void **addr, unsigned long *size);
int  io_close_file(void *address);

#ifdef __cplusplus
  }
#endif

#endif
