#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#ifdef __cplusplus
  extern "C" {
#endif

#include <l4/os3/MountReg.h>

extern struct FSRouter fsrouter;
void init_globals(void);

#ifdef __cplusplus
  }
#endif

#endif
