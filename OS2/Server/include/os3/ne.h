#ifndef __OS3_NE_H__
#define __OS3_NE_H__

#ifdef __cplusplus
  extern "C" {
#endif

unsigned long NEIdentify(void * addr, unsigned long size);
unsigned long NELoad(void * addr, unsigned long size, void * ixfModule);
unsigned long NEFixup(void * lx_exe_mod);

#ifdef __cplusplus
  }
#endif

#endif
