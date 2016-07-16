#ifndef _LX_H_
#define _LX_H_

#ifdef __cplusplus
  extern "C" {
#endif

/* Public functions */

unsigned long LXIdentify(void * addr, unsigned long size);
unsigned long LXLoad(void * addr, unsigned long size, void * ixfModule);
unsigned long LXFixup(void * lx_exe_mod);

/* Private functions */

/* Read in the header for the file from a memory buffer.
   ALso an constructor for struct LX_module. */
struct LX_module * LXLoadStream(char * stream_fh, int str_size, struct LX_module * lx_mod);

#define ISDLL(ixf) (E32_MFLAGS(*((struct LX_module *)(ixf->FormatStruct))->lx_head_e32_exe) & E32MODMASK) == E32MODDLL

#ifdef __cplusplus
  }
#endif

#endif
