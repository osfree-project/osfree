/*
 *  Load and fixup stage2
 *  through using uXFD's.
 *
 *  (c) osFree project
 *  author: valerius
 */

#include "freeldr.h"

#include "vsprintf.h"
#include "dos.h"
#include "stdarg.h"

#include <uXFD/uXFD.h>
#include <lip.h>

// Loader Interface Page
lip_t lip;
// BPB of the boot partition
extern char BPBTable[31];

char far *cpy(char far *dst, char far *src, int n)
{
   int i;

   for (i = 0; i < n; i++)
      dst[i] = src[i];

   return dst;
}

int xfd_call(char far *file,
             unsigned long image,
             unsigned long size,
             unsigned long load_addr,
             struct exe_params far *p)
{
   long fsize;
   unsigned long len;
   unsigned long buf    = 0x30000; // 0x3000:0x0  -- uXFD load address

   void *plip, *pbpb;
   int rc;

   int __cdecl (far *fmt_load) (unsigned long image,
                                unsigned long size,
                                unsigned long load_addr,
                                struct exe_params far *p);

   printk("xfd_call(): calling %s", file);

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

   printk("calling load() function of the uXFD...");
   printk("qwe: %d, %s", 3, "asd");

   __asm {
     mov  si, word ptr pbpb  // ds:si --> BPB
     mov  ax, ds
     mov  es, ax
     mov  di, word ptr plip  // es:di --> LIP
   }

   // Call uXFD load() function
   rc = fmt_load(image, size, load_addr, p);

   return rc;
}

int load(unsigned long image,
         unsigned long size,
         unsigned long load_addr,
         struct exe_params far *p)
{
   long fsize;
   unsigned long len;
   char far str[80];
   char far b[80];
   char far *file, far *line, far *s;
   int  cfgbuf_size = 0x400;
   char far *xfdpath = "/boot/freeldr/xfd/";
   char far *cfgfile = "xfd.cfg";
   unsigned long buf    = 0x30000;           // 0x3000:0x0  -- uXFD load address
   unsigned long cfgbuf = buf - cfgbuf_size; // 0x2f80:0x0  -- config file buffer
   char far *cfgbuf1;
   int rc = 1, r, q;

   freeldr_strcpy(str, xfdpath);
   cfgfile = freeldr_strcat(str, cfgfile);

   printk("config file is: %s", cfgfile);

   fsize = freeldr_open(cfgfile);
   if (fsize > cfgbuf_size)
   {
      printk("config size too large, must be < %u", cfgbuf_size);
      return -1;
   }
   if (fsize) {
      printk("file %s opened, size = %lu", cfgfile, fsize);
      len = freeldr_read(cfgbuf, fsize);
      printk("read %lu bytes", len);
      freeldr_close();
   }
   else {
      printk("error opening file %s", cfgfile);
      freeldr_close();
      return -1;
   }

   r       = 1;
   line    = (char far *)FP_FROM_PHYS(cfgbuf);
   cfgbuf1 = line;

   while (1)
   {
      r = freeldr_pos('\n', line);

      if (r) {
         s    = cpy(str, line, r - 1);
         line = line + r;
      }
      else {
         s    = cpy(str, line, cfgbuf1 + len - line - 1);
         r    = cfgbuf1 + len - line;
         line = cfgbuf1 + len;
      }

      s[r - 1] = '\0';

      if (s[r - 2] == '\r') s[r - 2] = '\0';
      if (s[0]     == '\r') ++s;
      if (line[0]  == '\r') ++line;

      q = freeldr_pos('#', s);
      if (q) s[q - 1] = '\0';

      if (s[0]) {
         freeldr_strcpy(b, xfdpath);
         file = freeldr_strcat(b, s);
         rc   = xfd_call(file, image, size, load_addr, p);
         if (!rc) break;
      }

      if (!r)                    break;
      if (r    >= cfgbuf_size)   break;
      if (line >= cfgbuf1 + len) break;
   }

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

   //lip.lip_printk  = MK_FP(current_seg, freeldr_printk);
   //lip.lip_printkc = MK_FP(current_seg, freeldr_printkc);

   printk("Stage2 loader started!");

   if (!load(0, 0, 0, MK_FP(current_seg, &exe_parm)))
       printk("file %s loaded successfully", file);
   else
       printk("file %s is not loaded!");
}
