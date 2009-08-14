#define INCL_DOS
#define INCL_BSEDOS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include <os2.h>
#include <os2errcodes.h>

#include <string.h>

#include <lx.h>
#include <io.h>

#ifndef ENEWHDR
  #define ENEWHDR 0x3c
#endif

#include "modlx.h"
#include "loadobjlx.h"
#include "fixuplx.h"
#include "cfgparser.h"

// Prototypes
unsigned long convert_entry_table_to_BFF(IXFModule * ixfModule);
unsigned long convert_fixup_table_to_BFF(IXFModule * ixfModule);
unsigned long calc_imp_fixup_obj_lx(struct LX_module * lx_exe_mod,
                                struct o32_obj * lx_obj, int *ret_rc);


unsigned long LXIdentify(void * addr, unsigned long size)
{
  unsigned long lx_module_header_offset=0;

  if (((*(char *)addr == 'M') && (*(char *)((unsigned long)addr+1) == 'Z')) ||
      ((*(char *)addr == 'Z') && (*(char *)((unsigned long)addr+1) == 'M')))
  {
    /* Found DOS stub. Get offset of LX module. */
    lx_module_header_offset=*(unsigned long *)((unsigned long)addr+ENEWHDR);
  }

  if ((*(char *)((unsigned long)addr+lx_module_header_offset) == 'L') &&
      (*(char *)((unsigned long)addr+lx_module_header_offset+1) == 'X'))
  {
    #ifdef __OS2__
    // This is check for internal relocations support. Specific for OS/2 host because
    // it is hard to manage virtual memory and processes without kernel driver support.
    // We don't want to loss forces for such driver so debug only code with internal relocations
    // support
    #endif
    return NO_ERROR;
  };

  return ERROR_BAD_FORMAT;
}

unsigned long LXLoad(void * addr, unsigned long size, void * ixfModule)
{
  unsigned long rc;
  long module_counter;
  char buf[256];
  IXFModule *ixf;

  ixf=(IXFModule*)ixfModule;

  ixf->FormatStruct=malloc(sizeof(struct LX_module));

  /* A file from a buffer.*/
  if(LXLoadStream((char*)addr, size, (struct LX_module *)(ixf->FormatStruct)))
  {
    /* Convert imported module table to BFF format */
    ixf->cbModules=((struct LX_module*)(ixf->FormatStruct))->lx_head_e32_exe->e32_impmodcnt;
    ixf->Modules=malloc(sizeof(char *)*ixf->cbModules);

    for (module_counter=1;
         module_counter<ixf->cbModules+1;
         module_counter++)
    {
      get_imp_mod_name_cstr((struct LX_module *)(ixf->FormatStruct), module_counter, &buf, sizeof(buf));
      ixf->Modules[module_counter-1]=malloc(strlen(buf)+1);
      strcpy(ixf->Modules[module_counter-1],buf);
    }

    /* Load all objects in dll.*/
    LXLoadObjects((struct LX_module *)(ixf->FormatStruct));

    /* Convert entry table to BFF format for DLL */
    if(ISDLL(ixf))
    {
      rc=convert_entry_table_to_BFF(ixf);
      if (rc!=0)
      {
        return rc;
      }
    }

    /* Convert entry table to BFF format for DLL */
    convert_fixup_table_to_BFF(ixf);
  }
  return NO_ERROR;
}

