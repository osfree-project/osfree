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

#define INCL_ERRORS
#define INCL_DOS
#define INCL_BSEDOS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#include <os2.h>

#include <modlx.h>
#include <processmgr.h>
#include <modmgr.h>
#include <fixuplx.h>
#include <io.h>

/* Small descriptions of possible fixups. (1=variable size, 2=optional fields)

        Internal Fixup: (Flags=0)
                SRC  FLAGS  SRCOFF/CNT1  OBJECT1  TRGOFF1,2  SRCOFF1,2 ... SRCOFFn2


        Import by Ordinal Fixup Record: (Flags=1)
                SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1  IMPORT_ORD1  ADDITIVE1,2  SRCOFF1,2 ... SRCOFFn2


        Import by Name Fixup Record:    (Flags=2)
                SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1  PROCEDURE_NAME_OFFSET1  ADDITIVE1,2  SRCOFF1,2 ... SRCOFFn2


        Internal Entry Table Fixup Record: (Flags=3)
                SRC  FLAGS  SRCOFF/CNT1  ORD(ENTRY)#1  ADDITIVE1,2  SRCOFF1,2 ... SRCOFFn2


        Internal Chaining Fixups (Flags=8)
                32 bit offset fixups (SRC=0x7) to internal references (target types 0x00 and 0x03)
                First record is a record in the Fixup Record Table and the remaining  fixups
                are located in the page referenced by the first fixup rather than as records in
                the Fixup Record Table.  ...

*/
/* Prints info about a fixup. */
void print_struct_r32_rlc_info(struct r32_rlc * rlc)
{
  int source_type_mask;
  int fixup_size;


//        io_printf("\n---------List of fixup data ------------- '%p'", rlc);
//        io_printf("\n");
        io_printf("   %02X    %02X   src off = %04X   ", rlc->nr_stype, rlc->nr_flags, get_srcoff_cnt1_rlc(rlc));

        if((rlc->nr_flags & NRRTYP) == NRRINT)
                io_printf("object #    = %02d   ",   get_mod_ord1_rlc(rlc), get_mod_ord1_rlc(rlc));
        else
                io_printf("mod ord #   = %02d   ",   get_mod_ord1_rlc(rlc), get_mod_ord1_rlc(rlc));

        if((rlc->nr_flags & NRRORD) == NRRORD)
                io_printf("import ord #     = %02d\n", get_imp_ord1_rlc(rlc));
        if((rlc->nr_flags & NRRNAM) == NRRNAM)
                io_printf("import name      = %s\n", get_imp_name_rlc(rlc));

        if(((rlc->nr_flags & NRRTYP) == NRRINT) && (source_type_mask != NRSSEG))
                io_printf("target off       = %04d\n", get_imp_ord1_rlc(rlc));
//                io_printf("  trgoff1,2: %4.d (0x%.4x)  \n", get_imp_ord1_rlc(rlc), get_imp_ord1_rlc(rlc));

        source_type_mask = rlc->nr_stype & NRSTYP;
        /*if(source_type_mask == NRSTYP)
                io_printf("    0x0F: Source type mask.\n"); */
#if 0
        if(source_type_mask == NRSBYT)
                io_printf("    0x00: Byte fixup (8-bits).\n");
        if(source_type_mask == NRSSEG)
                io_printf("    0x02: 16-bit segment (16-bits).\n");
        if(source_type_mask == NRSPTR)
                io_printf("    0x03: 16:16 pointer (32-bits).\n");
        if(source_type_mask == NRSOFF)
                io_printf("    0x05: 16-bit offset (16-bits).\n");
        if(source_type_mask == NRPTR48)
                io_printf("    0x06: 16:32 pointer (48-bits).\n");
        if(source_type_mask == NROFF32)
                io_printf("    0x07: 32-bit offset (32-bits).\n");
        if(source_type_mask == NRSOFF32)
                io_printf("    0x08: 32-bit Self-relative offset fixup(32-bits).\n");
        if((rlc->nr_stype & NRALIAS) == NRALIAS)
                io_printf("    0x10: Fixup to Alias Flag.\n");
        if((rlc->nr_stype & NRCHAIN) == NRCHAIN)
                io_printf("    0x20: Source List Flag.\n");


        io_printf("Flags: 0x%x \n", rlc->nr_flags);
        if((rlc->nr_flags & NR8BITORD) == NR8BITORD)  // 0x80
                io_printf("    0x80: 8-bit Import Ordinal Flag. \n");

        if(((rlc->nr_flags & NR32BITOFF) != NR32BITOFF)  // 0x10
                && ((rlc->nr_flags & NR8BITORD) != NR8BITORD))
                io_printf("   !0x10: 16-bit Import Ordinal Flag. \n");

        if((rlc->nr_flags & NRRORD) == NRRORD)
                io_printf("    0x01: Import by Ordinal Flag. \n");

        if((rlc->nr_flags /*& NRRINT*/) == NRRINT)
                io_printf("    0x00: Internal reference. \n");

        if((rlc->nr_flags & NRRNAM) == NRRNAM)
                io_printf("    0x02: Import by name. \n");
        if((rlc->nr_flags & NRADD) == NRADD)
                io_printf("    0x04: Additive fixup. \n");
        if((rlc->nr_flags & NRICHAIN) == NRICHAIN)
                io_printf("    0x08: Internal Chaining Fixup. \n");
        if((rlc->nr_flags & NRRENT) == NRRENT)
                io_printf("    0x03: Internal entry table fixup. \n");
        if((rlc->nr_flags & NR32BITOFF) == NR32BITOFF)
                io_printf("    0x10: 32-bit Target Offset. \n");
        if((rlc->nr_flags & NR32BITADD) == NR32BITADD)
                io_printf("    0x20: 32-bit Additive fixup. \n");
        if((rlc->nr_flags & NR16OBJMOD) == NR16OBJMOD)
                io_printf("    0x40: 16-bit Object/Module ordinal. \n");
        else
                io_printf("   !0x40:  8-bit Object/Module ordinal. \n");
#endif

        if((rlc->nr_stype & NRCHAIN) == NRCHAIN) { /* Print source list offsets. */
                int count1 = get_srcoff_cnt1_rlc(rlc);
                int i=0;
                io_printf(" Source list offsets: ");
                for(i=0; i<count1; i++) {
                        int src_off = get_srcoff12_rlc(rlc, i);
                        io_printf(" %d (0x%x)", src_off, src_off);
                }
                io_printf("\n");
        }

        if((rlc->nr_flags & NRADD) == NRADD)
                io_printf(" Additive fixup: %d\n", get_additive_rlc(rlc));
        /*io_printf("  r32_soff=0x%x, r32_objmod=0x%x, r32_target.intref.offset32=0x%x, r32_target.intref.offset16=0x%x \n ",
                rlc->r32_soff, rlc->r32_objmod,
                rlc->r32_target.intref.offset32,
                rlc->r32_target.intref.offset16 ); */
#if 0
        if((rlc->nr_stype & NRCHAIN) == NRCHAIN)  /* Internal Chaining Fixup */
                io_printf("\n get_  cnt1  _size: %d\n", get_srcoff_cnt1_size(rlc));
        else
                io_printf("\n get_ srcoff _size: %d\n", get_srcoff_cnt1_size(rlc));

        if((rlc->nr_flags & NRRTYP) == NRRINT)   /* Internal reference */
                io_printf(" get_ object _size: %d\n", get_mod_ord1_size(rlc));
        else
                io_printf(" get_mod_ord1_size: %d\n", get_mod_ord1_size(rlc));

        if((rlc->nr_flags & NRRTYP) == NRRINT)  /* Internal reference */
                io_printf(" get_trgoff_size:   %d\n", get_trgoff_size(rlc));
        if((rlc->nr_flags & NRRORD) == NRRORD)
                io_printf(" get_imp_ord1_size: %d\n", get_imp_ord1_size(rlc));

        if((rlc->nr_flags & NRRTYP) != NRRINT)  /* NOT Internal reference */
                io_printf(" get_additive_size: %d\n", get_additive_size(rlc));
#endif
        fixup_size = get_reloc_size_rlc(rlc);

//        io_printf(" Total fixup size: %d (0x%x)\n", fixup_size, fixup_size);
}

