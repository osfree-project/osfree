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

#define INCL_DOS
#define INCL_DOSEXCEPTIONS
#include <os2.h>


#include <io.h>
#include <cfgparser.h>
#include <loadobjlx.h>
#include <fixuplx.h>
#include <modlx.h>
#include <modmgr.h>
#include <ixfmgr.h>
#include <sys/mman.h>
#include <native_dynlink.h>
#include <os2errcodes.h>

#include <string.h>

extern struct t_mem_area os2server_root_mem_area;

#if 0
  /* Loads the objects for code and data, for programs. NOT used. */
int load_code_data_obj_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc) {
        struct o32_obj * kod_obj = (struct o32_obj *) get_code(lx_exe_mod);

        struct o32_obj * stack_obj = (struct o32_obj *) get_data_stack(lx_exe_mod);

        void * vm_code_obj = 0;
        void * vm_data_obj = 0;

        if((kod_obj != 0) && (kod_obj == stack_obj)) { /* Data and code in same object. */
                io_printf("load_code_data_obj_lx: Code and stack/data is the same object!\n");

                /* Allocate virtual memory at the address that kod_obj requires. */
                vm_code_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, kod_obj);

                /* Register the allocated memory with memmgr. */
                alloc_mem_area(&proc->root_mem_area, (void *) kod_obj->o32_base, kod_obj->o32_size);

                proc->code_mmap = vm_code_obj;

                if(vm_code_obj == MAP_FAILED) {
                        io_printf("Error mapping memory for(code/data)\n");
                        return 0;
                }
                /* Load code object. */
                load_obj_lx(lx_exe_mod, kod_obj, vm_code_obj);
                return 1;
        }

        /* Allocate virtual memory at the address that kod_obj requires. */
        vm_code_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, kod_obj);


        proc->code_mmap = vm_code_obj;

        if(vm_code_obj == MAP_FAILED) {
                io_printf("Error mapping memory for (code)\n");
                return 0;
        }
        /* Register the allocated memory with memmgr. */
        alloc_mem_area(&proc->root_mem_area, (void *) kod_obj->o32_base, kod_obj->o32_size);
        load_obj_lx(lx_exe_mod, kod_obj, vm_code_obj);

        if(stack_obj == 0) return 0;
        vm_data_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, stack_obj);

        /* Register the allocated memory with memmgr. */
        alloc_mem_area(&proc->root_mem_area, (void *) stack_obj->o32_base, stack_obj->o32_size);

        proc->stack_mmap = vm_data_obj;

        if(vm_data_obj == MAP_FAILED) {
                io_printf("Error mapping memory for (data/stack)\n");
                return 0;
        }
        load_obj_lx(lx_exe_mod, stack_obj, vm_data_obj);

        print_o32_obj_info(*kod_obj, " Info about kod_obj ");
        print_o32_obj_info(*stack_obj, " Info about stack_obj ");

        return 1;
}
#endif

  /* Loads all the objects in a dll and exe. */
