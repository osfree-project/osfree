/*
 *  i86.h       Defines the structs and unions used to handle the input and
 *              output registers for the Intel x86 and 386 interrupt interface
 *              routines.
 *
 *                          Open Watcom Project
 *
 *    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
 *
 *  ========================================================================
 *
 *    This file contains Original Code and/or Modifications of Original
 *    Code as defined in and that are subject to the Sybase Open Watcom
 *    Public License version 1.0 (the 'License'). You may not use this file
 *    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
 *    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
 *    provided with the Original Code and Modifications, and is also
 *    available at www.sybase.com/developer/opensource.
 *
 *    The Original Code and all software distributed under the License are
 *    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 *    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
 *    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
 *    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
 *    NON-INFRINGEMENT. Please see the License for the specific language
 *    governing rights and limitations under the License.
 *
 *  ========================================================================
 */
#ifndef _I86_H_INCLUDED
#define _I86_H_INCLUDED
#if !defined(_ENABLE_AUTODEPEND)
  #pragma read_only_file;
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if !defined(_M_IX86)
#error i86.h is for use with Intel platforms
#endif

#ifndef _COMDEF_H_INCLUDED
 #include "_comdef.h"
#endif

#if defined(_M_IX86)
  #pragma pack(__push,1);
#else
  #pragma pack(__push,8);
#endif
/* dword registers */

struct DWORDREGS {
        unsigned int eax;
        unsigned int ebx;
        unsigned int ecx;
        unsigned int edx;
        unsigned int esi;
        unsigned int edi;
        unsigned int cflag;
};
#define __FILLER(a) unsigned short a;

/* word registers */

struct WORDREGS {
        unsigned short ax;  __FILLER(_1)
        unsigned short bx;  __FILLER(_2)
        unsigned short cx;  __FILLER(_3)
        unsigned short dx;  __FILLER(_4)
        unsigned short si;  __FILLER(_5)
        unsigned short di;  __FILLER(_6)
        unsigned int cflag;
};

/* byte registers */

struct BYTEREGS {
        unsigned char al, ah;  __FILLER(_1)
        unsigned char bl, bh;  __FILLER(_2)
        unsigned char cl, ch;  __FILLER(_3)
        unsigned char dl, dh;  __FILLER(_4)
};

/* general purpose registers union - overlays the corresponding dword,
 * word, and byte registers.
 */

union REGS {
        struct DWORDREGS x;
        struct WORDREGS  w;
        struct BYTEREGS  h;
};
#define _REGS REGS

/* segment registers */

struct SREGS {
        unsigned short es, cs, ss, ds;
        unsigned short fs, gs;
};
#define _SREGS SREGS


/* intr structs */

struct REGPACKB {
        unsigned char al, ah;  __FILLER(_1)
        unsigned char bl, bh;  __FILLER(_2)
        unsigned char cl, ch;  __FILLER(_3)
        unsigned char dl, dh;  __FILLER(_4)
};

struct REGPACKW {
        unsigned short ax;  __FILLER(_1)
        unsigned short bx;  __FILLER(_2)
        unsigned short cx;  __FILLER(_3)
        unsigned short dx;  __FILLER(_4)
        unsigned short bp;  __FILLER(_5)
        unsigned short si;  __FILLER(_6)
        unsigned short di;  __FILLER(_7)
        unsigned short ds;
        unsigned short es;
        unsigned short fs;
        unsigned short gs;
        unsigned int flags;
};

struct REGPACKX {
        unsigned int   eax, ebx, ecx, edx, ebp, esi, edi;
        unsigned short ds, es, fs, gs;
        unsigned int   flags;
};

union REGPACK {
        struct REGPACKB h;
        struct REGPACKW w;
        struct REGPACKX x;
};

/* input parm to an 'interrupt' function is union INTPACK */
/* e.g.  interrupt int10( union INTPACK r ) {}            */

struct INTPACKX {
        unsigned gs,fs,es,ds,edi,esi,ebp,esp,ebx,edx,ecx,eax,eip,cs,flags;
};
/*
   NOTE: The gs and fs fields will not be correct unless the compiler
         is invoked with at least the /3 switch.
*/
struct INTPACKW {
        unsigned short gs;  __FILLER(_1)
        unsigned short fs;  __FILLER(_2)
        unsigned short es;  __FILLER(_3)
        unsigned short ds;  __FILLER(_4)
        unsigned short di;  __FILLER(_5)
        unsigned short si;  __FILLER(_6)
        unsigned short bp;  __FILLER(_7)
        unsigned short sp;  __FILLER(_8)
        unsigned short bx;  __FILLER(_9)
        unsigned short dx;  __FILLER(_a)
        unsigned short cx;  __FILLER(_b)
        unsigned short ax;  __FILLER(_c)
        unsigned short ip;  __FILLER(_d)
        unsigned short cs;  __FILLER(_e)
        unsigned flags;
};
struct INTPACKB {
        unsigned /*gs*/ :32,/*fs*/ :32,
                 /*es*/ :32,/*ds*/ :32,
                 /*edi*/:32,/*esi*/:32,
                 /*ebp*/:32,/*esp*/:32;
        unsigned char bl, bh; __FILLER(_1)
        unsigned char dl, dh; __FILLER(_2)
        unsigned char cl, ch; __FILLER(_3)
         unsigned char al, ah; __FILLER(_4)
};
union  INTPACK {
        struct INTPACKB h;
        struct INTPACKW w;
        struct INTPACKX x;
};

/* bits defined for flags field defined in REGPACKW and INTPACKW */

enum {
    INTR_CF     = 0x0001,       /* carry */
    INTR_PF     = 0x0004,       /* parity */
    INTR_AF     = 0x0010,       /* auxiliary carry */
    INTR_ZF     = 0x0040,       /* zero */
    INTR_SF     = 0x0080,       /* sign */
    INTR_TF     = 0x0100,       /* trace */
    INTR_IF     = 0x0200,       /* interrupt */
    INTR_DF     = 0x0400,       /* direction */
    INTR_OF     = 0x0800        /* overflow */
};

extern void _disable( void );
extern void _enable( void );

extern void    delay(unsigned int __milliseconds);
extern int     int386( int, union REGS *, union REGS * );
extern int     int386x( int, union REGS *, union REGS *,
                                  struct SREGS * );
extern void    intr( int, union REGPACK * );
extern void __cdecl  segread( struct SREGS * );

/* macros to break 'far' pointers into segment and offset components */

#define  FP_OFF(__p) ((unsigned)(__p))
#define _FP_OFF(__p) ((unsigned)(__p))

  unsigned short FP_SEG( const volatile void __far * );
  #pragma aux    FP_SEG = __parm __caller [ax dx] __value [dx];
#define _FP_SEG FP_SEG

/* make a far pointer from segment and offset */
#define MK_FP(__s,__o) (((unsigned short)(__s)):>((void __near *)(__o)))

#pragma pack(__pop);
#ifdef __cplusplus
};
#endif /* __cplusplus */
#endif
