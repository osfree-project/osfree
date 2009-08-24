/*    
	make_thunk.c	2.21
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

#include "windows.h"

#include "kerndef.h"
#include "BinTypes.h"
#include "Log.h"
#include "Resources.h"
#include "Module.h"
#include "DPMI.h"
#include "Driver.h"
#include "make_thunk.h"

#define	TRANS_ADDR	0x3c

struct BIN_THUNK_HASH{
	DWORD target;
	BINADDR binaddr;
	struct BIN_THUNK_HASH *next;
}bin_thunk_hash[256], nat_thunk_hash[256];

static void build_binary_thunk_seg(void);

/********************************************************************/

/*
 *	check_for_native_thunk(binaddr)
 *		search the nat_thunk_hash table for binaddr
 *	
 */
static DWORD
check_for_native_thunk(BINADDR binaddr)
{
	struct BIN_THUNK_HASH *temp;

	temp = &(nat_thunk_hash[binaddr & 0xff]);

	while (temp) {
		if (temp->binaddr == binaddr) return temp->target;
		temp = temp->next;
	}
	return (0);
}

/*
 *	check_for_binary_thunk(binaddr)
 *		search the bin_thunk_hash table for binaddr
 *	
 */

static DWORD
check_for_binary_thunk(DWORD targ)
{
	struct BIN_THUNK_HASH *temp;

	temp = &(bin_thunk_hash[(targ >> 2) & 0xff]);

	while (temp) {
		if (temp->target == targ) return temp->binaddr;
		temp = temp->next;
	}
	return (0);
}
/********************************************************************/
/*
 *	add_native_thunk_hash(targ,binaddr)
 *		add a native thunk to nat_thunk_hash
 *	
 */

static void 
add_native_thunk_hash(DWORD targ, BINADDR binaddr)
{
	struct BIN_THUNK_HASH *temp;
	
	temp = &(nat_thunk_hash[binaddr & 0xff]);

	if (temp->binaddr == 0) { /* no entry yet */
		temp->binaddr = binaddr;
		temp->target = targ;
		return;
	}
	
	/* otherwise a collision -- get to end and add new entry */
	while (temp->next) temp = temp->next;
	temp->next = (struct BIN_THUNK_HASH *) 
		WinMalloc(sizeof(struct BIN_THUNK_HASH));
	temp = temp->next;
	temp->binaddr = binaddr;
	temp->target = targ;
	temp->next = NULL;
	return;
}

/*
 *	add_binary_thunk_hash(targ,binaddr)
 *		add a binary thunk to bin_thunk_hash
 *	
 */

static void 
add_binary_thunk_hash(DWORD targ, BINADDR binaddr)
{
	struct BIN_THUNK_HASH *temp;
	
	temp = &(bin_thunk_hash[(targ >> 2) & 0xff]);

	if (temp->binaddr == 0) { /* no entry yet */
		temp->binaddr = binaddr;
		temp->target = targ;
		return;
	}
	
	/* otherwise a collision -- get to end and add new entry */
	while (temp->next) temp = temp->next;
	temp->next = (struct BIN_THUNK_HASH *) 
		WinMalloc(sizeof(struct BIN_THUNK_HASH));
	temp = temp->next;
	temp->binaddr = binaddr;
	temp->target = targ;
	temp->next = NULL;
	return;
}

/********************************************************************/

#define THUNK_MAGIC 0x48535754	/* "HSWT" */

#if defined(sparc)
DWORD make_native_thunk_template[] = {
		0x9de3bfa0,	/* save %sp, -96, %sp */
		0x40000005, /* call + 5 */
		0x01000000, /* nop */
		0,			/* data -- BINADDR */
		0,			/* data -- conv() */
		0,			/* data -- envp_global */
		0xd203e010, /* ld      [%o7 + 16], %o1 */
		0xd2024000, /* ld      [%o1], %o1 */
		0xd403e008, /* ld      [%o7 + 8], %o2 */
		0xd422603c, /* st      %o2, [%o1 + TRANS_ADDR] */
		0xd203e00c, /* ld      [%o7 + 12], %o1 */
		0x81c24000, /* jmp %o1 */
		0x81e80000, /* restore */
		THUNK_MAGIC	/* placed at end, so make_binary_thunk can check */
		};