int get_SRC_FLAGS_size() {
        return 2; /* SRC and FLAGS are each one byte. */
}

        /* Gets the size of field srcoff/cnt1 (source offset or count), variable size. */
        /* SRC  FLAGS  SRCOFF/CNT1 ... */
int get_srcoff_cnt1_size(struct r32_rlc * rlc) {
        if((rlc->nr_stype & NRCHAIN)!= NRCHAIN) //0x20. Om den inte är satt så är det en DW annars DB.
                return 2; /* 2 bytes size*/
        return 1;     /* 1 byte size*/
}

        /* Gets the size of field mod_ord1 (module ordinal), variable size. */
        /* SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1 ... */
int get_mod_ord1_size(struct r32_rlc * rlc) {
        if((rlc->nr_flags &NR16OBJMOD) == NR16OBJMOD) //0x40. Om den är satt så är det en DW annars DB.
                return 2; /* 2 bytes size*/
        return 1;     /* 1 byte size*/
}

        /* Gets the size of field imp_ord1 (import ordinal), variable size. */
        /* SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1  IMPORT_ORD1 ... */
int get_imp_ord1_size(struct r32_rlc * rlc) {
        if((rlc->nr_flags & NR8BITORD) == NR8BITORD) /* en byte */
                return 1;
        if((rlc->nr_flags & NR32BITOFF) != NR32BITOFF) /* en word */
                return 2;
        if((rlc->nr_flags & NR32BITOFF) == NR32BITOFF) /* en dubbelword */
                return 4;
        return 0;
}

        /* Gets the size of field additive, variable size. */
        /* Exists in all fixups except Internal Fixups.
        SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1  IMPORT_ORD1  ADDITIVE1,2 ...
    SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1  PROCEDURE_NAME_OFFSET1  ADDITIVE1,2 ...
    SRC  FLAGS  SRCOFF/CNT1  ORD(ENTRY)#1  ADDITIVE1,2 ...  */
