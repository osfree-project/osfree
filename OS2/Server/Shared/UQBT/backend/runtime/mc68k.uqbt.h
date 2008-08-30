/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2000, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       mc68k.uqbt.h
 * OVERVIEW:   Provides macros and definitions for the UQBT binary translator's
 *             generated code for Motorola 68328 targets
 *
 * Copyright (C) 2000, The University of Queensland, BT group
 * Copyright (C) 2000, Sun Microsystems, Inc
 *============================================================================*/

#include <PalmOS.h>         /* Assumes SDK 3.5 */

#define int32   long
#define int16   short
#define int8    char
#define int64   long
#define float32 float
#define float64 double
#define floatmax double

/* Note: this is a copy of the x86 swap code. Haven't taken the time to get
  this to work for actual 68K compilers, since we don't even have one at
  present */

/* Copy the source (if required) to register ax (a stands for al, ax, or eax
	as required). Exchange top and bottom halves. The result is %ax.
	Doing the copy in c (as opposed to emitting a move instruction) has two
	benefits: it will not be done if the source is already ax, and it will
	handle sources like %edi with the cast. */
#define _swap2(src) ( \
{	register short reg = (short)src; \
	asm("swap %%al, %%ah" : "+a" (reg)); \
	reg;} \
)

/* First copy the source to a general purpose register (eax, edx, esi, etc) (if
	required). Use the bswap instruction to do the byte swapping.
	Result is the register. */
#define _swap4(src) ( \
{       register int reg = src; \
        asm("bswap %0" : "+r" (reg)); \
        reg;} \
)

