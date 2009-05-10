/* boot.c - load and bootstrap a kernel */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004  Free Software Foundation, Inc.
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


#include "shared.h"

#include "freebsd.h"
#include "imgact_aout.h"
#include "i386-elf.h"

#include "fsys.h"
#include "fsd.h"
#include <lip.h>
#include <bpb.h>
#include <lvm_data.h>

#pragma aux entry_addr "*"
#pragma aux m          "*"

char buff[0x210];
DLA_Table_Sector *dlat;
DLA_Entry *dlae;

static int cur_addr;
entry_func entry_addr;
static struct mod_list mll[99];
static int linux_mem_size;
bios_parameters_block *bpb;

unsigned long part_start;
unsigned long current_drive;
unsigned long current_partition;
unsigned long saved_drive;
unsigned long saved_partition;
unsigned long extended_memory;
int filemax;
int filepos;

char *linux_data_real_addr;
char *linux_data_tmp_addr;
unsigned long linux_text_len;

grub_error_t errnum = ERR_NONE;

int root_func(char *arg, int flags);

/*
 *  The next two functions, 'load_image' and 'load_module', are the building
 *  blocks of the multiboot loader component.  They handle essentially all
 *  of the gory details of loading in a bootable image and the modules.
 */

kernel_t
load_image (char *kernel, char *arg, kernel_t suggested_type,
            unsigned long load_flags)
{
  int len, i, exec_type = 0, align_4k = 1;
  unsigned int size;
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
    }
  pu;

  /* presuming that MULTIBOOT_SEARCH is large enough to encompass an
     executable header */

#pragma pack(4)
  unsigned char b[3];
  unsigned char b2[4];
  unsigned char buffer[MULTIBOOT_SEARCH];
#pragma pack()

