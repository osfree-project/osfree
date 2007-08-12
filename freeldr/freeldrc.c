/*
 *  freeldr  --  The osFree kernel loader
 *  Copyright (C) 2005-2006 osFree-project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "freeldr.h"
#include "vsprintf.h"
#include "shared.h"
//#include "i86.h"
#include "dos.h"
#include "stdarg.h"
#include "imgact_aout.h"
#include "i386-elf.h"


// Memory size
extern unsigned short Int12Value;
extern unsigned short Int1588Value;

/* Error code */
grub_error_t errnum = ERR_NONE;

/* The type of kernel loaded.  */
kernel_t kernel_type;

/* The address for Multiboot command-line buffer.  */
static char far *mb_cmdline;

// Multiboot info structure
struct multiboot_info mbi;
static struct mod_list far mll[99];

// hardcoded now
unsigned long current_drive = 0x80;
unsigned long current_partition = 0x0;

/* filesystem common variables */
unsigned long filepos;
unsigned long filemax;

static long cur_addr;
entry_func entry_addr;

//read buffer pointer for freeldr_read
unsigned char far *readbuf1;
//read buffer physical address
unsigned long readbuf;


// The size of a read buffer
#define LOAD_CHUNK_SIZE 0x4000

#define VIDEO_HEIGHT 25   //высота экрана

/* freeldrc.c 2/7/99 dcz */
//unsigned long fSize;
//char far *farpfName;
//unsigned long far *farpfSize;
//static unsigned long linux_mem_size;
/* The boot device.  */
// static int bootdev;
/* True when the debug mode is turned on, and false
   when it is turned off.  */
//int debug = 0;
/* The default entry.  */
//int default_entry = 0;
/* The fallback entry.  */
//int fallback_entryno;
//int fallback_entries[MAX_FALLBACK_ENTRIES];
/* The number of current entry.  */
//int current_entryno;
/* The BIOS drive map.  */
//static unsigned short bios_drive_map[DRIVE_MAP_SIZE + 1];


long
freeldr_memcheck (unsigned long addr, long len)
{
    // Physical address:
    if ( (addr < RAW_ADDR (0x1000))
        || ((addr <  RAW_ADDR (0x100000)) && (RAW_ADDR(mbi.mem_lower * 1024) < (addr + len)))
        || ((addr >= RAW_ADDR (0x100000)) && (RAW_ADDR(mbi.mem_upper * 1024) < ((addr - 0x100000) + len))) )
    {
        errnum = ERR_WONT_FIT;
        printk("freeldr_memcheck: ERR_WONT_FIT");
        printk("freeldr_memcheck(): addr = 0x%08lx, len = %u", addr, len);
        return 0;
    }

    return 1;
}


void far *
freeldr_memset (void far *start, char c, long len)
{
    char far *p = start;

    if (freeldr_memcheck ((unsigned long)PHYS_FROM_FP(start), len))
    {
        while (len -- > 0)
            *p ++ = c;
    }

    return start;
}


void far *
freeldr_memmove (void far *_to, const void far *_from, long _len)
{

    char far *from = (char far *)_from;
    char far *to   = _to;

    if (freeldr_memcheck ((unsigned long)PHYS_FROM_FP(_to), _len))
    {
        if ( from == to )
        {
            return( to );
        }
        if ( from < to  &&  from + _len > to )
        {
            to += _len;
            from += _len;
            while( _len != 0 )
            {
                *--to = *--from;
                _len--;
            }
        }
        else
        {
            while( _len != 0 )
            {
                *to++ = *from++;
                _len--;
            }
        }
    }

    return( to );
}


char far *
freeldr_strcpy (char far *dest, const char far *src)
{
  freeldr_memmove (dest, src, freeldr_strlen (src) + 1);
  return dest;
}


long
freeldr_strcmp (const char far *s1, const char far *s2)
{
  while (*s1 || *s2)
    {
      if (*s1 < *s2)
        return -1;
      else if (*s1 > *s2)
        return 1;
      s1 ++;
      s2 ++;
    }

  return 0;
}


long
freeldr_memcmp (const char far *s1, const char far *s2, long n)
{
  while (n)
    {
      if (*s1 < *s2)
        return -1;
      else if (*s1 > *s2)
        return 1;
      s1++;
      s2++;
      n--;
    }

  return 0;
}


