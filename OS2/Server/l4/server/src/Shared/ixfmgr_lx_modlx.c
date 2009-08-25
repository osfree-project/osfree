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

#include "modlx.h"
#include "io.h"
#include "cfgparser.h"
#include "modmgr.h"


struct o32_obj *
    get_data_stack(struct LX_module * lx_mod);
int get_data_stack_num(struct LX_module * lx_mod);
unsigned long
    get_esp(struct LX_module * lx_mod);
struct o32_map *
    get_obj_page_tbl(struct LX_module * lx_mod);
struct o32_map *
    get_obj_map(struct LX_module * lx_mod, unsigned int nr);
int get_ldrsize(struct LX_module * lx_mod);
int get_fixupsize(struct LX_module * lx_mod);
void print_o32_obj_info(struct o32_obj o_obj, char * namn);
int get_fixup_pg_tbl_offs(struct LX_module * lx_mod, int logisk_sida);
struct r32_rlc *
    get_fixup_rec_tbl_obj(struct LX_module * lx_mod, int offs);
void print_struct_r32_rlc_info(struct r32_rlc * rlc);
int get_srcoff_cnt1_rlc(struct r32_rlc * rlc);
int get_mod_ord1_rlc(struct r32_rlc * rlc);
int get_imp_ord1_rlc(struct r32_rlc * rlc);
char * get_imp_mod_name(struct LX_module * lx_mod, int mod_idx);

int get_e32_pageshift(struct LX_module * lx_mod);


/* fseek(fh, 0, SEEK_SET);
   SEEK_SET move filepos relative to beginning of the file
   SEEK_END move filepos relative to the end of file,
   SEEK_CUR move filepos from current position.
   int fseek(FILE *stream, long offset, int whence); */
/* This does does a fseek on a memory buffer. So simulate the behavior.*/
int lx_fseek_stream(struct LX_module * lx_mod, int pos, int direction) {
        if((direction == SEEK_SET) && (pos <= lx_mod->lx_stream_size) && (pos >= 0)) {
                lx_mod->lx_stream_pos = pos;
                return lx_mod->lx_stream_pos;
        }
        if((direction == SEEK_END) && (pos <= lx_mod->lx_stream_size) && (pos >= 0)) {
                lx_mod->lx_stream_pos = lx_mod->lx_stream_size - pos;
                return lx_mod->lx_stream_pos;
        }
        if((direction == SEEK_CUR) && (pos <= lx_mod->lx_stream_size) && (pos >= 0)) {
                lx_mod->lx_stream_pos = lx_mod->lx_stream_pos + pos;
                return lx_mod->lx_stream_pos;
        }
        return -1;
}

/* Just do an ordinary fseek. */
int lx_fseek_disk_file(struct LX_module * lx_mod, int pos, int direction) {
        return fseek(lx_mod->fh, pos, direction);
}
/*
lx_mod->lx_fread(&exe_sig, 1, sizeof(exe_sig), lx_mod);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
The  function  fread()  reads  nmemb elements of data, each size bytes long, from the
       stream pointed to by stream, storing them at the location given by ptr.
*/
size_t lx_fread_stream(void *ptr, size_t size, size_t nmemb,
                                                struct LX_module * lx_mod) {
        unsigned int i;
        unsigned long int tmp_ptr;

        for(i=0; i<nmemb; i++) {
                /* lx_file_stream lx_stream_size lx_stream_pos */
                /* io_printf("memcpy(%p, %p, %d)\n",ptr, &lx_mod->lx_file_stream[lx_mod->lx_stream_pos], size);
                 io_printf("nmenb=%d\n", nmemb); */

                memcpy(ptr, &lx_mod->lx_file_stream[lx_mod->lx_stream_pos], size);
                lx_mod->lx_stream_pos += size;
                tmp_ptr = (unsigned int) ptr;
                tmp_ptr += size;
                ptr = (void *) tmp_ptr;
        }
        return i;
}
size_t lx_fread_disk_file(void *ptr, size_t size, size_t nmemb,
                                                struct LX_module * lx_mod) {
        return fread(ptr, size, nmemb, lx_mod->fh);
}


        /*
        Reads the lx file from filehandle fh into structure lx_mod.
        Constructor for disk based file.
        */
