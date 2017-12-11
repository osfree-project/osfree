#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/MountReg.h>

extern struct FSRouter fsrouter;
void init_globals(void);

#ifdef __cplusplus
  }
#endif

#endif