int
freeldr_strlen (const char far *str)
{
  int len = 0;

  while (*str++)
    len++;

  return len;
}


//Clear screen (actually, just scroll it up)
void __cdecl
freeldr_clear()
{
    int i;

    for (i = 0; i < VIDEO_HEIGHT; i++)
    {
        puts("\r\n");
    }
}


/*********************************\
**                               **
**  Filesystem access functions. **
**  ~~~~~~~~~~~~~~~~~~~~~~~~~~~  **
\*********************************/


/* Open a file or directory on the active device. */
unsigned long __cdecl
freeldr_open (char *filename)
{
    unsigned long *fSize;

    if (muOpen(filename, fSize))
        return 0;

    filemax = (*fSize);
    filepos = 0;

    return (*fSize);
}

/* Read len bytes to the physical address buf
   from the current seek position           */
unsigned long __cdecl
freeldr_read (unsigned long buf, unsigned long len)
{
    unsigned short chunk;
    unsigned long l, fp, rc, read = 0;
    int ret;

    //printk("grub_read()");
    /* Make sure "filepos" is a sane value */
    if ((filepos < 0) || (filepos > filemax))
        filepos = filemax;

    /* Make sure "len" is a sane value */
    if ((len < 0) || (len > (filemax - filepos)))
        len = filemax - filepos;

    /* if target file position is past the end of
       the supported/configured filesize, then
       there is an error */
    if (filepos + len > filemax)
    {
        printk("ERR_FILELENGTH");
        return 0;
    }

    fp = filepos;
    l  = len;

    while (l > 0)
    {

        if (l < LOAD_CHUNK_SIZE)
        {
            chunk = l;
            l     = 0;
        }
        else
        {
            chunk = LOAD_CHUNK_SIZE;
            l    -= LOAD_CHUNK_SIZE;
        }

        // Read a chunk of a file to the read buffer
        rc = muRead((unsigned long) fp,
                    (unsigned char far *) readbuf1,
                    (unsigned long) chunk);

        // move it to the proper place
        ret = loadhigh(buf, chunk, readbuf);
        if (ret)
        {
            printk("loadhigh1 error, rc = 0x%04x", ret);
            return 0;
        }

        read += rc;
        fp   += rc;
        buf  += rc;

        if (rc < chunk) break;

    }

    return read;
}


/* Reposition a file offset.  */
unsigned long __cdecl
freeldr_seek (unsigned long offset) // noch nicht fertig!
{
    if (offset > filemax || offset < 0)
        return (long)(-1);

    filepos = offset;
    return offset;
}


/* Close current file. */
void __cdecl
freeldr_close (void)
{
    muClose();
}


/* Terminate the work with files. */
void __cdecl
freeldr_term (void)
{
    muTerminate();
}


/* boot.c - load and bootstrap a kernel */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004,2005  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 *  The next two functions, 'load_image' and 'load_module', are the building
 *  blocks of the multiboot loader component.  They handle essentially all
 *  of the gory details of loading in a bootable image and the modules.
 */

kernel_t
load_image (char far *kernel, char far *arg, kernel_t suggested_type,
            unsigned long load_flags)

