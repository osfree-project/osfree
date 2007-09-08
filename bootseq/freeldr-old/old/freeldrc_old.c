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
#include "shared.h"
#include "dos.h"
#include "stdarg.h"
#include "imgact_aout.h"
#include "i386-elf.h"

/* The type of kernel loaded.  */
kernel_t kernel_type;
/* The boot device.  */
static int bootdev;
/* True when the debug mode is turned on, and false
   when it is turned off.  */
int debug = 0;
/* The default entry.  */
int default_entry = 0;
/* The fallback entry.  */
int fallback_entryno;
int fallback_entries[MAX_FALLBACK_ENTRIES];
/* The number of current entry.  */
int current_entryno;
/* The address for Multiboot command-line buffer.  */
static char *mb_cmdline;
/* The BIOS drive map.  */
static unsigned short bios_drive_map[DRIVE_MAP_SIZE + 1];

char fName[255];
unsigned long int fSize;
char far * farpfName;
unsigned long int far * farpfSize;


/* freeldrc.c 2/7/99 dcz */

#define VIDEO_HEIGHT 25   //высота экрана

//Вывод строки, заканчивающейся нуль-символом
void __cdecl puts(char *s)
{
  DispNTS(s);
}

int
memcheck (int addr, int len)
{
//  if ((addr < RAW_ADDR (0x1000))
//      || (addr < RAW_ADDR (0x100000)
//        && RAW_ADDR (mbi.mem_lower * 1024) < (addr + len))
//      || (addr >= RAW_ADDR (0x100000)
//        && RAW_ADDR (mbi.mem_upper * 1024) < ((addr - 0x100000) + len)))
    //printf("memcheck: ERR_WONT_FIT");

  return 0;
}


void *
memset (void *start, int c, int len)
{
  char *p = start;

  if (memcheck ((int) start, len))
    {
      while (len -- > 0)
        *p ++ = c;
    }

  return start;
}

strcmp (const char *s1, const char *s2)
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

int
strlen (const char *str)
{
  int len = 0;

  while (*str++ != '\0')
    len++;

  return len;
}

void *
memmove (void *_to, const void *_from, int _len)
{
   const char *    from = _from;
   char *          to = _to;

   if (memcheck ((int) _to, _len))
     {
        if( from == to ) {
            return( to );
        }
        if( from < to  &&  from + _len > to ) {
            to += _len;
            from += _len;
            while( _len != 0 ) {
                *--to = *--from;
                _len--;
            }
        } else {
            while( _len != 0 ) {
                *to++ = *from++;
                _len--;
            }
        }
     }

     return( to );
}


//Clear screen (actually, just scroll it up)
void __cdecl clear()
{
  int i;

  for (i = 0; i < VIDEO_HEIGHT; i++) {
    puts("\n\r");
  }

}


//Вывод одного символа в режиме телетайпа
void __cdecl putchar(char c)
{
  DispC(&c);
}


//#define putchar(c) DispC(&c)

void __cdecl puthexd(unsigned char digit)
{
  char table[]="0123456789ABCDEF";
  putchar(table[digit]);
}



void __cdecl putdec(unsigned int byte)
{
  unsigned char b1;
  int b[30];
  signed int nb;
  int i=0;

  while(1){
    b1=byte%10;
    b[i]=b1;
    nb=byte/10;
    if(nb<=0){
      break;
    }
    i++;
    byte=nb;
  }

  for(nb=i+1;nb>0;nb--){
    puthexd(b[nb-1]);
  }
}

void __cdecl puthex(unsigned char byte)
{
  unsigned  char lb, rb;

  lb=byte >> 4;

  rb=byte & 0x0F;


  puthexd(lb);
  puthexd(rb);
}

void __cdecl puthexw(unsigned int word)
{
  puthex( (word & 0xFF00) >>8);
  puthex( (word & 0x00FF));
}

void __cdecl puthexi(unsigned long int dword)
{
  puthex( (dword & 0xFF000000) >>24);
  puthex( (dword & 0x00FF0000) >>16);
  puthex( (dword & 0x0000FF00) >>8);
  puthex( (dword & 0x000000FF));
}


