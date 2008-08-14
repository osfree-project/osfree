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

#include <loadobjlx.h>
#include <fixuplx.h>
#include <modlx.h>
#include <sys/mman.h>
#include <dynlink.h>
#include <native_dynlink.h>


  /* Loads the objects for code and data, for programs. NOT used. */
int load_code_data_obj_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc) {
	struct o32_obj * kod_obj = (struct o32_obj *) get_code(lx_exe_mod);
	
	struct o32_obj * stack_obj = (struct o32_obj *) get_data_stack(lx_exe_mod);
	
	void * vm_code_obj = 0;
	void * vm_data_obj = 0;	
	
	if((kod_obj != 0) && (kod_obj == stack_obj)) { /* Data and code in same object. */ 
		printf("load_code_data_obj_lx: Code and stack/data is the same object!\n");
		
		/* Allocate virtual memory at the address that kod_obj requires. */
		vm_code_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, kod_obj);
		
		/* Register the allocated memory with memmgr. */
		alloc_mem_area(&proc->root_mem_area, (void *) kod_obj->o32_base, kod_obj->o32_size);
		
		proc->code_mmap = vm_code_obj;

		if(vm_code_obj == MAP_FAILED) {
			printf("Error mapping memory for(code/data)\n");
			return 0;
		}
		/* Load code object. */
		load_obj_lx(lx_exe_mod, proc, kod_obj, vm_code_obj);
		return 1;
	}
	
	/* Allocate virtual memory at the address that kod_obj requires. */
	vm_code_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, kod_obj);
	
	
	proc->code_mmap = vm_code_obj;
	
	if(vm_code_obj == MAP_FAILED) {
		printf("Error mapping memory for (code)\n");
		return 0;
	}
	/* Register the allocated memory with memmgr. */
	alloc_mem_area(&proc->root_mem_area, (void *) kod_obj->o32_base, kod_obj->o32_size);
	load_obj_lx(lx_exe_mod, proc, kod_obj, vm_code_obj);
	
	if(stack_obj == 0) return 0;
	vm_data_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, stack_obj);
	
	/* Register the allocated memory with memmgr. */
	alloc_mem_area(&proc->root_mem_area, (void *) stack_obj->o32_base, stack_obj->o32_size);
		
	proc->stack_mmap = vm_data_obj;
	
	if(vm_data_obj == MAP_FAILED) {
		printf("Error mapping memory for (data/stack)\n");
		return 0;
	}
	load_obj_lx(lx_exe_mod, proc, stack_obj, vm_data_obj);
	
	print_o32_obj_info(kod_obj, " Info about kod_obj ");
	print_o32_obj_info(stack_obj, " Info about stack_obj ");
			
	return 1;
}


  /* Loads all the objects in a dll and exe. */
int load_dll_code_obj_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc) {
	/*struct o32_obj * kod_obj = (struct o32_obj *) get_code(lx_exe_mod); */
	
	/*struct o32_obj * stack_obj = (struct o32_obj *) get_data_stack(lx_exe_mod);*/
	
	unsigned long int number_of_objects = get_obj_num(lx_exe_mod);
	
	unsigned int obj_cnt;
	
	for(obj_cnt=1; obj_cnt<=number_of_objects; obj_cnt++) {
		struct o32_obj * kod_obj = get_obj(lx_exe_mod, obj_cnt);
		printf("obj_cnt: %d, number_of_objects: %d\n", obj_cnt, number_of_objects);
		void * vm_code_obj = 0;
		if( !is_mem_used(&proc->root_mem_area, (void *) kod_obj->o32_base, kod_obj->o32_size) ) {
			vm_code_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, kod_obj);
			alloc_mem_area(&proc->root_mem_area, (void *) kod_obj->o32_base, kod_obj->o32_size);
		} else {
			unsigned long int new_base = (unsigned long int) seek_free_mem(&proc->root_mem_area, kod_obj->o32_size); 
							/* What kind of requirements is there on allocated memory 
								for DLLs is OS/2? Allocated in "Shared Area"? Where is that, 
								from 128 MiB and downwards to 64 MiB??? 
								This would be the place to make that kind of call.
								But, for now, just allocate the space somewhere. 
								Which just goes through the memory registry from beginning,
								position 1 and upward and grabs first free space. */
			if( is_mem_used(&proc->root_mem_area, (void *) new_base, kod_obj->o32_size) )
				printf("Error allocating used memory!!! 0x%lx \n", new_base);
			kod_obj->o32_base = new_base;
			printf(" new_base: 0x%lx, %lu \n", new_base, new_base);
			vm_code_obj = (void*) vm_alloc_obj_lx(lx_exe_mod, kod_obj);
			printf(" new_base == %p ?\n", vm_code_obj);
			alloc_mem_area(&proc->root_mem_area, (void *) kod_obj->o32_base, kod_obj->o32_size);
		}

		/*proc->code_mmap = vm_code_obj;*/

		if(vm_code_obj == MAP_FAILED) {
			printf("Error mapping memory for (code)\n");
			return 0;
		}
		load_obj_lx(lx_exe_mod, proc, kod_obj, vm_code_obj);

		print_o32_obj_info(kod_obj, " Info about kod_obj ");
	}
	
	return 1;
}