//printf("&buffer = %x\r\n", buffer);

  /* sets the header pointer to point to the beginning of the
     buffer by default */
  pu.aout = (struct exec *) buffer;

  u_parm(PARM_CURRENT_DRIVE, ACT_GET, (unsigned int *)&current_drive);
  u_parm(PARM_CURRENT_PARTITION, ACT_GET, (unsigned int *)&current_partition);
  u_parm(PARM_EXTENDED_MEMORY, ACT_GET, (unsigned int *)&extended_memory);

  if (u_open (kernel, &size))
    return KERNEL_TYPE_NONE;

  errnum = ERR_NONE;

  if (!(len = u_read (buffer, MULTIBOOT_SEARCH)) || len < 32)
    {
      u_close ();

      u_parm(PARM_ERRNUM, ACT_GET, (unsigned int *)&errnum);

      if (!errnum)
        errnum = ERR_EXEC_FORMAT;

      u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);

      return KERNEL_TYPE_NONE;
    }

  for (i = 0; i < len; i++)
    {
      if (MULTIBOOT_FOUND ((int) (buffer + i), len - i))
        {
          flags = ((struct multiboot_header *) (buffer + i))->flags;
          if (flags & MULTIBOOT_UNSUPPORTED)
            {
              u_close ();
              errnum = ERR_BOOT_FEATURES;
              u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
              return KERNEL_TYPE_NONE;
            }
          type = KERNEL_TYPE_MULTIBOOT;
          str2 = "Multiboot";
          break;
        }
    }

  //printf("multiboot\r\n");

  /* ELF loading supported if multiboot, FreeBSD and NetBSD.  */
  if ((type == KERNEL_TYPE_MULTIBOOT
       || pu.elf->e_ident[EI_OSABI] == ELFOSABI_FREEBSD
       || grub_strcmp (pu.elf->e_ident + EI_BRAND, "FreeBSD") == 0
       || suggested_type == KERNEL_TYPE_NETBSD)
      && len > sizeof (Elf32_Ehdr)
      && BOOTABLE_I386_ELF ((*((Elf32_Ehdr *) buffer))))
    {
      if (type == KERNEL_TYPE_MULTIBOOT)
        entry_addr = (entry_func) pu.elf->e_entry;
      else
        entry_addr = (entry_func) (pu.elf->e_entry & 0xFFFFFF);

      if (entry_addr < (entry_func) 0x100000) {
        errnum = ERR_BELOW_1MB;
        u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      }

      /* don't want to deal with ELF program header at some random
         place in the file -- this generally won't happen */
      if (pu.elf->e_phoff == 0 || pu.elf->e_phnum == 0
          || ((pu.elf->e_phoff + (pu.elf->e_phentsize * pu.elf->e_phnum))
              >= len)) {
        errnum = ERR_EXEC_FORMAT;
        u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      }
      str = "elf";

    }
  else if (flags & MULTIBOOT_AOUT_KLUDGE)
    {
      pu.mb = (struct multiboot_header *) (buffer + i);
      entry_addr = (entry_func) pu.mb->entry_addr;
      cur_addr = pu.mb->load_addr;
      /* first offset into file */
      u_seek (i - (pu.mb->header_addr - cur_addr));

      u_parm(PARM_FILEMAX, ACT_GET, (unsigned int *)&filemax);

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
      {
        errnum = ERR_EXEC_FORMAT;
        u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      }

      if (cur_addr < 0x100000)
      {
        errnum = ERR_BELOW_1MB;
        u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      }

      pu.aout = (struct exec *) buffer;
      exec_type = 2;
      str = "kludge";
    }
  else                          /* no recognizable format */
    {
      errnum = ERR_EXEC_FORMAT;
      u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
    }

  /* return if error */
  if (errnum)
    {
      u_close ();
      return KERNEL_TYPE_NONE;
    }

  /* fill the multiboot info structure */
  //m->cmdline = (int) arg;
  m->mods_count = 0;
  m->mods_addr = 0;
  m->boot_device = (current_drive << 24) | current_partition;
  m->flags &= ~(MB_INFO_MODS | MB_INFO_AOUT_SYMS | MB_INFO_ELF_SHDR);
  m->syms.a.tabsize = 0;
  m->syms.a.strsize = 0;
  m->syms.a.addr = 0;
  m->syms.a.pad = 0;

  printf ("   [%s-%s", str2, str);

  str = "";

  if (exec_type)                /* can be loaded like a.out */
    {
      if (flags & MULTIBOOT_AOUT_KLUDGE)
        str = "-and-data";

      printf (", loadaddr=0x%x, text%s=0x%x", cur_addr, str, text_len);

      /* read text, then read data */
      if (u_read ((char *) RAW_ADDR (cur_addr), text_len) == text_len)
        {
          cur_addr += text_len;

          if (!(flags & MULTIBOOT_AOUT_KLUDGE))
            {
              /* we have to align to a 4K boundary */
              if (align_4k)
                cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;
              else
                printf (", C");

              printf (", data=0x%x", data_len);

              if ((u_read ((char *) RAW_ADDR (cur_addr), data_len)
                   != data_len)
                  && !errnum)
              {
                errnum = ERR_EXEC_FORMAT;
                u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
              }
              cur_addr += data_len;
            }

          if (!errnum)
            {
              memset ((char *) RAW_ADDR (cur_addr), 0, bss_len);
              cur_addr += bss_len;

              printf (", bss=0x%x", bss_len);
            }
        }
      else if (!errnum)
      {
        errnum = ERR_EXEC_FORMAT;
        u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      }

      u_parm(PARM_FILEMAX, ACT_GET, (unsigned int *)&filemax);
      u_parm(PARM_FILEMAX, ACT_GET, (unsigned int *)&filepos);

      if (!errnum && pu.aout->a_syms
          && pu.aout->a_syms < (filemax - filepos))
        {
          int symtab_err, orig_addr = cur_addr;

          /* we should align to a 4K boundary here for good measure */
          if (align_4k)
            cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

          m->syms.a.addr = cur_addr;

          *((int *) RAW_ADDR (cur_addr)) = pu.aout->a_syms;
          cur_addr += sizeof (int);

          printf (", symtab=0x%x", pu.aout->a_syms);

          if (u_read ((char *) RAW_ADDR (cur_addr), pu.aout->a_syms)
              == pu.aout->a_syms)
            {
              cur_addr += pu.aout->a_syms;
              m->syms.a.tabsize = pu.aout->a_syms;

              if (u_read ((char *) &i, sizeof (int)) == sizeof (int))
                {
                  *((int *) RAW_ADDR (cur_addr)) = i;
                  cur_addr += sizeof (int);

                  m->syms.a.strsize = i;

                  i -= sizeof (int);

                  printf (", strtab=0x%x", i);

                  symtab_err = (u_read ((char *) RAW_ADDR (cur_addr), i)
                                != i);
                  cur_addr += i;
                }
              else
                symtab_err = 1;
            }
          else
            symtab_err = 1;

          if (symtab_err)
            {
              printf ("(bad)");
              cur_addr = orig_addr;
              m->syms.a.tabsize = 0;
              m->syms.a.strsize = 0;
              m->syms.a.addr = 0;
            }
          else
            m->flags |= MB_INFO_AOUT_SYMS;
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
            (pu.elf->e_phoff + ((int) buffer)
             + (pu.elf->e_phentsize * i));
          if (phdr->p_type == PT_LOAD)
            {
              /* offset into file */
              u_seek (phdr->p_offset);
              filesiz = phdr->p_filesz;

              //if (type == KERNEL_TYPE_FREEBSD || type == KERNEL_TYPE_NETBSD)
              //  memaddr = RAW_ADDR (phdr->p_paddr & 0xFFFFFF);
              //else
                memaddr = RAW_ADDR (phdr->p_paddr);

              memsiz = phdr->p_memsz;
              if (memaddr < RAW_ADDR (0x100000))
              {
                errnum = ERR_BELOW_1MB;
                u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
              }

              /* If the memory range contains the entry address, get the
                 physical address here.  */
              if (type == KERNEL_TYPE_MULTIBOOT
                  && (unsigned) entry_addr >= phdr->p_vaddr
                  && (unsigned) entry_addr < phdr->p_vaddr + memsiz)
                real_entry_addr = (entry_func) ((unsigned) entry_addr
                                                + memaddr - phdr->p_vaddr);

              /* make sure we only load what we're supposed to! */
              if (filesiz > memsiz)
                filesiz = memsiz;
              /* mark memory as used */
              if (cur_addr < memaddr + memsiz)
                cur_addr = memaddr + memsiz;
              printf (", <0x%x:0x%x:0x%x>", memaddr, filesiz,
                      memsiz - filesiz);
              /* increment number of segments */
              loaded++;

              /* load the segment */
              if (u_read ((char *) memaddr, filesiz) == filesiz) // memcheck (memaddr, memsiz) &&
                {
                  if (memsiz > filesiz)
                    memset ((char *) (memaddr + filesiz), 0, memsiz - filesiz);
                }
              else
                break;
            }
        }

      if (! errnum)
        {
          if (! loaded)
          {
            errnum = ERR_EXEC_FORMAT;
            u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
          }
          else
            {
              /* Load ELF symbols.  */
              Elf32_Shdr *shdr = NULL;
              int tab_size, sec_size;
              int symtab_err = 0;

              m->syms.e.num = pu.elf->e_shnum;
              m->syms.e.size = pu.elf->e_shentsize;
              m->syms.e.shndx = pu.elf->e_shstrndx;

              /* We should align to a 4K boundary here for good measure.  */
              if (align_4k)
                cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

              tab_size = pu.elf->e_shentsize * pu.elf->e_shnum;

              u_seek (pu.elf->e_shoff);
              if (u_read ((char *) RAW_ADDR (cur_addr), tab_size)
                  == tab_size)
                {
                  m->syms.e.addr = cur_addr;
                  shdr = (Elf32_Shdr *) m->syms.e.addr;
                  cur_addr += tab_size;

                  printf (", shtab=0x%x", cur_addr);

                  for (i = 0; i < m->syms.e.num; i++)
                    {
                      /* This section is a loaded section,
                         so we don't care.  */
                      if (shdr[i].sh_addr != 0)
                        continue;

                      /* This section is empty, so we don't care.  */
                      if (shdr[i].sh_size == 0)
                        continue;

                      /* Align the section to a sh_addralign bits boundary.  */
                      cur_addr = ((cur_addr + shdr[i].sh_addralign) &
                                  - (int) shdr[i].sh_addralign);

                      u_seek (shdr[i].sh_offset);

                      sec_size = shdr[i].sh_size;

                      if (! ((u_read ((char *) RAW_ADDR (cur_addr), // memcheck (cur_addr, sec_size) &&
                                            sec_size)
                                 == sec_size)))
                        {
                          symtab_err = 1;
                          break;
                        }

                      shdr[i].sh_addr = cur_addr;
                      cur_addr += sec_size;
                    }
                }
              else
                symtab_err = 1;

              if (m->syms.e.addr < RAW_ADDR(0x10000))
                symtab_err = 1;

              if (symtab_err)
                {
                  printf ("(bad)");
                  m->syms.e.num = 0;
                  m->syms.e.size = 0;
                  m->syms.e.addr = 0;
                  m->syms.e.shndx = 0;
                  cur_addr = 0;
                }
              else
                m->flags |= MB_INFO_ELF_SHDR;
            }
        }
    }

  if (! errnum)
    {
      printf (", entry=0x%x]\r\n", (unsigned) entry_addr);

      /* If the entry address is physically different from that of the ELF
         header, correct it here.  */
      if (real_entry_addr)
        entry_addr = real_entry_addr;
    }
  else
    {
      putchar ('\n');
      type = KERNEL_TYPE_NONE;
    }

  u_close ();

  /* Sanity check.  */
  if (suggested_type != KERNEL_TYPE_NONE && suggested_type != type)
    {
      errnum = ERR_EXEC_FORMAT;
      u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      return KERNEL_TYPE_NONE;
    }

  //u_parm(PARM_LINUX_DATA_REAL_ADDR, ACT_SET, (unsigned int *)(&linux_data_real_addr));
  //u_parm(PARM_LINUX_DATA_TMP_ADDR, ACT_SET,  (unsigned int *)(&linux_data_tmp_addr));
  //u_parm(PARM_LINUX_TEXT_LEN, ACT_SET, (unsigned int *)&linux_text_len);

  // add kernel command line after executable end
  cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;
  m->cmdline = cur_addr;
  grub_strcpy((char *)cur_addr, arg);
  cur_addr += grub_strlen(arg);

  return type;
}