{
    /* presuming that MULTIBOOT_SEARCH is large
       enough to encompass an executable header */
    unsigned char far buffer1[MULTIBOOT_SEARCH]; // far pointer
    unsigned long buffer;                        // physical address of the buffer

    unsigned long size, chunk = LOAD_CHUNK_SIZE;
    char kern[256];

    long exec_type = 0, align_4k = 1;
    long len, i;
    entry_func real_entry_addr = 0;
    kernel_t type = KERNEL_TYPE_NONE;
    unsigned long flags = 0, text_len = 0, data_len = 0, bss_len = 0;
    char *str = 0, *str2 = 0;
    //struct linux_kernel_header *lh;
    union
    {
         struct multiboot_header far *mb;
         struct exec far *aout;
         Elf32_Ehdr far *elf;
    } pu;

    //struct SREGS segs;
    unsigned long fSize;
    int rc, ret;

    /* sets the header pointer to point to the
       beginning of the buffer by default      */
    pu.aout = (struct exec far *)(buffer1);

    buffer = PHYS_FROM_FP(buffer1);

    //printk("buffer1 = 0x%04X:0x%04X", FP_SEG(buffer1), FP_OFF(buffer1));
    //printk("buffer  = 0x%08lX", buffer);

    //printk("suggested_type = %u", suggested_type);

    //return KERNEL_TYPE_NONE;  // Temporarily

    //printk("load_image: The kernel to load is %s",kernel);
    //filepos = 0;

    freeldr_strcpy(MK_FP(current_seg, kern), kernel);

    fSize = freeldr_open(kern);

    if (!fSize)
    {
        printk("Error opening file %s", kern);
        return KERNEL_TYPE_NONE;
    }

    freeldr_seek(0);

    //filemax = fSize;
    //printk("%s opened successfully, size: %lu", kernel, filemax);

    len = freeldr_read(buffer, MULTIBOOT_SEARCH);
    if (len < 32)
    {
        freeldr_close();
        printk("ERR_EXEC_FORMAT len=%lu", len);
        errnum = ERR_EXEC_FORMAT;
        return KERNEL_TYPE_NONE;
    }
    //else
    //{
    //    printk("len = %lu bytes read by grub_read()", len);
    //}

    for (i = 0; i < len; i++)
    {
        if (MULTIBOOT_FOUND ((long) (buffer1 + i), len - i))
        {
            flags = ((struct multiboot_header far *) (buffer1 + i))->flags;
            if (flags & MULTIBOOT_UNSUPPORTED)
            {
                freeldr_close(); /* Ersetzen */
                errnum = ERR_BOOT_FEATURES;
                return KERNEL_TYPE_NONE;
            }
            type = KERNEL_TYPE_MULTIBOOT;
            str2 = "Multiboot";
            break;
        }
    }


    /* ELF loading supported if multiboot, FreeBSD and NetBSD.  */
    if (((type == KERNEL_TYPE_MULTIBOOT && ! (flags & MULTIBOOT_AOUT_KLUDGE))
         || pu.elf->e_ident[EI_OSABI] == ELFOSABI_FREEBSD
         || freeldr_strcmp (pu.elf->e_ident + EI_BRAND, "FreeBSD") == 0 /* Ersetzen */
         || suggested_type == KERNEL_TYPE_NETBSD)
        && len > sizeof (Elf32_Ehdr)
        && BOOTABLE_I386_ELF ((*((Elf32_Ehdr far *) buffer1))))
    {
        if (type == KERNEL_TYPE_MULTIBOOT)
            entry_addr = (entry_func) pu.elf->e_entry;
        else entry_addr = (entry_func) (pu.elf->e_entry & 0xFFFFFF);

        if (entry_addr < (entry_func) 0x100000) printk("Below 1MB!");

        /* don't want to deal with ELF program header at some random
           place in the file -- this generally won't happen           */

        if (pu.elf->e_phoff == 0 ||
            pu.elf->e_phnum == 0 ||
            ((pu.elf->e_phoff + (pu.elf->e_phentsize * pu.elf->e_phnum)) >= len))
                    errnum = ERR_EXEC_FORMAT;
        str = "elf";
    }
    else if (flags & MULTIBOOT_AOUT_KLUDGE)
    {
        pu.mb = (struct multiboot_header far *) (buffer1 + i);
        entry_addr = (entry_func) pu.mb->entry_addr;
        cur_addr = pu.mb->load_addr;
        /* first offset into file */
        freeldr_seek (i - (pu.mb->header_addr - cur_addr));

        /* If the load end address is zero, load the whole contents.  */
        if (! pu.mb->load_end_addr)
            pu.mb->load_end_addr = cur_addr + filemax;

        text_len = pu.mb->load_end_addr - cur_addr;
        data_len = 0;

        /* If the bss end address is zero, assume that there is no bss area.  */
        if (! pu.mb->bss_end_addr)
            pu.mb->bss_end_addr = pu.mb->load_end_addr;

        bss_len = pu.mb->bss_end_addr - pu.mb->load_end_addr;

         if (pu.mb->header_addr < pu.mb->load_addr
          || pu.mb->load_end_addr <= pu.mb->load_addr
          || pu.mb->bss_end_addr < pu.mb->load_end_addr
          || (pu.mb->header_addr - pu.mb->load_addr) > i)
            errnum = ERR_EXEC_FORMAT;

        if (cur_addr < 0x100000)
            errnum = ERR_BELOW_1MB;

        pu.aout = (struct exec far *) buffer1;
        exec_type = 2;
        str = "kludge";
    }
    else if (len > sizeof (struct exec) && !N_BADMAG ((*(pu.aout))))
    {
        entry_addr = (entry_func) pu.aout->a_entry;

        /* first offset into file */
        freeldr_seek (N_TXTOFF (*(pu.aout)));
        text_len = pu.aout->a_text;
        data_len = pu.aout->a_data;
        bss_len = pu.aout->a_bss;

        if (cur_addr < 0x100000) printk("Below 1MB");

        exec_type = 1;
        str = "a.out";
    }
    else
    {
        /* no recognizable format */
        errnum = ERR_EXEC_FORMAT;
    }

    /* return if error */
    if (errnum)
    {
        freeldr_close();
        return KERNEL_TYPE_NONE;
    }

    /* fill the multiboot info structure */
    mbi.cmdline = (unsigned long) arg;
    mbi.mods_count = 0;
    mbi.mods_addr = 0;
    mbi.boot_device = (current_drive << 24) | current_partition;
    mbi.flags &= ~(MB_INFO_MODS | MB_INFO_AOUT_SYMS | MB_INFO_ELF_SHDR);
    mbi.syms.a.tabsize = 0;
    mbi.syms.a.strsize = 0;
    mbi.syms.a.addr = 0;
    mbi.syms.a.pad = 0;

    printkc ("   [%s-%s", str2, str);

    str = "";

    if (exec_type)   /* can be loaded like a.out */
    {
        if (flags & MULTIBOOT_AOUT_KLUDGE) str = "-and-data";

        printkc (", loadaddr=0x%lX, text%s=0x%lX", cur_addr, str, text_len);

        /* read text, then read data */
        if (freeldr_read (RAW_ADDR (cur_addr), text_len) == text_len)
        {
            cur_addr += text_len;

            if (!(flags & MULTIBOOT_AOUT_KLUDGE))
            {
                /* we have to align to a 4K boundary */
                if (align_4k) cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;
                else printkc (", C");

                printkc (", data=0x%lX", data_len);

                if ((freeldr_read (RAW_ADDR (cur_addr), data_len)
                     != data_len) && !errnum)
                {
                    cur_addr += data_len;
                }
            }

            if (!errnum)
            {
                //freeldr_memset ((void far *)(RAW_ADDR(cur_addr)), 0, bss_len);  //!!!!! cur_addr > 1 Mb!
                if (bss_len)
                {
                    freeldr_memset (readbuf1, 0, LOAD_CHUNK_SIZE);
                    size = bss_len;
                    chunk = LOAD_CHUNK_SIZE;
                    while (size)
                    {
                        if (size < chunk)
                        {
                            chunk = size;
                            size  = 0;
                        }
                        else
                        {
                            chunk = LOAD_CHUNK_SIZE;
                            size -= LOAD_CHUNK_SIZE;
                        }

                        ret = loadhigh(RAW_ADDR (cur_addr), chunk, readbuf);
                        if (ret)
                        {
                            printk("loadhigh5 error, rc = 0x%04x", ret);
                            return ERR_EXEC_FORMAT;
                        }
                    }
                }
                cur_addr += bss_len;

                printkc (", bss=0x%x", bss_len);
            }
        }
        else if (!errnum) errnum = ERR_EXEC_FORMAT;

        if(!errnum && pu.aout->a_syms && pu.aout->a_syms < (filemax - filepos))
        {
            unsigned long symtab_err, orig_addr = cur_addr;

            /* we should align to a 4K boundary here for good measure */
            if (align_4k) cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

            mbi.syms.a.addr = cur_addr;

            //*((int *) RAW_ADDR (cur_addr)) = pu.aout->a_syms;
            ret = loadhigh(RAW_ADDR (cur_addr), 4, PHYS_FROM_FP (MK_FP (current_seg, &pu.aout->a_syms)));
            if (ret)
            {
                printk("loadhigh2 error, rc = 0x%04x", ret);
                return ERR_EXEC_FORMAT;
            }

            cur_addr += sizeof (unsigned long);

            printkc (", symtab=0x%x", pu.aout->a_syms);

            if (freeldr_read (RAW_ADDR (cur_addr), pu.aout->a_syms)
                == pu.aout->a_syms)
            {
                cur_addr += pu.aout->a_syms;
                mbi.syms.a.tabsize = pu.aout->a_syms;

                if (freeldr_read ((unsigned long) PHYS_FROM_FP(MK_FP (current_seg, &i)),
                    sizeof (unsigned long)) == sizeof (unsigned long))
                {
                    //*((int *) RAW_ADDR (cur_addr)) = i;
                    ret = loadhigh(RAW_ADDR (cur_addr), 4, PHYS_FROM_FP (MK_FP (current_seg, &i)));
                    if (ret)
                    {
                        printk("loadhigh3 error, rc = 0x%04x", ret);
                        return ERR_EXEC_FORMAT;
                    }

                    cur_addr += sizeof (unsigned long);

                    mbi.syms.a.strsize = i;

                    i -= sizeof (unsigned long);

                    printkc (", strtab=0x%x", i);

                    symtab_err =(freeldr_read (RAW_ADDR (cur_addr), i) != i);
                    cur_addr += i;
                }
                else symtab_err = 1;
            }
            else symtab_err = 1;

            if (symtab_err)
            {
                printkc ("(bad)");
                cur_addr = orig_addr;
                mbi.syms.a.tabsize = 0;
                mbi.syms.a.strsize = 0;
                mbi.syms.a.addr = 0;
            }
            else mbi.flags |= MB_INFO_AOUT_SYMS;
        }
    }
    else /* ELF executable */
    {
        unsigned loaded = 0, memaddr, memsiz, filesiz;
        Elf32_Phdr far *phdr;

        /* reset this to zero for now */
        cur_addr = 0;

        /* scan for program segments */
        for (i = 0; i < pu.elf->e_phnum; i++)
        {
            phdr = (Elf32_Phdr far *)
            FP_FROM_PHYS(pu.elf->e_phoff + ((unsigned long) buffer) + (pu.elf->e_phentsize * i));
            if (phdr->p_type == PT_LOAD)
            {
                /* offset into file */
                freeldr_seek (phdr->p_offset);
                filesiz = phdr->p_filesz;

                if (type == KERNEL_TYPE_FREEBSD || type == KERNEL_TYPE_NETBSD)
                    memaddr = RAW_ADDR (phdr->p_paddr & 0xFFFFFF);
                else
                    memaddr = RAW_ADDR (phdr->p_paddr);

                memsiz = phdr->p_memsz;
                if (memaddr < RAW_ADDR (0x100000)) printkc("(Below 1MB!)");

                /* If the memory range contains the entry
                   address, get the physical address here.  */
                if (type == KERNEL_TYPE_MULTIBOOT
                    && (unsigned long) entry_addr >= phdr->p_vaddr
                    && (unsigned long) entry_addr < phdr->p_vaddr + memsiz)
                    real_entry_addr = (entry_func) ((unsigned long) entry_addr
                    + memaddr - phdr->p_vaddr);

                /* make sure we only load what we're supposed to! */
                if (filesiz > memsiz) filesiz = memsiz;
                /* mark memory as used */
                if (cur_addr < memaddr + memsiz) cur_addr = memaddr + memsiz;
                    printkc (", <0x%x:0x%x:0x%x>", memaddr, filesiz, memsiz - filesiz);
                /* increment number of segments */
                loaded++;

                /* load the segment */
                if (freeldr_memcheck (memaddr, memsiz)
                    && freeldr_read (memaddr, filesiz) == filesiz)
                {
                    if (memsiz > filesiz)
                    {
                        freeldr_memset (readbuf1, 0, LOAD_CHUNK_SIZE);
                        size = memsiz - filesiz;
                        chunk = LOAD_CHUNK_SIZE;
                        while (size > 0)
                        {
                            if (size < chunk)
                            {
                                chunk = size;
                                size  = 0;
                            }
                            else
                            {
                                chunk = LOAD_CHUNK_SIZE;
                                size -= LOAD_CHUNK_SIZE;
                            }

                            ret = loadhigh(RAW_ADDR (memaddr + filesiz), chunk, readbuf);
                            if (ret)
                            {
                                printk("loadhigh4 error, rc = 0x%04x", ret);
                                return ERR_EXEC_FORMAT;
                            }
                        }
                    }
                }
                else break;
            }
        }

        if (!errnum)
        {
            if (!loaded)
                //printk("ERR_EXEC_FORMAT");
                errnum = ERR_EXEC_FORMAT;
            else
            {
                /* Load ELF symbols.  */
                Elf32_Shdr *shdr = NULL;
                unsigned long tab_size, sec_size;
                unsigned long symtab_err = 0;

                mbi.syms.e.num = pu.elf->e_shnum;
                mbi.syms.e.size = pu.elf->e_shentsize;
                mbi.syms.e.shndx = pu.elf->e_shstrndx;

                /* We should align to a 4K boundary here for good measure.  */
                if (align_4k)
                    cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

                tab_size = pu.elf->e_shentsize * pu.elf->e_shnum;

                freeldr_seek (pu.elf->e_shoff);
                if (freeldr_read (RAW_ADDR (cur_addr), tab_size)
                    == tab_size)
                {
                    mbi.syms.e.addr = cur_addr;
                    shdr = (Elf32_Shdr *) mbi.syms.e.addr;
                    cur_addr += tab_size;

                    printkc (", shtab=0x%x", cur_addr);

                    for (i = 0; i < mbi.syms.e.num; i++)
                    {
                        /* This section is a loaded
                           section, so we don't care.  */
                        if (shdr[i].sh_addr != 0) continue;

                        /* This section is empty, so we don't care.  */
                        if (shdr[i].sh_size == 0)
                            continue;

                        /* Align the section to a sh_addralign bits boundary.  */
                        cur_addr = ((cur_addr + shdr[i].sh_addralign) &    //*** What's this???
                                     - (unsigned long) shdr[i].sh_addralign);

                        freeldr_seek (shdr[i].sh_offset);

                        sec_size = shdr[i].sh_size;

                        if (! (freeldr_memcheck (cur_addr, sec_size)
                               && (freeldr_read (RAW_ADDR (cur_addr), sec_size)
                               == sec_size)))
                        {
                            symtab_err = 1;
                            break;
                        }

                        shdr[i].sh_addr = cur_addr;
                        cur_addr += sec_size;
                    }
                }
                else symtab_err = 1;

                if (mbi.syms.e.addr < RAW_ADDR(0x10000)) symtab_err = 1;

                if (symtab_err)
                {
                    printkc ("(bad)");
                    mbi.syms.e.num = 0;
                    mbi.syms.e.size = 0;
                    mbi.syms.e.addr = 0;
                    mbi.syms.e.shndx = 0;
                    cur_addr = 0;
                }
                else mbi.flags |= MB_INFO_ELF_SHDR;
            }
        }
    }

    if (!errnum)
    {
        printk (", entry=0x%lx]", (unsigned long) entry_addr);

        /* If the entry address is physically different
           from that of the ELF header, correct it here.  */
        if (real_entry_addr) entry_addr = real_entry_addr;
    }
    else
    {
        putchar ('\n');
        type = KERNEL_TYPE_NONE;
    }

    freeldr_close();

    /* Sanity check.  */
    if (suggested_type != KERNEL_TYPE_NONE && suggested_type != type)
    {
        errnum = ERR_EXEC_FORMAT;
        //printk("ERR_EXEC_FORMAT");

        return KERNEL_TYPE_NONE;
    }

    return type;
}