int load_dll_code_obj_lx(struct LX_module * lx_exe_mod) {
        /*struct o32_obj * kod_obj = (struct o32_obj *) get_code(lx_exe_mod); */
        /*struct o32_obj * stack_obj = (struct o32_obj *) get_data_stack(lx_exe_mod);*/
        void * vm_code_obj;
        unsigned int obj_cnt;
        struct o32_obj * kod_obj;

        unsigned long int number_of_objects = get_obj_num(lx_exe_mod);


        for(obj_cnt=1; obj_cnt<=number_of_objects; obj_cnt++) {
                kod_obj = get_obj(lx_exe_mod, obj_cnt);
                //io_printf("obj_cnt: %d, number_of_objects: %d\n", obj_cnt, number_of_objects);
                vm_code_obj = 0;
#if !defined(__OS2__) && !defined(__LINUX__)
                if( !is_mem_used(&os2server_root_mem_area/*proc->root_mem_area*/, 
                                    (void *) kod_obj->o32_base, kod_obj->o32_size) ) {
                        vm_code_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, kod_obj);
                } else {
                        unsigned long int new_base = (unsigned long int) seek_free_mem(
                                                            &os2server_root_mem_area/*proc->root_mem_area*/,
                                                            kod_obj->o32_size);
                        /*  What kind of requirements is there on allocated memory
                            for DLLs is OS/2? Allocated in "Shared Area"? Where is that,
                            from 128 MiB and downwards to 64 MiB???
                            This would be the place to make that kind of call.
                            But, for now, just allocate the space somewhere.
                            Which just goes through the memory registry from beginning,
                            position 1 and upward and grabs first free space. */
                        if( is_mem_used(&os2server_root_mem_area/*proc->root_mem_area*/, 
                                        (void *) new_base, 
                                        kod_obj->o32_size) )
                                io_printf("Error allocating used memory!!! 0x%lx \n", new_base);
                        kod_obj->o32_base = new_base;
                        //io_printf(" new_base: 0x%lx, %lu \n", new_base, new_base);
                        vm_code_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, kod_obj);
                        //io_printf(" new_base == %p ?\n", vm_code_obj);
                }

#else
// In OS/2 no normal mmap() support, so we always relocate objects
                vm_code_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, kod_obj);
                kod_obj->o32_base = (unsigned long)vm_code_obj;
#endif
                // Register allocated area
                alloc_mem_area(&os2server_root_mem_area/*&proc->root_mem_area*/, (void *) kod_obj->o32_base, kod_obj->o32_size);

                /*proc->code_mmap = vm_code_obj;*/

                if(vm_code_obj == MAP_FAILED) {
                        io_printf("Error mapping memory for (code)\n");
                        print_o32_obj_info(*kod_obj, "object code");
                        return 0;
                }

                load_obj_lx(lx_exe_mod, kod_obj, vm_code_obj);

                if (options.debugmodmgr)
                  print_o32_obj_info(*kod_obj, "object 1");
        }

        return 1;
}

/* void exec_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc) { */

  /*   Allocates virtual memory for an object at an absolute virtual address.
       Some GNU/Linux specific flags to mmap(MAP_GROWSDOWN). */
unsigned int vm_alloc_obj_lx(struct LX_module * lx_exe_mod, struct o32_obj * lx_obj) {

        void * mmap_obj = 0;

#if !defined(__OS2__) && !defined(__LINUX__)
        mmap_obj = mmap((void *)lx_obj->o32_base, lx_obj->o32_size,
                                 PROT_WRITE | PROT_READ | PROT_EXEC  ,       /* | PROT_EXEC */
                                  MAP_GROWSDOWN | MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 0/*lx_exe_mod->fh*/,
                                 0 /*lx_exe_mod->lx_head_e32_exe->e32_datapage*/);
#else
// Under OS/2 return always unique address
  #if defined(__LINUX__)
        mmap_obj = malloc(lx_obj->o32_size);
  #else
        DosAllocMem(&mmap_obj, lx_obj->o32_size, PAG_COMMIT|PAG_EXECUTE|PAG_READ|PAG_WRITE);
  #endif
#endif
        return (unsigned int) mmap_obj;
}

        /* Loads an object to the memory area pointed by vm_ptr_obj. */
int load_obj_lx(struct LX_module * lx_exe_mod,
                                struct o32_obj * lx_obj, void *vm_ptr_obj)
{
  unsigned long int tmp_code;
  unsigned long int tmp_vm;
  unsigned long int tmp_vm_code;
  struct o32_map * obj_pg_sta;
  int ofs_page_sta;
  int startpage = lx_obj->o32_pagemap;
  int lastpage  = lx_obj->o32_pagemap + lx_obj->o32_mapsize;
  unsigned int data_pages_offs =  get_e32_datapage(lx_exe_mod);
  unsigned int code_mmap_pos = 0;
  int page_nr=0;