#define CUR_ENV_OFF	5
#define TARG_OFF	3
#define CONV_OFF	4
#define THUNK_SIZE 14
#define THUNK_MAGIC_OFFSET 13
#endif

#ifdef X386
char thunk_template[] = {
		0x90, 0x53,                     /* PUSH EBX */
		0x90, 0xbb,			/* MOV  EBX, 0 */ 
		0x00, 0x00, 0x00, 0x00,		/* data -- envp_global */
		0x8b, 0x1b, 0x90, 0x90,         /* MOV  EBX, [EBX]  */
		0x90, 0xc7, 0x43, TRANS_ADDR,	/* MOV  [EBX+TRANS_ADDR], 0L */
		0x90, 0x90, 0x90, 0x90, 	/* data -- binaddr    */
		0x90, 0x90,			/* alignment */
		0x5b,				/* POP  EBX */
		0xe9,				/* JMP  */
		0x90, 0x90, 0x90, 0x90, 	/* data -- conv()offset     */
		0x54, 0x57, 0x53, 0x48		/* THUNK_MAGIC */
	};

#define TARG_OFF        4
#define CONV_OFF        6
#define CUR_ENV_OFF	1
#define THUNK_MAGIC_OFFSET 7
#define THUNK_SIZE	8

LPDWORD make_native_thunk_template;
#endif

#if defined(ppc) 
DWORD make_native_thunk_template[] = {
		/* function descr for the thunk */
		0,
		0,
		0,
		/* code template here */
		/* save the return register */
		0x7c000278, /* xor 0, 0, 0 */
		0x7c0802a6, /* mflr 0 */
		0x3021fff0, /* ai 1, 1, -16 */
		0x90010000, /* st 0, 0(1) */

		0x48000011,	/* bl locallabel */
		0,			/* data &curr_env */
		0,			/* data &targ */
		0,			/* conv  	 */

		/* locallabel:   */
		/* store registers to be used as temps */
		0x90a10004, /* st 5, 4(1) */
		0x90c10008, /* st 6, 8(1) */
		0x9081000c, /* st 4, 12(1) */

		0x7ca802a6, /* mflr 5 */
		0x80c50000, /* l 6, 0(5) */
		0x80c60000, /* l 6, 0(6) */
		0x80850004, /* l 4, 4(5) */
		0x9086003c, /* st 4, TRANS_ADDR(6) targ into currenv*/

		/* load conv func descr */
		0x80c50008, /* l 6, 8(5) */
		0x80a60000, /* l 5, 0(6) */
		0x7ca903a6, /* mtctr 5 */

		/* restore link register */
		0x80010000, /* l 0, 0(1) */
		0x7c0803a6, /* mtlr 0 */

		0x80460004, /* l 2, 4(6) */

		/* restore registers used as temps */
		0x80a10004, /* l 5, 4(1) */
		0x80c10008, /* l 6, 8(1) */
		0x8081000c, /* l 4, 12(1) */

		0x30210010, /* ai 1, 1, 16 */
		0x4e800420, /* bctr */
		THUNK_MAGIC	/* placed at end, so make_binary_thunk can check */
		};
#define FUNC_DESC_OFF	   0   
#define FUNC_DESC_SIZE	3
#define CUR_ENV_OFF	8
#define TARG_OFF	9
#define CONV_OFF	10
#define THUNK_SIZE 31
#define THUNK_MAGIC_OFFSET 30
#endif

#if defined(mips)
DWORD make_native_thunk_template[] = {
	0x3c190000,	/* lui	   t9,HIWORD(THUNKADDRESS) */
	0x37390000,	/* ori	   t9,t9,LOWORD(THUNKADDRESS) */
	0x8f280024,	/* lw      t0,36(t9) */
	0x8f290028,	/* lw	   t1,40(t9) */
	0x8d080000,	/* lw	   t0,0(t0)  */
	0x8f39002c,	/* lw	   t9,44(t9) */
	0xad09003c,	/* sw	   t1,TRANS_ADDR(t0) */
	0x03200008,	/* jr	   t9	     */
	0x00000000,	/* nop		     */
	0,		/* ENVP		9    */
	0,		/* BINADDR	10   */
	0,		/* CONV		11   */
	THUNK_MAGIC	/* placed at end, so make_binary_thunk can check */
};