int
load_module (char *module, char *arg)
{
  int len;
  unsigned int size;

  /* if we are supposed to load on 4K boundaries */
  cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

  if (u_open (module, &size))
    return 0;

  u_parm(PARM_FILEMAX, ACT_GET, (unsigned int *)&filemax);

  if ((cur_addr + filemax) >= (1024*(1024+m->mem_upper)))
    {
      printf("Want to load module to 0x%x len 0x%x but only have 0x%x RAM\r\n",
             cur_addr, filemax, 1024*(1024+m->mem_upper));
      errnum = ERR_BADMODADDR;
      u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      u_close ();
      return 0;
    }

  printf("cur_addr=0x%x\r\n", cur_addr);

  len = u_read ((char *) cur_addr, size);

  printf("read 0x%x bytes\r\n", len);

  if (! len)
    {
      u_close ();
      return 0;
    }

  printf ("   [Multiboot-module @ 0x%x, 0x%x bytes]\r\n", cur_addr, len);

  /* these two simply need to be set if any modules are loaded at all */
  m->flags |= MB_INFO_MODS;
  m->mods_addr = (int) mll;

  mll[m->mods_count].mod_start = cur_addr;
  //mll[m->mods_count].cmdline = arg;
  cur_addr += len;
  mll[m->mods_count].mod_end = cur_addr;
  mll[m->mods_count].pad = 0;

  grub_strcpy((char *)cur_addr, arg);
  mll[m->mods_count].cmdline = cur_addr;
  cur_addr += grub_strlen(arg);

  /* increment number of modules included */
  m->mods_count++;

  u_close ();
  return 1;
}