  /*struct o32_map * obj_pg_ett = get_obj_map(lx_exe_mod ,startpage);*/
  /*  Reads in all pages from kodobject to designated virtual memory. */
  for(page_nr=startpage; page_nr<lastpage; page_nr++)
  {
    obj_pg_sta = get_obj_map(lx_exe_mod ,page_nr);
    ofs_page_sta = (obj_pg_sta->o32_pagedataoffset << get_e32_pageshift(lx_exe_mod))
                                                                + data_pages_offs;

    lx_exe_mod->lx_fseek(lx_exe_mod, ofs_page_sta, SEEK_SET);

    tmp_code = (unsigned long int) code_mmap_pos;
    tmp_vm = (unsigned long int) vm_ptr_obj;
    tmp_vm_code = tmp_code + tmp_vm;

    lx_exe_mod->lx_fread((void *)tmp_vm_code,
                         obj_pg_sta->o32_pagesize, 1, lx_exe_mod);
    code_mmap_pos += obj_pg_sta->o32_pagesize;
  }
  return 0;
}


        /* Applies fixups to all objects. Used for programs and dlls. 
           ret_rc is an OS/2 error number. */
int do_fixup_code_data_lx(struct LX_module * lx_exe_mod, int *ret_rc)
{
  unsigned long int i;
  /* If there is a code object (with a main function) then do a fixup on it and
     it's data/stack object if it exists.*/
  for(i=1; i<=get_obj_num(lx_exe_mod); i++)
  {
    struct o32_obj * obj = get_obj(lx_exe_mod, i);
    if(obj != 0)
      if(!do_fixup_obj_lx(lx_exe_mod, obj, ret_rc)) 
        return 0; /*Some error happend, return false and forward error in ret_rc.*/
  }           
 return 1;
}


/* Applies fixups for an object. Returns true(1) or false(0) to show status.*/
int do_fixup_obj_lx(struct LX_module * lx_exe_mod,
                                struct o32_obj * lx_obj, int *ret_rc)
{
  int ord_found;
  char *pas_imp_proc_name;
  int import_name_offs;
  char buf_import_name[260];
  char cont_buf_mod_name[255];
  char * mod_name;
  char * cont_mod_name;
  int import_ord;
  int mod_nr;
  unsigned long rc;
  unsigned long int * ptr_source;
  unsigned long int vm_source;
  unsigned long int vm_target;
  unsigned long int vm_start_target_obj;
  struct o32_obj * target_object;
  char buf_mod_name[255];
  char * org_mod_name;
  struct LX_module *found_module;
  int trgoffs;
  int object1;
  int addit;
  int srcoff_cnt1;
  int fixup_source_flag;
  int fixup_source;
  int fixup_offset;
  char *import_name;
  unsigned long int vm_start_of_page;
  struct r32_rlc * min_rlc;
  int pg_offs_fix;
  int pg_end_offs_fix;
  int page_nr=0;
  int startpage = lx_obj->o32_pagemap;
  int lastpage  = lx_obj->o32_pagemap + lx_obj->o32_mapsize;
  UCHAR uchLoadError[CCHMAXPATH] = {0}; /* Error info from DosExecPgm */

  //io_printf("--------------------Listing fixup data ------------------------- %p\n", lx_obj);

  /* Goes through every page of the object.
     The fixups are variable size and a bit messy to traverse.*/
  //print_o32_obj_info(*lx_obj, " Info about source object ");