#define CUR_ENV_OFF	9
#define TARG_OFF	10
#define CONV_OFF	11
#define THUNK_SIZE	 13
#define THUNK_MAGIC_OFFSET 12
#define	THUNK_ADDRESS_HI 0
#define	THUNK_ADDRESS_LO 1
#endif

#if defined(hppa)
/*
 *  Thunk code for HP PA-RISC systems.  Some general comments about
 *  this code:
 *
 *  1)  In addition to writing the three values ENVP, BINADDR, and CONV
 *      into the code stream as data words, the address of this thunk
 *      code must also be written into the code.  Part of the address
 *      goes in one instruction, part in another, as shown.  The format
 *      for encoding the literal constant is complex, see the code where
 *      where this is setup for details.
 *
 *  2)  This code is always referenced via a pointer to a function,
 *      and hence is always called via the $$dyncall millicode routine.
 *      This has already stored the return address information onto
 *      the proper areas on the stack, as well as in gr2, enabling
 *      the function we call to properly do an inter-space return.
 *
 *  3)  Under PA-RISC, the low two bits of the CONV function address are
 *      used to encode whether the pointer goes directly to the code,
 *      or if it instead points to a table, which in turn contains
 *      the function pointer, as well as a global data pointer that
 *      must be set in gr19.  The code that checks this, located
 *      between the "bb" and "be,n" instructions, is lifted directly
 *      from the $$dyncall millicode routine.
 *
 *  4)  The PA-RISC has delayed execution slots after any branch.
 *      Unless the execution of this "delayed" instruction is
 *      nullified, the instruction following the branch will
 *      normally be executed.
 */
DWORD make_native_thunk_template[] = {
	0xe8000018,     /* b    .+5       */
	0x22800000,	/* ldil	L'THUNKADDRESS,gr20 */
	0,		/* ENVP		2 */
	0,		/* BINADDR	3 */
	0,		/* CONV		4 */
	0x36940000,	/* ldo  R'THUNKADDRESS(gr20),gr20 */
	0x4a950010,	/* ldw  8(gr20),gr21  ; get ptr to envp */
	0x4ab50000,	/* ldw  0(gr21),gr21  ; deref ptr to envp */
	0x4a960018,	/* ldw  12(gr20),gr22 ; get binaddr */
	0x6ab60078,	/* stw  gr22,TRANS_ADDR(gr21) ; store binaddr in env */
	0x4a960020,	/* ldw  16(gr20),gr22 ; get conv */
	0xc7d6c022,	/* bb,>=,n gr22,30,.+4 ; check for table vs direct */
	0xd6c01c1e,	/* depi  0,31,2,gr22  ; table -- clear lower 2 bits */
	0x4ad30008,	/* ldw   4(gr22),gr19 ; get data pointer from table */
	0x4ad60000,	/* ldw   0(gr22),gr22 ; get real func addr from table */
	0x02c010b4,	/* ldsid (gr22),gr20  ; get space identifier */
	0x00141820,	/* mtsp  gr20, sr0    ; set space identifier */
	0xe2c00002,	/* be,n  0(sr0,gr22)  ; external branch to conv */
	0x08000240,	/* nop                ; delayed instruction */
	THUNK_MAGIC	/* placed at end, so make_binary_thunk can check */
};

#define CUR_ENV_OFF	2
#define TARG_OFF	3
#define CONV_OFF	4
#define	THUNK_ADDRESS_HI 1
#define	THUNK_ADDRESS_LO 5
#define THUNK_MAGIC_OFFSET 19
#define THUNK_SIZE	20
#endif

