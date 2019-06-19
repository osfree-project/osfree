/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	db_disasm.c,v $
 * Revision 2.6  92/01/03  20:05:00  dbg
 * 	Add a switch to disassemble 16-bit code.
 * 	Fix spelling of 'lods' opcodes.
 * 	[91/10/30            dbg]
 * 
 * Revision 2.5  91/10/09  16:05:58  af
 * 	Supported disassemble of non current task by passing task parameter.
 * 	[91/08/29            tak]
 * 
 * Revision 2.4  91/05/14  16:05:04  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:11:03  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:31:03  mrt]
 * 
 * Revision 2.2  90/08/27  21:55:56  dbg
 * 	Fix register operand for move to/from control/test/debug
 * 	register instructions.  Add i486 instructions.
 * 	[90/08/27            dbg]
 * 
 * 	Import db_sym.h.  Print instruction displacements in
 * 	current radix (signed).  Change calling sequence of
 * 	db_disasm.
 * 	[90/08/21            dbg]
 * 	Fix includes.
 * 	[90/08/08            dbg]
 * 	Created.
 * 	[90/07/25            dbg]
 * 
 */

/*
 * Instruction disassembler.
 */
#if 0
#include <mach/boolean.h>
#include "db_machdep.h"

#include "db_access.h"
#include "db_sym.h"

#include <kern/task.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include "db_disasm.h"
#include "debugger.h"
#include "stdarg.h"

extern FILE *debug_outfp;

/*
 * Switch to disassemble 16-bit code.
 */
static boolean_t	db_disasm_16 = FALSE;

/*
 * Size attributes
 */
#define	SIZE_BYTE	0
#define	SIZE_WORD	1
#define	SIZE_LONG	2
#define	SIZE_QUAD	3
#define	SIZE_SNGL	4
#define	SIZE_DBLR	5
#define	SIZE_EXTR	6
#define	SIZE_SDEP	7
#define	SIZE_NONE	8

/*
 * Addressing modes
 */
#define	E	1			/* general effective address */
#define	Eind	2			/* indirect address (jump, call) */
#define	Ew	3			/* address, word size */
#define	Eb	4			/* address, byte size */
#define	R	5			/* register, in 'reg' field */
#define	Rw	6			/* word register, in 'reg' field */
#define	Ri	7			/* register in instruction */
#define	S	8			/* segment reg, in 'reg' field */
#define	Si	9			/* segment reg, in instruction */
#define	A	10			/* accumulator */
#define	BX	11			/* (bx) */
#define	CL	12			/* cl, for shifts */
#define	DX	13			/* dx, for IO */
#define	SI	14			/* si */
#define	DI	15			/* di */
#define	CR	16			/* control register */
#define	DR	17			/* debug register */
#define	TR	18			/* test register */
#define	I	19			/* immediate, unsigned */
#define	Is	20			/* immediate, signed */
#define	Ib	21			/* byte immediate, unsigned */
#define	Ibs	22			/* byte immediate, signed */
#define	Iw	23			/* word immediate, unsigned */
#define	Il	24			/* long immediate */
#define	O	25			/* direct address */
#define	Db	26			/* byte displacement from EIP */
#define	Dl	27			/* long displacement from EIP */
#define	o1	28			/* constant 1 */
#define	o3	29			/* constant 3 */
#define	OS	30			/* immediate offset/segment */
#define	ST	31			/* FP stack top */
#define	STI	32			/* FP stack */
#define	X	33			/* extended FP op */
#define	XA	34			/* for 'fstcw %ax' */

struct inst {
	char *	i_name;			/* name */
	short	i_has_modrm;		/* has regmodrm byte */
	short	i_size;			/* operand size */
	int	i_mode;			/* addressing modes */
	char *	i_extra;		/* pointer to extra opcode table */
};

#define	op1(x)		(x)
#define	op2(x,y)	((x)|((y)<<8))
#define	op3(x,y,z)	((x)|((y)<<8)|((z)<<16))

struct finst {
	char *	f_name;			/* name for memory instruction */
	int	f_size;			/* size for memory instruction */
	int	f_rrmode;		/* mode for rr instruction */
	char *	f_rrname;		/* name for rr instruction
					   (or pointer to table) */
};

static char *	db_Grp6[] = {
	"sldt",
	"str",
	"lldt",
	"ltr",
	"verr",
	"verw",
	"",
	""
};

static char *	db_Grp7[] = {
	"sgdt",
	"sidt",
	"lgdt",
	"lidt",
	"smsw",
	"",
	"lmsw",
	"invlpg"
};

static char *	db_Grp8[] = {
	"",
	"",
	"",
	"",
	"bt",
	"bts",
	"btr",
	"btc"
};

static struct inst db_inst_0f0x[] = {
/*00*/	{ "",	   TRUE,  SIZE_NONE,  op1(Ew),     (char *)db_Grp6 },
/*01*/	{ "",	   TRUE,  SIZE_NONE,  op1(Ew),     (char *)db_Grp7 },
/*02*/	{ "lar",   TRUE,  SIZE_LONG,  op2(E,R),    0 },
/*03*/	{ "lsl",   TRUE,  SIZE_LONG,  op2(E,R),    0 },
/*04*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*05*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*06*/	{ "clts",  FALSE, SIZE_NONE,  0,	      0 },
/*07*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },

/*08*/	{ "invd",  FALSE, SIZE_NONE,  0,	      0 },
/*09*/	{ "wbinvd",FALSE, SIZE_NONE,  0,	      0 },
/*0a*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*0b*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*0c*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*0d*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*0e*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*0f*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
};

static struct inst	db_inst_0f2x[] = {
/*20*/	{ "mov",   TRUE,  SIZE_LONG,  op2(CR,E),   0 }, /* use E for reg */
/*21*/	{ "mov",   TRUE,  SIZE_LONG,  op2(DR,E),   0 }, /* since mod == 11 */
/*22*/	{ "mov",   TRUE,  SIZE_LONG,  op2(E,CR),   0 },
/*23*/	{ "mov",   TRUE,  SIZE_LONG,  op2(E,DR),   0 },
/*24*/	{ "mov",   TRUE,  SIZE_LONG,  op2(TR,E),   0 },
/*25*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*26*/	{ "mov",   TRUE,  SIZE_LONG,  op2(E,TR),   0 },
/*27*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },

/*28*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*29*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*2a*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*2b*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*2c*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*2d*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*2e*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*2f*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
};

static struct inst	db_inst_0f8x[] = {
/*80*/	{ "jo",    FALSE, SIZE_NONE,  op1(Dl),     0 },
/*81*/	{ "jno",   FALSE, SIZE_NONE,  op1(Dl),     0 },
/*82*/	{ "jb",    FALSE, SIZE_NONE,  op1(Dl),     0 },
/*83*/	{ "jnb",   FALSE, SIZE_NONE,  op1(Dl),     0 },
/*84*/	{ "jz",    FALSE, SIZE_NONE,  op1(Dl),     0 },
/*85*/	{ "jnz",   FALSE, SIZE_NONE,  op1(Dl),     0 },
/*86*/	{ "jbe",   FALSE, SIZE_NONE,  op1(Dl),     0 },
/*87*/	{ "jnbe",  FALSE, SIZE_NONE,  op1(Dl),     0 },

/*88*/	{ "js",    FALSE, SIZE_NONE,  op1(Dl),     0 },
/*89*/	{ "jns",   FALSE, SIZE_NONE,  op1(Dl),     0 },
/*8a*/	{ "jp",    FALSE, SIZE_NONE,  op1(Dl),     0 },
/*8b*/	{ "jnp",   FALSE, SIZE_NONE,  op1(Dl),     0 },
/*8c*/	{ "jl",    FALSE, SIZE_NONE,  op1(Dl),     0 },
/*8d*/	{ "jnl",   FALSE, SIZE_NONE,  op1(Dl),     0 },
/*8e*/	{ "jle",   FALSE, SIZE_NONE,  op1(Dl),     0 },
/*8f*/	{ "jnle",  FALSE, SIZE_NONE,  op1(Dl),     0 },
};