void __cdecl vprintf(const char *fmt, va_list args)
{
  while (*fmt) {

    switch (*fmt) {
    case '%':
      fmt++;

      switch (*fmt) {
      case 's':
        puts(va_arg(args, char *));
        break;

      case 'c':
        putchar(va_arg(args, unsigned short int));
        break;

      case 'i':
      case 'd':
        putdec(va_arg(args, unsigned short int));
        break;

      case 'I':
      case 'D':
        putdec(va_arg(args, unsigned long int));
        break;

      case 'x':
        puthex(va_arg(args, unsigned short int));
        break;

      case 'h':
        puthexw(va_arg(args, unsigned short int));
        break;

      case 'X':
        puthexi(va_arg(args, unsigned long int));
        break;

      }
      break;

    default:
      putchar(*fmt);
      break;
    }

    fmt++;
  }
}

void __cdecl printf(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  vprintf(fmt, args);

  va_end(args);
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


/* filesystem common variables */
int filepos;
int filemax;


static int cur_addr;
entry_func entry_addr;
static struct mod_list mll[99];
static int linux_mem_size;

extern struct multiboot_info mbi;

/*
 *  The next two functions, 'load_image' and 'load_module', are the building
 *  blocks of the multiboot loader component.  They handle essentially all
 *  of the gory details of loading in a bootable image and the modules.
 */

int
grub_read (char *buf, int len)
{
  printf("grub_read\n\r ");
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
      printf("ERR_FILELENGTH\n\r");
      return 0;
    }

  return muRead(filepos, buf, len);
}


/* Reposition a file offset.  */
int
grub_seek (int offset) // noch nicht fertig!
{
  if (offset > filemax || offset < 0)
    return -1;

  filepos = offset;
  return offset;
}

kernel_t
load_image (char *kernel, char *arg, kernel_t suggested_type,
            unsigned long load_flags)