int get_additive_size(struct r32_rlc * rlc) {
        if((rlc->nr_flags & NRADD) == NRADD) {
                if((rlc->nr_flags & NRCHAIN) == NRCHAIN)  /* 32 bitars additive fixup. */
                        return 4;
                else
                        return 2;   /* 16 bit */
        }
        return 0;     /* Not avalable. */
}

        /* Gets the size of field trgoff (target offset), variable size. */
        /* SRC  FLAGS  SRCOFF/CNT1  OBJECT1  TRGOFF1,2 ... */
int get_trgoff_size(struct r32_rlc * rlc) {
        if((rlc->nr_flags & NRSSEG) != NRSSEG) {
                if((rlc->nr_flags & NR32BITOFF) == NR32BITOFF)  /* 32 bitars target fixup. */
                        return 4;
                else
                        return 2;   /* 16 bit */
        }
        return 0;     /* Not avalable. */
}


/* Gets the size of field imp_name (import name(Procedure name offset)). */
/* SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1  PROCEDURE_NAME_OFFSET1 ... */
unsigned int get_imp_name_size(struct r32_rlc * rlc) {

        if((rlc->nr_flags & NR32BITOFF) == NR32BITOFF) /* 32 bit offset to import procedure table.*/
                return 4;
        else                                           /* 16 bit offset to import procedure table.*/
                return 2;
        return 0;
}
        /* Gets the size of field ord1_entry (target offset), variable size. */
        /* SRC  FLAGS  SRCOFF/CNT1  ORD(ENTRY)#1  ADDITIVE1,2 ... */
int get_ord1_entry_size(struct r32_rlc * rlc) {
        if((rlc->nr_flags & NRSSEG) != NRSSEG) {
                if((rlc->nr_flags & NR32BITOFF) == NR32BITOFF)  /* 16 bitars entry fixup. */
                        return 2;
                else
                        return 1;   /* 16 bit */
        }
        return 0;     /* Not avalable. */
}
/*
                              Fixup Record Table
==============================================================================
Source  Target
  type  flags
  ====  ====
   08    01   src off = 0008   mod ord #   = 01   import ord #     = 0157
   08    01   src off = 0012   mod ord #   = 01   import ord #     = 0109
*/
/*

// ***ET+ r32_rlc - Relocation item

struct r32_rlc                           // Relocation item
{
    unsigned char       nr_stype;        // Source type - field shared with new_rlc
    unsigned char       nr_flags;        // Flag byte - field shared with new_rlc
    short               r32_soff;        // Source offset
    unsigned short      r32_objmod;      // Target object number or Module ordinal

    union targetid
    {
        offset             intref;       // Internal fixup
                // r32_target.intref.offset16

        union extfixup
        {
            offset         proc;         // Procedure name offset
            unsigned long  ord;          // Procedure odrinal
                        // r32_target.extref.ord
        }
                           extref;       // External fixup

        struct addfixup
        {
            unsigned short entry;       // Entry ordinal
            offset         addval;      // Value added to the address
                        // r32_target->addfix->addval
        }
                           addfix;       // Additive fixup
    }
                        r32_target;      // Target data
    unsigned short      r32_srccount;    // Number of chained fixup records
    unsigned short      r32_chain;       // Chain head
};
*/


        /* Retrieves the data in field srcoff/cnt (source offset or count). */
        /* SRC  FLAGS  SRCOFF/CNT1 ... */
  /* Om SOURCE är 0x20 så är srcoff_cnt1 en byte, annars en word. */
