/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/*
 * $Id$
 */

#ifndef RHBVARGS_H
#define RHBVARGS_H

/*************************************************************
 * this can be the trickiest part of porting to a new CPU
 *   ( not actually true, having callable method tokens is )
 *
 * the goal is be to able to build va_lists on the fly so that
 * the platform va_arg will be able to pull the arguments
 *
 * this is *not* a supported feature of any C standard so we
 * have to do it the hard way
 *
 * the first thing to determine is if va_list is a pointer or not,
 *  MkLinux/powerpc, 64bit-x86 and DEC Alpha are not, this requires extra special handling
 *
 *  the next is the alignment, what is the minimum alignment eg,
 *
 * 16 bit, eg 8086, 80286, alignment is normally 16 bit 'int'.
 * 68000, 80386, sparcv7 etc, alignment is a 4 byte 'int',
 * for 32bit mips, it is 8 bytes,
 * for LP64 alignment is normally 8 bytes
 *
 * finally, we have the stack direction and argument order, 
 * majority use grow-downwards stacks, some HP use grow-upwards, 
 * C calling convention dictates push right to left (eg first to pop is the first arg, last pushed)
 *
 * where a platform uses a struct etc, we have to build the va_list as if all the
 * contents are in the overflow area, eg some conventions have some args held in registers
 * these are no use for us, so pretend that the registers are 'already used'
 */

/*********************************
 * MkLinux/powerpc looks like...
 * typedef struct __va_list_tag
 * {
 *	unsigned char gpr;
 *	unsigned char fpr;
 *	char *overflow_arg_area;
 *	char *reg_save_area;
 * } va_list_tag[1];
 *
 * #define va_list  va_list_tag
 * #define __va_overflow(ap)   (ap)->overflow_arg_area
 */

/*****************************************
 * NetBSD/alpha looks like....
 * typedef struct
 * {
 *		char *__base;
 *		int __offset;
 *		int __pad;
 * } __va_list;
 *
 * #define __va_arg_offset
 */

/********************************************
 * Linux/Playstation2 has a different ABI to IRIX/mips
 * with IRIX N32, vargs are all 64 bit wide
 * with Linux/Playstation2, vargs are both 32bit and 64bit
 *		with 64 bit args being aligned on 8byte boundaries
 *		eg      int(4 bytes), space(4 bytes), double(4 bytes)
 *		or		int(4 bytes), int(4 bytes),
 *
 */

#if defined(__mips) && !defined(__mips__)
#	if __mips
#		define __mips__    __mips
#	endif
#endif

#if defined(__alpha) && !defined(__alpha__)
	#if __alpha
		#define __alpha__    __alpha
	#endif
#endif

#if defined(__ia64) && !defined(__ia64__)
#	if __ia64
#		define __ia64__    __ia64
#	endif
#endif

#if defined(__powerpc) && defined(HAVE_VA_LIST_OVERFLOW_ARG_AREA) && !defined(HAVE___VA_REGSAVE_T)
	/* gcc 4 misses this struct out */
	typedef struct 
	{ 
		long __gp_save[8]; 
		double __fp_save[8]; 
	} __va_regsave_t;
#endif

typedef struct RHBVARGS
{
	char * alloc;
	va_list args;
	va_list push;
} RHBVARGS;

#if defined(_WIN32) && defined(_WIN64)
#	define RHBVARGS_REG_T		__int64
#else
#	if defined(__mips__) || defined(__ia64__)
	/* IRIX 6.5 picked this one up 
		and 32 bit HPUX on Itanium */
#		define RHBVARGS_REG_T		long long
#	else
#		if defined(_LP64) || defined(__alpha__)
			/* 64-bit Solaris, DEC-Alpha etc ... */
#			define RHBVARGS_REG_T		long
#		else
#			define RHBVARGS_REG_T		int
#		endif
#	endif
#endif

#if defined(_PLATFORM_BIG_ENDIAN_) && defined(__hppa) && !defined(_LP64)
#	define RHBVARGS_REVERSE_ORDER
#endif

#if defined(HAVE_VA_ARG_ASSIGN) && defined(__mips__)
#	define RHBVARGS_VA_ARG_ASSIGN
#endif