void
create_vbe_module(void *ctrl_info, int ctrl_info_len,
                  void *mode_info, int mode_info_len,
                  int mode, int pmif, int pmif_len,
                  unsigned int version)
{
  /* if we are supposed to load on 4K boundaries */
  cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

  printf ("   [VESA %d.%d info @ 0x%x, 0x%x bytes]\r\n",
      version >> 8, version & 0xFF,
      cur_addr, ctrl_info_len + mode_info_len);

  grub_memmove((char*)cur_addr, ctrl_info, ctrl_info_len);
  m->vbe_control_info = (int)cur_addr;
  cur_addr += ctrl_info_len;

  grub_memmove((char*)cur_addr, mode_info, mode_info_len);
  m->vbe_mode_info    = (int)cur_addr;
  cur_addr += mode_info_len;

  m->flags |= MB_INFO_VIDEO_INFO;

  m->vbe_mode         = mode;
  m->vbe_interface_seg = (pmif >> 16) & 0xFFFF;
  m->vbe_interface_off =  pmif        & 0xFFFF;
  m->vbe_interface_len = pmif_len;
}

void
create_lip_module(lip2_t **l)
{
  int lip2_len;
  char *lip_magic = "*lip";
  /* if we are supposed to load on 4K boundaries */
  cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

  printf("   [Lip-module @ 0x%x, 0x%x bytes]\r\n",
         cur_addr, sizeof(lip2_t));

  lip2_len = sizeof(lip2_t);
  grub_memmove((char *)cur_addr, *l, lip2_len);

  *l = (lip2_t *)cur_addr;
  u_setlip(*l);

  m->flags |= MB_INFO_MODS;
  m->mods_addr = (int) mll;

  grub_strcpy((char *)cur_addr + lip2_len, lip_magic);
  mll[m->mods_count].cmdline = (unsigned long)cur_addr + lip2_len;
  mll[m->mods_count].mod_start = cur_addr;
  cur_addr += lip2_len;
  mll[m->mods_count].mod_end = cur_addr;
  mll[m->mods_count].pad = 0;
  cur_addr += 5;

  /* increment number of modules included */
  m->mods_count++;
}