/* void exec_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc) { */

  /*   Allocates virtual memory for an object at an absolute virtual address. 
       Some GNU/Linux specific flags to mmap(MAP_GROWSDOWN). */
unsigned int vm_alloc_obj_lx(struct LX_module * lx_exe_mod, struct o32_obj * lx_obj) {
				
	void * mmap_obj = 0;
	
	mmap_obj = mmap((void *)lx_obj->o32_base, lx_obj->o32_size, 
				 PROT_WRITE | PROT_READ | PROT_EXEC  ,       /* | PROT_EXEC */
				  MAP_GROWSDOWN | MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 0/*lx_exe_mod->fh*/, 
				 0 /*lx_exe_mod->lx_head_e32_exe->e32_datapage*/);
	return (unsigned int) mmap_obj;
}

	/* Loads an object to the memory area pointed by vm_ptr_obj. */
int load_obj_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc, 
				struct o32_obj * lx_obj, void *vm_ptr_obj) {

		int startpage = lx_obj->o32_pagemap;
		int lastpage  = lx_obj->o32_pagemap + lx_obj->o32_mapsize;
		unsigned int data_pages_offs =  get_e32_datapage(lx_exe_mod);
		unsigned int code_mmap_pos = 0;
		int page_nr=0;
		/*struct o32_map * obj_pg_ett = get_obj_map(lx_exe_mod ,startpage);*/
		  /*  Reads in all pages from kodobject to designated virtual memory. */
		for(page_nr=startpage; page_nr<lastpage; page_nr++) {
			struct o32_map * obj_pg_sta = get_obj_map(lx_exe_mod ,page_nr);
			int ofs_page_sta = (obj_pg_sta->o32_pagedataoffset << get_e32_pageshift(lx_exe_mod)) 
								+ data_pages_offs;

			lx_exe_mod->lx_fseek(lx_exe_mod, ofs_page_sta, SEEK_SET);
			
			unsigned long int tmp_code = (unsigned long int) code_mmap_pos;
			unsigned long int tmp_vm = (unsigned long int) vm_ptr_obj;
			unsigned long int tmp_vm_code = tmp_code + tmp_vm;
			
			lx_exe_mod->lx_fread((void *)tmp_vm_code,  
								obj_pg_sta->o32_pagesize, 1, lx_exe_mod);
			code_mmap_pos += obj_pg_sta->o32_pagesize;
		}
		return 0;
}		
		

	/* Applies fixups to all objects. Used for programs and dlls. */
int do_fixup_code_data_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc) {

	struct o32_obj * kod_obj = (struct o32_obj *) get_code(lx_exe_mod);
	
	struct o32_obj * stack_obj = (struct o32_obj *) get_data_stack(lx_exe_mod);
	
	printf("do_fixup_code_data_lx: %d, kod_obj=%p\n", __LINE__, kod_obj);
	
	/* If there is a code object (with a main functoion) then do a fixup on it and
	   it's data/stack object if it exists.*/
	   /*
	if((kod_obj != 0) && do_fixup_obj_lx(lx_exe_mod, proc, kod_obj)) {
		printf("do_fixup_code_data_lx: %d, stack_obj=%p\n", __LINE__, stack_obj);
		if(stack_obj != 0)
			do_fixup_obj_lx(lx_exe_mod, proc, stack_obj);
		return 1;
	}
	 else { // do a fixup on all loaded objects, because it's an dll. 
		if( (get_mflags(lx_exe_mod) & E32NOTP)==E32NOTP) {
	*/
			unsigned long int i;
			for(i=1; i<=get_obj_num(lx_exe_mod); i++) {
				struct o32_obj * obj = get_obj(lx_exe_mod, i);
				if(obj != 0)
					do_fixup_obj_lx(lx_exe_mod, proc, obj);
			}
		/*}
	}*/
		
	return 1;
}
	
	
	/* Applies fixups for an object. 
	 */	