int get_srcoff_cnt1_rlc(struct r32_rlc * rlc)
{
  unsigned char * ptr_uchar;

        if((rlc->nr_stype & NRCHAIN)!= NRCHAIN) //0x20. Om den är satt så är det en DW annars DB.
                return rlc->r32_soff;
        ptr_uchar = (unsigned char *)&rlc->r32_soff;
        return *ptr_uchar /*>> 8*/;
}


/* Retrieves the data in field mod_ord1 (module ordinal). */
/* SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1 ... */
  /* NR8BITORD  = 0x80
     NRCHAIN    = 0x20
         NR16OBJMOD = 0x40
     NR32BITOFF = 0x10    */
        /* int mod_ord1_offs = get_SRC_FLAGS_size()get_srcoff_cnt1_size()
           get_mod_ord1_size()
        */
int get_mod_ord1_rlc(struct r32_rlc * rlc) {

        int mod_ord1_offs = get_SRC_FLAGS_size()+ get_srcoff_cnt1_size(rlc);
        int mod_ord1_val=0;
        int mod_ord1_size = get_mod_ord1_size(rlc);
        unsigned long int uint_rlc = (unsigned long int)rlc;
        void *void_mod_ord1 = (void*)(uint_rlc + mod_ord1_offs);

        switch(mod_ord1_size) {
                case 1: mod_ord1_val = (int)*((unsigned char*)void_mod_ord1);break;
                case 2: mod_ord1_val = *((unsigned short int*)void_mod_ord1);
        }
        return mod_ord1_val;
}

/*struct r32_rlc_8bit_ord                      // Relocation item
{
    unsigned char       nr_stype;        // Source type - field shared with new_rlc
    unsigned char       nr_flags;        // Flag byte - field shared with new_rlc
    short               r32_soff;        // Source offset
    unsigned short      r32_objmod;      // Target object number or Module ordinal
        unsigned char       r32_impord
};*/


/* Retrieves the data in field imp_ord1 (import ordinal).
    */
/* SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1  IMPORT_ORD1 ... */
int get_imp_ord1_rlc(struct r32_rlc * rlc) {

        int imp_ord1_offs = get_SRC_FLAGS_size()+get_srcoff_cnt1_size(rlc)+get_mod_ord1_size(rlc);
        int imp_ord1_val=0;
        int imp_ord1_size = get_imp_ord1_size(rlc);
        unsigned long int uint_rlc = (unsigned long int)rlc;
        void *void_imp_ord1 = (void*)(uint_rlc + imp_ord1_offs);

        switch(imp_ord1_size) {
                case 1: imp_ord1_val = (int)*((unsigned char*)void_imp_ord1);break;
                case 2: imp_ord1_val = *((unsigned short int*)void_imp_ord1);break;
                case 4: imp_ord1_val = *((unsigned long int*)void_imp_ord1);
        }
        return imp_ord1_val;

}

/* Retrieves the data in field additive (additive fixup value). Variable size and might nor exist.*/
/* Exists in all fixups except Internal Fixups.
   SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1  IMPORT_ORD1  ADDITIVE1,2 ...            (Flags=1)(Import by Ordinal)
   SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1  PROCEDURE_NAME_OFFSET1  ADDITIVE1,2 ... (Flags=2)(Import by Name)
   SRC  FLAGS  SRCOFF/CNT1  ORD(ENTRY)#1  ADDITIVE1,2 ...                      (Flags=3)(Internal Entry) */