  for(page_nr=startpage; page_nr < lastpage; page_nr++)
  {
    io_printf("-----  Object %d of %d\n",startpage, lastpage);

    /* Go and get byte position for fixup from the page logisk_sida.
       Start offset for fixup in the page*/
    pg_offs_fix = get_fixup_pg_tbl_offs(lx_exe_mod, page_nr);

    /* Offset for next page.*/
    pg_end_offs_fix = get_fixup_pg_tbl_offs(lx_exe_mod, page_nr+1);

    /*struct r32_rlc * get_fixup_rec_tbl_obj(struct LX_module * lx_mod, int offs) */

    /* Fetches a relocations structure from offset pg_offs_fix.*/
    min_rlc = get_fixup_rec_tbl_obj(lx_exe_mod, pg_offs_fix);

    fixup_offset = pg_offs_fix;

    /* struct o32_map * active_page = get_obj_map(lx_exe_mod, page_nr); */

    /* Get the memory address for the page. The page number is
       from the beginning of all pages a need to be adapted to the beginning of this
       object. */
    vm_start_of_page = lx_obj->o32_base +
                                    get_e32_pagesize(lx_exe_mod) * (page_nr-lx_obj->o32_pagemap);

    /* obj_pg_sta->o32_pagedataoffset << get_e32_pageshift(lx_exe_mod) */
    //io_printf("#######   lx_obj->o32_base = %lu, %lu, %lu, lx_obj->o32_pagemap=%d\n",
    //                lx_obj->o32_base, get_e32_pagesize(lx_exe_mod), page_nr, lx_obj->o32_pagemap);
    //io_printf("#######   vm_start_of_page = %lu (0x%lx)\n", vm_start_of_page, vm_start_of_page);

    /*
    This loop traverses the fixups and increases
    the pointer min_rlc with the size of previoues fixup.
    while(min_rlc is within the offset of current page) {
    */
    while(fixup_offset < pg_end_offs_fix)
    {
      //io_printf("while( %d < %d ) {\n", fixup_offset, pg_end_offs_fix);
      min_rlc = get_fixup_rec_tbl_obj(lx_exe_mod, fixup_offset);
      print_struct_r32_rlc_info(min_rlc);
      //io_printf(" pg_offs_fix = %d (0x%x)\n", pg_offs_fix, pg_offs_fix);
      //io_printf(" pg_end_offs_fix = %d (0x%x)\n",pg_end_offs_fix, pg_end_offs_fix);
      //io_printf(" fixup_offset = %d (0x%x)\n",  fixup_offset, fixup_offset); 

      fixup_source = min_rlc->nr_stype & 0xf;
      fixup_source_flag = min_rlc->nr_stype & 0xf0;
      //io_printf(" fixup_source: 0x%x, fixup_src_flag: 0x%x \n", fixup_source, fixup_source_flag);



      switch(min_rlc->nr_flags & NRRTYP)
      {
        case NRRINT :

          { /* Internal Fixup*/
            //io_printf("Internal Fixup \n");
            /* Indata: lx_exe_mod, min_rlc*/
            srcoff_cnt1 = get_srcoff_cnt1_rlc(min_rlc);
            object1 = get_mod_ord1_rlc(min_rlc); /* On the same offset as Object1. */
            trgoffs = get_trgoff_size(min_rlc);
            //io_printf("srcoff_cnt1=%d, object1=%d",srcoff_cnt1, object1);
            //if(trgoffs > 0)
            //        io_printf(", trgoffs=%d \n", trgoffs);

            target_object = get_obj(lx_exe_mod, object1);
            vm_start_target_obj = target_object->o32_base;

            /* Get address of target offset and put in source offset. */
            vm_target = vm_start_target_obj + get_imp_ord1_rlc(min_rlc)/*trgoffs*/;
            vm_source = vm_start_of_page + srcoff_cnt1;

            //io_printf(" ###### vm_start_of_page= %lu, srcoff_cnt1 = %lu \n",
            //                vm_start_of_page, srcoff_cnt1);

            ptr_source = (unsigned long int *)vm_source;
            *ptr_source = vm_target;
            //io_printf(" ###### vm_target = %lu, vm_source = %lu, ptr_source =%p\n",
            //                vm_target,vm_source,ptr_source );
            //print_o32_obj_info(*target_object, " Info about target object ");

            /* TODO */
            /* print_struct_r32_rlc_info((struct r32_rlc *) &min_rlc); */

          }
          /*fixup_offset += 7;  Seven bytes to inc to next fixup. */
          fixup_offset += get_reloc_size_rlc(min_rlc);
          break;

        case NRRORD:
          {/* Import by ordinal */
            //io_printf("Import by ordinal \n");
            /* Indata: lx_exe_mod, min_rlc */
            mod_nr = get_mod_ord1_rlc(min_rlc);
            import_ord = get_imp_ord1_rlc(min_rlc);
            srcoff_cnt1 = get_srcoff_cnt1_rlc(min_rlc);
            addit = get_additive_rlc(min_rlc);

            mod_name = (char*)&buf_mod_name;
                                            /* Get name of imported module. */
            org_mod_name = get_imp_mod_name(lx_exe_mod,mod_nr);
            copy_pas_str(mod_name, org_mod_name);

            //io_printf(" module name: '%s' \n", mod_name);
            /* Look for module if it's already loaded, if it's not try to load it.*/
            //found_module = (struct LX_module *)find_module(mod_name);
            rc=ModLoadModule(uchLoadError, sizeof(uchLoadError), 
                              mod_name, (unsigned long *)&found_module);
            if (found_module)
               found_module=(struct LX_module *)(((IXFModule *)found_module)->FormatStruct);
            if(!found_module) { /* Unable to find and load module. */
                   io_printf("Can't find module: '%s' \n", mod_name);
                   *ret_rc = rc;
                   return 0;
            }
            cont_mod_name = (char*)&cont_buf_mod_name;
            copy_pas_str(cont_mod_name, get_module_name_res_name_tbl_entry(lx_exe_mod));

            //io_printf(" Done loaded: '%s' (Import by ordinal), continuing with: %s\n",
            //                mod_name,  cont_mod_name);

            /*
            apply_import_fixup(struct LX_module * this_module, struct LX_module * found_module,
                                    struct o32_obj * lx_obj,
                                    int mod_nr, int import_ord, int addit, int srcoff_cnt1,
                                    struct r32_rlc * min_rlc) */

            if(!apply_import_fixup(lx_exe_mod, found_module, lx_obj, mod_nr, import_ord,
                                                    addit, srcoff_cnt1, min_rlc, ret_rc)) {
               char tmp_buf[255];
               char *s_buf=(char*) &tmp_buf[0];
               copy_pas_str(s_buf, get_imp_proc_name(found_module,import_ord)); 
               io_printf("Import error in '%s', can't find '%s'(%d)\n", 
                            mod_name, s_buf, import_ord);
               *ret_rc = 182; /* ERROR_ORDINAL_NOT_FOUND 182, ERROR_FILE_NOT_FOUND 2*/
               return 0;
            }


          }
          fixup_offset += get_reloc_size_rlc(min_rlc);
          break;

        case NRRNAM:
          {/* Import by name */
            //io_printf("Import by name \n");
            mod_nr = get_mod_ord1_rlc(min_rlc);
            import_name_offs = get_imp_ord1_rlc(min_rlc);
            srcoff_cnt1 = get_srcoff_cnt1_rlc(min_rlc);
            addit = get_additive_rlc(min_rlc);

            pas_imp_proc_name = get_imp_proc_name(lx_exe_mod, import_name_offs);
            copy_pas_str(buf_import_name, pas_imp_proc_name);
            import_name = (char*)&buf_import_name;
            mod_name = (char*)&buf_mod_name;
                                            /* Get name of imported module. */
            org_mod_name = get_imp_mod_name(lx_exe_mod,mod_nr);
            copy_pas_str(mod_name, org_mod_name);

            //io_printf(" function name: '%s' \n", buf_import_name);
            //io_printf(" module name: '%s' \n", mod_name);
                  /* Look for module if it's already loaded, if it's not try to load it.*/
            //found_module = (struct LX_module *)find_module(mod_name);
            rc=ModLoadModule(uchLoadError, sizeof(uchLoadError), 
                             mod_name, (unsigned long *)&found_module);
            if (found_module)
               found_module=(struct LX_module *)(((IXFModule *)found_module)->FormatStruct);
               
            if(!found_module) { /* Unable to find and load module. */
              //io_printf("Can't find module: '%s' \n", mod_name);
              *ret_rc = rc;
              return 0;
            }

            ord_found = get_res_name_tbl_entry(found_module, buf_import_name);
            if(ord_found == 0)
                    ord_found = get_non_res_name_tbl_entry(found_module, buf_import_name);

            /*
            apply_import_fixup(struct LX_module * this_module, struct LX_module * found_module,
                                    struct o32_obj * lx_obj,
                                    int mod_nr, int import_ord, int addit, int srcoff_cnt1,
                                    struct r32_rlc * min_rlc) */

            if(!apply_import_fixup(lx_exe_mod, found_module, lx_obj, mod_nr,
                                   ord_found, addit, srcoff_cnt1, min_rlc, ret_rc)) {
               char tmp_buf[255];
               char *s_buf=(char*) &tmp_buf[0];
               copy_pas_str(s_buf, get_imp_proc_name(found_module,import_ord)); 
               io_printf("Import error in '%s', can't find '%s'\n", mod_name, s_buf);
               *ret_rc = 182; /* ERROR_ORDINAL_NOT_FOUND 182, ERROR_FILE_NOT_FOUND 2*/
               return 0;
            }


            /* TODO
              Leta efter funktionen  buf_import_name i mod_name. Den är en sträng
              så skapa funktioner att leta i Entry Table i mod_name.
              Nåt med en pascalsträng och en ordinal, använd ordinalen som
              vanligt och leta upp funktionen med den.

            */


          }
          fixup_offset += get_reloc_size_rlc(min_rlc);
          break;

        default: io_printf("Unsupported Fixup! SRC: 0x%x \n", fixup_source); 
                 return 0; /* Is there any OS/2 error number for this? */
      } /* switch(fixup_source) */
    } /* while(fixup_offset < pg_end_offs_fix) { */
  }
  return 1;
}