#if   defined(alpha)
unsigned int thunk_template[] = {
	0xc0200007,	/*	br	t0,1f		*/
	0,		/* padding to 64-bit word	*/
	0,0,		/* intel_entry	(32 bits)	*/
	0,0,		/* envp_global	(64 bits)	*/
	0,0,		/* conv		(64 bits)	*/
	0xa441000c,	/*	ldq	t1,12(t0)		*/
	0xa0610004,	/*	ldl	t2,4(t0)		*/
	0xa4420000,	/*	ldq	t1,0(t1)		*/
	0xa7610014,	/*	ldq	r27,20(r1)		*/
	0xb0620078,	/*	stl	r3,120(r2)		*/
	0x6bfb0000,	/*	jmp	zero,(r27)		*/
	THUNK_MAGIC,0	/* placed at end, so make_binary_thunk can check */
};

unsigned long *make_native_thunk_template;

#define CUR_ENV_OFF	2
#define TARG_OFF	1
#define CONV_OFF	3
#define THUNK_SIZE	 8
#define THUNK_MAGIC_OFFSET 7
#endif

extern DSCR *LDT;
extern ENV *envp_global;

int thunk_size = THUNK_SIZE;
int thunk_magic_index = THUNK_MAGIC_OFFSET;

/********************************************************************/

/*
 *	make_native_thunk(targ,conv)
 *		make a native thunk		
 *		
 */
DWORD
make_native_thunk(DWORD targ, DWORD conv)
{
	LPDWORD thunk;
    	DWORD i;
    	WORD wSel;
    	WORD wOffset;

	wSel = HIWORD(targ);
	wOffset = LOWORD(targ);

#if defined(X386) || defined(alpha)
	make_native_thunk_template = (LPDWORD) thunk_template;
#endif
	if (GetSelectorType(wSel) == TRANSFER_BINARY){
		/* we are passed a BINADDR of a binary thunk */
		thunk = (LPDWORD)(*(long *)GetPhysicalAddress(wSel));
		thunk += ((targ & 0xffff) >> 2);
		return *(thunk+1);
	}

	if ((i = check_for_native_thunk(targ))) 
		return i;

	/* we are passed a BINADDR of an API routine, e.g. DefWndProc */
 	/* ie. the native code address has already been assigned a BINADDR */
	if (GetSelectorType(wSel) == TRANSFER_CALLBACK) 
	    return ((SEGIMAGE *)
		(*(long *)GetPhysicalAddress(wSel)))[wOffset >> 3].targ;

    	if ((thunk = (LPDWORD)WinMalloc(THUNK_SIZE * sizeof(long)))) {
		for (i = 0; i < THUNK_SIZE; i++) 
			thunk[i] = make_native_thunk_template[i];

		/* now fill in the "special" pieces */
		thunk[CUR_ENV_OFF] =  (DWORD) &envp_global;
		thunk[TARG_OFF] = targ;
#ifdef	X386
		thunk[CONV_OFF] = (conv - (DWORD)&thunk[CONV_OFF+1]);
#else
		thunk[CONV_OFF] = conv;
#endif

#if defined(ppc) 
		thunk[FUNC_DESC_OFF] = (long)(thunk + FUNC_DESC_SIZE);
		DRVCALL_SYSTEM(PSSH_MAKEEXEDATA,
			(THUNK_SIZE - FUNC_DESC_SIZE)*4,
			0,
			(thunk + FUNC_DESC_SIZE));
#endif

#if defined(mips)
		*((LPWORD)(&thunk[THUNK_ADDRESS_HI])+1) = HIWORD((DWORD)thunk);
		*((LPWORD)(&thunk[THUNK_ADDRESS_LO])+1) = LOWORD((DWORD)thunk);
		DRVCALL_SYSTEM(PSSH_MAKEEXEDATA,THUNK_SIZE*sizeof(long),
				0,thunk);
#endif

#if defined(hppa)
		/*
		 *  Some rather ugly code here.  We have to encode
		 *  the address of the thunk within the instruction
		 *  stream, as immediate values.  However, one
		 *  instruction gets the top 21 bits, the other
		 *  instruction gets the low 11 bits.  Worse yet,
		 *  the 21-bit field is scrambled into 5 different
		 *  sub-fields.  Hence all the mask/shift operations
		 *  below.
		 */
		i = thunk[THUNK_ADDRESS_HI];
		i |= ((DWORD)thunk & 0x80000000) >> 31;
		i |= ((DWORD)thunk & 0x7ff00000) >> 19;
		i |= ((DWORD)thunk & 0x000c0000) >>  4;
		i |= ((DWORD)thunk & 0x0003e000) <<  3;
		i |= ((DWORD)thunk & 0x00001800) <<  1;
		thunk[THUNK_ADDRESS_HI] = i;
		thunk[THUNK_ADDRESS_LO] |= ((DWORD)thunk & 0x07ff) << 1;
		DRVCALL_SYSTEM(PSSH_MAKEEXEDATA,THUNK_SIZE*sizeof(long),
				0,thunk);
#endif

		add_native_thunk_hash((DWORD)thunk, targ);
		return (DWORD)thunk;
	}
	return 0;
}