int get_additive_rlc(struct r32_rlc * rlc)
{
        int additive_val;
        int additive_size;
        unsigned long int uint_rlc;
        void *void_additive;

        int additive_offs = 0;
        if((rlc->nr_flags &NRRORD)==NRRORD) {
          /*1*/ additive_offs = get_SRC_FLAGS_size()  + get_srcoff_cnt1_size(rlc)+
                                  get_mod_ord1_size(rlc)+ get_imp_ord1_size(rlc);
        }
        if((rlc->nr_flags &NRRNAM)==NRRNAM) {
          /*2*/ additive_offs = get_SRC_FLAGS_size()  + get_srcoff_cnt1_size(rlc)+
                                  get_mod_ord1_size(rlc)+ get_imp_name_size(rlc);
        }
        if((rlc->nr_flags &NRRENT)==NRRENT) {
          /*3*/ additive_offs = get_SRC_FLAGS_size()  + get_srcoff_cnt1_size(rlc)+
                                  get_ord1_entry_size(rlc);
        }
        io_printf(" additive_offs: %d (0x%x)\n", additive_offs, additive_offs);
        additive_val=0;
        additive_size = get_additive_size(rlc);
        uint_rlc = (unsigned long int)rlc;
        void_additive = (void*)(uint_rlc + additive_offs);

        switch(additive_size) {
                /*case 1: additive_val = (int)*((unsigned char*)void_additive);break;*/
                case 2: additive_val = *((unsigned short int*)void_additive);break;
                case 4: additive_val = *((unsigned long int*)void_additive);
        }
        return additive_val;
}

        /* Gets the source offset with index(zero based), from the fixup. */
int get_srcoff12_rlc(struct r32_rlc * rlc, int idx)
{
    int srcoff12_val;
    int srcoff12_size;
    unsigned long int uint_rlc;
    void *void_srcoff12;

        int srcoff12_offs = 0;
        if((rlc->nr_flags &NRRORD)==NRRORD) {
          /*1*/ srcoff12_offs = get_SRC_FLAGS_size()  + get_srcoff_cnt1_size(rlc)+
                                  get_mod_ord1_size(rlc)+ get_imp_ord1_size(rlc);
        }
        if((rlc->nr_flags &NRRNAM)==NRRNAM) {
          /*2*/ srcoff12_offs = get_SRC_FLAGS_size()  + get_srcoff_cnt1_size(rlc)+
                                  get_mod_ord1_size(rlc)+ get_imp_name_size(rlc);
        }
        if((rlc->nr_flags &NRRENT)==NRRENT) {
          /*3*/ srcoff12_offs = get_SRC_FLAGS_size()  + get_srcoff_cnt1_size(rlc)+
                                  get_ord1_entry_size(rlc);
        }
        /* io_printf(" srcoff12_offs: %d (0x%x)\n", srcoff12_offs+ idx * 2, srcoff12_offs+ idx * 2); */
        srcoff12_val=0;
        srcoff12_size = get_additive_size(rlc);
        uint_rlc = (unsigned long int)rlc;
        void_srcoff12 = (void*)(uint_rlc + srcoff12_offs + idx * 2);


        srcoff12_val = *((unsigned short int*)void_srcoff12);

        return srcoff12_val;

}


/* Retrieves the data in field imp_name (import name(Procedure name offset)). */
/* SRC  FLAGS  SRCOFF/CNT1  MOD_ORD#1  PROCEDURE_NAME_OFFSET1 ... */
unsigned int get_imp_name_rlc(struct r32_rlc * rlc) {
        struct r32_rlc_8bit_ord *ptr_r32_rlc_8bit_ord = (struct r32_rlc_8bit_ord *) rlc;

        int imp_name_offs = get_SRC_FLAGS_size()  + get_srcoff_cnt1_size(rlc)+
                                  get_mod_ord1_size(rlc);

        unsigned long int uint_rlc = (unsigned long int)rlc;
        void *void_imp_name = (void*)(uint_rlc + imp_name_offs);
        int imp_name_val=0;

        int imp_name_size = get_imp_ord1_size(rlc);
        switch(imp_name_size) {
                /*case 1: additive_val = (int)*((unsigned char*)void_additive);break;*/
                case 2: imp_name_val = *((unsigned short int*)void_imp_name);break;
                case 4: imp_name_val = *((unsigned long int*)void_imp_name);
        }

        /*if((rlc->nr_flags & NR32BITOFF) == NR32BITOFF) // 32 bit offset till import procedure table.
                return *((unsigned int *) &ptr_r32_rlc_8bit_ord->_8_16_impord.r32_impord16);
        else                                           // 16 bit offset till import procedure table.
                return ptr_r32_rlc_8bit_ord->_8_16_impord.r32_impord16;
        */
        return imp_name_val;
}