/*  Used functions: get_entry, get_imp_mod_name, get_module_name_res_name_tbl_entry, 
                    copy_pas_str, ModLoadModule, native_find_module, get_imp_proc_name,
                    native_get_func_ptr_ord_handle, native_get_func_ptr_handle_modname,
                    get_obj
*/
int apply_import_fixup(struct LX_module * this_module, struct LX_module * found_module,
                            struct o32_obj * lx_obj,
                            int mod_nr, int import_ord, int addit, int srcoff_cnt1,
                            struct r32_rlc * min_rlc, int *ret_rc)
{
  unsigned long rc;
  char * org_mod_name;
  void * native_module;
  unsigned int i_offs_to_fix_in_exe;
  unsigned long int i_dll_func_offs;
  unsigned long int obj_vm_base;
  int * ptr_fixup_code;
  struct LX_module *forward_found_module;
  struct LX_module * prev_mod;
  /* Name of module from forward entry. Chunk of variables for Forward Entrys.*/
  void *fn_ptr;
  char frw_buf_mod_name[255];
  char * frw_mod_name = (char *) &frw_buf_mod_name;
  void *frw_fn_ptr = 0; /* Returned from get_entry for forward entries, used as status check.*/
  int ret_flags, ret_obj, ret_modord, ret_type;
  unsigned long int ret_offset;
  char buf_mod_name[255];
  char * mod_name = (char*)&buf_mod_name;
  UCHAR uchLoadError[CCHMAXPATH] = {0}; /* Error info from DosExecPgm */

