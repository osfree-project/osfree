/*
    LXLoader - Loads LX exe files or DLLs for execution or to extract information from.
    Copyright (C) 2007  Sven Rosén (aka Viking)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
    Or see <http://www.gnu.org/licenses/>
*/

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include <l4/os3/modmgr.h>
#include <l4/os3/io.h>
#include <l4/os3/cfgparser.h>
#include <l4/os3/modlx.h>
#include <l4/os3/ixfmgr.h>

#ifndef ENEWHDR
  #define ENEWHDR 0x3c
#endif

/* Prototypes */
int LXLoadHeader(struct LX_module * lx_mod);
int LXLoadLoaderSection(struct LX_module * lx_mod);
int LXLoadFixupSection(struct LX_module * lx_mod);
int LXLoadObjects(IXFModule * ixfModule);

/* @brief Reads in the lx header from the buffer pointed at stram_fh
   with the size of str_size.
   Constructor for buffer based file.
*/

extern struct t_mem_area root_area;

struct LX_module *
LXLoadStream(char * stream_fh, int str_size, struct LX_module * lx_mod)
{
  lx_mod->lx_fseek = &lx_fseek_stream; /* Copy functionpointer which does fseek on
                                          memory buffer. An attempt to streamline
                                          access to booth disk files and memory buffers.*/
  lx_mod->lx_fread = &lx_fread_stream;
  lx_mod->lx_file_stream = stream_fh;
  lx_mod->lx_stream_size = str_size;
  lx_mod->lx_stream_pos = 0;
  lx_mod->lx_head_e32_exe = 0;
  lx_mod->offs_lx_head = 0;

  lx_mod->fh = 0; /* No filehandle to a disk file is used. */

  if(LXLoadHeader(lx_mod)) {
          //io_printf("Succeeded to load LX header.\n");
          LXLoadLoaderSection(lx_mod);
          LXLoadFixupSection(lx_mod);
  }
  else {
          io_printf("Could not load LX header!!!!\n");
          return 0;
  }

  return lx_mod;
}

  /* Läser in huvudet på lxfilen. Testar först om den har en DOS stub (MZ eller ZM) i början och
     sen om den har en giltig LX signatur. */
  /* Testar först om det är en giltig fil och laddar huvudet på lxfilen. */
  /* Två alternativ av LX finns. En med en DOS-header och sen LX-header.
     Även rena LX-filer finns, men är inte så vanliga. */
int LXLoadHeader(struct LX_module * lx_mod)
{
  char exe_sig[2];
  struct e32_exe * lx_e32_exe;
  int lx_module_header_offset=0;
  struct exe hdr;

  lx_mod->lx_fseek(lx_mod, 0, SEEK_SET);
  lx_mod->lx_fread(&exe_sig, sizeof(exe_sig), 1, lx_mod);

  if(((exe_sig[0] == 'M') && (exe_sig[1] == 'Z')) ||
     ((exe_sig[0] == 'Z') && (exe_sig[1] == 'M')))
  {
    /* Found DOS stub. */

    lx_mod->lx_fseek(lx_mod, ENEWHDR, SEEK_SET);

    lx_mod->lx_fread(&lx_module_header_offset, sizeof(lx_module_header_offset), 1, lx_mod);

    lx_mod->lx_fseek(lx_mod, lx_module_header_offset, SEEK_SET);

    lx_mod->lx_fread(&exe_sig, 1, sizeof(exe_sig), lx_mod);

    lx_mod->offs_lx_head = lx_module_header_offset;
    if (options.debugixfmgr)
    {
      lx_mod->lx_fseek(lx_mod, 0, SEEK_SET);
      lx_mod->lx_fread(&hdr, sizeof(hdr), 1, lx_mod);
      dump_header_mz(hdr);
    }
  }

  if(lx_module_header_offset == 0) {
    //io_printf(" Not an MZ/ZM Dos exe. MZ=%d ZM=%d %c%c\n",
    //   ((exe_sig[0] == 'M') && (exe_sig[1] == 'Z')),
    //   ((exe_sig[0] == 'Z') && (exe_sig[1] == 'M')), exe_sig[0], exe_sig[1] );
    //io_printf("LX header offset: %d \n", lx_module_header_offset);
    lx_module_header_offset = 0;
    lx_mod->offs_lx_head = lx_module_header_offset;
  }

  if((exe_sig[0] == 'L') && (exe_sig[1] == 'X')) {
    //io_printf("Valid LX header.\n");

    lx_e32_exe = (struct e32_exe *) malloc(sizeof(struct e32_exe));

    //int storlek = sizeof(struct e32_exe);
    //io_printf("Läser %d bytes av LX header.\n", storlek);

    lx_mod->lx_fseek(lx_mod, lx_module_header_offset, SEEK_SET);

    lx_mod->lx_fread(lx_e32_exe, sizeof(struct e32_exe), 1, lx_mod);

    lx_mod->lx_head_e32_exe = lx_e32_exe;

    if (options.debugixfmgr)
    {
      dump_header_lx(*lx_e32_exe);
    }
    E32_MFLAGS(*lx_e32_exe);
    return TRUE;
  } else {
    io_printf("Invalid LX file !!!! (%c%c)\n", exe_sig[0], exe_sig[1]);
    return FALSE;
  }

  return FALSE;
}