/* Gets the total size of an fixup record. */
int get_reloc_size_rlc(struct r32_rlc * rlc) {

        /*int source_type_mask = rlc->nr_stype & NRSTYP;*/

        int reloc_size = 0;

        if((rlc->nr_stype & NRCHAIN) == NRCHAIN) { /* Calculate size of source list offsets. */
                int count1 = get_srcoff_cnt1_rlc(rlc);
                reloc_size = count1 * 2;
        }

        if((rlc->nr_flags & NRRTYP) == NRRINT) { /*1*/
          reloc_size += get_SRC_FLAGS_size()  + get_srcoff_cnt1_size(rlc)+
          get_mod_ord1_size(rlc)+ get_imp_ord1_size(rlc);
          return reloc_size;
        }

        if((rlc->nr_flags & NRRORD) == NRRORD) { /*1*/
          reloc_size += get_SRC_FLAGS_size()  + get_srcoff_cnt1_size(rlc)+
          get_mod_ord1_size(rlc)+ get_imp_ord1_size(rlc)+get_additive_size(rlc);
          return reloc_size;
        }
        if((rlc->nr_flags & NRRNAM) == NRRNAM) { /*2*/
          reloc_size += get_SRC_FLAGS_size()  + get_srcoff_cnt1_size(rlc)+
          get_mod_ord1_size(rlc)+ get_imp_name_size(rlc)+get_additive_size(rlc);
          return reloc_size;
        }
        if((rlc->nr_flags & NRRENT) == NRRENT) { /*3*/
          reloc_size += get_SRC_FLAGS_size()  + get_srcoff_cnt1_size(rlc)+
          get_ord1_entry_size(rlc)+get_additive_size(rlc);
          return reloc_size;
        }
        io_printf(" Warning! Unrecognized fixup in fixuplx.c:get_reloc_size_rlc() \n");
        io_printf(" Risk for infinite loop! \n Type of fixup:\n");
        io_printf("SRC: 0x%x, FLAGS: 0x%x \n", rlc->nr_stype, rlc->nr_flags);
        /*print_struct_r32_rlc_info(rlc);*/


        return reloc_size;
}


        /* Är tänkt att simulera ett sidfel på position offs i kod eller dataobjektet.
           Och det innebär att den tar emot en position som ett sidfel ägt rum på.
           Så koden måste först ta reda på i vilken sida positionen finns i och
           sen utföra fixup på den?
             Sen hämtas den aktiva sidans "Fixup Record Table" instans från "Fixup Page Table".
        */

