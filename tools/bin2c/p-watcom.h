/* Portable include file for Watcom C */

#ifdef __WATCOMC__

#ifndef __LINUX__
#include <direct.h>
#endif

#include <stdlib.h>


/* Global argc/argv variables */
#define G_ARGV _argv		/* global char **argv	variable */
#define G_ARGC _argc		/* global int argc	variable */
#define G_ARG_DEF			/* definition of global variables */
#define G_ARG_INIT			/* initialization of global variables */



/* byte/word/dword type */
typedef unsigned char byte;
typedef unsigned word;
typedef unsigned FLAG;
typedef unsigned FLAG8;
typedef unsigned long dword;

#ifndef _CLIB_
#define peekb(segm,ofs) (*(byte __far*)MK_FP((segm),(ofs)))
#define peekw(segm,ofs) (*(word __far*)MK_FP((segm),(ofs)))
#define pokeb(segm,ofs,value) (peekb((segm),(ofs)) = (byte)(value))
#define pokew(segm,ofs,value) (peekw((segm),(ofs)) = (word)(value))
#endif


#define _osversion MK_OSVERS(_osmajor, _osminor)

/* get/set interrupt vector */
typedef void (__interrupt __far *isr)();
#define set_isrfct(num,fct) _dos_setvect((num), (isr)(fct))
#define get_isr(num,var) ((var) = _dos_getvect(num))
#define set_isr(num,var) _dos_setvect((num), (var))

/* calling an interrupt */
typedef struct {
	unsigned int	r_ax;
	unsigned int	r_bx;
	unsigned int	r_cx;
	unsigned int	r_dx;
    unsigned int    r_bp;
    unsigned int    r_si;
    unsigned int    r_di;
    unsigned int    r_ds;
    unsigned int    r_es;
    unsigned int    r_flags;
} IREGS;
#define intrpt(num,regs) intr((num), (union REGPACK*)(regs))

/* get/set current working drive */
extern short getdisk(void);
#pragma aux getdisk = \
	"mov ah, 19h"	\
	"int 21h"	\
	"xor ah, ah"	\
	value [ax]

extern short setdisk(int newdrive);
#pragma aux setdisk = \
	"mov ah, 0eh"	\
	"int 21h"	\
	"xor ah, ah"	\
	parm [dx]	\
	modify [dx]	\
	value [ax]

#endif