static struct inst	db_inst_0f9x[] = {
/*90*/	{ "seto",  TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*91*/	{ "setno", TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*92*/	{ "setb",  TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*93*/	{ "setnb", TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*94*/	{ "setz",  TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*95*/	{ "setnz", TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*96*/	{ "setbe", TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*97*/	{ "setnbe",TRUE,  SIZE_NONE,  op1(Eb),     0 },

/*98*/	{ "sets",  TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*99*/	{ "setns", TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*9a*/	{ "setp",  TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*9b*/	{ "setnp", TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*9c*/	{ "setl",  TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*9d*/	{ "setnl", TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*9e*/	{ "setle", TRUE,  SIZE_NONE,  op1(Eb),     0 },
/*9f*/	{ "setnle",TRUE,  SIZE_NONE,  op1(Eb),     0 },
};

static struct inst	db_inst_0fax[] = {
/*a0*/	{ "push",  FALSE, SIZE_NONE,  op1(Si),     0 },
/*a1*/	{ "pop",   FALSE, SIZE_NONE,  op1(Si),     0 },
/*a2*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*a3*/	{ "bt",    TRUE,  SIZE_LONG,  op2(E,R),    0 },
/*a4*/	{ "shld",  TRUE,  SIZE_LONG,  op3(Ib,E,R), 0 },
/*a5*/	{ "shld",  TRUE,  SIZE_LONG,  op3(CL,E,R), 0 },
/*a6*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*a7*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },

/*a8*/	{ "push",  FALSE, SIZE_NONE,  op1(Si),     0 },
/*a9*/	{ "pop",   FALSE, SIZE_NONE,  op1(Si),     0 },
/*aa*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*ab*/	{ "bts",   TRUE,  SIZE_LONG,  op2(E,R),    0 },
/*ac*/	{ "shrd",  TRUE,  SIZE_LONG,  op3(Ib,E,R), 0 },
/*ad*/	{ "shrd",  TRUE,  SIZE_LONG,  op3(CL,E,R), 0 },
/*a6*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*a7*/	{ "imul",  TRUE,  SIZE_LONG,  op2(E,R),    0 },
};

static struct inst	db_inst_0fbx[] = {
/*b0*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*b1*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*b2*/	{ "lss",   TRUE,  SIZE_LONG,  op2(E, R),   0 },
/*b3*/	{ "bts",   TRUE,  SIZE_LONG,  op2(R, E),   0 },
/*b4*/	{ "lfs",   TRUE,  SIZE_LONG,  op2(E, R),   0 },
/*b5*/	{ "lgs",   TRUE,  SIZE_LONG,  op2(E, R),   0 },
/*b6*/	{ "movzb", TRUE,  SIZE_LONG,  op2(E, R),   0 },
/*b7*/	{ "movzw", TRUE,  SIZE_LONG,  op2(E, R),   0 },

/*b8*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*b9*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*ba*/	{ "",      TRUE,  SIZE_LONG,  op2(Is, E),  (char *)db_Grp8 },
/*bb*/	{ "btc",   TRUE,  SIZE_LONG,  op2(R, E),   0 },
/*bc*/	{ "bsf",   TRUE,  SIZE_LONG,  op2(E, R),   0 },
/*bd*/	{ "bsr",   TRUE,  SIZE_LONG,  op2(E, R),   0 },
/*be*/	{ "movsb", TRUE,  SIZE_LONG,  op2(E, R),   0 },
/*bf*/	{ "movsw", TRUE,  SIZE_LONG,  op2(E, R),   0 },
};

static struct inst	db_inst_0fcx[] = {
/*c0*/	{ "xadd",  TRUE,  SIZE_BYTE,	 op2(R, E),   0 },
/*c1*/	{ "xadd",  TRUE,  SIZE_LONG,	 op2(R, E),   0 },
/*c2*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c3*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c4*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c5*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c6*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c7*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c8*/	{ "bswap", FALSE, SIZE_LONG,  op1(Ri),     0 },
/*c9*/	{ "bswap", FALSE, SIZE_LONG,  op1(Ri),     0 },
/*ca*/	{ "bswap", FALSE, SIZE_LONG,  op1(Ri),     0 },
/*cb*/	{ "bswap", FALSE, SIZE_LONG,  op1(Ri),     0 },
/*cc*/	{ "bswap", FALSE, SIZE_LONG,  op1(Ri),     0 },
/*cd*/	{ "bswap", FALSE, SIZE_LONG,  op1(Ri),     0 },
/*ce*/	{ "bswap", FALSE, SIZE_LONG,  op1(Ri),     0 },
/*cf*/	{ "bswap", FALSE, SIZE_LONG,  op1(Ri),     0 },
};

static struct inst	db_inst_0fdx[] = {
/*c0*/	{ "cmpxchg",TRUE, SIZE_BYTE,	 op2(R, E),   0 },
/*c1*/	{ "cmpxchg",TRUE, SIZE_LONG,	 op2(R, E),   0 },
/*c2*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c3*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c4*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c5*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c6*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c7*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c8*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*c9*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*ca*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*cb*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*cc*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*cd*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*ce*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
/*cf*/	{ "",	   FALSE, SIZE_NONE,	 0,	      0 },
};

static struct inst *db_inst_0f[] = {
	db_inst_0f0x,
	0,
	db_inst_0f2x,
	0,
	0,
	0,
	0,
	0,
	db_inst_0f8x,
	db_inst_0f9x,
	db_inst_0fax,
	db_inst_0fbx,
	db_inst_0fcx,
	db_inst_0fdx,
	0,
	0
};

static char *	db_Esc92[] = {
	"fnop",	"",	"",	"",	"",	"",	"",	""
};
static char *	db_Esc93[] = {
	"",	"",	"",	"",	"",	"",	"",	""
};
static char *	db_Esc94[] = {
	"fchs",	"fabs",	"",	"",	"ftst",	"fxam",	"",	""
};
static char *	db_Esc95[] = {
	"fld1",	"fldl2t","fldl2e","fldpi","fldlg2","fldln2","fldz",""
};
static char *	db_Esc96[] = {
	"f2xm1","fyl2x","fptan","fpatan","fxtract","fprem1","fdecstp",
	"fincstp"
};
static char *	db_Esc97[] = {
	"fprem","fyl2xp1","fsqrt","fsincos","frndint","fscale","fsin","fcos"
};

static char *	db_Esca4[] = {
	"",	"fucompp","",	"",	"",	"",	"",	""
};

static char *	db_Escb4[] = {
	"",	"",	"fnclex","fninit","",	"",	"",	""
};

static char *	db_Esce3[] = {
	"",	"fcompp","",	"",	"",	"",	"",	""
};

static char *	db_Escf4[] = {
	"fnstsw","",	"",	"",	"",	"",	"",	""
};

static struct finst db_Esc8[] = {
/*0*/	{ "fadd",   SIZE_SNGL,  op2(STI,ST),	0 },
/*1*/	{ "fmul",   SIZE_SNGL,  op2(STI,ST),	0 },
/*2*/	{ "fcom",   SIZE_SNGL,  op2(STI,ST),	0 },
/*3*/	{ "fcomp",  SIZE_SNGL,  op2(STI,ST),	0 },
/*4*/	{ "fsub",   SIZE_SNGL,  op2(STI,ST),	0 },
/*5*/	{ "fsubr",  SIZE_SNGL,  op2(STI,ST),	0 },
/*6*/	{ "fdiv",   SIZE_SNGL,  op2(STI,ST),	0 },
/*7*/	{ "fdivr",  SIZE_SNGL,  op2(STI,ST),	0 },
};

static struct finst db_Esc9[] = {
/*0*/	{ "fld",    SIZE_SNGL,  op1(STI),	0 },
/*1*/	{ "",       SIZE_NONE,  op1(STI),	"fxch" },
/*2*/	{ "fst",    SIZE_SNGL,  op1(X),	(char *)db_Esc92 },
/*3*/	{ "fstp",   SIZE_SNGL,  op1(X),	(char *)db_Esc93 },
/*4*/	{ "fldenv", SIZE_NONE,  op1(X),	(char *)db_Esc94 },
/*5*/	{ "fldcw",  SIZE_NONE,  op1(X),	(char *)db_Esc95 },
/*6*/	{ "fnstenv",SIZE_NONE,  op1(X),	(char *)db_Esc96 },
/*7*/	{ "fnstcw", SIZE_NONE,  op1(X),	(char *)db_Esc97 },
};

static struct finst db_Esca[] = {
/*0*/	{ "fiadd",  SIZE_WORD,  0,		0 },
/*1*/	{ "fimul",  SIZE_WORD,  0,		0 },
/*2*/	{ "ficom",  SIZE_WORD,  0,		0 },
/*3*/	{ "ficomp", SIZE_WORD,  0,		0 },
/*4*/	{ "fisub",  SIZE_WORD,  op1(X),	(char *)db_Esca4 },
/*5*/	{ "fisubr", SIZE_WORD,  0,		0 },
/*6*/	{ "fidiv",  SIZE_WORD,  0,		0 },
/*7*/	{ "fidivr", SIZE_WORD,  0,		0 }
};

static struct finst db_Escb[] = {
/*0*/	{ "fild",   SIZE_WORD,  0,		0 },
/*1*/	{ "",       SIZE_NONE,  0,		0 },
/*2*/	{ "fist",   SIZE_WORD,  0,		0 },
/*3*/	{ "fistp",  SIZE_WORD,  0,		0 },
/*4*/	{ "",       SIZE_WORD,  op1(X),	(char *)db_Escb4 },
/*5*/	{ "fld",    SIZE_EXTR,  0,		0 },
/*6*/	{ "",       SIZE_WORD,  0,		0 },
/*7*/	{ "fstp",   SIZE_EXTR,  0,		0 },
};

static struct finst db_Escc[] = {
/*0*/	{ "fadd",   SIZE_DBLR,  op2(ST,STI),	0 },
/*1*/	{ "fmul",   SIZE_DBLR,  op2(ST,STI),	0 },
/*2*/	{ "fcom",   SIZE_DBLR,  op2(ST,STI),	0 },
/*3*/	{ "fcomp",  SIZE_DBLR,  op2(ST,STI),	0 },
/*4*/	{ "fsub",   SIZE_DBLR,  op2(ST,STI),	"fsubr" },
/*5*/	{ "fsubr",  SIZE_DBLR,  op2(ST,STI),	"fsub" },
/*6*/	{ "fdiv",   SIZE_DBLR,  op2(ST,STI),	"fdivr" },
/*7*/	{ "fdivr",  SIZE_DBLR,  op2(ST,STI),	"fdiv" },
};

static struct finst db_Escd[] = {
/*0*/	{ "fld",    SIZE_DBLR,  op1(STI),	"ffree" },
/*1*/	{ "",       SIZE_NONE,  0,		0 },
/*2*/	{ "fst",    SIZE_DBLR,  op1(STI),	0 },
/*3*/	{ "fstp",   SIZE_DBLR,  op1(STI),	0 },
/*4*/	{ "frstor", SIZE_NONE,  op1(STI),	"fucom" },
/*5*/	{ "",       SIZE_NONE,  op1(STI),	"fucomp" },
/*6*/	{ "fnsave", SIZE_NONE,  0,		0 },
/*7*/	{ "fnstsw", SIZE_NONE,  0,		0 },
};

static struct finst db_Esce[] = {
/*0*/	{ "fiadd",  SIZE_LONG,  op2(ST,STI),	"faddp" },
/*1*/	{ "fimul",  SIZE_LONG,  op2(ST,STI),	"fmulp" },
/*2*/	{ "ficom",  SIZE_LONG,  0,		0 },
/*3*/	{ "ficomp", SIZE_LONG,  op1(X),	(char *)db_Esce3 },
/*4*/	{ "fisub",  SIZE_LONG,  op2(ST,STI),	"fsubrp" },
/*5*/	{ "fisubr", SIZE_LONG,  op2(ST,STI),	"fsubp" },
/*6*/	{ "fidiv",  SIZE_LONG,  op2(ST,STI),	"fdivrp" },
/*7*/	{ "fidivr", SIZE_LONG,  op2(ST,STI),	"fdivp" },
};

static struct finst db_Escf[] = {
/*0*/	{ "fild",   SIZE_LONG,  0,		0 },
/*1*/	{ "",       SIZE_LONG,  0,		0 },
/*2*/	{ "fist",   SIZE_LONG,  0,		0 },
/*3*/	{ "fistp",  SIZE_LONG,  0,		0 },
/*4*/	{ "fbld",   SIZE_NONE,  op1(XA),	(char *)db_Escf4 },
/*5*/	{ "fld",    SIZE_QUAD,  0,		0 },
/*6*/	{ "fbstp",  SIZE_NONE,  0,		0 },
/*7*/	{ "fstp",   SIZE_QUAD,  0,		0 },
};

static struct finst *db_Esc_inst[] = {
	db_Esc8, db_Esc9, db_Esca, db_Escb,
	db_Escc, db_Escd, db_Esce, db_Escf
};

static char *	db_Grp1[] = {
	"add",
	"or",
	"adc",
	"sbb",
	"and",
	"sub",
	"xor",
	"cmp"
};

static char *	db_Grp2[] = {
	"rol",
	"ror",
	"rcl",
	"rcr",
	"shl",
	"shr",
	"shl",
	"sar"
};

static struct inst db_Grp3[] = {
	{ "test",  TRUE, SIZE_NONE, op2(I,E), 0 },
	{ "test",  TRUE, SIZE_NONE, op2(I,E), 0 },
	{ "not",   TRUE, SIZE_NONE, op1(E),   0 },
	{ "neg",   TRUE, SIZE_NONE, op1(E),   0 },
	{ "mul",   TRUE, SIZE_NONE, op2(E,A), 0 },
	{ "imul",  TRUE, SIZE_NONE, op2(E,A), 0 },
	{ "div",   TRUE, SIZE_NONE, op2(E,A), 0 },
	{ "idiv",  TRUE, SIZE_NONE, op2(E,A), 0 },
};

static struct inst	db_Grp4[] = {
	{ "inc",   TRUE, SIZE_BYTE, op1(E),   0 },
	{ "dec",   TRUE, SIZE_BYTE, op1(E),   0 },
	{ "",      TRUE, SIZE_NONE, 0,	 0 },
	{ "",      TRUE, SIZE_NONE, 0,	 0 },
	{ "",      TRUE, SIZE_NONE, 0,	 0 },
	{ "",      TRUE, SIZE_NONE, 0,	 0 },
	{ "",      TRUE, SIZE_NONE, 0,	 0 },
	{ "",      TRUE, SIZE_NONE, 0,	 0 }
};

static struct inst	db_Grp5[] = {
	{ "inc",   TRUE, SIZE_LONG, op1(E),   0 },
	{ "dec",   TRUE, SIZE_LONG, op1(E),   0 },
	{ "call",  TRUE, SIZE_NONE, op1(Eind),0 },
	{ "lcall", TRUE, SIZE_NONE, op1(Eind),0 },
	{ "jmp",   TRUE, SIZE_NONE, op1(Eind),0 },
	{ "ljmp",  TRUE, SIZE_NONE, op1(Eind),0 },
	{ "push",  TRUE, SIZE_LONG, op1(E),   0 },
	{ "",      TRUE, SIZE_NONE, 0,	 0 }
};

static struct inst db_inst_table[256] = {
/*00*/	{ "add",   TRUE,  SIZE_BYTE,  op2(R, E),  0 },
/*01*/	{ "add",   TRUE,  SIZE_LONG,  op2(R, E),  0 },
/*02*/	{ "add",   TRUE,  SIZE_BYTE,  op2(E, R),  0 },
/*03*/	{ "add",   TRUE,  SIZE_LONG,  op2(E, R),  0 },
/*04*/	{ "add",   FALSE, SIZE_BYTE,  op2(Is, A), 0 },
/*05*/	{ "add",   FALSE, SIZE_LONG,  op2(Is, A), 0 },
/*06*/	{ "push",  FALSE, SIZE_NONE,  op1(Si),    0 },
/*07*/	{ "pop",   FALSE, SIZE_NONE,  op1(Si),    0 },

/*08*/	{ "or",    TRUE,  SIZE_BYTE,  op2(R, E),  0 },
/*09*/	{ "or",    TRUE,  SIZE_LONG,  op2(R, E),  0 },
/*0a*/	{ "or",    TRUE,  SIZE_BYTE,  op2(E, R),  0 },
/*0b*/	{ "or",    TRUE,  SIZE_LONG,  op2(E, R),  0 },
/*0c*/	{ "or",    FALSE, SIZE_BYTE,  op2(I, A),  0 },
/*0d*/	{ "or",    FALSE, SIZE_LONG,  op2(I, A),  0 },
/*0e*/	{ "push",  FALSE, SIZE_NONE,  op1(Si),    0 },
/*0f*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },

/*10*/	{ "adc",   TRUE,  SIZE_BYTE,  op2(R, E),  0 },
/*11*/	{ "adc",   TRUE,  SIZE_LONG,  op2(R, E),  0 },
/*12*/	{ "adc",   TRUE,  SIZE_BYTE,  op2(E, R),  0 },
/*13*/	{ "adc",   TRUE,  SIZE_LONG,  op2(E, R),  0 },
/*14*/	{ "adc",   FALSE, SIZE_BYTE,  op2(Is, A), 0 },
/*15*/	{ "adc",   FALSE, SIZE_LONG,  op2(Is, A), 0 },
/*16*/	{ "push",  FALSE, SIZE_NONE,  op1(Si),    0 },
/*17*/	{ "pop",   FALSE, SIZE_NONE,  op1(Si),    0 },

/*18*/	{ "sbb",   TRUE,  SIZE_BYTE,  op2(R, E),  0 },
/*19*/	{ "sbb",   TRUE,  SIZE_LONG,  op2(R, E),  0 },
/*1a*/	{ "sbb",   TRUE,  SIZE_BYTE,  op2(E, R),  0 },
/*1b*/	{ "sbb",   TRUE,  SIZE_LONG,  op2(E, R),  0 },
/*1c*/	{ "sbb",   FALSE, SIZE_BYTE,  op2(Is, A), 0 },
/*1d*/	{ "sbb",   FALSE, SIZE_LONG,  op2(Is, A), 0 },
/*1e*/	{ "push",  FALSE, SIZE_NONE,  op1(Si),    0 },
/*1f*/	{ "pop",   FALSE, SIZE_NONE,  op1(Si),    0 },

/*20*/	{ "and",   TRUE,  SIZE_BYTE,  op2(R, E),  0 },
/*21*/	{ "and",   TRUE,  SIZE_LONG,  op2(R, E),  0 },
/*22*/	{ "and",   TRUE,  SIZE_BYTE,  op2(E, R),  0 },
/*23*/	{ "and",   TRUE,  SIZE_LONG,  op2(E, R),  0 },
/*24*/	{ "and",   FALSE, SIZE_BYTE,  op2(I, A),  0 },
/*25*/	{ "and",   FALSE, SIZE_LONG,  op2(I, A),  0 },
/*26*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },
/*27*/	{ "aaa",   FALSE, SIZE_NONE,  0,	     0 },

/*28*/	{ "sub",   TRUE,  SIZE_BYTE,  op2(R, E),  0 },
/*29*/	{ "sub",   TRUE,  SIZE_LONG,  op2(R, E),  0 },
/*2a*/	{ "sub",   TRUE,  SIZE_BYTE,  op2(E, R),  0 },
/*2b*/	{ "sub",   TRUE,  SIZE_LONG,  op2(E, R),  0 },
/*2c*/	{ "sub",   FALSE, SIZE_BYTE,  op2(Is, A), 0 },
/*2d*/	{ "sub",   FALSE, SIZE_LONG,  op2(Is, A), 0 },
/*2e*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },
/*2f*/	{ "das",   FALSE, SIZE_NONE,  0,	     0 },

/*30*/	{ "xor",   TRUE,  SIZE_BYTE,  op2(R, E),  0 },
/*31*/	{ "xor",   TRUE,  SIZE_LONG,  op2(R, E),  0 },
/*32*/	{ "xor",   TRUE,  SIZE_BYTE,  op2(E, R),  0 },
/*33*/	{ "xor",   TRUE,  SIZE_LONG,  op2(E, R),  0 },
/*34*/	{ "xor",   FALSE, SIZE_BYTE,  op2(I, A),  0 },
/*35*/	{ "xor",   FALSE, SIZE_LONG,  op2(I, A),  0 },
/*36*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },
/*37*/	{ "daa",   FALSE, SIZE_NONE,  0,	     0 },

/*38*/	{ "cmp",   TRUE,  SIZE_BYTE,  op2(R, E),  0 },
/*39*/	{ "cmp",   TRUE,  SIZE_LONG,  op2(R, E),  0 },
/*3a*/	{ "cmp",   TRUE,  SIZE_BYTE,  op2(E, R),  0 },
/*3b*/	{ "cmp",   TRUE,  SIZE_LONG,  op2(E, R),  0 },
/*3c*/	{ "cmp",   FALSE, SIZE_BYTE,  op2(Is, A), 0 },
/*3d*/	{ "cmp",   FALSE, SIZE_LONG,  op2(Is, A), 0 },
/*3e*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },
/*3f*/	{ "aas",   FALSE, SIZE_NONE,  0,	     0 },

/*40*/	{ "inc",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*41*/	{ "inc",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*42*/	{ "inc",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*43*/	{ "inc",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*44*/	{ "inc",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*45*/	{ "inc",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*46*/	{ "inc",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*47*/	{ "inc",   FALSE, SIZE_LONG,  op1(Ri),    0 },

/*48*/	{ "dec",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*49*/	{ "dec",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*4a*/	{ "dec",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*4b*/	{ "dec",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*4c*/	{ "dec",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*4d*/	{ "dec",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*4e*/	{ "dec",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*4f*/	{ "dec",   FALSE, SIZE_LONG,  op1(Ri),    0 },

/*50*/	{ "push",  FALSE, SIZE_LONG,  op1(Ri),    0 },
/*51*/	{ "push",  FALSE, SIZE_LONG,  op1(Ri),    0 },
/*52*/	{ "push",  FALSE, SIZE_LONG,  op1(Ri),    0 },
/*53*/	{ "push",  FALSE, SIZE_LONG,  op1(Ri),    0 },
/*54*/	{ "push",  FALSE, SIZE_LONG,  op1(Ri),    0 },
/*55*/	{ "push",  FALSE, SIZE_LONG,  op1(Ri),    0 },
/*56*/	{ "push",  FALSE, SIZE_LONG,  op1(Ri),    0 },
/*57*/	{ "push",  FALSE, SIZE_LONG,  op1(Ri),    0 },

/*58*/	{ "pop",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*59*/	{ "pop",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*5a*/	{ "pop",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*5b*/	{ "pop",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*5c*/	{ "pop",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*5d*/	{ "pop",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*5e*/	{ "pop",   FALSE, SIZE_LONG,  op1(Ri),    0 },
/*5f*/	{ "pop",   FALSE, SIZE_LONG,  op1(Ri),    0 },

/*60*/	{ "pusha", FALSE, SIZE_LONG,  0,	     0 },
/*61*/	{ "popa",  FALSE, SIZE_LONG,  0,	     0 },
/*62*/  { "bound", TRUE,  SIZE_LONG,  op2(E, R),  0 },
/*63*/	{ "arpl",  TRUE,  SIZE_NONE,  op2(Ew,Rw), 0 },

/*64*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },
/*65*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },
/*66*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },
/*67*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },

/*68*/	{ "push",  FALSE, SIZE_LONG,  op1(I),     0 },
/*69*/  { "imul",  TRUE,  SIZE_LONG,  op3(I,E,R), 0 },
/*6a*/	{ "push",  FALSE, SIZE_LONG,  op1(Ib),    0 },
/*6b*/  { "imul",  TRUE,  SIZE_LONG,  op3(Ibs,E,R),0 },
/*6c*/	{ "ins",   FALSE, SIZE_BYTE,  op2(DX, DI), 0 },
/*6d*/	{ "ins",   FALSE, SIZE_LONG,  op2(DX, DI), 0 },
/*6e*/	{ "outs",  FALSE, SIZE_BYTE,  op2(SI, DX), 0 },
/*6f*/	{ "outs",  FALSE, SIZE_LONG,  op2(SI, DX), 0 },

/*70*/	{ "jo",    FALSE, SIZE_NONE,  op1(Db),     0 },
/*71*/	{ "jno",   FALSE, SIZE_NONE,  op1(Db),     0 },
/*72*/	{ "jb",    FALSE, SIZE_NONE,  op1(Db),     0 },
/*73*/	{ "jnb",   FALSE, SIZE_NONE,  op1(Db),     0 },
/*74*/	{ "jz",    FALSE, SIZE_NONE,  op1(Db),     0 },
/*75*/	{ "jnz",   FALSE, SIZE_NONE,  op1(Db),     0 },
/*76*/	{ "jbe",   FALSE, SIZE_NONE,  op1(Db),     0 },
/*77*/	{ "jnbe",  FALSE, SIZE_NONE,  op1(Db),     0 },

/*78*/	{ "js",    FALSE, SIZE_NONE,  op1(Db),     0 },
/*79*/	{ "jns",   FALSE, SIZE_NONE,  op1(Db),     0 },
/*7a*/	{ "jp",    FALSE, SIZE_NONE,  op1(Db),     0 },
/*7b*/	{ "jnp",   FALSE, SIZE_NONE,  op1(Db),     0 },
/*7c*/	{ "jl",    FALSE, SIZE_NONE,  op1(Db),     0 },
/*7d*/	{ "jnl",   FALSE, SIZE_NONE,  op1(Db),     0 },
/*7e*/	{ "jle",   FALSE, SIZE_NONE,  op1(Db),     0 },
/*7f*/	{ "jnle",  FALSE, SIZE_NONE,  op1(Db),     0 },

/*80*/  { "",	   TRUE,  SIZE_BYTE,  op2(I, E),   (char *)db_Grp1 },
/*81*/  { "",	   TRUE,  SIZE_LONG,  op2(I, E),   (char *)db_Grp1 },
/*82*/  { "",	   TRUE,  SIZE_BYTE,  op2(Is,E),   (char *)db_Grp1 },
/*83*/  { "",	   TRUE,  SIZE_LONG,  op2(Ibs,E),  (char *)db_Grp1 },
/*84*/	{ "test",  TRUE,  SIZE_BYTE,  op2(R, E),   0 },
/*85*/	{ "test",  TRUE,  SIZE_LONG,  op2(R, E),   0 },
/*86*/	{ "xchg",  TRUE,  SIZE_BYTE,  op2(R, E),   0 },
/*87*/	{ "xchg",  TRUE,  SIZE_LONG,  op2(R, E),   0 },

/*88*/	{ "mov",   TRUE,  SIZE_BYTE,  op2(R, E),   0 },
/*89*/	{ "mov",   TRUE,  SIZE_LONG,  op2(R, E),   0 },
/*8a*/	{ "mov",   TRUE,  SIZE_BYTE,  op2(E, R),   0 },
/*8b*/	{ "mov",   TRUE,  SIZE_LONG,  op2(E, R),   0 },
/*8c*/  { "mov",   TRUE,  SIZE_NONE,  op2(S, Ew),  0 },
/*8d*/	{ "lea",   TRUE,  SIZE_LONG,  op2(E, R),   0 },
/*8e*/	{ "mov",   TRUE,  SIZE_NONE,  op2(Ew, S),  0 },
/*8f*/	{ "pop",   TRUE,  SIZE_LONG,  op1(E),      0 },

/*90*/	{ "nop",   FALSE, SIZE_NONE,  0,	      0 },
/*91*/	{ "xchg",  FALSE, SIZE_LONG,  op2(A, Ri),  0 },
/*92*/	{ "xchg",  FALSE, SIZE_LONG,  op2(A, Ri),  0 },
/*93*/	{ "xchg",  FALSE, SIZE_LONG,  op2(A, Ri),  0 },
/*94*/	{ "xchg",  FALSE, SIZE_LONG,  op2(A, Ri),  0 },
/*95*/	{ "xchg",  FALSE, SIZE_LONG,  op2(A, Ri),  0 },
/*96*/	{ "xchg",  FALSE, SIZE_LONG,  op2(A, Ri),  0 },
/*97*/	{ "xchg",  FALSE, SIZE_LONG,  op2(A, Ri),  0 },

/*98*/	{ "cbw",   FALSE, SIZE_SDEP,  0,	      "cwde" },	/* cbw/cwde */
/*99*/	{ "cwd",   FALSE, SIZE_SDEP,  0,	      "cdq"  },	/* cwd/cdq */
/*9a*/	{ "lcall", FALSE, SIZE_NONE,  op1(OS),     0 },
/*9b*/	{ "wait",  FALSE, SIZE_NONE,  0,	      0 },
/*9c*/	{ "pushf", FALSE, SIZE_LONG,  0,	      0 },
/*9d*/	{ "popf",  FALSE, SIZE_LONG,  0,	      0 },
/*9e*/	{ "sahf",  FALSE, SIZE_NONE,  0,	      0 },
/*9f*/	{ "lahf",  FALSE, SIZE_NONE,  0,	      0 },

/*a0*/	{ "mov",   FALSE, SIZE_BYTE,  op2(O, A),   0 },
/*a1*/	{ "mov",   FALSE, SIZE_LONG,  op2(O, A),   0 },
/*a2*/	{ "mov",   FALSE, SIZE_BYTE,  op2(A, O),   0 },
/*a3*/	{ "mov",   FALSE, SIZE_LONG,  op2(A, O),   0 },
/*a4*/	{ "movs",  FALSE, SIZE_BYTE,  op2(SI,DI),  0 },
/*a5*/	{ "movs",  FALSE, SIZE_LONG,  op2(SI,DI),  0 },
/*a6*/	{ "cmps",  FALSE, SIZE_BYTE,  op2(SI,DI),  0 },
/*a7*/	{ "cmps",  FALSE, SIZE_LONG,  op2(SI,DI),  0 },

/*a8*/	{ "test",  FALSE, SIZE_BYTE,  op2(I, A),   0 },
/*a9*/	{ "test",  FALSE, SIZE_LONG,  op2(I, A),   0 },
/*aa*/	{ "stos",  FALSE, SIZE_BYTE,  op1(DI),     0 },
/*ab*/	{ "stos",  FALSE, SIZE_LONG,  op1(DI),     0 },
/*ac*/	{ "lods",  FALSE, SIZE_BYTE,  op1(SI),     0 },
/*ad*/	{ "lods",  FALSE, SIZE_LONG,  op1(SI),     0 },
/*ae*/	{ "scas",  FALSE, SIZE_BYTE,  op1(SI),     0 },
/*af*/	{ "scas",  FALSE, SIZE_LONG,  op1(SI),     0 },

/*b0*/	{ "mov",   FALSE, SIZE_BYTE,  op2(I, Ri),  0 },
/*b1*/	{ "mov",   FALSE, SIZE_BYTE,  op2(I, Ri),  0 },
/*b2*/	{ "mov",   FALSE, SIZE_BYTE,  op2(I, Ri),  0 },
/*b3*/	{ "mov",   FALSE, SIZE_BYTE,  op2(I, Ri),  0 },
/*b4*/	{ "mov",   FALSE, SIZE_BYTE,  op2(I, Ri),  0 },
/*b5*/	{ "mov",   FALSE, SIZE_BYTE,  op2(I, Ri),  0 },
/*b6*/	{ "mov",   FALSE, SIZE_BYTE,  op2(I, Ri),  0 },
/*b7*/	{ "mov",   FALSE, SIZE_BYTE,  op2(I, Ri),  0 },

/*b8*/	{ "mov",   FALSE, SIZE_LONG,  op2(I, Ri),  0 },
/*b9*/	{ "mov",   FALSE, SIZE_LONG,  op2(I, Ri),  0 },
/*ba*/	{ "mov",   FALSE, SIZE_LONG,  op2(I, Ri),  0 },
/*bb*/	{ "mov",   FALSE, SIZE_LONG,  op2(I, Ri),  0 },
/*bc*/	{ "mov",   FALSE, SIZE_LONG,  op2(I, Ri),  0 },
/*bd*/	{ "mov",   FALSE, SIZE_LONG,  op2(I, Ri),  0 },
/*be*/	{ "mov",   FALSE, SIZE_LONG,  op2(I, Ri),  0 },
/*bf*/	{ "mov",   FALSE, SIZE_LONG,  op2(I, Ri),  0 },

/*c0*/	{ "",	   TRUE,  SIZE_BYTE,  op2(Ib, E),  (char *)db_Grp2 },
/*c1*/	{ "",	   TRUE,  SIZE_LONG,  op2(Ib, E),  (char *)db_Grp2 },
/*c2*/	{ "ret",   FALSE, SIZE_NONE,  op1(Iw),     0 },
/*c3*/	{ "ret",   FALSE, SIZE_NONE,  0,	      0 },
/*c4*/	{ "les",   TRUE,  SIZE_LONG,  op2(E, R),   0 },
/*c5*/	{ "lds",   TRUE,  SIZE_LONG,  op2(E, R),   0 },
/*c6*/	{ "mov",   TRUE,  SIZE_BYTE,  op2(I, E),   0 },
/*c7*/	{ "mov",   TRUE,  SIZE_LONG,  op2(I, E),   0 },

/*c8*/	{ "enter", FALSE, SIZE_NONE,  op2(Ib, Iw), 0 },
/*c9*/	{ "leave", FALSE, SIZE_NONE,  0,           0 },
/*ca*/	{ "lret",  FALSE, SIZE_NONE,  op1(Iw),     0 },
/*cb*/	{ "lret",  FALSE, SIZE_NONE,  0,	      0 },
/*cc*/	{ "int",   FALSE, SIZE_NONE,  op1(o3),     0 },
/*cd*/	{ "int",   FALSE, SIZE_NONE,  op1(Ib),     0 },
/*ce*/	{ "into",  FALSE, SIZE_NONE,  0,	      0 },
/*cf*/	{ "iret",  FALSE, SIZE_NONE,  0,	      0 },

/*d0*/	{ "",	   TRUE,  SIZE_BYTE,  op2(o1, E),  (char *)db_Grp2 },
/*d1*/	{ "",	   TRUE,  SIZE_LONG,  op2(o1, E),  (char *)db_Grp2 },
/*d2*/	{ "",	   TRUE,  SIZE_BYTE,  op2(CL, E),  (char *)db_Grp2 },
/*d3*/	{ "",	   TRUE,  SIZE_LONG,  op2(CL, E),  (char *)db_Grp2 },
/*d4*/	{ "aam",   TRUE,  SIZE_NONE,  0,	      0 },
/*d5*/	{ "aad",   TRUE,  SIZE_NONE,  0,	      0 },
/*d6*/	{ "",      FALSE, SIZE_NONE,  0,	      0 },
/*d7*/	{ "xlat",  FALSE, SIZE_BYTE,  op1(BX),     0 },

/*d8*/  { "",      TRUE,  SIZE_NONE,  0,	      (char *)db_Esc8 },
/*d9*/  { "",      TRUE,  SIZE_NONE,  0,	      (char *)db_Esc9 },
/*da*/  { "",      TRUE,  SIZE_NONE,  0,	      (char *)db_Esca },
/*db*/  { "",      TRUE,  SIZE_NONE,  0,	      (char *)db_Escb },
/*dc*/  { "",      TRUE,  SIZE_NONE,  0,	      (char *)db_Escc },
/*dd*/  { "",      TRUE,  SIZE_NONE,  0,	      (char *)db_Escd },
/*de*/  { "",      TRUE,  SIZE_NONE,  0,	      (char *)db_Esce },
/*df*/  { "",      TRUE,  SIZE_NONE,  0,	      (char *)db_Escf },

/*e0*/	{ "loopne",FALSE, SIZE_NONE,  op1(Db),     0 },
/*e1*/	{ "loope", FALSE, SIZE_NONE,  op1(Db),     0 },
/*e2*/	{ "loop",  FALSE, SIZE_NONE,  op1(Db),     0 },
/*e3*/	{ "jcxz",  FALSE, SIZE_SDEP,  op1(Db),     "jecxz" },
/*e4*/	{ "in",    FALSE, SIZE_BYTE,  op2(Ib, A),  0 },
/*e5*/	{ "in",    FALSE, SIZE_LONG,  op2(Ib, A) , 0 },
/*e6*/	{ "out",   FALSE, SIZE_BYTE,  op2(A, Ib),  0 },
/*e7*/	{ "out",   FALSE, SIZE_LONG,  op2(A, Ib) , 0 },

/*e8*/	{ "call",  FALSE, SIZE_NONE,  op1(Dl),     0 },
/*e9*/	{ "jmp",   FALSE, SIZE_NONE,  op1(Dl),     0 },
/*ea*/	{ "ljmp",  FALSE, SIZE_NONE,  op1(OS),     0 },
/*eb*/	{ "jmp",   FALSE, SIZE_NONE,  op1(Db),     0 },
/*ec*/	{ "in",    FALSE, SIZE_BYTE,  op2(DX, A),  0 },
/*ed*/	{ "in",    FALSE, SIZE_LONG,  op2(DX, A) , 0 },
/*ee*/	{ "out",   FALSE, SIZE_BYTE,  op2(A, DX),  0 },
/*ef*/	{ "out",   FALSE, SIZE_LONG,  op2(A, DX) , 0 },

/*f0*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },
/*f1*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },
/*f2*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },
/*f3*/	{ "",      FALSE, SIZE_NONE,  0,	     0 },
/*f4*/	{ "hlt",   FALSE, SIZE_NONE,  0,	     0 },
/*f5*/	{ "cmc",   FALSE, SIZE_NONE,  0,	     0 },
/*f6*/	{ "",      TRUE,  SIZE_BYTE,  0,	     (char *)db_Grp3 },
/*f7*/	{ "",	   TRUE,  SIZE_LONG,  0,	     (char *)db_Grp3 },

/*f8*/	{ "clc",   FALSE, SIZE_NONE,  0,	     0 },
/*f9*/	{ "stc",   FALSE, SIZE_NONE,  0,	     0 },
/*fa*/	{ "cli",   FALSE, SIZE_NONE,  0,	     0 },
/*fb*/	{ "sti",   FALSE, SIZE_NONE,  0,	     0 },
/*fc*/	{ "cld",   FALSE, SIZE_NONE,  0,	     0 },
/*fd*/	{ "std",   FALSE, SIZE_NONE,  0,	     0 },
/*fe*/	{ "",	   TRUE,  SIZE_NONE,  0,	     (char *)db_Grp4 },
/*ff*/	{ "",	   TRUE,  SIZE_NONE,  0,	     (char *)db_Grp5 },
};

static struct inst	db_bad_inst =
	{ "???",   FALSE, SIZE_NONE,  0,	      0 }
;

#define	f_mod(byte)	((byte)>>6)
#define	f_reg(byte)	(((byte)>>3)&0x7)
#define	f_rm(byte)	((byte)&0x7)

#define	sib_ss(byte)	((byte)>>6)
#define	sib_index(byte)	(((byte)>>3)&0x7)
#define	sib_base(byte)	((byte)&0x7)

struct i_addr {
	int		is_reg;	/* if reg, reg number is in 'disp' */
	int		disp;
	char *		base;
	char *		index;
	int		ss;
};

static char *	db_index_reg_16[8] = {
	"%bx,%si",
	"%bx,%di",
	"%bp,%si",
	"%bp,%di",
	"%si",
	"%di",
	"%bp",
	"%bx"
};

static char *	db_reg[3][8] = {
	{ "%al",  "%cl",  "%dl",  "%bl",  "%ah",  "%ch",  "%dh",  "%bh" },
	{ "%ax",  "%cx",  "%dx",  "%bx",  "%sp",  "%bp",  "%si",  "%di" },
	{ "%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi" }
};

static char *	db_seg_reg[8] = {
	"%es", "%cs", "%ss", "%ds", "%fs", "%gs", "", ""
};

/*
 * lengths for size attributes
 */
static int db_lengths[] = {
	1,	/* SIZE_BYTE */
	2,	/* SIZE_WORD */
	4,	/* SIZE_LONG */
	8,	/* SIZE_QUAD */
	4,	/* SIZE_SNGL */
	8,	/* SIZE_DBLR */
	10,	/* SIZE_EXTR */
};

static db_disable_print = 0;

void db_printf(const char *fmt, ...)
{
    va_list ap;

    if (!db_disable_print)
    {
	va_start(ap, fmt);
	vfprintf(debug_outfp, fmt, ap);
	va_end(ap);
    }
}

static unsigned int db_get_task_value(unsigned int loc, int size, int is_signed)
{
  unsigned int result;
  switch(size)
    {
    case 4:
      if (is_signed)
	result = (unsigned int) *((int *) loc);
      else
	result = (unsigned int) *((unsigned int *) loc);
      break;
    case 2:
      if (is_signed)
	result = (unsigned int) *((short int *) loc);
      else
	result = *((unsigned short int *) loc);
      break;
    case 1:
      if (is_signed)
	result = (unsigned int) *((char *) loc);
      else
	result = *((unsigned char *) loc);
      break;
    default:
      db_printf("Illegal size specified\n");
      result = 0;
      break;
    }
  return result;
}

#define	get_value_inc(result, loc, size, is_signed) \
	result = db_get_task_value((loc), (size), (is_signed)); \
        (loc) += (size);

/*
 * Read address at location and return updated location.
 */
db_addr_t
db_read_address(loc, short_addr, regmodrm, addrp)
	db_addr_t	loc;
	int		short_addr;
	int		regmodrm;
	struct i_addr	*addrp;		/* out */
{
	int		mod, rm, sib, index, disp;

	mod = f_mod(regmodrm);
	rm  = f_rm(regmodrm);

	if (mod == 3) {
	    addrp->is_reg = TRUE;
	    addrp->disp = rm;
	    return (loc);
	}
	addrp->is_reg = FALSE;
	addrp->index = 0;

	if (short_addr) {
	    addrp->index = 0;
	    addrp->ss = 0;
	    switch (mod) {
		case 0:
		    if (rm == 6) {
			get_value_inc(disp, loc, 2, TRUE);
			addrp->disp = disp;
			addrp->base = 0;
		    }
		    else {
			addrp->disp = 0;
			addrp->base = db_index_reg_16[rm];
		    }
		    break;
		case 1:
		    get_value_inc(disp, loc, 1, TRUE);
		    addrp->disp = disp;
		    addrp->base = db_index_reg_16[rm];
		    break;
		case 2:
		    get_value_inc(disp, loc, 2, TRUE);
		    addrp->disp = disp;
		    addrp->base = db_index_reg_16[rm];
		    break;
	    }
	}
	else {
	    if (mod != 3 && rm == 4) {
		get_value_inc(sib, loc, 1, FALSE);
		rm = sib_base(sib);
		index = sib_index(sib);
		if (index != 4)
		    addrp->index = db_reg[SIZE_LONG][index];
		addrp->ss = sib_ss(sib);
	    }

	    switch (mod) {
		case 0:
		    if (rm == 5) {
			get_value_inc(addrp->disp, loc, 4, FALSE);
			addrp->base = 0;
		    }
		    else {
			addrp->disp = 0;
			addrp->base = db_reg[SIZE_LONG][rm];
		    }
		    break;

		case 1:
		    get_value_inc(disp, loc, 1, TRUE);
		    addrp->disp = disp;
		    addrp->base = db_reg[SIZE_LONG][rm];
		    break;

		case 2:
		    get_value_inc(disp, loc, 4, FALSE);
		    addrp->disp = disp;
		    addrp->base = db_reg[SIZE_LONG][rm];
		    break;
	    }
	}
	return (loc);
}

static void db_task_printsym(unsigned int addr, int size)
{
    switch(size)
    {
    case SIZE_BYTE:
        db_printf("0x%2.2x", addr & 0xff );
        break;
    case SIZE_WORD:
        db_printf("0x%4.4x", addr & 0xffff );
        break;
    case SIZE_LONG:
	if (!db_disable_print)
	    TWIN_print_address(addr, debug_outfp, db_disasm_16 ? 16 : 32);
        break;
    }
}

void
db_print_address(seg, size, addrp)
	char *		seg;
	int		size;
	struct i_addr	*addrp;
{
	if (addrp->is_reg) {
	    db_printf("%s", db_reg[size][addrp->disp]);
	    return;
	}

	if (seg) {
	    db_printf("%s:", seg);
	}

	if (addrp->base != 0 || addrp->index != 0) {
	    db_printf("0x%x", addrp->disp);
	    db_printf("(");
	    if (addrp->base)
		db_printf("%s", addrp->base);
	    if (addrp->index)
		db_printf(",%s,%d", addrp->index, 1<<addrp->ss);
	    db_printf(")");
	} else
	    db_task_printsym((db_addr_t)addrp->disp, size);
}

/*
 * Disassemble floating-point ("escape") instruction
 * and return updated location.
 */
db_addr_t
db_disasm_esc(loc, inst, short_addr, size, seg)
	db_addr_t	loc;
	int		inst;
	int		short_addr;
	int		size;
	char *		seg;
{
	int		regmodrm;
	struct finst	*fp;
	int		mod;
	struct i_addr	address;
	char *		name;

	get_value_inc(regmodrm, loc, 1, FALSE);
	fp = &db_Esc_inst[inst - 0xd8][f_reg(regmodrm)];
	mod = f_mod(regmodrm);
	if (mod != 3) {
	    /*
	     * Normal address modes.
	     */
	    loc = db_read_address(loc, short_addr, regmodrm, &address);
	    db_printf(fp->f_name);
	    switch(fp->f_size) {
		case SIZE_SNGL:
		    db_printf("s");
		    break;
		case SIZE_DBLR:
		    db_printf("l");
		    break;
		case SIZE_EXTR:
		    db_printf("t");
		    break;
		case SIZE_WORD:
		    db_printf("s");
		    break;
		case SIZE_LONG:
		    db_printf("l");
		    break;
		case SIZE_QUAD:
		    db_printf("q");
		    break;
		default:
		    break;
	    }
	    db_printf("\t");
	    db_print_address(seg, SIZE_BYTE, &address);
	}
	else {
	    /*
	     * 'reg-reg' - special formats
	     */
	    switch (fp->f_rrmode) {
		case op2(ST,STI):
		    name = (fp->f_rrname) ? fp->f_rrname : fp->f_name;
		    db_printf("%s\t%%st,%%st(%d)",name,f_rm(regmodrm));
		    break;
		case op2(STI,ST):
		    name = (fp->f_rrname) ? fp->f_rrname : fp->f_name;
		    db_printf("%s\t%%st(%d),%%st",name, f_rm(regmodrm));
		    break;
		case op1(STI):
		    name = (fp->f_rrname) ? fp->f_rrname : fp->f_name;
		    db_printf("%s\t%%st(%d)",name, f_rm(regmodrm));
		    break;
		case op1(X):
		    db_printf("%s", ((char **)fp->f_rrname)[f_rm(regmodrm)]);
		    break;
		case op1(XA):
		    db_printf("%s\t%%ax",
				 ((char **)fp->f_rrname)[f_rm(regmodrm)]);
		    break;
		default:
		    db_printf("<bad instruction>");
		    break;
	    }
	}

	return (loc);
}

/*
 * Disassemble instruction at 'loc'.  'altfmt' specifies an
 * (optional) alternate format.  Return address of start of
 * next instruction.
 */
db_addr_t
db_disasm(loc, altfmt, flag16, flagnoprint)
	db_addr_t	loc;
	boolean_t	altfmt;
        boolean_t       flag16;
        boolean_t       flagnoprint;
{
	int	inst;
	int	size;
	int	short_addr;
	char *	seg;
	struct inst *	ip;
	char *	i_name;
	int	i_size;
	int	i_mode;
	int	regmodrm = 0;
	boolean_t	first;
	int	displ;
	int	prefix;
	int	imm;
	int	imm2;
	int	len;
	struct i_addr	address;

	db_disasm_16 = flag16;
	db_disable_print = flagnoprint;

	get_value_inc(inst, loc, 1, FALSE);

	if (db_disasm_16) {
	    short_addr = TRUE;
	    size = SIZE_WORD;
	}
	else {
	    short_addr = FALSE;
	    size = SIZE_LONG;
	}
	seg = 0;

	/*
	 * Get prefixes
	 */
	prefix = TRUE;
	do {
	    switch (inst) {
		case 0x66:		/* data16 */
		    if (size == SIZE_LONG)
			size = SIZE_WORD;
		    else
			size = SIZE_LONG;
		    break;
		case 0x67:
		    short_addr = !short_addr;
		    break;
		case 0x26:
		    seg = "%es";
		    break;
		case 0x36:
		    seg = "%ss";
		    break;
		case 0x2e:
		    seg = "%cs";
		    break;
		case 0x3e:
		    seg = "%ds";
		    break;
		case 0x64:
		    seg = "%fs";
		    break;
		case 0x65:
		    seg = "%gs";
		    break;
		case 0xf0:
		    db_printf("lock ");
		    break;
		case 0xf2:
		    db_printf("repne ");
		    break;
		case 0xf3:
		    db_printf("repe ");	/* XXX repe VS rep */
		    break;
		default:
		    prefix = FALSE;
		    break;
	    }
	    if (prefix) {
		get_value_inc(inst, loc, 1, FALSE);
	    }
	} while (prefix);

	if (inst >= 0xd8 && inst <= 0xdf) {
	    loc = db_disasm_esc(loc, inst, short_addr, size, seg);
	    db_printf("\n");
	    return (loc);
	}

	if (inst == 0x0f) {
	    get_value_inc(inst, loc, 1, FALSE);
	    ip = db_inst_0f[inst>>4];
	    if (ip == 0) {
		ip = &db_bad_inst;
	    }
	    else {
		ip = &ip[inst&0xf];
	    }
	}
	else
	    ip = &db_inst_table[inst];

	if (ip->i_has_modrm) {
	    get_value_inc(regmodrm, loc, 1, FALSE);
	    loc = db_read_address(loc, short_addr, regmodrm, &address);
	}

	i_name = ip->i_name;
	i_size = ip->i_size;
	i_mode = ip->i_mode;

	if (ip->i_extra == (char *)db_Grp1 ||
	    ip->i_extra == (char *)db_Grp2 ||
	    ip->i_extra == (char *)db_Grp6 ||
	    ip->i_extra == (char *)db_Grp7 ||
	    ip->i_extra == (char *)db_Grp8) {
	    i_name = ((char **)ip->i_extra)[f_reg(regmodrm)];
	}
	else if (ip->i_extra == (char *)db_Grp3) {
	    ip = (struct inst *)ip->i_extra;
	    ip = &ip[f_reg(regmodrm)];
	    i_name = ip->i_name;
	    i_mode = ip->i_mode;
	}
	else if (ip->i_extra == (char *)db_Grp4 ||
		 ip->i_extra == (char *)db_Grp5) {
	    ip = (struct inst *)ip->i_extra;
	    ip = &ip[f_reg(regmodrm)];
	    i_name = ip->i_name;
	    i_mode = ip->i_mode;
	    i_size = ip->i_size;
	}

	if (i_size == SIZE_SDEP) {
	    if (size == SIZE_WORD)
		db_printf(i_name);
	    else
		db_printf(ip->i_extra);
	}
	else {
	    db_printf(i_name);
	    if (i_size != SIZE_NONE) {
		if (i_size == SIZE_BYTE) {
		    db_printf("b");
		    size = SIZE_BYTE;
		}
		else if (i_size == SIZE_WORD) {
		    db_printf("w");
		    size = SIZE_WORD;
		}
		else if (size == SIZE_WORD)
		    db_printf("w");
		else
		    db_printf("l");
	    }
	}
	db_printf("\t");
	for (first = TRUE;
	     i_mode != 0;
	     i_mode >>= 8, first = FALSE)
	{
	    if (!first)
		db_printf(",");

	    switch (i_mode & 0xFF) {

		case E:
		    db_print_address(seg, size, &address);
		    break;

		case Eind:
		    db_printf("*");
		    db_print_address(seg, size, &address);
		    break;

		case Ew:
		    db_print_address(seg, SIZE_WORD, &address);
		    break;

		case Eb:
		    db_print_address(seg, SIZE_BYTE, &address);
		    break;

		case R:
		    db_printf("%s", db_reg[size][f_reg(regmodrm)]);
		    break;

		case Rw:
		    db_printf("%s", db_reg[SIZE_WORD][f_reg(regmodrm)]);
		    break;

		case Ri:
		    db_printf("%s", db_reg[size][f_rm(inst)]);
		    break;

		case S:
		    db_printf("%s", db_seg_reg[f_reg(regmodrm)]);
		    break;

		case Si:
		    db_printf("%s", db_seg_reg[f_reg(inst)]);
		    break;

		case A:
		    db_printf("%s", db_reg[size][0]);	/* acc */
		    break;

		case BX:
		    if (seg)
			db_printf("%s:", seg);
		    db_printf("(%s)", short_addr ? "%bx" : "%ebx");
		    break;

		case CL:
		    db_printf("%%cl");
		    break;

		case DX:
		    db_printf("%%dx");
		    break;

		case SI:
		    if (seg)
			db_printf("%s:", seg);
		    db_printf("(%s)", short_addr ? "%si" : "%esi");
		    break;

		case DI:
		    db_printf("%%es:(%s)", short_addr ? "%di" : "%edi");
		    break;

		case CR:
		    db_printf("%%cr%d", f_reg(regmodrm));
		    break;

		case DR:
		    db_printf("%%dr%d", f_reg(regmodrm));
		    break;

		case TR:
		    db_printf("%%tr%d", f_reg(regmodrm));
		    break;

		case I:
		    len = db_lengths[size];
		    get_value_inc(imm, loc, len, FALSE);/* unsigned */
		    db_printf("$0x%x", imm);
		    break;

		case Is:
		    len = db_lengths[size];
		    get_value_inc(imm, loc, len, TRUE);	/* signed */
		    db_printf("$%d", imm);
		    break;

		case Ib:
		    get_value_inc(imm, loc, 1, FALSE);	/* unsigned */
		    db_printf("$0x%x", imm);
		    break;

		case Ibs:
		    get_value_inc(imm, loc, 1, TRUE);	/* signed */
		    db_printf("$%d", imm);
		    break;

		case Iw:
		    get_value_inc(imm, loc, 2, FALSE);	/* unsigned */
		    db_printf("$0x%x", imm);
		    break;

		case Il:
		    get_value_inc(imm, loc, 4, FALSE);
		    db_printf("$0x%x", imm);
		    break;

		case O:
		    if (short_addr) {
			get_value_inc(displ, loc, 2, TRUE);
		    }
		    else {
			get_value_inc(displ, loc, 4, TRUE);
		    }
		    if (seg)
			db_printf("%s:%d",seg, displ);
		    else
			db_task_printsym((db_addr_t)displ,
                                         short_addr ? SIZE_WORD : SIZE_LONG);
		    break;

		case Db:
		    get_value_inc(displ, loc, 1, TRUE);
		    displ = displ + loc;
		    db_task_printsym((db_addr_t)displ, SIZE_LONG);
		    break;

		case Dl:
		    if (short_addr) {
			get_value_inc(displ, loc, 2, TRUE);
			displ = displ + loc;
		    }
		    else {
			get_value_inc(displ, loc, 4, TRUE);
			displ = displ + loc;
		    }
		    db_task_printsym((db_addr_t)displ, SIZE_LONG);
		    break;

		case o1:
		    db_printf("$1");
		    break;

		case o3:
		    db_printf("$3");
		    break;

		case OS:
		    if (short_addr) {
			get_value_inc(imm, loc, 2, FALSE); /* offset */
		    }
		    else {
			get_value_inc(imm, loc, 4, FALSE); /* offset */
		    }
		    get_value_inc(imm2, loc, 2, FALSE);	/* segment */

		    if (!db_disable_print)
			TWIN_print_address((unsigned int) GetAddress(imm2, imm),
				debug_outfp, db_disasm_16 ? 16 : 32);
		    break;
	    }
	}

	if (altfmt == 0 && !db_disasm_16) {
	    if (inst == 0xe9 || inst == 0xeb) {
		/*
		 * GAS pads to longword boundary after unconditional jumps.
		 */
#if 0
		loc = (loc + (4-1)) & ~(4-1);
#endif
	    }
	}
	return (loc);
}