int do_fixup_obj_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc, 
				struct o32_obj * lx_obj) {
	
		int page_nr=0;
		int startpage = lx_obj->o32_pagemap;
		int lastpage  = lx_obj->o32_pagemap + lx_obj->o32_mapsize;
		printf("--------------------Listing fixup data ------------------------- %p\n", lx_obj);
		
		/* Goes through every page of the object. 
		   The fixups are variable size and a bit messy to traverse.*/
		print_o32_obj_info(lx_obj, " Info about source object ");
							   
		for(page_nr=startpage; page_nr < lastpage; page_nr++) {
			printf("-----  Object %d of %d\n",startpage, lastpage);
			
			/* Go and get byte position for fixup from the page logisk_sida. 
			   Start offset for fixup in the page*/
			int pg_offs_fix = get_fixup_pg_tbl_offs(lx_exe_mod, page_nr);
			
			/* Offset for next page.*/
			int pg_end_offs_fix = get_fixup_pg_tbl_offs(lx_exe_mod, page_nr+1);

			/*struct r32_rlc * get_fixup_rec_tbl_obj(struct LX_module * lx_mod, int offs) */
			
			/* Fetches a relocations structure from offset pg_offs_fix.*/
			struct r32_rlc * min_rlc = get_fixup_rec_tbl_obj(lx_exe_mod, pg_offs_fix);
			
			int fixup_offset = pg_offs_fix;
			
			/* struct o32_map * active_page = get_obj_map(lx_exe_mod, page_nr); */
			
			/* Get the memory address for the page. The page number is 
			   from the beginning of all pages a need to be adapted to the beginning of this 
			   object. */
			unsigned long int vm_start_of_page = lx_obj->o32_base + 
							get_e32_pagesize(lx_exe_mod) * (page_nr-lx_obj->o32_pagemap);
							
			/* obj_pg_sta->o32_pagedataoffset << get_e32_pageshift(lx_exe_mod) */
			printf("#######   lx_obj->o32_base = %lu, %lu, %lu, lx_obj->o32_pagemap=%d\n", 
					lx_obj->o32_base, get_e32_pagesize(lx_exe_mod), page_nr, lx_obj->o32_pagemap); 
			printf("#######   vm_start_of_page = %lu (0x%lx)\n", vm_start_of_page, vm_start_of_page);
			
			/* 
			This loop traverses the fixups and increases
			the pointer min_rlc with the size of previoues fixup. 
			while(min_rlc is within the offset of current page) {
			*/
			while(fixup_offset < pg_end_offs_fix) {
				printf("while( %d < %d ) {\n", fixup_offset, pg_end_offs_fix);
				min_rlc = get_fixup_rec_tbl_obj(lx_exe_mod, fixup_offset);		
				print_struct_r32_rlc_info(min_rlc);
				printf(" pg_offs_fix = %d (0x%x)\n", pg_offs_fix, pg_offs_fix);
				printf(" pg_end_offs_fix = %d (0x%x)\n",pg_end_offs_fix, pg_end_offs_fix);
				printf(" fixup_offset = %d (0x%x)\n",  fixup_offset, fixup_offset);

				int fixup_source = min_rlc->nr_stype & 0xf;
				int fixup_source_flag = min_rlc->nr_stype & 0xf0;
				printf(" fixup_source: 0x%x, fixup_src_flag: 0x%x \n", fixup_source, fixup_source_flag);

				
					 
				switch(min_rlc->nr_flags & NRRTYP) {
					case NRRINT : 
											
						{ /* Internal Fixup*/
							printf("Internal Fixup \n");
							/* Indata: lx_exe_mod, min_rlc*/
							int srcoff_cnt1 = get_srcoff_cnt1_rlc(min_rlc);
							int object1 = get_mod_ord1_rlc(min_rlc); /* On the same offset as Object1. */
							int trgoffs = get_trgoff_size(min_rlc);
							printf("srcoff_cnt1=%d, object1=%d",srcoff_cnt1, object1);
							if(trgoffs > 0)
								printf(", trgoffs=%d \n", trgoffs);
								
							struct o32_obj * target_object = get_obj(lx_exe_mod, object1);
							unsigned long int vm_start_target_obj = target_object->o32_base;
							
							/* Get address of target offset and put in source offset. */
							unsigned long int vm_target = vm_start_target_obj + trgoffs;
							unsigned long int vm_source = vm_start_of_page + srcoff_cnt1;
							
							printf(" ###### vm_start_of_page= %lu, srcoff_cnt1 = %lu \n",
									vm_start_of_page, srcoff_cnt1);
							
							unsigned long int * ptr_source = (unsigned long int *)vm_source;
							*ptr_source = vm_target;
							printf(" ###### vm_target = %lu, vm_source = %lu, ptr_source =%p\n", 
									vm_target,vm_source,ptr_source );
							/* print_o32_obj_info(lx_obj, " Info about source object "); */
							print_o32_obj_info(target_object, " Info about target object ");
							
							/* TODO */
							print_struct_r32_rlc_info((struct r32_rlc *) &min_rlc);
						
						}
						/*fixup_offset += 7;  Seven bytes to inc to next fixup. */
						fixup_offset += get_reloc_size_rlc(min_rlc);
						break;

					case NRRORD:   
					     {/* Import by ordinal */
							printf("Import by ordinal \n");
							/* Indata: lx_exe_mod, min_rlc */
							int mod_nr = get_mod_ord1_rlc(min_rlc);
							int import_ord = get_imp_ord1_rlc(min_rlc); 
							int srcoff_cnt1 = get_srcoff_cnt1_rlc(min_rlc);
							int addit = get_additive_rlc(min_rlc);

							char buf_mod_name[255];
							char * mod_name = (char*)&buf_mod_name;
											/* Get name of imported module. */
							char * org_mod_name = get_imp_mod_name(lx_exe_mod,mod_nr);
							copy_pas_str(mod_name, org_mod_name);

							printf(" module name: '%s' \n", mod_name); 
									/* Look for module if it's already loaded, if it's not try to load it.*/
							struct LX_module *found_module = (struct LX_module *)find_module(mod_name, proc);
							if(!found_module) { /* Unable to find and load module. */
								printf("Can't find module: '%s' \n", mod_name);
								return 0;
							}
							char cont_buf_mod_name[255];
							char * cont_mod_name = (char*)&cont_buf_mod_name;
							copy_pas_str(cont_mod_name, get_module_name_res_name_tbl_entry(lx_exe_mod));
							
							printf(" Done loaded: '%s' (Import by ordinal), continuing with: %s\n", 
									mod_name,  cont_mod_name);
							
							/*
							apply_import_fixup(struct LX_module * this_module, struct LX_module * found_module, 
										struct t_processlx * proc, struct o32_obj * lx_obj,
										int mod_nr, int import_ord, int addit, int srcoff_cnt1,
										struct r32_rlc * min_rlc) */
							
							apply_import_fixup(lx_exe_mod, found_module, proc, lx_obj, mod_nr, import_ord,
												addit, srcoff_cnt1, min_rlc);
										
				
						}
						fixup_offset += get_reloc_size_rlc(min_rlc);
						break;
						
					case NRRNAM:
						{/* Import by name */
							printf("Import by name \n");
							int mod_nr = get_mod_ord1_rlc(min_rlc);
							int import_name_offs = get_imp_ord1_rlc(min_rlc); 
							int srcoff_cnt1 = get_srcoff_cnt1_rlc(min_rlc);
							int addit = get_additive_rlc(min_rlc);
							
							char buf_import_name[260];
							char *pas_imp_proc_name = get_imp_proc_name(lx_exe_mod, import_name_offs);
							copy_pas_str(buf_import_name, pas_imp_proc_name);
							char *import_name = (char*)&buf_import_name;
							char buf_mod_name[255];
							char * mod_name = (char*)&buf_mod_name;
											/* Get name of imported module. */
							char * org_mod_name = get_imp_mod_name(lx_exe_mod,mod_nr);
							copy_pas_str(mod_name, org_mod_name);
							
							printf(" function name: '%s' \n", buf_import_name);
							printf(" module name: '%s' \n", mod_name); 
									/* Look for module if it's already loaded, if it's not try to load it.*/
							struct LX_module *found_module = (struct LX_module *)find_module(mod_name, proc);
							if(!found_module) { /* Unable to find and load module. */
								printf("Can't find module: '%s' \n", mod_name);
								return 0;
							}
							
							int ord_found = get_res_name_tbl_entry(found_module, buf_import_name);
							if(ord_found == 0)
								ord_found = get_non_res_name_tbl_entry(found_module, buf_import_name);
							
							/*
							apply_import_fixup(struct LX_module * this_module, struct LX_module * found_module, 
										struct t_processlx * proc, struct o32_obj * lx_obj,
										int mod_nr, int import_ord, int addit, int srcoff_cnt1,
										struct r32_rlc * min_rlc) */
							
							apply_import_fixup(lx_exe_mod, found_module, proc, lx_obj, mod_nr, 
												ord_found, addit, srcoff_cnt1, min_rlc);
							

							/* TODO
							  Leta efter funktionen  buf_import_name i mod_name. Den är en sträng 
							  så skapa funktioner att leta i Entry Table i mod_name.
							  Nåt med en pascalsträng och en ordinal, använd ordinalen som 
							  vanligt och leta upp funktionen med den.
							
							*/
							
							
						}
						fixup_offset += get_reloc_size_rlc(min_rlc);
						break;            

					default: printf("Unsupported Fixup! SRC: 0x%x \n", fixup_source); return 0;
				} /* switch(fixup_source) */
			} /* while(fixup_offset < pg_end_offs_fix) { */
		}
		return 1;
}