/**********************************************
 * for the __x86_64__ style on gcc
 *	it's like the PowerPC style, except
 *
 *	regsave area is
 *
 *		%rd1	0
 *		%rs1	8
 *		%rdx	16
 *		%rcx	24
 *		%r8		32
 *		%r9		40
 *		%xnm0	48
 *		%xnm1	64
 *		%xnm15	288
 *
 *		288+16=304
 *
 *	struct
 *	{
 *		unsigned int gp_offset=48			magic cut off is 48-num_fp*8, so 48 bytes, 6 slots?
 *		unsigned int fp_offset=304			magic cut off is 304-num_fp*16, so 304 bytes, 19 slots?
 *		void *overflow_arg_area
 *		void *reg_save_area
 *	}
 *
 * for Solaris x86_64 it is
 *  struct __va_list_element
 *  {
 *      unsigned int __va_gp_offset;
 *      unsigned int __va_fp_offset;
 *      void *__va_overflow_arg_area;
 *      void *__va_reg_svr_area;
 *  }
 */

#if defined(HAVE_VA_LIST_OVERFLOW_ARG_AREA) || defined(__alpha__) || defined(HAVE_VA_LIST__OFFSET) || defined(HAVE_VA_LIST___VA_OVERFLOW_ARG_AREA)
	#if defined(HAVE_VA_LIST_OVERFLOW_ARG_AREA) || defined(HAVE_VA_LIST___VA_OVERFLOW_ARG_AREA)
		/* 
			as per MkLinux on PowerPC...
			va_list is actually an array of one item,
			 */
		#if defined(HAVE_VA_LIST_GPR) && defined(HAVE_VA_LIST_FPR)
			#define RHBVARGS_number(x)		(sizeof(x)/sizeof(x[0])) 
			#define RHBVARGS_GPRN		RHBVARGS_number(((__va_regsave_t *)NULL)->__gp_save)
			#define RHBVARGS_FPRN		RHBVARGS_number(((__va_regsave_t *)NULL)->__fp_save)
			#define RHBVARGS_LIST_GP_OFFSET		gpr
			#define RHBVARGS_LIST_FP_OFFSET		fpr
		#else
			#if ( defined(HAVE_VA_LIST_GP_OFFSET) && defined(HAVE_VA_LIST_FP_OFFSET) ) || ( defined(HAVE_VA_LIST___VA_GP_OFFSET) && defined(HAVE_VA_LIST___VA_GP_OFFSET) )
				#ifdef __x86_64__
					/* these are magic numbers from the AMD64 ABI */
					#define RHBVARGS_GPRN	48
					#define RHBVARGS_FPRN	304

					#if defined(HAVE_VA_LIST_GP_OFFSET) && defined(HAVE_VA_LIST_FP_OFFSET)
						#define RHBVARGS_LIST_GP_OFFSET		gp_offset
						#define RHBVARGS_LIST_FP_OFFSET		fp_offset
					#else
						#define RHBVARGS_LIST_GP_OFFSET		__va_gp_offset
						#define RHBVARGS_LIST_FP_OFFSET		__va_fp_offset
					#endif
				#else
					#error __x86_64__ not defined
				#endif
			#else
				#error dont know how to initialise va_list
			#endif
		#endif

		#define RHBVARGS_INIT(x)	\
				x.RHBVARGS_LIST_GP_OFFSET=RHBVARGS_GPRN; \
				x.RHBVARGS_LIST_FP_OFFSET=RHBVARGS_FPRN;

		#ifdef HAVE_VA_LIST___VA_OVERFLOW_ARG_AREA
			#define RHBVARGS_LIST_OVERFLOW_ARG_AREA		__va_overflow_arg_area
			#define RHBVARGS_LIST_REG_SAVE_AREA			__va_reg_sve_area
		#else
			#define RHBVARGS_LIST_OVERFLOW_ARG_AREA		overflow_arg_area
			#define RHBVARGS_LIST_REG_SAVE_AREA			reg_save_area
		#endif

		#define RHBVARGS_alloc(a,s)		\
				{	a.alloc=(char *)SOMMalloc(s);  \
					RHBVARGS_INIT(a.args[0]) \
					a.args[0].RHBVARGS_LIST_OVERFLOW_ARG_AREA=a.alloc; \
					a.args[0].RHBVARGS_LIST_REG_SAVE_AREA=NULL; \
					RHBVARGS_INIT(a.push[0]) \
					a.push[0].RHBVARGS_LIST_OVERFLOW_ARG_AREA=a.alloc; \
					a.push[0].RHBVARGS_LIST_REG_SAVE_AREA=NULL; }
		#define RHBVARGS_push(arg,type,val)  \
				if (sizeof(type) < sizeof(RHBVARGS_REG_T)) \
				{   *((RHBVARGS_REG_T *)(arg.push->RHBVARGS_LIST_OVERFLOW_ARG_AREA))=(RHBVARGS_REG_T)(val); \
					arg.push->RHBVARGS_LIST_OVERFLOW_ARG_AREA=(void *)(((char *) \
						arg.push->RHBVARGS_LIST_OVERFLOW_ARG_AREA)+sizeof(RHBVARGS_REG_T)); } else \
				{   *((type *)(arg.push->RHBVARGS_LIST_OVERFLOW_ARG_AREA))=val; \
					arg.push->RHBVARGS_LIST_OVERFLOW_ARG_AREA=(void *)(((char *) \
						arg.push->RHBVARGS_LIST_OVERFLOW_ARG_AREA)+sizeof(type)); }
	#else
		/* offset to deal with doubles, appears that first six or seven
				 are in registers, or held in different location?,
				code refers variously to <=48 and 56, so 64 is to make sure 
				our args are above that offset */
		#define RHBVARGS_offset     64

		#if defined(HAVE_VA_LIST__OFFSET) && defined(HAVE_VA_LIST__A0)
			#define RHBVARGS_LIST_BASE		_a0
			#define RHBVARGS_LIST_OFFSET	_offset
		#else
			#define RHBVARGS_LIST_BASE		__base
			#define RHBVARGS_LIST_OFFSET	__offset
		#endif

		#define RHBVARGS_alloc(a,s)		\
			{	a.alloc=(char *)SOMMalloc(s); \
				a.args.RHBVARGS_LIST_BASE=(void *)(a.alloc-RHBVARGS_offset); \
				a.args.RHBVARGS_LIST_OFFSET=RHBVARGS_offset; \
				a.push.RHBVARGS_LIST_BASE=(void *)(a.alloc-RHBVARGS_offset); \
				a.push.RHBVARGS_LIST_OFFSET=RHBVARGS_offset; }
		#define RHBVARGS_push(arg,type,val) \
			{   *((type *)(((char *)arg.push.RHBVARGS_LIST_BASE)+arg.push.RHBVARGS_LIST_OFFSET))=(type)val; \
				arg.push.RHBVARGS_LIST_OFFSET+=(sizeof(type) > sizeof(RHBVARGS_REG_T)) ? \
					sizeof(type) : sizeof(RHBVARGS_REG_T); }
	#endif

	#define RHBVARGS_free(a)  if (a.alloc) { SOMFree(a.alloc); a.alloc=0; }