  /* Get name of imported module. */
  org_mod_name = get_imp_mod_name(this_module, mod_nr);

  *frw_mod_name = 0;
  copy_pas_str(mod_name, org_mod_name);

  /* Get the entry for an function from a module, assume only two types for now,
     an 32 bit entry or an forward entry.*/
  //io_printf("loadobjlx.c:%d, get_entry( %p, %d \n", __LINE__, found_module, import_ord);
  fn_ptr = get_entry((struct LX_module *)found_module, import_ord,
                     &ret_flags, (int*)&ret_offset, &ret_obj, &ret_modord, &ret_type);

  /* Forward Entry */
  /* A Forward Entry refers to another module and entry point which itself
     can refer to another module and entrypoint.
     So, to load the modules, find the chain of entrypoints as long it is a
     forward entry. Up to a maximum of 1024 forward entrys.
     
     There is some sort of bug with the loop below, it works as long the entries 
     are forward entries but does not look up the last (which could be a 32-bit entry) 
     with the correct function ordinal from the previous forward entry.
     
     An example:
      forward entry              32-bit entry
      MSG.5 (DosPutMessage)  ->  DOSCALLS.387
                                   ^
                                   Error it checks for DOSCALLS.5
  */

  if((ret_type & ENTRYFWD)==ENTRYFWD)
  {
    int frw_flags=0, frw_offset=0, frw_obj=0, frw_modord=0, frw_type=0;
    int frw_mod_nr;
    int forward_counter;
    //io_printf(" Forward Entry.\n");
    forward_found_module=found_module;

    org_mod_name = get_imp_mod_name(found_module, ret_modord);
    frw_mod_nr = ret_offset;
    forward_counter=1;
    
    //frw_mod_name = (char *) &frw_buf_mod_name;
    //get_res_name_tbl_entry(struct LX_module * lx_mod, char *entry_name)
    //org_mod_name = get_imp_mod_name(forward_found_module, 0); /* Find out the module name.*/
    //copy_pas_str(frw_mod_name, org_mod_name);
    io_printf("Start module:%s, forward_found_module=0x%x\n", 
                                get_module_name_res_name_tbl_entry(forward_found_module), 
                                forward_found_module);

    do {
      frw_mod_name = (char *) &frw_buf_mod_name;
      org_mod_name = get_imp_mod_name(forward_found_module, ret_modord);
      copy_pas_str(frw_mod_name, org_mod_name);
      //io_printf("Forward imp mod: %s, ret_offset: %d\n",
      //                frw_mod_name, ret_offset);
      prev_mod = forward_found_module;
      //forward_found_module = (struct LX_module *)find_module(frw_mod_name);
      rc=ModLoadModule(uchLoadError, sizeof(uchLoadError), frw_mod_name, 
                        (unsigned long *)&forward_found_module);
      
      if (forward_found_module) {
         forward_found_module=(struct LX_module *)(((IXFModule *)forward_found_module)->FormatStruct);
         io_printf("Loading module: %s, forward_found_module=0x%x\n", 
                     get_module_name_res_name_tbl_entry(forward_found_module),
                     forward_found_module);
      }
      *ret_rc = rc;
      if(!forward_found_module) { /* Unable to find and load module. */
        io_printf("Can't find forward module: '%s' \n", frw_mod_name);
#if 0
        native_module = native_find_module(frw_mod_name);
        if(native_module != 0) {
          /* Try to load a native library instead. */
          /* void * native_find_module(char * name, struct t_processlx *proc);
             void * native_get_func_ptr_handle_modname(char * funcname, void * native_mod_handle);
          */
          *ret_rc = 0; /*NO_ERROR 0;*/
          char buf_native_name[255];
          char * native_name = (char*)&buf_native_name;
          /* Get name of imported module. */
          char * org_native_name = get_imp_proc_name(prev_mod, frw_offset);
          copy_pas_str(native_name, org_native_name);

          if (strlen(native_name)==0)
          {
          #ifndef __LINUX__
            io_printf("Native ordinal: %d, module name: %s\n", ret_offset, frw_mod_name);
            frw_fn_ptr = native_get_func_ptr_ord_handle(ret_offset , native_module);
          #endif
          } else {
            io_printf("Native name: %s, module name: %s\n", native_name, frw_mod_name);
            frw_fn_ptr = native_get_func_ptr_handle_modname(native_name , native_module);
          }
          ret_offset = (unsigned long int)frw_fn_ptr;
          frw_type = ENTRYNATIVE;
          goto cont_native_entry;
        }
#endif
        return 0;
      }
      io_printf(" frw_type=%d, frw_modord=%d, frw_mod_nr=%d\n", frw_type, frw_modord, frw_mod_nr);
      
      frw_fn_ptr = get_entry(forward_found_module, frw_mod_nr,
                              &frw_flags, &frw_offset, &frw_obj, &frw_modord, &frw_type);
      io_printf(" get_entry: forward_found_module=0x%x, ordinal:%d, entry type:%d\n", 
                 forward_found_module, frw_offset, frw_type );
      frw_mod_nr = frw_offset;
      forward_counter++;
      /*io_printf("## }while((frw_type & ... forward_counter=%d\n", forward_counter);*/
    }while((frw_type & ENTRYFWD)==ENTRYFWD && (forward_counter<1024));
    found_module = forward_found_module;
    
    cont_native_entry:; /* A label that is used from inside the do..while-loop. */
    ret_flags=frw_flags;  ret_offset=frw_offset; ret_obj=frw_obj;
    ret_modord=frw_modord; ret_type=frw_type;
    fn_ptr = frw_fn_ptr;
    io_printf(" frw_modord=%d, frw_mod_nr=%d, frw_type:%d, ENTRYFWD:%d, ENTRY32:%d\n", 
                   frw_modord, frw_mod_nr, frw_type, ENTRYFWD, ENTRY32);
    //io_printf(" Done with Forward Entry running. (%d) \n", forward_counter);
  }