{
        int len, i, exec_type = 0, align_4k = 1;
        entry_func real_entry_addr = 0;
        kernel_t type = KERNEL_TYPE_NONE;
        unsigned long flags = 0, text_len = 0, data_len = 0, bss_len = 0;
        char *str = 0, *str2 = 0;
        struct linux_kernel_header *lh;
        union
                {
                        struct multiboot_header *mb;
                        struct exec *aout;
                        Elf32_Ehdr *elf;
                }pu;

        struct SREGS segs;
        unsigned long int fSize;
        int rc;

        /* presuming that MULTIBOOT_SEARCH is large enough to encompass an
                executable header */
        unsigned char buffer[MULTIBOOT_SEARCH];

        /* sets the header pointer to point to the beginning of the
                buffer by default */
        pu.aout = (struct exec *) buffer;

//   segread(&segs);

//      strcpy(fName,kernel);
        printf("load_image: The kernel to load is %s\n\r",kernel);
//      farpfName = MK_FP(segs.ds,fName);
//      farpfSize = MK_FP(segs.ds,&fSize);
//      filemax = farpfSize;
        filepos = 0;
//      if (!(*muOpen) (farpfName,farpfSize))
        if (!muOpen(kernel,&fSize))
        {
            printf("Error opening file %s\n\r", kernel);
                return KERNEL_TYPE_NONE;
        }
        filemax=fSize;
    printf("Size of file %s %d\n\r", kernel, filemax);

        if ((!(len = grub_read (buffer, MULTIBOOT_SEARCH))) || (len < 32)) /* Ersetzen */
        {
                muClose();

                printf("ERR_EXEC_FORMAT len=%d\n\r", len);
                return KERNEL_TYPE_NONE;
        }

        for (i = 0; i < len; i++)
        {
                if (MULTIBOOT_FOUND ((int) (buffer + i), len - i))
                {
                        flags = ((struct multiboot_header *) (buffer + i))->flags;
                        if (flags & MULTIBOOT_UNSUPPORTED)
                        {
                                muClose(); /* Ersetzen */
                                //errnum = ERR_BOOT_FEATURES;
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
                        || strcmp (pu.elf->e_ident + EI_BRAND, "FreeBSD") == 0 /*
Ersetzen */
                        || suggested_type == KERNEL_TYPE_NETBSD)
                        && len > sizeof (Elf32_Ehdr)
                        && BOOTABLE_I386_ELF ((*((Elf32_Ehdr *) buffer))))
        {
                if (type == KERNEL_TYPE_MULTIBOOT)
                        entry_addr = (entry_func) pu.elf->e_entry;
                else entry_addr = (entry_func) (pu.elf->e_entry & 0xFFFFFF);

                if (entry_addr < (entry_func) 0x100000) printf("Below 1MB!\n\r");

                /* don't want to deal with ELF program header at some random
                        place in the file -- this generally won't happen */

                if (pu.elf->e_phoff == 0 || pu.elf->e_phnum == 0 || ((pu.elf->e_phoff + (pu.elf->e_phentsize * pu.elf->e_phnum)) >= len))
                        printf("ERR_EXEC_FORMAT\n\r");
                        //errnum = ERR_EXEC_FORMAT;
                str = "elf";
        }
        else if (flags & MULTIBOOT_AOUT_KLUDGE)
        {
                pu.mb = (struct multiboot_header *) (buffer + i);
                entry_addr = (entry_func) pu.mb->entry_addr;
                cur_addr = pu.mb->load_addr;
                /* first offset into file */
                grub_seek (i - (pu.mb->header_addr - cur_addr));

                /* If the load end address is zero, load the whole contents.  */
                if (! pu.mb->load_end_addr) pu.mb->load_end_addr = cur_addr + filemax;

                text_len = pu.mb->load_end_addr - cur_addr;
                data_len = 0;

                /* If the bss end address is zero, assume that there is no bss area.  */
                if (! pu.mb->bss_end_addr)      pu.mb->bss_end_addr = pu.mb->load_end_addr;

                bss_len = pu.mb->bss_end_addr - pu.mb->load_end_addr;

                if (pu.mb->header_addr < pu.mb->load_addr
                                || pu.mb->load_end_addr <= pu.mb->load_addr
                                || pu.mb->bss_end_addr < pu.mb->load_end_addr
                                || (pu.mb->header_addr - pu.mb->load_addr) > i)
                        //errnum = ERR_EXEC_FORMAT;
                        printf("ERR_EXEC_FORMAT\n\r");

                if (cur_addr < 0x100000) printf("Below 1MB\n\r");

                pu.aout = (struct exec *) buffer;
                exec_type = 2;
                str = "kludge";
        }
        else if (len > sizeof (struct exec) && !N_BADMAG ((*(pu.aout))))
        {
                entry_addr = (entry_func) pu.aout->a_entry;

                /* first offset into file */
                grub_seek (N_TXTOFF (*(pu.aout)));
                text_len = pu.aout->a_text;
                data_len = pu.aout->a_data;
                bss_len = pu.aout->a_bss;

                if (cur_addr < 0x100000) printf("Below 1MB\n\r");

                exec_type = 1;
                str = "a.out";
        }

        else                            /* no recognizable format */
                printf("ERR_EXEC_FORMAT\n\r");

        /* return if error
        if (errnum)
        {
        muClose();
        return KERNEL_TYPE_NONE;
        }
        */
        /* fill the multiboot info structure */
        mbi.cmdline = (int) arg;
        mbi.mods_count = 0;
        mbi.mods_addr = 0;
 //     mbi.boot_device = (current_drive << 24) | current_partition;
        mbi.flags &= ~(MB_INFO_MODS | MB_INFO_AOUT_SYMS | MB_INFO_ELF_SHDR);
        mbi.syms.a.tabsize = 0;
        mbi.syms.a.strsize = 0;
        mbi.syms.a.addr = 0;
        mbi.syms.a.pad = 0;

        printf ("   [%s-%s", str2, str);

        str = "";

        if (exec_type)          /* can be loaded like a.out */
        {
        if (flags & MULTIBOOT_AOUT_KLUDGE) str = "-and-data";

        printf (", loadaddr=0x%x, text%s=0x%x", cur_addr, str, text_len);

        /* read text, then read data */
        if (grub_read ((char *) RAW_ADDR (cur_addr), text_len) == text_len)
                {
                        cur_addr += text_len;

                        if (!(flags & MULTIBOOT_AOUT_KLUDGE))
                        {
                        /* we have to align to a 4K boundary */
                        if (align_4k) cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;
                                else printf (", C");

                        printf (", data=0x%x", data_len);

                if ((grub_read ((char *) RAW_ADDR (cur_addr), data_len)
                                        != data_len) /*&& !errnum*/)
                                        printf("ERR_EXEC_FORMAT\n\r");
                cur_addr += data_len;
                }

                        /*if (!errnum)
                {
                memset ((char *) RAW_ADDR (cur_addr), 0, bss_len);
                cur_addr += bss_len;

                printf (", bss=0x%x", bss_len);
                }*/
                }
        /*else if (!errnum) errnum = ERR_EXEC_FORMAT;*/

        if(/*!errnum &&*/ pu.aout->a_syms && pu.aout->a_syms < (filemax - filepos))
                {
                        int symtab_err, orig_addr = cur_addr;

                        /* we should align to a 4K boundary here for good measure */
                        if (align_4k) cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

                        mbi.syms.a.addr = cur_addr;

                        *((int *) RAW_ADDR (cur_addr)) = pu.aout->a_syms;
                        cur_addr += sizeof (int);

                        printf (", symtab=0x%x", pu.aout->a_syms);

                        if (grub_read ((char *) RAW_ADDR (cur_addr), pu.aout->a_syms)
                        == pu.aout->a_syms)
                {
                cur_addr += pu.aout->a_syms;
                mbi.syms.a.tabsize = pu.aout->a_syms;

                if (grub_read ((char *) &i, sizeof (int)) == sizeof (int))
                                {
                                        *((int *) RAW_ADDR (cur_addr)) = i;
                                        cur_addr += sizeof (int);

                                        mbi.syms.a.strsize = i;

                                        i -= sizeof (int);

                                        printf (", strtab=0x%x", i);

                                        symtab_err =(grub_read ((char *)RAW_ADDR (cur_addr), i) != i);
                                        cur_addr += i;
                                }
                        else symtab_err = 1;
                        }
                        else symtab_err = 1;

                        if (symtab_err)
                        {
                        printf ("(bad)");
                        cur_addr = orig_addr;
                        mbi.syms.a.tabsize = 0;
                        mbi.syms.a.strsize = 0;
                        mbi.syms.a.addr = 0;
                        }
                        else mbi.flags |= MB_INFO_AOUT_SYMS;
                }
   }
        else
   /* ELF executable */
        {
      unsigned loaded = 0, memaddr, memsiz, filesiz;
      Elf32_Phdr *phdr;

      /* reset this to zero for now */
      cur_addr = 0;

      /* scan for program segments */
      for (i = 0; i < pu.elf->e_phnum; i++)
                {
                        phdr = (Elf32_Phdr *)
                (pu.elf->e_phoff + ((int) buffer) + (pu.elf->e_phentsize * i));
                        if (phdr->p_type == PT_LOAD)
                {
                        /* offset into file */
                        grub_seek (phdr->p_offset);
                        filesiz = phdr->p_filesz;

                if (type == KERNEL_TYPE_FREEBSD || type == KERNEL_TYPE_NETBSD)
                                        memaddr = RAW_ADDR (phdr->p_paddr & 0xFFFFFF);
                else memaddr = RAW_ADDR (phdr->p_paddr);

                memsiz = phdr->p_memsz;
                if (memaddr < RAW_ADDR (0x100000)) printf("Below 1MB!\n\r");

                /* If the memory range contains the entry address, get the
                                                physical address here.  */
                if (type == KERNEL_TYPE_MULTIBOOT
                                                && (unsigned) entry_addr >= phdr->p_vaddr
                                                && (unsigned) entry_addr < phdr->p_vaddr + memsiz)
                                                real_entry_addr = (entry_func) ((unsigned) entry_addr
                                                + memaddr - phdr->p_vaddr);

                /* make sure we only load what we're supposed to! */
                if (filesiz > memsiz) filesiz = memsiz;
                /* mark memory as used */
                if (cur_addr < memaddr + memsiz) cur_addr = memaddr + memsiz;
                        printf (", <0x%x:0x%x:0x%x>", memaddr, filesiz, memsiz - filesiz);
                /* increment number of segments */
                loaded++;

                /* load the segment */
                if (memcheck (memaddr, memsiz)
                                                && grub_read ((char *) memaddr, filesiz) == filesiz)
                                {
                                        if (memsiz > filesiz)
                                        memset ((char *) (memaddr + filesiz), 0, memsiz - filesiz);
                                }
                else break;
                }
                }

      if (! 1 /*errnum*/)
                {
                        if (! loaded)
                        printf(" ERR_EXEC_FORMAT\n\r");
                        else
                {
                /* Load ELF symbols.  */
                Elf32_Shdr *shdr = NULL;
                int tab_size, sec_size;
                int symtab_err = 0;

                           mbi.syms.e.num = pu.elf->e_shnum;
                mbi.syms.e.size = pu.elf->e_shentsize;
                mbi.syms.e.shndx = pu.elf->e_shstrndx;

                /* We should align to a 4K boundary here for good measure.  */
                if (align_4k)
                                        cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

                tab_size = pu.elf->e_shentsize * pu.elf->e_shnum;

                grub_seek (pu.elf->e_shoff);
                if (grub_read ((char *) RAW_ADDR (cur_addr), tab_size)
                                                == tab_size)
                                {
                                        mbi.syms.e.addr = cur_addr;
                                        shdr = (Elf32_Shdr *) mbi.syms.e.addr;
                                        cur_addr += tab_size;

                                        printf (", shtab=0x%x", cur_addr);

                                        for (i = 0; i < mbi.syms.e.num; i++)
                                {
                                /* This section is a loaded section,
                                                        so we don't care.  */
                                if (shdr[i].sh_addr != 0) continue;

                                /* This section is empty, so we don't care.  */
                                if (shdr[i].sh_size == 0)
                                                        continue;

                                /* Align the section to a sh_addralign bits boundary.  */
                                cur_addr = ((cur_addr + shdr[i].sh_addralign) &
                                                                                - (int) shdr[i].sh_addralign);

                                grub_seek (shdr[i].sh_offset);

                                sec_size = shdr[i].sh_size;

                                if (! (memcheck (cur_addr, sec_size)
                                                        && (grub_read ((char *) RAW_ADDR (cur_addr), sec_size)
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
                                        printf ("(bad)");
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

        if (! 1 /*errnum*/)
        {
                 printf (", entry=0x%x]\n\r", (unsigned) entry_addr);

        /* If the entry address is physically different from that of the ELF
                        header, correct it here.  */
        if (real_entry_addr)    entry_addr = real_entry_addr;
   }
        else
   {
      putchar ('\n');
        type = KERNEL_TYPE_NONE;
   }

        muClose();

        /* Sanity check.  */
        if (suggested_type != KERNEL_TYPE_NONE && suggested_type != type)
   {
      //errnum = ERR_EXEC_FORMAT;
                printf("ERR_EXEC_FORMAT\n\r");
        return KERNEL_TYPE_NONE;
   }

        return type;
}




int
load_module (char *module, char *arg)
{
        int len;

   struct SREGS segs;
   int rc;
   unsigned long int fSize;

   segread(&segs);


        /* if we are supposed to load on 4K boundaries */
        cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

//      strcpy(fName,module);

//      farpfName = MK_FP(segs.ds,fName);
//   farpfSize = MK_FP(segs.ds,&fSize);

        filepos = 0;
//      filemax = farpfSize;
//   if (!(*muOpen) (farpfName,farpfSize)) return 0;
   if (!muOpen(module,&fSize))
   {
     printf("Error opening file %s\n\r", module);
     return 0;
   }
   filemax=fSize;
   printf("Size of file %s %d\n\r", module, filemax);

   len = grub_read ((char *) cur_addr, -1);
        if (! len)
   {
        muClose();
      return 0;
   }

        printf ("   [Multiboot-module @ 0x%x, 0x%x bytes]\n\r", cur_addr, len);

        /* these two simply need to be set if any modules are loaded at all */
        mbi.flags |= MB_INFO_MODS;
        mbi.mods_addr = (int) mll;

        mll[mbi.mods_count].cmdline = (int) arg;
        mll[mbi.mods_count].mod_start = cur_addr;
        cur_addr += len;
        mll[mbi.mods_count].mod_end = cur_addr;
        mll[mbi.mods_count].pad = 0;

        /* increment number of modules included */
        mbi.mods_count++;

        muClose();
        return 1;
}


/* Find the next word from CMDLINE and return the pointer. If
   AFTER_EQUAL is non-zero, assume that the character `=' is treated as
   a space. Caution: this assumption is for backward compatibility.  */
char *
skip_to (int after_equal, char *cmdline)
{
  /* Skip until we hit whitespace, or maybe an equal sign. */
  while (*cmdline && *cmdline != ' ' && *cmdline != '\t' &&
    ! (after_equal && *cmdline == '='))
    cmdline ++;

  /* Skip whitespace, and maybe equal signs. */
  while (*cmdline == ' ' || *cmdline == '\t' ||
    (after_equal && *cmdline == '='))
    cmdline ++;

  return cmdline;
}


/* boot */
static int
boot_func (char *arg, int flags)
{
  /* Clear the int15 handler if we can boot the kernel successfully.
     This assumes that the boot code never fails only if KERNEL_TYPE is
     not KERNEL_TYPE_NONE. Is this assumption is bad?  */
  if (kernel_type != KERNEL_TYPE_NONE)
    //unset_int15_handler ();
        printf("unset_int15_handler();\n\r");
  switch (kernel_type)
    {
    case KERNEL_TYPE_MULTIBOOT:
      /* Multiboot */
      //multi_boot ((int) entry_addr, (int) &mbi);
                printf("Multiboot\n\r");
      break;

    default:
      //errnum = ERR_BOOT_COMMAND;
      return 1;
    }

  return 0;
}



/* kernel */
static int
kernel_func (char *arg, int flags)
{
  int len;
  kernel_t suggested_type = KERNEL_TYPE_NONE;
  unsigned long load_flags = 0;

  /* Deal with GNU-style long options.  */
   printf("kernel_func: Attempting to load %s\n\r",arg);
 // while (1)
   // {
      /* If the option `--type=TYPE' is specified, convert the string to
         a kernel type.  */
   /*   if (grub_memcmp (arg, "--type=", 7) == 0)
        {
          arg += 7;

          if (grub_memcmp (arg, "multiboot", 9) == 0)*/
            suggested_type = KERNEL_TYPE_MULTIBOOT;
                /*
          else
            {
              //errnum = ERR_BAD_ARGUMENT;
              return 1;
            }
        }*/
      /* If the `--no-mem-option' is specified, don't pass a Linux's mem
         option automatically. If the kernel is another type, this flag
         has no effect.  */
     /* else if (grub_memcmp (arg, "--no-mem-option", 15) == 0)
        load_flags |= KERNEL_LOAD_NO_MEM_OPTION;
      else
        break;*/

      /* Try the next.  */
     // arg = skip_to (0, arg);
    //}

  len = strlen (arg);
        printf("kernel_func: strlen arg = %d\n\r",len);
  /* Reset MB_CMDLINE.  */
  mb_cmdline = (char *) MB_CMDLINE_BUF;
  if (len + 1 > MB_CMDLINE_BUFLEN)
    {
      printf("ERR_WONT_FIT\n\r");
      return 1;
    }

  /* Copy the command-line to MB_CMDLINE.  */
  memmove(mb_cmdline, arg, len + 1);
  kernel_type = load_image (arg, mb_cmdline, suggested_type, load_flags);
  if (kernel_type == KERNEL_TYPE_NONE)
  {
    printf("unknown kernel type\n\r");
    return 1;
  }

  mb_cmdline += len + 1;
  return 0;
}


/* module */
static int
module_func (char *arg, int flags)
{
  int len = strlen (arg);
  printf("module_func: %s\n\r",arg);
  switch (kernel_type)
    {
    case KERNEL_TYPE_MULTIBOOT:
      if (mb_cmdline + len + 1 > (char *) MB_CMDLINE_BUF + MB_CMDLINE_BUFLEN)
        {
          //errnum = ERR_WONT_FIT;
          return 1;
        }
      memmove (mb_cmdline, arg, len + 1);
      if (! load_module (arg, mb_cmdline))
        return 1;
      mb_cmdline += len + 1;
      break;

    }

  return 0;
}

void near __cdecl auxil()
{
  printf("OS2LDR:FREELDRC.C Start %i\n\r",7993);
}

void __cdecl KernelLoader(void far * filetbl)
{
 unsigned short int word1,word2;
 unsigned long int fSize;
 struct SREGS segs;
 int rc;

// segread(&segs);

 /* Open is 1A and 1C */
// word1 = *(unsigned short int *)(((unsigned char *) filetbl) + 0x1a);
// word2 = *(unsigned short int *)(((unsigned char *) filetbl) + 0x1c);

// printf("OS2LDR:FREELDRC.C: for Open: word1 is %h,word2 is %h\n\r",
//      word1,word2);

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
// printf("%s %s\n\r",fName, farpfName);
// rc = (*muOpen) (fName,&fSize);
// printf("freeldrc.c: 1 fSize is %D\n\r", fSize);
// rc = (*muOpen) ("OS2KRNL",&fSize);

// printf("freeldrc.c: 2 fSize is %D\n\r", fSize);

/* Kernel loader code fits in here!!! filetbl bzw. muOpen muss
 * mitgegeben werden! */
   if(!kernel_func("OS2LDR",0x2))
                printf("An error occured during execution of kernel_func\n\r");
   module_func("ia32-kernel",0x2);
   module_func("sigma0",0x2);
   module_func("pingpong",0x2);
   boot_func("Argumete einfuegen","!!!");
   muTerminate();
}

void __STK(){}
#pragma aux __STK "*";

