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

#ifndef MODLX_H
#define MODLX_H

#ifdef SDIOS
#define lseek fseek
#endif



#include <stdlib.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif
/*#ifndef FALSE */
  #define FALSE 0
/*#endif*/
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/exe386.h>

/* /mnt/c/Projekt/osfree/src/include/exe386.h */

struct LX_module {

        struct e32_exe * lx_head_e32_exe;

        char * loader_section;
        /*  The size of Loader Section is in lx_head_e32_exe->e32_ldrsize  */

        char * fixup_section;
        /* The size of Fixup Section is in lx_head_e32_exe->e32_fixupsize */

        int offs_lx_head;

        FILE* fh;                          /* File handle, if the LX file is an ordinary disk file. */
        char * lx_file_stream; /* If the LX file is a buffer, null otherwise. */
        int lx_stream_size;    /* Size of buffer with LX file. */
        int lx_stream_pos;     /* Position in the buffer. */

        /* Two function pointers for seeking and reading the LX file. To have a bit of polymorfism
           to disk file and and a buffer. */
        int (* lx_fseek)(struct LX_module * lx_mod, int pos, int direction);
        size_t (* lx_fread)(void *ptr, size_t size, size_t nmemb, struct LX_module * lx_mod);
};


        /* An earlier experiment with a modified struct r32_rlc. */
struct r32_rlc_8bit_ord                      // Relocation item
{
    unsigned char       nr_stype;        // Source type - field shared with new_rlc
    unsigned char       nr_flags;        // Flag byte - field shared with new_rlc
    short               r32_soff;        // Source offset
    //unsigned short      r32_objmod;      // Target object number or Module ordinal
        union _8_16_obj_mod {
                unsigned char       r32_objmod[1];
                unsigned short      r32_objmod16;
        } _8_16_objmod; // _8_16_objmod.r32_objmod[1]

        union _8_16_imp_ord {
                unsigned char       r32_impord[1];
                unsigned short      r32_impord16;
        } _8_16_impord; //  _8_16_impord.r32_impord[

        union _16_32_add_fix {
                unsigned short       s16_addfix[1];
                unsigned int         i32_addfix;
        } _16_32_addfix; //  _16_32_addfix.s16_addfix[
};

int lx_fseek_stream(struct LX_module * lx_mod, int pos, int direction);
int lx_fseek_disk_file(struct LX_module * lx_mod, int pos, int direction);

size_t lx_fread_stream(void *ptr, size_t size, size_t nmemb, struct LX_module * lx_mod);
size_t lx_fread_disk_file(void *ptr, size_t size, size_t nmemb, struct LX_module * lx_mod);

/* Read in the header from a disk file.
   ALso an constructor for struct LX_module. */
struct LX_module * load_lx(FILE* fh, struct LX_module * lx_mod);




/* Get an object from Object Table. */
struct o32_obj * get_obj(struct LX_module * lx_mod, unsigned int nr);


/* Get the number of objects from Object Table. */
unsigned long int
get_obj_num(struct LX_module * lx_mod);


/* Get the number of elements in Object Page Table */
int get_obj_map_num(struct LX_module * lx_mod);


/* Get the code object från Object Table. The object which contain the main() function. */
struct o32_obj * get_code(struct LX_module * lx_mod);


/* Gets the number of the code object. */
unsigned long int get_code_num(struct LX_module * lx_mod);

/* Gets the value for the (eip) instruction pointer. Where main() starts. */
unsigned long get_eip(struct LX_module * lx_mod);


/* Get object which have the stack/data from Object Table. */
struct o32_obj * get_data_stack(struct LX_module * lx_mod);


/* Gets the number of the stack/data object. */
int get_data_stack_num(struct LX_module * lx_mod);


/* Get the value for stackpointer. */
unsigned long get_esp(struct LX_module * lx_mod);


/* Gets the first element in Object Page Table. Use the returned pointer as an array. */
struct o32_map * get_obj_page_tbl(struct LX_module * lx_mod);


/* Gets an element from Object Page Table */
struct o32_map * get_obj_map(struct LX_module * lx_mod, unsigned int nr);





/* Gets the size of the loader section. */
int get_ldrsize(struct LX_module * lx_mod);


int get_fixupsize(struct LX_module * lx_mod);

  /* Print info abount an Object. */
void print_o32_obj_info(struct o32_obj o_obj, char * namn);

int get_fixup_pg_tbl_offs(struct LX_module * lx_mod, int logisk_sida);

/*struct r32_rlc                           Relocation item  */
struct r32_rlc * get_fixup_rec_tbl_obj(struct LX_module * lx_mod, int offs);


        /* Get a name as a pascal string from Import Module Table. Which DLLs this module needs. */
char * get_imp_mod_name(struct LX_module * lx_mod, int mod_idx);

        /* Get a name as a C string from Import Module Table at index mod_idx. */
char * get_imp_mod_name_cstr(struct LX_module * lx_mod, int mod_idx, char *buf, int len);

        /* Get a pascal string from Import Procedure Name Table. The names of imported functions. */
char * get_imp_proc_name(struct LX_module * lx_mod, int proc_idx);

        /* The size of a page, usually 4096 byte. But you never know if the module has some other size.*/
int get_e32_pagesize(struct LX_module * lx_mod);

  /* Gets offset in file where the pages for the objects start. */
int get_e32_datapage(struct LX_module * lx_mod);

  /* Get the page shift, which is the number of bits an entry in Object Page Table (field Page Data Offset)
     value should be shifted left .*/
int get_e32_pageshift(struct LX_module * lx_mod);

        /* Copy an pascal string from src to dest. */
char * copy_pas_str(char * dest, char * src);

        /* Get an entry point with number ent_ord from the module. */
void * get_entry(struct LX_module * lx_mod, int ent_ord,
                                        int * ret_flags,
                                        int * ret_offset,
                                        int * ret_obj,
                                        int * ret_modord,
                                        int * ret_type);

        /* Gets the flags from the module. For example, if it's an dll or program. */
unsigned long int get_mflags(struct LX_module * lx_mod);

    /* Gets the magic for the module (LX). Obs! It's just a char pointer to the beginning of the
           module and has no zero after the two characters. So only read the two first chars.*/
unsigned char * get_magic(struct LX_module * lx_mod);

int get_res_name_tbl_entry(struct LX_module * lx_mod, char *entry_name);
int get_non_res_name_tbl_entry(struct LX_module * lx_mod, char *entry_name);

char * get_module_name_res_name_tbl_entry(struct LX_module * lx_mod);



#ifdef __cplusplus
};
#endif

#endif