unsigned long
load_module (char far *module, char far *arg)
{
    unsigned long len;
    //struct SREGS segs;
    int rc;
    unsigned long fSize;
    char mod[256];

    //segread(&segs);
    freeldr_strcpy(MK_FP(current_seg, mod), module);

    /* if we are supposed to load on 4K boundaries */
    cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

    //strcpy(fName,module);

    //farpfName = MK_FP(segs.ds,fName);
    //farpfSize = MK_FP(segs.ds,&fSize);

    //filepos = 0;
    //filemax = farpfSize;
    //if (!(*muOpen) (farpfName,farpfSize)) return 0;
    fSize = freeldr_open(mod);
    if (!fSize)
    {
        printk("Error opening file %s", module);
        return 0;
    }
    //filemax=fSize;
    //printk("Size of file %s: %lu", module, filemax);

    len = freeldr_read (cur_addr, -1);
    if (!len)
    {
        freeldr_close();
        return 0;
    }

    printk ("   [Multiboot-module @ 0x%lX, %lu bytes]", cur_addr, len);

    /* these two simply need to be set if any modules are loaded at all */
    mbi.flags |= MB_INFO_MODS;
    mbi.mods_addr = (unsigned long) PHYS_FROM_FP (mll);

    //mll[mbi.mods_count].cmdline = (int) arg;
    mll[mbi.mods_count].cmdline = (unsigned long) PHYS_FROM_FP (arg);
    mll[mbi.mods_count].mod_start = cur_addr;
    cur_addr += len;
    mll[mbi.mods_count].mod_end = cur_addr;
    mll[mbi.mods_count].pad = 0;

    /* increment number of modules included */
    mbi.mods_count++;

    freeldr_close();
    return 1;
}