/*
  Name  : CRC-32
  Poly  : 0x04C11DB7	x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 
                       + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
  Init  : 0xFFFFFFFF
  Revert: true
  XorOut: 0xFFFFFFFF
  Check : 0xCBF43926 ("123456789")
  MaxLen: 268 435 455 байт (2 147 483 647 бит) - обнаружение
   одинарных, двойных, пакетных и всех нечетных ошибок

  (Got from russian wikipedia, http://ru.wikipedia.org/wiki/CRC32)
  and corrected ;) 
*/
unsigned long crc32(unsigned char *buf, unsigned long len)
{
    unsigned long crc_table[256];
    unsigned long crc;
    int i, j; 

    for (i = 0; i < 256; i++)
    {
        crc = i;
        for (j = 0; j < 8; j++)
            crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
 
        crc_table[i] = crc;
    };
 
    crc = 0xFFFFFFFFUL;
 
    while (len--) 
      crc = crc_table[(crc ^ *buf++) & 0xff] ^ ((crc >> 8) & 0x00ffffffL);

    return crc;
}

/* Determine a drive letter through DLA tables */
int dla(char *driveletter)
{
  unsigned long    part;
  unsigned long    sec;
  struct geometry  geo;
  char *p;
  unsigned long CRC32, crc; 

  u_parm(PARM_CURRENT_DRIVE, ACT_GET, (unsigned int *)&current_drive);
  u_parm(PARM_CURRENT_PARTITION, ACT_GET, (unsigned int *)&current_partition);

  part = (current_partition >> 16) & 0xff;

  // get drive geometry
  u_diskctl(BIOSDISK_GEO, current_drive, &geo, 0, 0, 0);

  if (part <= 3) // primary partition
    sec = geo.sectors - 1;
  else // logical partition
    sec = part_start - 1;

  p = buff;

  /* make a pointer 16-byte aligned */
  p = (char *)(((int)p >> 4) << 4);
  if (p < buff) p += 0x10;

  memset(p, 0, 0x200);

  // read DLAT sector
  u_diskctl(BIOSDISK_READ, current_drive, &geo, sec, 1, (int)p);

  dlat = (DLA_Table_Sector *)p;

  if (dlat->DLA_Signature1 == DLA_TABLE_SIGNATURE1 &&
      dlat->DLA_Signature2 == DLA_TABLE_SIGNATURE2)
  {
    /* Calculate DLAT CRC */
    crc = dlat->DLA_CRC;
    /* zero-out CRC field and unused sector space */
    dlat->DLA_CRC = 0;
    memset(p + sizeof(DLA_Table_Sector), 0, 0x200 - sizeof(DLA_Table_Sector));
    CRC32 = crc32(p, 0x200);
    dlat->DLA_CRC = crc; // return back
    if (crc == CRC32)    // crc ok
    {
      /* Get and parse partition DLAT entry */
      dlae = (DLA_Entry *)dlat->DLA_Array;
      if (part <= 3) dlae += part; // for primary partitions
      *driveletter = grub_toupper(dlae->Drive_Letter);      

      return 1;
    }
  }

  return 0;
}