/* @brief Reads loader section from file, it's place is in the header. */
int LXLoadLoaderSection(struct LX_module * lx_mod)
{
  lx_mod->lx_fseek(lx_mod, lx_mod->offs_lx_head + lx_mod->lx_head_e32_exe->e32_objtab, SEEK_SET);

  //io_printf("Reads Loader Section.\n");
  lx_mod->loader_section = (char *) malloc(lx_mod->lx_head_e32_exe->e32_ldrsize);

  lx_mod->lx_fread(lx_mod->loader_section,  lx_mod->lx_head_e32_exe->e32_ldrsize,1, lx_mod);
  return TRUE;
}



/* @brief Reads fixup part. */
int LXLoadFixupSection(struct LX_module * lx_mod)
{
  lx_mod->fixup_section = (char *) malloc(lx_mod->lx_head_e32_exe->e32_fixupsize);

  lx_mod->lx_fread(lx_mod->fixup_section,  lx_mod->lx_head_e32_exe->e32_fixupsize,1, lx_mod);
  return TRUE;
}

/* @brief Loads all the objects in a dll and exe. */
int LXLoadObjects(IXFModule * ixfModule)
{
  struct LX_module * lx_exe_mod=(struct LX_module *)(ixfModule->FormatStruct);
  void * vm_code_obj;
  unsigned int obj_cnt;
  struct o32_obj * kod_obj;
  unsigned long int number_of_objects = get_obj_num(lx_exe_mod);
  
  for(obj_cnt=1; obj_cnt<=number_of_objects; obj_cnt++)
  {
    kod_obj = get_obj(lx_exe_mod, obj_cnt);
    vm_code_obj = 0;
#if 0 /*!defined(__OS2__) && !defined(__LINUX__)*/
                if( !is_mem_used(&root_area/*proc->root_mem_area*/,
                                    (void *) kod_obj->o32_reserved, kod_obj->o32_size) ) {
                                    //(void *) kod_obj->o32_base, kod_obj->o32_size) ) {
                        vm_code_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, kod_obj);
                } else {
                        unsigned long int new_base = (unsigned long int) seek_free_mem(
                                                            &root_area/*proc->root_mem_area*/,
                                                            kod_obj->o32_size);
                        /*  What kind of requirements is there on allocated memory
                            for DLLs is OS/2? Allocated in "Shared Area"? Where is that,
                            from 128 MiB and downwards to 64 MiB???
                            This would be the place to make that kind of call.
                            But, for now, just allocate the space somewhere.
                            Which just goes through the memory registry from beginning,
                            position 1 and upward and grabs first free space. */
                        if( is_mem_used(&root_area/*proc->root_mem_area*/,
                                        (void *) new_base,
                                        kod_obj->vm_size) )
                                        //kod_obj->o32_size) )
                                io_printf("Error allocating used memory!!! 0x%lx \n", new_base);
                        //kod_obj->o32_base = new_base;
                        kod_obj->o32_reserved = new_base;
			//io_printf(" new_base: 0x%lx, %lu \n", new_base, new_base);
                        vm_code_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, kod_obj);
                        //io_printf(" new_base == %p ?\n", vm_code_obj);
                }

#else
// In OS/2 no normal mmap() support, so we always relocate objects
    vm_code_obj = (void*) vm_alloc_obj_lx(ixfModule, kod_obj);
    // use 'reserved' field for VM address of an object
    kod_obj->o32_reserved = (unsigned long)vm_code_obj;
    if (ixfModule->PIC)
      kod_obj->o32_base = kod_obj->o32_reserved;
    //io_printf("o32_base=%x\n", kod_obj->o32_base);
    //io_printf("o32_reserved=%x\n", kod_obj->o32_reserved);
#endif
    // Register allocated area
    //alloc_mem_area(&root_area, (void *) kod_obj->o32_base, kod_obj->o32_size);
    alloc_mem_area(&root_area, (void *) kod_obj->o32_reserved, kod_obj->o32_size);

    if(vm_code_obj == MAP_FAILED) {
                        io_printf("Error mapping memory for (code)\n");
                        print_o32_obj_info(*kod_obj, "object code");
                        return 0;
                }

                load_obj_lx(lx_exe_mod, kod_obj, vm_code_obj);

                if (options.debugixfmgr)
                  print_o32_obj_info(*kod_obj, "object 1");
        }

        return 1;
}