/* Find the next word from CMDLINE and return the pointer. If
   AFTER_EQUAL is non-zero, assume that the character `=' is treated as
   a space. Caution: this assumption is for backward compatibility.  */
char far *
skip_to (unsigned long after_equal, char far *cmdline)
{
    /* Skip until we hit whitespace, or maybe an equal sign. */
    while (*cmdline && *cmdline != ' ' && *cmdline != '\t' &&
           !(after_equal && *cmdline == '='))
        cmdline ++;

    /* Skip whitespace, and maybe equal signs. */
    while (*cmdline == ' ' || *cmdline == '\t' ||
           (after_equal && *cmdline == '='))
        cmdline ++;

    return cmdline;
}


/* boot */
static unsigned long
boot_func (char far *arg, unsigned long flags)
{
    /* Clear the int15 handler if we can boot the kernel successfully.
       This assumes that the boot code never fails only if KERNEL_TYPE is
       not KERNEL_TYPE_NONE. Is this assumption is bad?  */
    //if (kernel_type != KERNEL_TYPE_NONE)
        //unset_int15_handler ();
        //printk("unset_int15_handler();");
    switch (kernel_type)
    {
        case KERNEL_TYPE_MULTIBOOT:
            /* Multiboot */
            //multi_boot ((int) entry_addr, (int) &mbi);
            //multi_boot ((unsigned long) entry_addr, (unsigned long) &mbi);
            printk("Multiboot");
            break;

        default:
            errnum = ERR_BOOT_COMMAND;
            return 1;
    }

    return 0;
}



