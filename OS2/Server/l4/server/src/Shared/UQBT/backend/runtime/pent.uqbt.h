/*==============================================================================
 * FILE:       pent.uqbt.h
 * OVERVIEW:   Provides macros and definitions for the UQBT binary translator's
 *             generated code for Pentium targets
 *
 * Copyright (C) 1999-2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * $revision$
 *
 * 13 Feb 01 - Mike: Fixed swap8f() macro (needed heavy duty cast)
 */

#define int32   int
#define int16   short
#define int8    char
#define int64   long long
#define float32 float
#define float64 double
#define floatmax long double

/* Copy the source (if required) to register ax (a stands for al, ax, or eax
	as required). Exchange top and bottom halves. The result is %ax.
	Doing the copy in c (as opposed to emitting a move instruction) has two
	benefits: it will not be done if the source is already ax, and it will
	handle sources like %edi with the cast. */
/* Note that these two macros require gcc, if they are used */
#define _swap2(src) ( \
{	register short reg = (short)src; \
	asm("xchgb %%al, %%ah" : "+a" (reg)); \
	reg;} \
)

/* First copy the source to a general purpose register (eax, edx, esi, etc) (if
	required). Use the bswap instruction to do the byte swapping.
	Result is the register. */
#define _swap4(src) ( \
{   register int reg = src; \
    asm("bswap %0" : "+r" (reg)); \
    reg;} \
)

/* First copy the source into two GP registers; swap, and store to the int[2]
  array _res8. Result is res8 */
#define _swap8(src) ( \
{   register int reg1 = *(int*)&(src); \
    register int reg2 = *(((int*)&(src))+1); \
    asm("bswap %0" : "+r" (reg1)); \
    asm("bswap %0" : "+r" (reg2)); \
    _res8[0]=reg2; \
    _res8[1]=reg1; \
    _res8;} \
)
#define _swap8i(x) _swap8(x)
#define _swap8f(x) (*(float64*)&_swap8(x))
    
extern unsigned _uqbt_nat;           /* Used by register call runtime logic */