#else
	#ifdef HAVE_VA_LIST___AP
		#define RHBVARGS_VA_LIST_PTR(x)		(x).__ap
	#else
		#define RHBVARGS_VA_LIST_PTR(x)		(x)
	#endif

	#ifdef RHBVARGS_REVERSE_ORDER
		/* HP PA-RISC 32bit seems to use arguments in opposite order to standard
			C calling conventions (this is dependent on CPU type) */

		#define RHBVARGS_alloc(a,s) \
			a.alloc=(char *)SOMMalloc(s); a.args=(va_list)&a.alloc[s]; a.push=a.args;

		#define RHBVARGS_push(arg,type,val)			\
			{ va_arg(arg.push,type);  *((type *)arg.push)=val;	 }
	#else
		#define RHBVARGS_alloc(a,s) \
			a.alloc=(char *)SOMMalloc(s); RHBVARGS_VA_LIST_PTR(a.args)=(void *)a.alloc; a.push=a.args;

		#ifdef RHBVARGS_VA_ARG_ASSIGN
			#define RHBVARGS_push(arg,type,val)  va_arg(arg.push,type)=val
		#else
			#ifdef RHBVARGS_REG_T
				#define RHBVARGS_push(arg,type,val)  \
					if (sizeof(type) < sizeof(RHBVARGS_REG_T)) { \
					*((RHBVARGS_REG_T *)(RHBVARGS_VA_LIST_PTR(arg.push)))=(RHBVARGS_REG_T)(long)val; \
					RHBVARGS_VA_LIST_PTR(arg.push)=(void *)(((char *)(RHBVARGS_VA_LIST_PTR(arg.push)))+sizeof(RHBVARGS_REG_T)); } else { \
					*((type *)(RHBVARGS_VA_LIST_PTR(arg.push)))=val; \
					RHBVARGS_VA_LIST_PTR(arg.push)=(void *)(((char *)(RHBVARGS_VA_LIST_PTR(arg.push)))+sizeof(type)); }
			#else
				#define RHBVARGS_push(arg,type,val)  \
					*((type *)(RHBVARGS_VA_LIST_PTR(arg.push)))=val; \
					RHBVARGS_VA_LIST_PTR(arg.push)=(void *)(((char *)(RHBVARGS_VA_LIST_PTR(arg.push)))+sizeof(type))
			#endif
		#endif
	#endif

	#define RHBVARGS_free(a)  if (a.alloc) { SOMFree(a.alloc); a.alloc=NULL; }
#endif

#endif /* RHBVARGS_H */
