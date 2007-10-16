

#include <stdlib.h>
#include <stdio.h>
#include "../modlx.h"
#include "../fixuplx.h"
#include "../gcc_os2def.h"

/* #include <exe386.h> */

/*
08   01   08   00 01   57 01
08   01   20   00 01   8b 04   ### 16 bits import ordinal
07   81   42   00 01   13      ###  8 bits import ordinal

07    10   src off = 0AAD   object #    = 01   target off       = 00011AB4
07    10             ad0a                 01                      b41a0100
*/
void test_fixup() {
	printf(" Testing fixups. \n");
	/* print_struct_r32_rlc_info(struct r32_rlc * rlc) */
	/* Different types of fixup structures (variable sizes, variable fields).*/
	char test_fixup1[] = {0x08,  0x01,  0x08,0x00,  0x01,  0x57,0x01};
	char test_fixup2[] = {0x08,  0x01,  0x20,0x00,  0x01,  0x8b,0x04};
	char test_fixup3[] = {0x08,  0x81,  0x08,0x00,  0x01,  0x13};
	char test_fixup4[] = {0x07,  0x81,  0x42,0x00,  0x01,  0x13};
	char test_fixup5[] = {0x07,  0x00,  0x26,0x00,  0x02,  0x04, 0x00};
	char test_fixup6[] = {0x07,  0x00,  0x3e,0x00,  0x02,  0x0d, 0x00};
	char test_fixup7[] = {0x08,  0x02,  0x2e,0x00,  0x01,  0x01, 0x00};
	
	char test_fixup8[] = {0x07,  0x01,  0x48,0x0c,  0x01,  0x65,0x01};
	char test_fixup9[] = {0x07,  0x81,  0x00,0x0d,  0x04,  0x13};
	char test_fixup10[] ={0x07,  0x85,  0x8f,0x05,  0x01,  0x03,      0x00,0x02};
	char test_fixup11[] ={0x08,  0x01,  0xf7,0x03,  0x01,  0xec,0x03};
	char test_fixup12[] ={0x08,  0x81,  0x20,0x07,  0x01,  0xa1};
	char test_fixup13[] ={0x28,  0x01,  0x03,       0x01,  0xa9,0x01, 0x75,0x0a,  0xa9,0x0a,  0xb3,0x0a};
	
	char test_fixup14[] ={0x28,  0x81,  0x02,       0x01,  0xdf,      0x16,0x05,  0xeb,0x04};
	char test_fixup15[] ={0x27,  0x81,  0x02,       0x04,  0x0f,      0x63,0x0d,  0xe4,0x0c};
	char test_fixup16[] ={0x08,  0x02,  0xe7,0x09,  0x02,  0x13,0x00};
	char test_fixup17[] ={0x07,  0x10,  0xad,0x0a,  0x01,  0xb4,0x1a,0x01,0x00};
	
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup1);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup2);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup3);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup4);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup5);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup6);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup7);
	
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup8);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup9);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup10);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup11);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup12);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup13);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup14);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup15);
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup16); 
	
	print_struct_r32_rlc_info( (struct r32_rlc *) &test_fixup17);
}


int main() {

	test_fixup();

	return 0;
}
