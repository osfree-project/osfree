/*
 *
 *
 *
 */

#include <uXFD/uXFD.h>
#include <lip.h>

extern far bpb[31];     // ALIAS bpb=_BPBPtr
extern lip_t far *l;    // ALIAS l=_LIPPtr

int fmt_load(unsigned long image, unsigned long size, unsigned long load_addr, struct exe_params far *p)
{

   l->lip_printk("raw binary uXFD loaded!");

   return 0;
};
