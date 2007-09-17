/*
 *
 *
 *
 */

#include <uXFD/uXFD.h>
#include <lip.h>

extern char  far bpb[31]; // ALIAS bpb=_BPBPtr
extern lip_t far *l;      // ALIAS l=_LIPPtr

/*
 *  \param  image      -- Phys address of the binary, loaded by uFSD
 *  \param  size       -- Size of the binary
 *  \param  load_addr  -- Optional binary load base. 0 if load at linked position,
 *                        load address otherwise
 *  \retval p          -- Far pointer to the structure with exe parameters, like
 *                        Stack pointer and segment, and also entry points
 *  \return            -- 0 if successful, non-zero if error.
 */

int __cdecl
fmt_load(unsigned long image,
         unsigned long size,
         unsigned long load_addr,
         struct exe_params far *p)
{

   l->lip_printk("Raw binary uXFD loaded!");
   l->lip_printk("uXFD finished");

   return 0;
};