int
get_dlat_info(char *part)
{
  char drv[2];

  if (root_func(part, 0x2))
  {
    printf("No such partition: %s\r\n", part);
    return 1;
  }

  if (dla(drv)) // DLA table present
  {
    printf("\r\nDLA table found for %s\r\n", part);
    drv[1] = '\0';
  }
  else
  {
    printf("\r\nNo DLAT present for %s\r\n", part);
    return 1;
  }

  printf("  DLAT sector crc: 0x%x\r\n"
         "  Disk serial number: 0x%x\r\n"
         "  Boot disk serial number: 0x%x\r\n"
         "  Install flags: 0x%x\r\n"
         "  Cylinders: 0x%x, Heads: 0x%x, Sectors: 0x%x"
         "  Disk name: %s\r\n"
         "  Partition serial number: 0x%x\r\n"
         "  Partition size: 0x%x, Partition start: 0x%x\r\n"
         "  On boot manager menu: %s, Installable: %s, driveletter: %s\r\n"
         "  Volume name: %s\r\n"
         "  Partition name: %s\r\n",
         dlat->DLA_CRC, dlat->Disk_Serial_Number, dlat->Boot_Disk_Serial_Number,
         dlat->Install_Flags, dlat->Cylinders, dlat->Heads_Per_Cylinder, dlat->Sectors_Per_Track,
         dlat->Disk_Name, dlae->Partition_Serial_Number, dlae->Partition_Size, dlae->Partition_Start,
         dlae->On_Boot_Manager_Menu ? "yes" : "no", dlae->Installable ? "yes" : "no",
         drv[0] ? drv : "none", dlae->Partition_Name, dlae->Volume_Name);

  return 0;
}