/* kernel */
static unsigned long
kernel_func (char far *arg, unsigned long flags)
{
    unsigned short len;
    kernel_t suggested_type = KERNEL_TYPE_NONE;
    unsigned long load_flags = 0;

    /* Deal with GNU-style long options.  */
    printk("kernel_func: %s",arg);
    while (1)
    {
        /* If the option `--type=TYPE' is specified,
           convert the string to a kernel type.  */
        if (freeldr_memcmp (arg, "--type=", 7) == 0)
        {
            arg += 7;

            if (freeldr_memcmp (arg, "multiboot", 9) == 0)
                suggested_type = KERNEL_TYPE_MULTIBOOT;
            else
            {
                errnum = ERR_BAD_ARGUMENT;
                return 1;
            }
        }
        /* If the `--no-mem-option' is specified, don't pass a Linux's mem
           option automatically. If the kernel is another type, this flag
           has no effect.  */
        else
        if (freeldr_memcmp (arg, "--no-mem-option", 15) == 0)
            load_flags |= KERNEL_LOAD_NO_MEM_OPTION;
        else
            break;

        /* Try the next.  */
        arg = skip_to (0, arg);
    }

    len = freeldr_strlen (arg);
    //printk("kernel_func: strlen(%s) = %d", (char *) FP_OFF(arg), len);
    /* Reset MB_CMDLINE.  */
    mb_cmdline = (char far *) FP_FROM_PHYS(MB_CMDLINE_BUF);
    if (len + 1 > MB_CMDLINE_BUFLEN)
    {
        printk("ERR_WONT_FIT");
        return 1;
    }

    /* Copy the command-line to MB_CMDLINE.  */
    freeldr_memmove(mb_cmdline, arg, len + 1);
    kernel_type = load_image (arg, mb_cmdline, suggested_type, load_flags);
    if (kernel_type == KERNEL_TYPE_NONE)
    {
        printk("unknown kernel type");
        return 1;
    }

    mb_cmdline += len + 1;
    return 0;
}


