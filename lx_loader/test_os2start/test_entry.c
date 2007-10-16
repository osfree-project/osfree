

#include <stdlib.h>
#include <stdio.h>
#include "../modlx.h" 
#include "../fixuplx.h"
#include "../gcc_os2def.h"

/* #include <exe386.h> */

/*
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
*/

void print_entry_fields(struct e32_entry * entry_post);

void test_entry() {
	printf(" Testing entrys. \n");
	
	/* Different types of entry structures (variable sizes).*/
	char test_entry1[] = {0x03,  0x00}; /* Bundle, Unused entry, 3 elements, type:0 */
	
	/* Bundle, Forward entry, 1 element, type:4 */                                     
	char test_entry2[] = {0x01,  0x04,  0x00,0x00,  
	                                               0x01,  0x00,0x01,  0x00,0x00,0x00,0xff,  
												   0x00};   /* end counter*/
	
	/* Bundle, 32 bit entry, 2 elements, type:3 */
	char test_entry3[] = {0x02,  0x03,  0x00,0x01,  
	                                               0x01,  0x00,0x00,0x00,0xf0, 
												   0x01,  0x00,0x00,0x00,0xe0, 
												   0x00};
												   
		/* A combination of above. */	
		/* entry points 1-3, Unused entry,
		                4-5,   Forward entry
						6-7, 32 bit entry
						8   Unused entry, zero count as end point.
		*/									   
	char test_entry4[] = {0x03,  0x00,       /*1-3*/
						  0x02,  0x04,  0x00,0x0d,  
	                                               0x01,  0x00,0x08,  0x00,0x00,0x00,0xff, /*4*/
												   0x01,  0x00,0x09,  0x00,0x00,0x00,0xf1, /*5*/
						  0x02,  0x03,  0x00,0x01,  
	                                               0x01,  0x00,0x00,0x00,0xf0, /*6*/ 
												   0x01,  0x00,0x00,0x00,0xe0, /*7*/
						  0x00}; /* End marker, CNT=0 */
						  
	/*entry table in msg.dll (tiny version). position 0x15b - 0x168*/
	char test_entry5[] = {0x04,  0x00,       /*1-4*/
						  0x01,  0x04,  0x00,0x00,  
	                                               0x01,  0x01,0x00,  0x83,0x01,0x00,0x00, /*5*/
						  0x00}; /* End marker, CNT=0 */
	/*entry_post->e32_flags, 
	entry_post->e32_variant.e32_fwd.value,
	entry_post->e32_variant.e32_fwd.modord);*/
	
  /*entry_post->e32_flags
	entry_post->offset16
	entry_post->offset32
	entry_post->e32_variant.e32_fwd.value
	entry_post->e32_variant.e32_fwd.modord
	entry_post->offset
	entry_post->callgate*/
					
	/*get_entry(struct LX_module * lx_mod, int ent_ord, 
					int * ret_flags, 
					int * ret_offset,
					int * ret_obj,
					int * ret_modord,
					int * ret_type)  */
	int ent_ord=1;
	int ret_flags; 
	int ret_offset;
	int ret_obj;
	int ret_modord;
	int ret_type = (int) &test_entry4;
	print_entry_fields((struct e32_entry *)&test_entry4[2]);
	
	for(ent_ord=0; ent_ord <= 8; ent_ord++) {
		printf("start ptr: %p, end ptr: %p \n", 
				(void*)&test_entry5, (void*)&test_entry5[sizeof(test_entry5)-1 /*13*/]);
		ret_type = (int) &test_entry5;
	    void* entry_return =
		    get_entry(0, ent_ord,
					&ret_flags, 
					&ret_offset,
					&ret_obj,
					&ret_modord,
					&ret_type);
		printf(" ent_ord: 0x%x, flags: 0x%x, offset: 0x%x, obj: 0x%x, modord: 0x%x, type: 0x%x, \n",
				ent_ord, ret_flags, ret_offset, ret_obj, ret_modord, ret_type);
		printf(" ------------------------------------------------------ \n");
	}
}


int main() {

	test_entry();

	return 0;
}

/***ET+ b32_bundle - Entry Table */
/*
struct b32_bundle
{
    unsigned char       b32_cnt;        / Number of entries in this bundle 
    unsigned char       b32_type;       / Bundle type 
    unsigned short      b32_obj;        / Object number 
};                                      / Follows entry types 

struct e32_entry
{
    unsigned char       e32_flags;      / Entry point flags 
    union entrykind
    {
        offset          e32_offset;     / 16-bit/32-bit offset entry 
        struct callgate
        {
            unsigned short offset;      / Offset in segment 
            unsigned short callgate;    / Callgate selector 
        }
                        e32_callgate;   / 286 (16-bit) call gate 
        struct fwd
        {
            unsigned short  modord;     / Module ordinal number 
            unsigned long   value;      / Proc name offset or ordinal 
        }
                        e32_fwd;        / Forwarder 
    }
                        e32_variant;    / Entry variant 
}; */

void print_entry_fields(struct e32_entry * entry_post) {
    
	printf("pointers of fields in struct e32_entry * entry_post:\n");
    printf(" entry_post->e32_flags                 %p\n", &entry_post->e32_flags); /*0*/
	printf(" entry_post->e32_offset.offset16       %p\n", &entry_post->e32_variant.e32_offset.offset16); /*1*/
	printf(" entry_post->e32_offset.offset32       %p\n", &entry_post->e32_variant.e32_offset.offset32); /*1*/	
	printf(" entry_post->e32_variant.e32_fwd.modor %p\n", &entry_post->e32_variant.e32_fwd.modord);      /*1*/
	printf(" entry_post->e32_callgate.offset       %p\n", &entry_post->e32_variant.e32_callgate.offset); /*1*/
	printf(" entry_post->e32_variant.e32_fwd.value %p\n", &entry_post->e32_variant.e32_fwd.value);       /*3*/
	printf(" entry_post->e32_callgate.callgate     %p\n", &entry_post->e32_variant.e32_callgate.callgate);/*3*/
}