int
create_bs_module(char *module)
{
  int len;
  unsigned int size;
  char *parm;
  char drv;

  /* if we are supposed to load on 4K boundaries */
  cur_addr = (cur_addr + 0xFFF) & 0xFFFFF000;

  if (u_open (module, &size))
    return 0;

  if ((cur_addr + size) >= (1024*(1024+m->mem_upper)))
    {
      printf("Want to load module to 0x%x len 0x%x but only have 0x%x RAM\r\n",
             cur_addr, size, 1024*(1024+m->mem_upper));
      errnum = ERR_BADMODADDR;
      u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      u_close ();
      return 0;
    }

  printf("cur_addr=0x%x\r\n", cur_addr);

  len = u_read ((char *) cur_addr, size);

  printf("read 0x%x bytes\r\n", len);

  if (! len)
    {
      u_close ();
      return 0;
    }

  printf ("   [Bs-module @ 0x%x, 0x%x bytes]\r\n", cur_addr, len);

  /* these two simply need to be set if any modules are loaded at all */
  m->flags |= MB_INFO_MODS;
  m->mods_addr = (int) mll;

  mll[m->mods_count].mod_start = cur_addr;
  bpb = (bios_parameters_block *)(cur_addr + 0xb);

  cur_addr += len;
  mll[m->mods_count].mod_end = cur_addr;
  mll[m->mods_count].pad = 0;

  /* increment number of modules included */
  m->mods_count++;

  u_close ();

  u_parm(PARM_PART_START, ACT_GET, (unsigned int *)&part_start);

  bpb->hidden_secs = part_start;

  parm = grub_strstr(module, "--drv");

  if (parm)
  {
    parm = skip_to(1, parm);
    drv = grub_toupper(*parm);
  }
  else
    dla(&drv);

  if ('A' > drv || drv > 'Z') drv = 'C';

  drv = (drv - 'C') + 0x80;  
  bpb->log_drive = drv;

  return 1;
}

void
set_load_addr (int addr)
{
  printf ("Setting module load address to 0x%x\r\n", addr);
  cur_addr = addr;
}

vbe_mode_set(struct vbe_controller *controller, int mode_number,
             struct vbe_mode *mode, unsigned int *pmif_segoff,
             unsigned int *pmif_len)
{
  struct pmif pmif;

  if (!mode_number)
    {
      //reset_vbe_mode ();
      u_vbectl(VBE_FUNC_RESET, 0, 0);
      return -1;
    }

  /* Preset `VBE2'.  */
  grub_memmove (controller->signature, "VBE2", 4);

  /* Detect VBE BIOS.  */
  if (u_vbectl(VBE_FUNC_GET_CTRLR_INFO, 0, controller) != 0x004F)
      //get_vbe_controller_info (controller) != 0x004F)
    {
      printf (" VBE BIOS is not present.\r\n");
      return 1;
    }

  if (controller->version < 0x0200)
    {
      printf (" VBE version %u.%u is not supported.\r\n",
              (int) (controller->version >> 8),
              (int) (controller->version & 0xFF));
      errnum = MAX_ERR_NUM;
      return 1;
    }

  if (u_vbectl(VBE_FUNC_GET_MODE_INFO, mode_number, mode) != 0x004F
      //get_vbe_mode_info (mode_number, mode) != 0x004F
      || (mode->mode_attributes & 0x0091) != 0x0091)
    {
      printf (" Mode 0x%x is not supported.\r\n", mode_number);
      errnum = MAX_ERR_NUM;
      return 1;
    }

  /* Now trip to the graphics mode.  */
  if (u_vbectl(VBE_FUNC_SET_MODE, mode_number | (1 << 14), 0) != 0x004F)
      //set_vbe_mode (mode_number | (1 << 14)) != 0x004F)
    {
      printf (" Switching to Mode 0x%x failed.\r\n", mode_number);
      errnum = MAX_ERR_NUM;
      return 1;
    }

  //get_vbe_pmif(pmif_segoff, pmif_len);
  pmif.pmif_segoff = pmif_segoff;
  pmif.pmif_len = pmif_len;
  u_vbectl(VBE_FUNC_GET_PMIF, 0, &pmif);

  return 0;
}