unsigned long LXFixup(void * lx_exe_mod)
{
  int rc=NO_ERROR;
  /* Apply internal fixups. */
  do_fixup_code_data_lx((struct LX_module *)lx_exe_mod, &rc);
  return rc; /*NO_ERROR;*/
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
        const int UNUSED_ENTRY_SIZE = 2;
        const int ENTRY_HEADER_SIZE = 4; /* For all entries except UNUSED ENTRY.*/
        const int _16BIT_ENTRY_SIZE = 3;
        const int _286_CALL_GATE_ENTRY_SIZE = 5;
        const int _32BIT_ENTRY_SIZE         = 5;
        const int FORWARD_ENTRY_SIZE        = 7;


unsigned long convert_entry_table_to_BFF(IXFModule * ixfModule)
{
  char buf[256];
  unsigned long cbEntries;   /* Number of items in entry table */
  struct LX_module * lx_mod; /* LX_format structure */

  int offs_to_entry_tbl;
  struct b32_bundle *entry_table_start;
  struct b32_bundle *entry_table,
                    *prev_entry_table;
  char *cptr_ent_tbl;
  struct e32_entry *entry_post;
  char bbuf[3];
  int entry_ord_index;
  int prev_ord_index;
  int unused_entry;
  unsigned long int i_cptr_ent_tbl;
  int elements_in_bundle;
  unsigned long i;


  lx_mod=(struct LX_module *)(ixfModule->FormatStruct);
  /* Offset to Entry Table inside the Loader Section. */
  offs_to_entry_tbl = lx_mod->lx_head_e32_exe->e32_enttab - lx_mod->lx_head_e32_exe->e32_objtab;

  entry_table_start = (struct b32_bundle *) &lx_mod->loader_section[offs_to_entry_tbl];

  entry_table = entry_table_start;
  cptr_ent_tbl = &lx_mod->loader_section[offs_to_entry_tbl];

  ixfModule->cbEntries=0;

  /* Count number of entries */
  while (1)
  {
    switch(entry_table->b32_type) {
      case EMPTY:;
      case ENTRYFWD:;
      case ENTRY32: break;
      default: io_printf("Unsupported entry type! %d, entry_table: %p\n",
               entry_table->b32_type, entry_table);
        return 1; /* Invalid entry. Return ERROR_INVALID_FUNCTION*/
    }


    if (entry_table->b32_cnt == 0) {
        break; /* end of table*/
    }

/* Unused Entry, just skip over them.*/
    if (entry_table->b32_type == EMPTY)
    {
      ixfModule->cbEntries += entry_table->b32_cnt;
      cptr_ent_tbl += UNUSED_ENTRY_SIZE;
      entry_table = (struct b32_bundle *)cptr_ent_tbl;
    } else if (entry_table->b32_type == ENTRY32)
    { /* Jump over that bundle. */
      cptr_ent_tbl = (char*)entry_table;
      cptr_ent_tbl += ENTRY_HEADER_SIZE;
      i_cptr_ent_tbl = (unsigned long)cptr_ent_tbl;
      ixfModule->cbEntries += entry_table->b32_cnt;
      i_cptr_ent_tbl += _32BIT_ENTRY_SIZE*entry_table->b32_cnt;

      cptr_ent_tbl = (char*)i_cptr_ent_tbl;
      entry_table= (struct b32_bundle *)cptr_ent_tbl;
    } else if (entry_table->b32_type == ENTRYFWD)
    { /* Jump over the that bundle. */
      cptr_ent_tbl = (char*)entry_table;
      cptr_ent_tbl = &cptr_ent_tbl[ENTRY_HEADER_SIZE];
      i_cptr_ent_tbl = (unsigned long int)cptr_ent_tbl;
      ixfModule->cbEntries += entry_table->b32_cnt;
      i_cptr_ent_tbl += FORWARD_ENTRY_SIZE*entry_table->b32_cnt;

      cptr_ent_tbl = (char*)i_cptr_ent_tbl;
      entry_table= (struct b32_bundle *)cptr_ent_tbl;
    }

  }

  /* Allocate memory for entries table */
  ixfModule->Entries=malloc(ixfModule->cbEntries*sizeof(IXFMODULEENTRY));

  /* Fill entries table in BFF format */

  entry_table = entry_table_start;
  cptr_ent_tbl = &lx_mod->loader_section[offs_to_entry_tbl];

  ixfModule->cbEntries=1;

  while (1)
  {
    switch(entry_table->b32_type) {
      case EMPTY:;
      case ENTRYFWD:;
      case ENTRY32: break;
      default: io_printf("Invalid entry type! %d, entry_table: %p\n",
               entry_table->b32_type, entry_table);
        return 0; /* Invalid entry.*/
    }


    if (entry_table->b32_cnt == 0) {
        ixfModule->cbEntries--;
        break; /* end of table */
    }

    io_printf("number of entries in bundle %d\n",entry_table->b32_cnt);
    io_printf("type = %d\n",entry_table->b32_type);

    /* Unused Entry, just skip over them.*/
    if (entry_table->b32_type == EMPTY)
    {
      for (i=ixfModule->cbEntries;i<ixfModule->cbEntries+entry_table->b32_cnt;i++)
      {
        ixfModule->Entries[i-1].FunctionName=NULL;
        ixfModule->Entries[i-1].Address=NULL;
        ixfModule->Entries[i-1].Ordinal=NULL;
        ixfModule->Entries[i-1].ModuleName=NULL;
      }

      ixfModule->cbEntries += entry_table->b32_cnt;

      cptr_ent_tbl += UNUSED_ENTRY_SIZE;
      entry_table = (struct b32_bundle *)cptr_ent_tbl;
    } else if (entry_table->b32_type == ENTRY32)
    {
      cptr_ent_tbl = (char*)entry_table;
      cptr_ent_tbl += ENTRY_HEADER_SIZE;
      i_cptr_ent_tbl = (unsigned long)cptr_ent_tbl;

      for (i=ixfModule->cbEntries;i<ixfModule->cbEntries+entry_table->b32_cnt;i++)
      {
        ixfModule->Entries[i-1].FunctionName=NULL;
        ixfModule->Entries[i-1].Address=((struct e32_entry *)(i_cptr_ent_tbl))->e32_variant.e32_offset.offset32+get_obj(lx_mod, entry_table->b32_obj)->o32_base;
        ixfModule->Entries[i-1].Ordinal=NULL;
        ixfModule->Entries[i-1].ModuleName=NULL;
        i_cptr_ent_tbl += _32BIT_ENTRY_SIZE;
      }

      ixfModule->cbEntries += entry_table->b32_cnt;

      cptr_ent_tbl = (char*)i_cptr_ent_tbl;
      entry_table= (struct b32_bundle *)cptr_ent_tbl;
    } else if (entry_table->b32_type == ENTRYFWD)
    {
      cptr_ent_tbl = (char*)entry_table;
      cptr_ent_tbl = &cptr_ent_tbl[ENTRY_HEADER_SIZE];
      i_cptr_ent_tbl = (unsigned long)cptr_ent_tbl;

      for (i=ixfModule->cbEntries;i<ixfModule->cbEntries+entry_table->b32_cnt;i++)
      {
        ixfModule->Entries[i-1].FunctionName=NULL;
        ixfModule->Entries[i-1].Address=NULL;
        ixfModule->Entries[i-1].Ordinal=((struct e32_entry *)(i_cptr_ent_tbl))->e32_variant.e32_fwd.value;
        copy_pas_str(&buf, get_imp_mod_name((struct LX_module *)(ixfModule->FormatStruct), ((struct e32_entry *)(i_cptr_ent_tbl))->e32_variant.e32_fwd.modord));
        ixfModule->Entries[i-1].ModuleName=malloc(strlen(buf)+1);
        strcpy(ixfModule->Entries[i-1].ModuleName,buf);
        i_cptr_ent_tbl += FORWARD_ENTRY_SIZE;
      }

      ixfModule->cbEntries += entry_table->b32_cnt;

      cptr_ent_tbl = (char*)i_cptr_ent_tbl;
      entry_table= (struct b32_bundle *)cptr_ent_tbl;
    }

  }

  return 0;
}


unsigned long calc_imp_fixup_obj_lx(struct LX_module * lx_exe_mod,
                                struct o32_obj * lx_obj, int *ret_rc)
{
  unsigned long fixups;

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

  fixups=0;

  /* Goes through every page of the object.
     The fixups are variable size and a bit messy to traverse.*/

  for(page_nr=startpage; page_nr < lastpage; page_nr++)
  {
    /* Go and get byte position for fixup from the page logisk_sida.
       Start offset for fixup in the page*/
    pg_offs_fix = get_fixup_pg_tbl_offs(lx_exe_mod, page_nr);

    /* Offset for next page.*/
    pg_end_offs_fix = get_fixup_pg_tbl_offs(lx_exe_mod, page_nr+1);

    /* Fetches a relocations structure from offset pg_offs_fix.*/
    min_rlc = get_fixup_rec_tbl_obj(lx_exe_mod, pg_offs_fix);

    fixup_offset = pg_offs_fix;

    /* Get the memory address for the page. The page number is
       from the beginning of all pages a need to be adapted to the beginning of this
       object. */
    vm_start_of_page = lx_obj->o32_base +
                                    get_e32_pagesize(lx_exe_mod) * (page_nr-lx_obj->o32_pagemap);

    /*
    This loop traverses the fixups and increases
    the pointer min_rlc with the size of previoues fixup.
    while(min_rlc is within the offset of current page) {
    */
    while(fixup_offset < pg_end_offs_fix)
    {
      min_rlc = get_fixup_rec_tbl_obj(lx_exe_mod, fixup_offset);
      print_struct_r32_rlc_info(min_rlc);

      fixup_source = min_rlc->nr_stype & 0xf;
      fixup_source_flag = min_rlc->nr_stype & 0xf0;

      switch(min_rlc->nr_flags & NRRTYP)
      {
        case NRRINT :
          fixup_offset += get_reloc_size_rlc(min_rlc);
          break;
        case NRRORD:
          {/* Import by ordinal */
          fixups++;
          fixup_offset += get_reloc_size_rlc(min_rlc);
          break;
          }
        case NRRNAM:
          {/* Import by name */
          fixups++;
          fixup_offset += get_reloc_size_rlc(min_rlc);
          break;
          }

        default: io_printf("Unsupported Fixup! SRC: 0x%x \n", fixup_source);
                 return 0; /* Is there any OS/2 error number for this? */
      } /* switch(fixup_source) */
    } /* while(fixup_offset < pg_end_offs_fix) { */
  }
  return fixups;
}


unsigned long convert_fixup_table_to_BFF(IXFModule * ixfModule)
{
  unsigned long int i;
  unsigned long ret_rc;

  ixfModule->cbFixups=0;

  /* If there is a code object (with a main function) then do a fixup on it and
     it's data/stack object if it exists.*/
  for(i=1; i<=get_obj_num((struct LX_module *)(ixfModule->FormatStruct)); i++)
  {
    struct o32_obj * obj = get_obj((struct LX_module *)(ixfModule->FormatStruct), i);
    if(obj != 0)
      ixfModule->cbFixups += calc_imp_fixup_obj_lx((struct LX_module *)(ixfModule->FormatStruct), obj, &ret_rc);
  }

  if (ixfModule->cbFixups==0)
  {
    ixfModule->Fixups=NULL;
    return 0;
  }
  ixfModule->Fixups=malloc(ixfModule->cbFixups*sizeof(IXFFIXUPENTRY));

  /* Fill table... */
  for(i=1; i<=get_obj_num((struct LX_module *)(ixfModule->FormatStruct)); i++)
  {
    struct o32_obj * obj = get_obj((struct LX_module *)(ixfModule->FormatStruct), i);
    if(obj != 0)
      /*ixfModule->cbFixups +=*/ convert_imp_fixup_obj_lx(ixfModule, obj, &ret_rc);
  }


 return 0; /* NO_ERROR */
}

/* Applies fixups for an object. Returns true(1) or false(0) to show status.*/
int convert_imp_fixup_obj_lx(IXFModule * ixfModule,
                                struct o32_obj * lx_obj, int *ret_rc)
{
  struct LX_module * lx_exe_mod;

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
  unsigned long fixup_counter;

  fixup_counter=0;
  lx_exe_mod=(struct LX_module *)(ixfModule->FormatStruct);

  //io_printf("--------------------Listing fixup data ------------------------- %p\n", lx_obj);

  /* Goes through every page of the object.
     The fixups are variable size and a bit messy to traverse.*/

  for(page_nr=startpage; page_nr < lastpage; page_nr++)
  {
    if (options.debugixfmgr) io_printf("-----  Object %d of %d\n",startpage, lastpage);

    /* Go and get byte position for fixup from the page logisk_sida.
       Start offset for fixup in the page*/
    pg_offs_fix = get_fixup_pg_tbl_offs(lx_exe_mod, page_nr);

    /* Offset for next page.*/
    pg_end_offs_fix = get_fixup_pg_tbl_offs(lx_exe_mod, page_nr+1);

    /* Fetches a relocations structure from offset pg_offs_fix.*/
    min_rlc = get_fixup_rec_tbl_obj(lx_exe_mod, pg_offs_fix);

    fixup_offset = pg_offs_fix;

    /* Get the memory address for the page. The page number is
       from the beginning of all pages a need to be adapted to the beginning of this
       object. */
    vm_start_of_page = lx_obj->o32_base +
                                    get_e32_pagesize(lx_exe_mod) * (page_nr-lx_obj->o32_pagemap);

    /*
    This loop traverses the fixups and increases
    the pointer min_rlc with the size of previoues fixup.
    while(min_rlc is within the offset of current page) {
    */
    while(fixup_offset < pg_end_offs_fix)
    {
      min_rlc = get_fixup_rec_tbl_obj(lx_exe_mod, fixup_offset);
      print_struct_r32_rlc_info(min_rlc);

      fixup_source = min_rlc->nr_stype & 0xf;
      fixup_source_flag = min_rlc->nr_stype & 0xf0;

      switch(min_rlc->nr_flags & NRRTYP)
      {
        case NRRINT : /* Skip internal */
          fixup_offset += get_reloc_size_rlc(min_rlc);
          break;
        case NRRORD:
          {/* Import by ordinal */
            /* Indata: lx_exe_mod, min_rlc */
            mod_nr = get_mod_ord1_rlc(min_rlc); // Request module number
            import_ord = get_imp_ord1_rlc(min_rlc); // Request ordinal number
            srcoff_cnt1 = get_srcoff_cnt1_rlc(min_rlc);
            addit = get_additive_rlc(min_rlc);

            mod_name = (char*)&buf_mod_name;

            /* Get name of imported module. */
            org_mod_name = get_imp_mod_name(lx_exe_mod,mod_nr);
            copy_pas_str(mod_name, org_mod_name);


            ixfModule->Fixups[fixup_counter].SrcAddress=lx_obj->o32_base + srcoff_cnt1;
            ixfModule->Fixups[fixup_counter].ImportEntry.FunctionName=NULL;
            ixfModule->Fixups[fixup_counter].ImportEntry.ModuleName=malloc(strlen(mod_name)+1);
            strcpy(ixfModule->Fixups[fixup_counter].ImportEntry.ModuleName, mod_name);
            ixfModule->Fixups[fixup_counter].ImportEntry.Ordinal=import_ord;
            fixup_counter++;


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

            ixfModule->Fixups[fixup_counter].SrcAddress=lx_obj->o32_base + srcoff_cnt1;
            ixfModule->Fixups[fixup_counter].ImportEntry.FunctionName=malloc(strlen(import_name)+1);
            strcpy(ixfModule->Fixups[fixup_counter].ImportEntry.FunctionName, import_name);
            ixfModule->Fixups[fixup_counter].ImportEntry.ModuleName=malloc(strlen(mod_name)+1);
            strcpy(ixfModule->Fixups[fixup_counter].ImportEntry.ModuleName, mod_name);
            ixfModule->Fixups[fixup_counter].ImportEntry.Ordinal=0;
            fixup_counter++;

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