struct LX_module *
load_lx(FILE* fh, struct LX_module * lx_mod)
{
        int fstorlek;

        lx_mod->lx_head_e32_exe = 0;
        lx_mod->offs_lx_head = 0;
        lx_mod->lx_file_stream = 0; /* Zero out the memory buffer, which is not used here. */
        lx_mod->fh = fh;
        lx_mod->lx_fseek = &lx_fseek_disk_file;
        lx_mod->lx_fread = &lx_fread_disk_file;
        //io_printf("Load from filehandle: %p \n", fh);

        /* Find out size of file. */
        fstorlek = fseek(fh, 0, SEEK_END);
        fstorlek = fstorlek;
        fseek(fh, 0, SEEK_SET);


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


/*
#include <sys/mman.h>

void *mmap(void *start, size_t length, int prot, int flags,
           int fd, off_t offset);

int munmap(void *start, size_t length);

DESCRIPTION
 The mmap() function asks to map length bytes starting at offset offset
 from the file (or other object) specified by the file descriptor fd into
 memory, preferably at address start. This latter address is a hint only,
 and is usually specified as 0. The actual place where the object is
 mapped is returned by mmap(), and is never 0.
*/

/*
API:er i inf2html.obj:
        DosSetCurrentDir
        DosExit
        DosOpen
        DosQueryFileInfo
        DosRead
        DosClose
        DosCreateDir
        DosQueryCurrentDir

        och diverse funktioner i C-biblioteket.

Använda moduler i inf2html.exe:
        DOSCALLS
        NLS
        MSG

Några OS/2 exe-filer:
/pub/projekt_src/mylxread/hello_os2v2.exe
/mnt/rei2/inf2html/inf2html.exe

*/



/*    unsigned long       e32_startobj;    Object # for instruction pointer
//    unsigned long       e32_eip;         Extended instruction pointer
//    unsigned long       e32_stackobj;    Object # for stack pointer
//    unsigned long       e32_esp;         Extended stack pointer */

        /* Objekt table.
           Gets an o32_obj for a object. An o32_obj describes an object
           it's virtuall addressize, relocatable base address, flaggs,
           page table index, number of pages in Object Page Table. First element starts at one. */
struct o32_obj *
get_obj(struct LX_module * lx_mod, unsigned int nr) {
        struct o32_obj * obj_ent = (struct  o32_obj *) lx_mod->loader_section;
        return &obj_ent[nr-1];
}


unsigned long int
get_obj_num(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_objcnt;
}

  /* Get the number of elements in Object Page Table */
int get_obj_map_num(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_mpages;
}


struct o32_obj *
get_code(struct LX_module * lx_mod) {
        if(lx_mod->lx_head_e32_exe->e32_startobj == 0) {
                if (options.debugixfmgr) 
                {
                  io_printf("Invalid start object for code ==0\n");
                }
                return (struct o32_obj *)0;
        }
        return get_obj(lx_mod, lx_mod->lx_head_e32_exe->e32_startobj);
}


unsigned long int
get_code_num(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_startobj;
}


unsigned long
get_eip(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_eip;
}


struct o32_obj *
get_data_stack(struct LX_module * lx_mod) {
        if((lx_mod->lx_head_e32_exe->e32_stackobj == 0) ||
                (lx_mod->lx_head_e32_exe->e32_stackobj > get_obj_num(lx_mod))) {
                if (options.debugixfmgr) 
                {
                  io_printf("Invalid data/stack object ==%lu, max=%lu\n",
                                  lx_mod->lx_head_e32_exe->e32_stackobj,
                                  get_obj_num(lx_mod));
                }
                return (struct o32_obj *)0;
        }
        return get_obj(lx_mod, lx_mod->lx_head_e32_exe->e32_stackobj);
}


int get_data_stack_num(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_stackobj;
}


unsigned long
get_esp(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_esp;
}


        /* Hämta pekare till Page Map Table.
        Används så här:
                struct o32_map * obj = get_obj_page_tbl( lx_module );
                return obj[nr];*/
struct o32_map *
get_obj_page_tbl(struct LX_module * lx_mod) {

        int storlek_lx_hdr = sizeof(*(lx_mod->lx_head_e32_exe)); /* Storlek på LX huvudet.*/
        /*lx_mod->e32_objmap             Position på Page Map Table från början av LX huvudet. */

        int pg_tbl_off = lx_mod->lx_head_e32_exe->e32_objmap - storlek_lx_hdr;

        struct o32_map * obj_ent = (struct o32_map *) &lx_mod->loader_section[pg_tbl_off];
        return obj_ent;//obj_ent->o32_pagesize;obj_ent->o32_pagedataoffset;
}                                  //obj_ent->o32_pageflags;


/* Hämta ett objekt från Object Page Table.*/
struct o32_map *
get_obj_map(struct LX_module * lx_mod, unsigned  int nr) {
        struct o32_map * obj_ent = (struct  o32_map *) get_obj_page_tbl(lx_mod);
        return &obj_ent[nr-1];
}


int get_ldrsize(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_ldrsize;
}


int get_fixupsize(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_fixupsize;
}


void print_o32_obj_info(struct o32_obj o_obj, char * name)
{
  io_printf("%s: virtual memory size              = %08XH\n", name, O32_SIZE(o_obj));
  io_printf("          relocation base address          = %08XH\n", O32_BASE(o_obj));
  io_printf("          object flag bits                 = %08XH\n", O32_FLAGS(o_obj));
  io_printf("          object page table index          = %08XH\n", O32_PAGEMAP(o_obj));
  io_printf("          # of object page table entries   = %08XH\n", O32_MAPSIZE(o_obj));
  io_printf("          reserved                         = %08XH\n", O32_RESERVED(o_obj));
  io_printf("          flags = READABLE|EXECUTABLE|16:16_ALIAS\n");
  io_printf("\n");
}

/* Hämtar offset i fixuptabellen för en sida (logisk_sida). */
int get_fixup_pg_tbl_offs(struct LX_module * lx_mod, int logisk_sida) {
        int * i_ptr = (int *) lx_mod->fixup_section;
        return i_ptr[logisk_sida-1];
}

/*struct r32_rlc                           Relocation item  */
/* Hämtar en relokeringselement på offset i fixuptabellen. */
struct r32_rlc * get_fixup_rec_tbl_obj(struct LX_module * lx_mod, int offs){
        /*lx_mod->lx_head_e32_exe->e32_fpagetab 234 "Fixup Page Table Off"
        //lx_mod->lx_head_e32_exe->e32_frectab  242 "Fixup Record Table Off" */
        int offs_to_reloc_struct = lx_mod->lx_head_e32_exe->e32_frectab -
                                                                        lx_mod->lx_head_e32_exe->e32_fpagetab;
        return (struct r32_rlc *) &lx_mod->fixup_section[offs + offs_to_reloc_struct];
}

/* Get's a name (pascal string) from Import Module Table at index in mod_idx.*/
char * get_imp_mod_name(struct LX_module * lx_mod, int mod_idx)
{
  char * mod_name;
  long i;

  int offs_mod_table = lx_mod->lx_head_e32_exe->e32_impmod -
                     lx_mod->lx_head_e32_exe->e32_fpagetab;

  mod_name=(&lx_mod->fixup_section[offs_mod_table]);

  for (i=0;
       i!=(mod_idx-1);
       i++)
  {
    mod_name=mod_name+(char)*mod_name+1;
  }

  return mod_name;
}


char * get_imp_mod_name_cstr(struct LX_module * lx_mod, int mod_idx, char *buf, int len) {

                                        /* Get name of imported module. */
        char * org_mod_name = get_imp_mod_name(lx_mod, mod_idx);
        copy_pas_str(buf, org_mod_name);
        return buf;
}

/* unsigned long       e32_impproc;    // Offset of Import Procedure Name Table */

        /* Get a (pascal) string from Import Procedure Name Table. The names of imported functions. */
char * get_imp_proc_name(struct LX_module * lx_mod, int proc_idx) {
        int offs_imp_proc_table = lx_mod->lx_head_e32_exe->e32_impproc -
                                                     lx_mod->lx_head_e32_exe->e32_fpagetab;
        int c_len = lx_mod->fixup_section[offs_imp_proc_table+proc_idx];
        c_len = c_len;
        if(proc_idx == 0) /* First entry is a null string, size zero. Skip it.*/
                proc_idx = 1;
        return &lx_mod->fixup_section[offs_imp_proc_table+proc_idx];

}


        /* Returns the size of an page. */
int get_e32_pagesize(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_pagesize;
}

/* Gets the "Data Pages Offset". The position to where the "Object Pages" start. */
int get_e32_datapage(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_datapage;
}


int get_e32_pageshift(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_pageshift;
}

        /* Copies an pascal string from src to dest. First byte in src is the lenght byte.
           Make sure there is enough space in dest or it will probably misbehave.*/
char * copy_pas_str(char * dest, char * src) {
        int i=(int) src[0];
        int j=0;
        for(j=0; j<i; j++)
                dest[j] = src[j+1];
        dest[j] = (char) 0;
        return dest;
}


/* struct e32_exe {
   unsigned long       e32_objtab;     // Object table offset
   unsigned long       e32_enttab;     // Offset of Entry Table

   struct b32_bundle
{
    unsigned char       b32_cnt;        // Number of entries in this bundle
    unsigned char       b32_type;       // Bundle type
    unsigned short      b32_obj;        // Object number


 An entry is a description of an exported function with info about where it is inside
 an object and it's offset inside that object. This info is used to apply fixups.

 Type of Entries:
        Unused Entry (2 bytes in size): (Numbers to skip over)
                CNT  TYPE

        16-bit Entry (7 bytes in size):
                CNT  TYPE  OBJECT  FLAGS  OFFSET16

        286 Call Gate Entry (9 bytes in size):
                CNT  TYPE  OBJECT  FLAGS  OFFSET16  CALLGATE

        32-bit Entry (9 bytes in size):
                CNT  TYPE  OBJECT  FLAGS  OFFSET

        Forwarder Entry (9 bytes in size):
                CNT  TYPE  RESERVED  FLAGS  MOD_ORD#  OFFSET/ORDNUM

        Field sizes:
                CNT, TYPE, FLAGS, = DB
                OBJECT, OFFSET16, CALLGATE, RESERVED, MOD_ORD# = DW
                OFFSET, ORDNUM = DD
 ----------------------------------------------------------------
  Get's the function number in entry_ord_to_search from the module lx_mod.
  Returns data in the pointers beginning with "ret_*"
   */

void * get_entry(struct LX_module * lx_mod, int entry_ord_to_search,
                                        int * ret_flags,
                                        int * ret_offset,
                                        int * ret_obj,
                                        int * ret_modord,
                                        int * ret_type) {


        const int UNUSED_ENTRY_SIZE = 2;
        const int ENTRY_HEADER_SIZE = 4; /* For all entries except UNUSED ENTRY.*/
        const int _16BIT_ENTRY_SIZE = 3;
        const int _286_CALL_GATE_ENTRY_SIZE = 5;
        const int _32BIT_ENTRY_SIZE         = 5;
        const int FORWARD_ENTRY_SIZE        = 7;
        /* For testing: lx_mod is null and ret_type contains a pointer to some entry table.*/
        int offs_to_entry_tbl;
        struct b32_bundle *entry_table,
                          *prev_entry_table;
        char *cptr_ent_tbl;
        struct e32_entry *entry_post;
        unsigned char * magic;
        char bbuf[3];
        int entry_ord_index;
        int prev_ord_index;
        int unused_entry;
        unsigned long int i_cptr_ent_tbl;
        int elements_in_bundle;

        if(lx_mod != 0) {

                /* Offset to Entry Table inside the Loader Section. */
                offs_to_entry_tbl = lx_mod->lx_head_e32_exe->e32_enttab - lx_mod->lx_head_e32_exe->e32_objtab;

                entry_table = (struct b32_bundle *) &lx_mod->loader_section[offs_to_entry_tbl];
                cptr_ent_tbl = &lx_mod->loader_section[offs_to_entry_tbl];

                magic = get_magic(lx_mod);
                if((magic[0] != 'L') && (magic[1] != 'X')) { /* Invalid module */
                        *ret_type = -1;
                        return (void *)0;
                }

                bbuf[0] = magic[0];
                bbuf[1] = magic[1];
                bbuf[2] = 0;
                /*io_printf("magic: %s \n", (char *)&bbuf);
                  io_printf("get_entry( lx_mod: %p, entry_ord_to_search: %d \n",
                              lx_mod, entry_ord_to_search); */
        }
        else {
                //io_printf("Testing get_entry.\n");
                cptr_ent_tbl = (char*) *ret_type;
                entry_table = (struct b32_bundle *)cptr_ent_tbl;
        }

        entry_ord_index=1;

        /*io_printf("entry_table: %p \n", entry_table);*/

      /*io_printf(
          "get_entry: entry_ord_to_search: %d, b32_cnt: %d, b32_type: %d, b32_obj: %d\n",
                          entry_ord_to_search,
                         entry_table->b32_cnt, entry_table->b32_type, entry_table->b32_obj);
         */

        /*io_printf("EMPTY: %d, ENTRYFWD: %d, _32BIT_ENTRY_SIZE: %d \n",
          EMPTY, ENTRYFWD, _32BIT_ENTRY_SIZE);*/

        unused_entry = 0;
        while(entry_ord_index <= entry_ord_to_search) {
                while (entry_table->b32_type == EMPTY) { /* Unused Entry, just skip over them.*/
                        prev_ord_index = entry_ord_index;
                        entry_ord_index += entry_table->b32_cnt;
                        unused_entry += UNUSED_ENTRY_SIZE;
                        cptr_ent_tbl += UNUSED_ENTRY_SIZE;
                        entry_table = (struct b32_bundle *)cptr_ent_tbl;
                        prev_entry_table = entry_table;
                        io_printf("EMPTY, entry_table: %p\n", entry_table);
                }
                io_printf("entry_ord_index: %d (0x%x), entry_ord_to_search: %d (0x%x)\n",
                  entry_ord_index,entry_ord_index,
                  entry_ord_to_search,entry_ord_to_search);
                if(entry_ord_to_search < entry_ord_index) {
                        if (prev_ord_index <= entry_ord_to_search)
                        {
                                entry_ord_index = prev_ord_index;
                                entry_table = prev_entry_table;
                                goto sw;
                        }
                        *ret_flags = 0; // Unused entry ==0
                        *ret_offset = 0;
                        *ret_obj = 0;
                        *ret_type = 0;
                        io_printf("RET, Can't find entry.\n");
                        io_printf("entry_ord_index: %d, entry_ord_to_search: %d\n", entry_ord_index, entry_ord_to_search);
                        return (void *) 0; // Can't find entry.
                }
                if(entry_table->b32_cnt == 0) {
                       //io_printf("End of entry table reached! %d, entry_table: %p\n",
                       //                  entry_table->b32_cnt, entry_table);
                                *ret_flags = 0;
                                *ret_offset = 0;
                                *ret_obj = 0;
                                *ret_type = 0;
                                return (void *) 0; /* Invalid entry.*/
                }
        sw:
                switch(entry_table->b32_type) {
                    case EMPTY:;
                    case ENTRYFWD:;
                    case ENTRY32: break;
                        default: io_printf("Invalid entry type! %d, entry_table: %p\n",
                                         entry_table->b32_type, entry_table);
                                *ret_flags = 0;
                                *ret_offset = 0;
                                *ret_obj = 0;
                                *ret_type = 0;
                                return (void *) 0; /* Invalid entry.*/
                }


                /*io_printf("get_entry,2nd bundle: cnt: %d, type: %d, obj: %d\n",
                                entry_table->b32_cnt, entry_table->b32_type, entry_table->b32_obj);     */
                /* If ordinal to search for is less than present index ordinal and
                   ordinal to search for is less than the number of ordinals plus current index ordinal and
                   the type of bundle is 32-bit entry.
                */
//                if((entry_ord_to_search >= entry_ord_index)
//                   && (entry_ord_to_search<(entry_table->b32_cnt+entry_ord_index))
//                   &&  (entry_table->b32_type == ENTRY32))
                if((entry_ord_to_search <= entry_ord_index)
                   && (entry_ord_index - entry_ord_to_search <= entry_table->b32_cnt)
                   &&  (entry_table->b32_type == ENTRY32))
                { /* 32-bit Entry.*/

                                //io_printf("32-bit Entry.\n");
                                cptr_ent_tbl = (char*)entry_table;
                                cptr_ent_tbl += ENTRY_HEADER_SIZE;
                                entry_post = (struct e32_entry *)cptr_ent_tbl;
                                i_cptr_ent_tbl = (unsigned long)cptr_ent_tbl;
                                elements_in_bundle = entry_table->b32_cnt + entry_ord_index;
                                for(entry_ord_index; entry_ord_index < elements_in_bundle; entry_ord_index++) {
                                        //io_printf("(entry_ord_to_search %d == entry_ord_index %d)\n",
                                                        //entry_ord_to_search, entry_ord_index);
                                    if(entry_ord_to_search == entry_ord_index)
                                                break;
                                        i_cptr_ent_tbl += _32BIT_ENTRY_SIZE;
                                }
                                /*entry_ord_index += entry_table->b32_cnt;
                                i_cptr_ent_tbl += _32BIT_ENTRY_SIZE*entry_table->b32_cnt;*/

                                cptr_ent_tbl = (char*)i_cptr_ent_tbl;
                                entry_post = (struct e32_entry *)cptr_ent_tbl;
                                //io_printf("cptr_ent_tbl: %p \n", cptr_ent_tbl);

                                //io_printf("entry_post: %p \n", entry_post);
                                //io_printf("32-bit Entry: Flags=0x%x, Offset=%lu \n",
                                //                entry_post->e32_flags, entry_post->e32_variant.e32_offset.offset32);
                                *ret_flags = entry_post->e32_flags;
                                *ret_offset = entry_post->e32_variant.e32_offset.offset32;
                                *ret_obj = entry_table->b32_obj;
                                *ret_type = entry_table->b32_type;
                                return (void *) entry_table;

                } else if(entry_table->b32_type == ENTRY32) { /* Jump over that bundle. */
                        //io_printf("ENTRY32\n")
                        cptr_ent_tbl = (char*)entry_table;
                        cptr_ent_tbl += ENTRY_HEADER_SIZE;
                        //prev_ord_index = entry_ord_index;
                        //prev_entry_table = entry_table;
                        i_cptr_ent_tbl = (unsigned long)cptr_ent_tbl;
                        entry_ord_index += entry_table->b32_cnt;
                        i_cptr_ent_tbl += _32BIT_ENTRY_SIZE*entry_table->b32_cnt;

                        cptr_ent_tbl = (char*)i_cptr_ent_tbl;
                        entry_table= (struct b32_bundle *)cptr_ent_tbl;
                        io_printf("ENTRY32, entry_table: %p\n", entry_table);
                }

                if((entry_ord_to_search >= entry_ord_index)
                    && (entry_ord_to_search<(entry_table->b32_cnt+entry_ord_index))
                    && (entry_table->b32_type == ENTRYFWD)) { /* Forward Entry.*/
                        //io_printf("Forward Entry.\n");
                        cptr_ent_tbl = (char*)entry_table;
                        //io_printf("cptr_ent_tbl: %p \n", cptr_ent_tbl);
                        entry_post = (struct e32_entry *)&cptr_ent_tbl[ENTRY_HEADER_SIZE];
                        cptr_ent_tbl = &cptr_ent_tbl[ENTRY_HEADER_SIZE];
                        //io_printf("cptr_ent_tbl: %p \n", cptr_ent_tbl);
                                i_cptr_ent_tbl = (unsigned long int)cptr_ent_tbl;
                                elements_in_bundle = entry_table->b32_cnt + entry_ord_index;
                                for(entry_ord_index; entry_ord_index < elements_in_bundle; entry_ord_index++) {
                                    //io_printf("(entry_ord_to_search %d == entry_ord_index %d)\n",
                                    //                    entry_ord_to_search, entry_ord_index);
                                        if(entry_ord_to_search == entry_ord_index)
                                                break;
                                        i_cptr_ent_tbl += FORWARD_ENTRY_SIZE;
                                }
                                cptr_ent_tbl = (char*)i_cptr_ent_tbl;
                                entry_post = (struct e32_entry *)cptr_ent_tbl;
                        io_printf("entry_post: %p \n", entry_post);
                        io_printf("Forward Entry: Flags=0x%x, Proc name offset or ordinal=%lu, Module ordinal number: %d \n",
                                        entry_post->e32_flags,
                                        entry_post->e32_variant.e32_fwd.value,
                                        entry_post->e32_variant.e32_fwd.modord);
                                 /* Flags */
                        *ret_flags = entry_post->e32_flags;
                         /* Procedure Name Offset or Import Ordinal Number */
                        *ret_offset = entry_post->e32_variant.e32_fwd.value;
                                /* Module ordinal */
                        *ret_modord = entry_post->e32_variant.e32_fwd.modord;
                                /* Not used */
                        *ret_obj = entry_table->b32_obj;
                                /* Entry type*/
                        *ret_type = entry_table->b32_type;
                         /* unsigned short  modord;     / Module ordinal number
                unsigned long   value;      / Proc name offset or ordinal

                                e32_fwd;        / Forwarder */
                        return (void *) entry_table;
                } else if(entry_table->b32_type == ENTRYFWD) { /* Jump over the that bundle. */
                    //io_printf("ENTRYFWD\n");

                        cptr_ent_tbl = (char*)entry_table;
                        //io_printf("cptr_ent_tbl: %p\n", cptr_ent_tbl);
                        cptr_ent_tbl = &cptr_ent_tbl[ENTRY_HEADER_SIZE];
                        //io_printf("cptr_ent_tbl: %p\n", cptr_ent_tbl);
                        i_cptr_ent_tbl = (unsigned long int)cptr_ent_tbl;
                        entry_ord_index += entry_table->b32_cnt;
                        //io_printf("FORWARD_ENTRY_SIZE: %d, entry_table->b32_cnt: %d\n", FORWARD_ENTRY_SIZE, entry_table->b32_cnt);
                        i_cptr_ent_tbl += FORWARD_ENTRY_SIZE*entry_table->b32_cnt;

                        cptr_ent_tbl = (char*)i_cptr_ent_tbl;
                        entry_table= (struct b32_bundle *)cptr_ent_tbl;
                        //io_printf("ENTRYFWD, entry_table: %p\n", entry_table);
                }

        }
        //io_printf("RET, get_entry()\n");
        return (void *) entry_table;
}

/* Return the module flags. 0x8000 == library module*/
unsigned long int get_mflags(struct LX_module * lx_mod) {
        return lx_mod->lx_head_e32_exe->e32_mflags;
}

unsigned char * get_magic(struct LX_module * lx_mod) {
        /* unsigned char       e32_magic[2];  */
        return (unsigned char *)&lx_mod->lx_head_e32_exe->e32_magic;
}


/*  The Non-resident Names Table is not read into memory so we need to read it
    from the file.
   unsigned long       e32_nrestab;    // Offset of Non-resident Names Table
   unsigned long       e32_cbnrestab;  // Size of Non-resident Name Table

   unsigned long       e32_restab;     // Offset of resident name table
   unsigned long       e32_enttab;     // Offset of Entry Table     */


        /* Get a string from Import Procedure Name Table. */
char * get_imp_proc_name_2(struct LX_module * lx_mod, int proc_idx) {
        int offs_imp_proc_table = lx_mod->lx_head_e32_exe->e32_impproc -
                                                     lx_mod->lx_head_e32_exe->e32_fpagetab;
        int c_len = lx_mod->fixup_section[offs_imp_proc_table+proc_idx];
        c_len = c_len;
        return &lx_mod->fixup_section[offs_imp_proc_table+proc_idx];

}

        /* Get the ordinal for the entry_name from the Resident Names Table. */
int get_res_name_tbl_entry(struct LX_module * lx_mod, char *entry_name)
{
        int c_len;
        char buf_name[255];
        char *ptr_buf_name = (char *)&buf_name;
        int entry_lenghts = 0;
        int ordinal = 0;

        int offs_res_name_table = lx_mod->lx_head_e32_exe->e32_restab -
                                                                lx_mod->lx_head_e32_exe->e32_objtab;
        int size_res_name_table = lx_mod->lx_head_e32_exe->e32_enttab -
                                                                lx_mod->lx_head_e32_exe->e32_restab;

        io_printf("Resident Names Table size: %d, 0x%x \n",
                                lx_mod->lx_head_e32_exe->e32_enttab - lx_mod->lx_head_e32_exe->e32_restab,
                                lx_mod->lx_head_e32_exe->e32_enttab - lx_mod->lx_head_e32_exe->e32_restab );
        io_printf("Resident Names Table offset: %d, 0x%x \n",
                                lx_mod->lx_head_e32_exe->e32_restab,
                                lx_mod->lx_head_e32_exe->e32_restab );
        c_len = lx_mod->loader_section[offs_res_name_table];


        while(entry_lenghts < (size_res_name_table-1)) {
                c_len = lx_mod->loader_section[offs_res_name_table + entry_lenghts];
                copy_pas_str(ptr_buf_name,
                                        (char *)&lx_mod->loader_section[offs_res_name_table + entry_lenghts]);
                io_printf(" Found: %s \n", ptr_buf_name);

                ordinal = *((unsigned short int*)&lx_mod->loader_section[
                                                        offs_res_name_table + c_len+1+entry_lenghts]);
                io_printf(" Found (ord): %d \n", ordinal);
                if(strcmp(entry_name,ptr_buf_name)==0)
                        return ordinal;

                entry_lenghts += c_len+1+2;
                io_printf("entry_lenghts: %d \n", entry_lenghts);
        }
        return 0; /* The ordinal is not found. */
}

        /* Get the ordinal for the entry_name from the Non-resident Names Table.
           If the table is not loaded, it is loaded and a pointer is saved in LX_module.
        */
int get_non_res_name_tbl_entry(struct LX_module * lx_mod, char *entry_name) {

        io_printf("Non-resident Names Table size: %d, 0x%x \n",
                                lx_mod->lx_head_e32_exe->e32_cbnrestab,
                                lx_mod->lx_head_e32_exe->e32_cbnrestab );
        io_printf("Non-resident Names Table offset: %d, 0x%x \n",
                                lx_mod->lx_head_e32_exe->e32_nrestab,
                                lx_mod->lx_head_e32_exe->e32_nrestab );
        return 0;
}


        /* Get the module name from the Resident Names Table. */
char * get_module_name_res_name_tbl_entry(struct LX_module * lx_mod) {

        int offs_res_name_table = lx_mod->lx_head_e32_exe->e32_restab -
                                                                lx_mod->lx_head_e32_exe->e32_objtab;
        int size_res_name_table = lx_mod->lx_head_e32_exe->e32_enttab -
                                                                lx_mod->lx_head_e32_exe->e32_restab;

        int c_len = lx_mod->loader_section[offs_res_name_table];
        char buf_name[255];
        char *ptr_buf_name = (char *)&buf_name;
        int entry_lenghts = 0;
        int ordinal = 0;

        /*while(entry_lenghts < (size_res_name_table-1)) { */
                c_len = lx_mod->loader_section[offs_res_name_table + entry_lenghts];
                copy_pas_str(ptr_buf_name,
                                        (char *)&lx_mod->loader_section[offs_res_name_table + entry_lenghts]);


                ordinal = *((unsigned short int*)&lx_mod->loader_section[
                                                        offs_res_name_table + c_len+1+entry_lenghts]);

                /*if(strcmp(entry_name,ptr_buf_name)==0)
                        return ordinal;*/

                /*entry_lenghts += c_len+1+2;
                io_printf("entry_lenghts: %d \n", entry_lenghts); */
        /*}*/
        return (char *)&lx_mod->loader_section[offs_res_name_table + entry_lenghts];
}
