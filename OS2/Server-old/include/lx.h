#ifndef _LX_H_
#define _LX_H_

/* Public functions */

unsigned long LXIdentify(void * addr, unsigned long size);
unsigned long LXLoad(void * addr, unsigned long size, void * ixfModule);
unsigned long LXFixup(void * lx_exe_mod);

#endif