/*
void fake_pagefault_code(struct LX_module * lx_mod, unsigned int offs, struct t_os2process * proc) {

        //struct o32_obj * kod_obj = get_obj(lx_mod->lx_head_e32_exe->e32_startobj);
        //Sök först i vilket objekt som offs finns i.
        //Loopa igenom alla objekt och leta i deras ägda sidor om offs rymms inuti dom.
        //offs? vilken address kommer den att ha? Räknat från den virtuella basaddressen?

        struct o32_map * kod_sida =0;
        struct o32_obj * kod_obj=0;
        int hittat_sida = 0;
        unsigned int i,j,logisk_sida=0;
        for(i=1; i<=lx_mod->lx_head_e32_exe->e32_objcnt; i++) {
                kod_obj = get_obj(lx_mod, i);
                //kod_obj->o32_pagemap; //Startindex för sidor.
                //kod_obj->o32_mapsize; //Antal sidor.


                for(j = 1; j <= kod_obj->o32_mapsize; j++) {
                        kod_sida = get_obj_map(lx_mod, j);      // HÄmtar sidan j.

                        // Finns offs i den här sidan?

                        io_printf("---offset(in code) in %lu > offs:%d < %lu ---\n",
                                kod_sida->o32_pagedataoffset, offs,
                                (kod_sida->o32_pagedataoffset + kod_sida->o32_pagesize));

                        if((offs > kod_sida->o32_pagedataoffset)
                                && (offs < (kod_sida->o32_pagedataoffset + kod_sida->o32_pagesize))) {
                                hittat_sida = 1; logisk_sida = j + (kod_obj->o32_pagemap-1);
                                io_printf("Hittar offs i sida %d i objekt %d logisk_sida %d\n", j, i, logisk_sida);
                                break;
                        }
                }
                if(hittat_sida) break;
        }

        if(!hittat_sida)
                io_printf("Can't find offset for page in code/data object! offs:%d\n", offs);

        //Läs in sidan från exe/dll filen.
        // Gör fixup på logisk_sida.

        // Hämtar byteposition för fixup från sidan logisk_sida.
        int pg_offs_fix = get_fixup_pg_tbl_offs(lx_mod, logisk_sida);

        //struct r32_rlc * get_fixup_rec_tbl_obj(struct LX_module * lx_mod, int offs)

        // Hämtar relokeringsstruktur från offset pg_offs_fix.
        struct r32_rlc * min_rlc = get_fixup_rec_tbl_obj(lx_mod, pg_offs_fix);
        print_struct_r32_rlc_info(min_rlc);
        char * imp_mod_name= get_imp_mod_name(lx_mod, get_mod_ord1_rlc(min_rlc));


        // Skriver ut namnet på dll-modulen och kopierar den till b_ptr.
        char buf[30];
        char * b_ptr = (char*)&buf;
        int s=0;
        for(s=1; s<=imp_mod_name[0]; s++){
                b_ptr[s-1] = imp_mod_name[s];
                //io_printf("%c", imp_mod_name[s]);
        }
        b_ptr[imp_mod_name[0]] = 0;

        load_dyn_link(imp_mod_name); // En stub fÃ¶r framtida laddning av dll-filer.

        // HÃ¤mtar en pekare till DosPutMessage.
        void * dos_put_ptr = (void *) get_func_ptr_ord_modname(get_imp_ord1_rlc(min_rlc),b_ptr);

        if(min_rlc->nr_stype == NRSOFF32) { // 32-bit self-relative offset (32-bits)
                unsigned int src_off         = get_srcoff_cnt1_rlc(min_rlc);
                unsigned int int_dos_put_ptr = (unsigned int)dos_put_ptr;
                // void *       ptr_code_mmap   = (void*) proc->code_mmap;
                unsigned int int_code_mmap   = (unsigned int)((void*)proc->code_mmap);
                int_dos_put_ptr  -= (int_code_mmap+src_off+4);
                //void *       rel_dos_put_ptr = (void*) dos_put_ptr - (ptr_code_mmap+src_off+4);
                void *       rel_dos_put_ptr = (void*) int_dos_put_ptr;

                io_printf(" DosPut... ptr %p, src_off %d, dos_put_rel_ptr %p \n",
                                                                dos_put_ptr, src_off, rel_dos_put_ptr);

                io_printf("Adress att placera i kod: %lu (%p) \n", (unsigned long int)rel_dos_put_ptr, rel_dos_put_ptr);

                unsigned long int tmp_patch_offs = int_code_mmap + src_off;
                void * patch_offs = (void *) tmp_patch_offs;
                io_printf(" (void *)          patch_offs: %p \n", patch_offs);
                io_printf(" (unsigned int *) *patch_offs: 0x%x \n", *((unsigned int*)patch_offs));
                io_printf(" (char *)         *patch_offs-1: 0x%x \n", *((char*)patch_offs-1));
                // *((unsigned int *)patch_offs) = (unsigned int *)rel_dos_put_ptr;
                unsigned long int * iptr_patch_offs = (unsigned long int *)patch_offs;
                unsigned long int * iptr_rel_dos_put_ptr = (unsigned long int *)rel_dos_put_ptr;
                *iptr_patch_offs = (unsigned long int) iptr_rel_dos_put_ptr;
                io_printf("Efter fixup: (unsigned int *) *patch_offs: 0x%x \n", *((unsigned int*)patch_offs));
        } else {
                io_printf("Unknown fixup! source type:%d flags:%d \n", min_rlc->nr_stype, min_rlc->nr_flags );
        }
        io_printf("\n%s på %d\n", __func__, offs);
}
*/