  if (fn_ptr == 0)
  {
    char entry_buf_mod_name[255];
    char * entry_mod_name = (char*)&entry_buf_mod_name;

    io_printf("Error, can't find entrypoint: %d, in module: %d (%s) \n",
                    import_ord, mod_nr, copy_pas_str(entry_mod_name,get_imp_mod_name(found_module, mod_nr)));
    if(frw_mod_name != 0)
      io_printf("frw_mod_name=%s", frw_mod_name);
    if(mod_name != 0)
      io_printf(" mod_name=%s", mod_name);
    io_printf("\n");
    return 0; /* Error can't find an entry point.*/
  }

  /* 32 bit Entry */
  if((ret_type & ENTRY32)==ENTRY32 || ((ret_type & ENTRYNATIVE)== ENTRYNATIVE))
  {

    io_printf(" Additive = %d\n", addit);
    obj_vm_base =0;
    i_dll_func_offs =0;

    if((ret_type & ENTRY32)==ENTRY32)
    {
      obj_vm_base =  get_obj(found_module, ret_obj)->o32_base;
      i_dll_func_offs = obj_vm_base + ret_offset;
    } else {
      obj_vm_base     =  (unsigned long int)frw_fn_ptr;
      i_dll_func_offs =  (unsigned long int)frw_fn_ptr;
    }

    i_offs_to_fix_in_exe = lx_obj->o32_base + srcoff_cnt1;
    //io_printf("DLL module, obj_vm_base: 0x%x, func_offs: 0x%x \n",
    //                obj_vm_base, ret_offset);
    //io_printf("EXE module, obj_vm_base: 0x%lx, source position: 0x%x\n",
    //                lx_obj->o32_base, srcoff_cnt1);

    /* Is the EXE module placed under the DLL module in memory? */
    if((lx_obj->o32_base + srcoff_cnt1) < (obj_vm_base+ ret_offset))
    {
      int relative_jmp = i_dll_func_offs - (lx_obj->o32_base + srcoff_cnt1);
      //io_printf("Relative jump: %d, %lu, %lx \n", relative_jmp, relative_jmp, relative_jmp);
      ptr_fixup_code = (int *) i_offs_to_fix_in_exe;
      *ptr_fixup_code = relative_jmp;
      //io_printf("Patched address: %p \n", ptr_fixup_code);
      //io_printf("Content of patched address: %d \n", *ptr_fixup_code);
    } else {
      int relative_jmp2 = 0xffffffff-((lx_obj->o32_base + srcoff_cnt1) - i_dll_func_offs)-3;
      //io_printf("Relative jump (DLL under EXE): %d, %lu \n",relative_jmp2, relative_jmp2);
      ptr_fixup_code = (int *) i_offs_to_fix_in_exe;
      *ptr_fixup_code = relative_jmp2;
      //io_printf("Patched address: %p \n", ptr_fixup_code);
      //io_printf("Content of patched address: %d \n", *ptr_fixup_code);
    }
  }

  return 1;
}