int apply_import_fixup(struct LX_module * this_module, struct LX_module * found_module, 
						struct t_processlx * proc, struct o32_obj * lx_obj,
						int mod_nr, int import_ord, int addit, int srcoff_cnt1,
						struct r32_rlc * min_rlc) {
	/* Namn on module from forward entry. Chunk of variables for Forward Entrys.*/
	char frw_buf_mod_name[255];
	char * frw_mod_name = (char *) &frw_buf_mod_name;
	*frw_mod_name = 0;
	void *frw_fn_ptr = 0; /* Returned from get_entry for forward entries, used as status check.*/

	char buf_mod_name[255];
	char * mod_name = (char*)&buf_mod_name;
					/* Get name of imported module. */
	char * org_mod_name = get_imp_mod_name(this_module, mod_nr);
	copy_pas_str(mod_name, org_mod_name);

	int ret_flags, ret_obj, ret_modord, ret_type;
	unsigned long int ret_offset;
		/* Get the entry for an function from a module, assume only two types for now, 
		   an 32 bit entry or an forward entry.*/
	printf("loadobjlx.c:%d, get_entry( %p, %d \n", __LINE__, found_module, import_ord);
	void *fn_ptr = get_entry((struct LX_module *)found_module, import_ord,
					&ret_flags, (int*)&ret_offset, &ret_obj, &ret_modord, &ret_type);
	int has_done_forward_jumping = 0;
	/* A label to continue execution after an chain of forward entries 
	   has been traversed. */
	test_entry:;	
	if(fn_ptr == 0) {
		char entry_buf_mod_name[255];
		char * entry_mod_name = (char*)&entry_buf_mod_name;

		printf("Error, can't find entrypoint: %d, in module: %d (%s) \n", 
				import_ord, mod_nr, copy_pas_str(entry_mod_name,get_imp_mod_name(found_module, mod_nr)));
		if(frw_mod_name != 0)
			printf("frw_mod_name=%s", frw_mod_name);
		if(mod_name != 0)
			printf(" mod_name=%s", mod_name);
		printf("\n");	
		return 0; /* Error can't find an entry point.*/					
	}		
	/* 32 bit Entry */
	if((ret_type & ENTRY32)==ENTRY32 || ((ret_type & ENTRYNATIVE)== ENTRYNATIVE)) {

		printf(" Additive = %d\n", addit);
		unsigned long int obj_vm_base =0;
		unsigned long int i_dll_func_offs =0;
		
		if((ret_type & ENTRY32)==ENTRY32) {
			obj_vm_base =  get_obj(found_module, ret_obj)->o32_base;
			i_dll_func_offs = obj_vm_base + ret_offset;
		} else {
			obj_vm_base     =  (unsigned long int)frw_fn_ptr;
			i_dll_func_offs =  (unsigned long int)frw_fn_ptr;
		}
		
		unsigned int i_offs_to_fix_in_exe = lx_obj->o32_base + srcoff_cnt1;
		printf("DLL module, obj_vm_base: 0x%x, func_offs: 0x%x \n", 
				obj_vm_base, ret_offset);
		printf("EXE module, obj_vm_base: 0x%lx, source position: 0x%x\n", 
				lx_obj->o32_base, srcoff_cnt1); 

		/* Is the EXE module placed under the DLL module in memory? */
		if((lx_obj->o32_base + srcoff_cnt1) < (obj_vm_base+ ret_offset)) {
			int relative_jmp = i_dll_func_offs - (lx_obj->o32_base + srcoff_cnt1);
			printf("Relative jump: %d, %lu, %lx \n", relative_jmp, relative_jmp, relative_jmp);
			int * ptr_fixup_code = (int *) i_offs_to_fix_in_exe;
			*ptr_fixup_code = relative_jmp;
			printf("Patched address: %p \n", ptr_fixup_code);
			printf("Content of patched address: %d \n", *ptr_fixup_code);
		} else {
			int relative_jmp2 = (lx_obj->o32_base + srcoff_cnt1) - i_dll_func_offs;
			printf("Relative jump (DLL under EXE): %d, %lu \n",relative_jmp2, relative_jmp2);
		}
	}
	/* Forward Entry */
	/* A Forward Entry refers to another module and entry point which itself
	   can refer to another module and entrypoint.
	   So, to load the modules, find the chain of entrypoints as long it is a 
	   forward entry. Up to a maximum of 1024 forward entrys.
	*/
	if((has_done_forward_jumping == 0) &&(ret_type & ENTRYFWD)==ENTRYFWD) {
		printf(" Forward Entry.\n");
		struct LX_module *forward_found_module=found_module;

		char * org_mod_name = get_imp_mod_name(found_module, ret_modord);
		int frw_flags=0, frw_offset=0, frw_obj=0, frw_modord=0, frw_type=0;
		int frw_mod_nr = ret_offset;
		int forward_counter=1;

		do {
			/*printf("## do {\n");*/							
			frw_mod_name = (char *) &frw_buf_mod_name;
			org_mod_name = get_imp_mod_name(forward_found_module, ret_modord);
			copy_pas_str(frw_mod_name, org_mod_name);
			printf("Forward imp mod: %s, ret_offset: %d\n", 
					frw_mod_name, ret_offset);
			struct LX_module * prev_mod = forward_found_module;
			forward_found_module = (struct LX_module *)find_module(frw_mod_name, proc);
			
			if(!forward_found_module) { /* Unable to find and load module. */
				printf("Can't find forward module: '%s' \n", mod_name);
				
				void * native_module = native_find_module(frw_mod_name, proc);
				if(native_module != 0) {
					/* Try to load a native library instead. */
					/* void * native_find_module(char * name, struct t_processlx *proc); 
					   void * native_get_func_ptr_handle_modname(char * funcname, void * native_mod_handle);
					*/
					
					char buf_native_name[255];
					char * native_name = (char*)&buf_native_name;
									/* Get name of imported module. */
					char * org_native_name = get_imp_proc_name(prev_mod, frw_offset);
					copy_pas_str(native_name, org_native_name);					
					
					printf("Native name: %s, module name: %s\n", native_name, frw_mod_name);
					frw_fn_ptr = native_get_func_ptr_handle_modname(native_name , native_module);
					ret_offset = (unsigned long int)frw_fn_ptr;
					frw_type = ENTRYNATIVE;
					goto cont_native_entry;
				}
				return 0;
			}  
			frw_fn_ptr = get_entry(forward_found_module, frw_mod_nr,
						&frw_flags, &frw_offset, &frw_obj, &frw_modord, &frw_type);
			frw_mod_nr = frw_offset;
			forward_counter++;
			/*printf("## }while((frw_type & ... forward_counter=%d\n", forward_counter);*/
		}while((frw_type & ENTRYFWD)==ENTRYFWD && (forward_counter<1024));
		found_module = forward_found_module;
		cont_native_entry:;
		ret_flags=frw_flags;  ret_offset=frw_offset; ret_obj=frw_obj; 
		ret_modord=frw_modord; ret_type=frw_type;
		fn_ptr = frw_fn_ptr;
		printf(" frw_modord=%d, frw_mod_nr=%d", frw_modord, frw_mod_nr);
		printf(" Done with Forward Entry running. (%d) \n", forward_counter);
		has_done_forward_jumping = 1;
		goto test_entry;
	}
	return 1;
}						


