/*
 *  Load and fixup stage2
 *  through using uXFD's.
 *
 *  (c) osFree project
 *  author: valerius
 */


#include "vsprintf.h"
#include "dos.h"
#include "stdarg.h"

#include "freeldr.h"

#include <uXFD/uXFD.h>
#include <lip.h>

// Loader Interface Page
lip_t lip;
// BPB of the boot partition
extern char BPBTable[31];

int load(unsigned long image, unsigned long size, unsigned long load_addr, struct exe_params far *p)
{
   unsigned long fsize, len;
   char far *file = "/boot/freeldr/xfd/bin.xfd";
   unsigned long buf = 0x30000; // 0x3000:0x0  -- uXFD load address
   int __cdecl (far *fmt_load) (unsigned long image,
                                unsigned long size,
                                unsigned long load_addr,
                                struct exe_params far *p);
   void *plip, *pbpb;
   int rc;

   printk("load(): loading %s", file);

   fsize = freeldr_open(file);
   if (fsize)
      printk("file %s opened, size = %lu", file, fsize);
   else
      printk("error opening file %s", file);

   if (fsize) {
      len = freeldr_read(buf, fsize);
      printk("read %lu bytes", len);
   }

   freeldr_close();

   plip     = &lip;
   pbpb     = &BPBTable;
   fmt_load = FP_FROM_PHYS(buf);

   __asm {
     mov  si, pbpb  // ds:si --> BPB
     mov  ax, ds
     mov  es, ax
     mov  di, plip  // es:di --> LIP
   }

   printk("calling load() function of the uXFD...");

   // Call uXFD load() function
   rc = fmt_load(image, size, load_addr, p);

   return rc;
}

void __cdecl
auxil()
{
  printk("==> OS2LDR:LOAD.C Started");
}

void Stage2Loader(void far *filetbl)
{
   char far *file = "/boot/freeldr/stage2";
   struct exe_params exe_parm;

   readbuf1 = MK_FP(current_seg + 0x1000, 0);
   readbuf  = PHYS_FROM_FP (readbuf1);

   lip.lip_open   = MK_FP(current_seg, freeldr_open);
   lip.lip_read   = MK_FP(current_seg, freeldr_read);
   lip.lip_close  = MK_FP(current_seg, freeldr_close);
   lip.lip_term   = MK_FP(current_seg, freeldr_term);
   lip.lip_seek   = MK_FP(current_seg, freeldr_seek);

   lip.lip_printk = MK_FP(current_seg, freeldr_printk);

   printk("Stage2 loader started!");

   if (!load(0, 0, 0, MK_FP(current_seg, &exe_parm)))
       printk("file %s loaded successfully", file);
   else
       printk("file %s is not loaded!");
}