static long next_binary_thunk = 0;
static WORD binary_thunk_selector;
static LPDWORD binary_thunk_segment;
static LPDWORD binary_thunk_image;
#define MAX_BINARY_THUNK 4000

BOOL
InitThunks(void)
{
    if (!next_binary_thunk)
	build_binary_thunk_seg();
    return (next_binary_thunk)?TRUE:FALSE;
}

static void
build_binary_thunk_seg()
{
	/* build a new segment, with TRANSFER_BINARY as its transfer
	   routine, with room for MAX_BINARY_THUNK entries (8 bytes each) */
	
	binary_thunk_selector = 
		MakeSegment(MAX_BINARY_THUNK*8,TRANSFER_BINARY); 
	binary_thunk_image = (LPDWORD)WinMalloc(MAX_BINARY_THUNK*8);
	binary_thunk_segment =(LPDWORD)GetPhysicalAddress(binary_thunk_selector);
	*binary_thunk_segment = (DWORD)binary_thunk_image;
	next_binary_thunk = 2;
}

/*
 *	make_binary_thunk(targ,conv)
 *		make a binary thunk		
 *		
 */
BINADDR
make_binary_thunk(LPDWORD targ, DWORD conv)
{
	BINADDR binaddr;

#ifdef hppa
	/*
	 *  On PA-RISC, the second-to-last bit of a function pointer
	 *  is used to distinguish between an indirect jump
	 *  through a table and a direct jump.  The thunks we
	 *  create use direct addresses.  Do not even try to
	 *  access the data if we have a table pointer, since
	 *  it is not a true pointer (the indicator bit must
	 *  be cleared to get the pointer to the table), and 
	 *  will cause an unaligned-access fault.  We could clear
	 *  the indicator bit and then go ahead and try the access,
	 *  but since we know that our thunks never have the bit
	 *  set, simply avoid the test in this case.
	 *
	 *  Yes, this is ugly.  But unless we change our code
	 *  to allow testing for a native thunk without using
	 *  the magic number (using the hash table, perhaps?),
	 *  this is an unavoidable system-specific piece of code.
	 */
	if (((DWORD)targ & 0x02) == 0) {
		if (targ[thunk_magic_index] == THUNK_MAGIC) {
			return (BINADDR)targ[TARG_OFF];
		}
	} 
#else
	if (targ[thunk_magic_index] == THUNK_MAGIC) {
		return (BINADDR)targ[TARG_OFF];
	}
#endif
	if (next_binary_thunk == 0) 
		build_binary_thunk_seg();

	if ((binaddr = check_for_binary_thunk((DWORD)targ)))
		return binaddr;

	if (next_binary_thunk == MAX_BINARY_THUNK) build_binary_thunk_seg();

	binaddr = (binary_thunk_selector << 16) + (next_binary_thunk << 2);
	/* the shift by two used to be left by one.  Changed 12/16/92 mfh */

	binary_thunk_image[next_binary_thunk++] = (DWORD)conv;
	binary_thunk_image[next_binary_thunk++] = (DWORD)targ;
	add_binary_thunk_hash((DWORD)targ, binaddr);
	return (binaddr);
}