/* module */
static unsigned long
module_func (char far *arg, unsigned long flags)
{
    unsigned long len = freeldr_strlen (arg);
    printk("module_func: %s", arg);
    switch (kernel_type)
    {
        case KERNEL_TYPE_MULTIBOOT:
            if (mb_cmdline + len + 1 > (char far *) FP_FROM_PHYS(MB_CMDLINE_BUF) + MB_CMDLINE_BUFLEN)
            {
                errnum = ERR_WONT_FIT;
                return 1;
            }
            freeldr_memmove (mb_cmdline, arg, len + 1);
            if (! load_module (arg, mb_cmdline))
            return 1;
            mb_cmdline += len + 1;
            break;

    }

    return 0;
}


void __cdecl
auxil()
{
  printk("==> OS2LDR:FREELDRC.C Started");
}


void __cdecl
KernelLoader(void far *filetbl)
{
    unsigned long int fSize;
    // unsigned short int word1,word2;
    // struct SREGS segs;
    // int rc;
    // segread(&segs);

    // printk("current_seg=0x%x", current_seg);

    /* Open is 1A and 1C */
    // word1 = *(unsigned short int *)(((unsigned char *) filetbl) + 0x1a);
    // word2 = *(unsigned short int *)(((unsigned char *) filetbl) + 0x1c);

    // printk("OS2LDR:FREELDRC.C: for Open: word1 is %h,word2 is %h\n\r",
    // word1,word2);

#if 0
    muOpen = *
    /* here's the cast! */
        ( int (far pascal * far *) (char far *,unsigned long int far *) )
        (((unsigned char far *) filetbl) + 0x1a);
    muClose = *
        (void (far pascal * far *) ())
        (((unsigned char far *) filetbl) + 0x1a);
    muRead = *
        (unsigned long int (far pascal * far *)(unsigned long int, unsigned char far *, unsigned long int) )
        (((unsigned char far *) filetbl) + 0x1a);
#endif

    // strcpy(fName,"kickstart");

    // farpfName = MK_FP(segs.ds,fName);
    // farpfSize = MK_FP(segs.ds,&fSize);
    // printk("%s %s\n\r",fName, farpfName);
    // rc = (*muOpen) (fName,&fSize);
    // printk("freeldrc.c: 1 fSize is %D\n\r", fSize);
    // rc = (*muOpen) ("OS2KRNL",&fSize);

    // printk("freeldrc.c: 2 fSize is %D\n\r", fSize);

    //mbi.mem_lower = 639;       // Hardcoded
    //mbi.mem_upper = 16384;     // (temporarily)

    // mbi.mem_lower = Int12Value;
    // mbi.mem_upper = Int1588Value;

    // If current_seg is a segment address of a loader,
    // set freeldr_read read buffer segment to current_seg + 64k
    readbuf1 = MK_FP(current_seg + 0x1000, 0);
    readbuf  = PHYS_FROM_FP (readbuf1);

    printk("KernelLoader: multiboot-load");

    /* Kernel loader code fits in here!!! filetbl bzw. muOpen muss
     * mitgegeben werden! */

    init_bios_info();

    if (kernel_func("kickstart",0x2))
        printk("An error occured during execution of kernel_func");

    if (module_func("ia32-kernel",0x2))
        printk("An error occured during execution of module_func");

    if (module_func("sigma0",0x2))
        printk("An error occured during execution of module_func");

    if (module_func("pingpong",0x2))
        printk("An error occured during execution of module_func");

    freeldr_term();

    if (mbi.mmap_length)
        mbi.flags |= MB_INFO_MEM_MAP;

    if (boot_func("kickstart",0x2))
        printk("An error occured during execution of boot_func");

    printk("(4) mbi.mem_lower = %u", mbi.mem_lower);
    printk("(4) mbi.mem_upper = %u", mbi.mem_upper);

    printk("mbi.flags = %08lx", mbi.flags);
    printk("mbi.mmap_length = %lu", mbi.mmap_length);
}

void __STK(){}
#pragma aux __STK "*";
