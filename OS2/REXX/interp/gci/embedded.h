/*
 *  Generic Call Interface for Rexx
 *  Copyright © 2003, Florian Groﬂe-Coosmann
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * ----------------------------------------------------------------------------
 *
 * This file contains redefinitions of the maintainer of the current
 * implementation. I don't wanna have these ugly
 * "#if defined(MACHINE_SUBTYPE_FLAVOUR_SMALL_STRAWBERRY) && !defined(...
 * in the code.
 *
 * Proper implementors change the following definitions to their requirements.
 * They may do their "defines" here and not in the code itself. It will be
 * nice to get a note for extensions here!
 */

/*
 * The include file may have the name rexxsaa.h or rexx.h depending on the
 * interpreter.
 */

#include "regina_c.h"

#ifndef __REXXSAA_H_INCLUDED
# include "../rexxsaa.h"
# define DONT_TYPEDEF_PFN
#endif
#ifndef __REXX_H_INCLUDED
# include "../rexx.h"
#endif

#include <setjmp.h>
#include <string.h>

/*
 * Define some little helpers to those functions that are available.
 * The "hidden" parameter is available in all cases where runtime assistance
 * is needed. Thus, the following is possible:
 * #define GCI_isspace(c) z(hidden,c)
 */
#define GCI_isspace(c) rx_isspace(c)
#define GCI_isdigit(c) rx_isdigit(c)
#define GCI_isprint(c) rx_isprint(c)
#define GCI_toupper(c) rx_toupper(c)

/*
 * Though we have RXSTRING, GCI_str has its own implementation for speedup
 * or usage in a direct implementation.
 * We don't use or check for a terminator.
 */
typedef struct {
   int   used; /* really used bytes of val */
   int   max;  /* allocated size of val */
   char *val;  /* buffer for the content */
} GCI_str;

/*
 * We sometimes want to create a GCI_str from a native character buffer.
 * We provide a macro for doing so.
 *
 * .max is set to the buffers size, .used is set to 0.
 */
#define GCI_strOfCharBuffer(buf) GCI_str str_##buf;             \
                                 str_##buf.used = 0;            \
                                 str_##buf.max = sizeof( buf ); \
                                 str_##buf.val = buf

/*
 * malloc and free will be redirected to an implementor's specific code
 * which provides some magic in speed or defragmentation.
 */
#define GCI_malloc(hidden,size) Malloc_TSD( (tsd_t *) (hidden), size )
#define GCI_free(hidden,block)  Free_TSD( (tsd_t *) (hidden), block )

/*
 * GCI_ALIGNMENT sets the alignment for indirections. It is best to use
 * those values used by the memory allocator, but what value shall be used?
 * If you don't know, you should use 16 as a good default value.
 */
#define GCI_ALIGNMENT 16

/*
 * GCI_REXX_ARGS is the maximum number of arguments that can be passed to a
 * GCI defined function.
 * It must be 10 at least, higher values with a maximum of 50 may be better.
 */
#define GCI_REXX_ARGS 32

/*
 * GCI_JUMPVAR is a macro which defines a jmp_buf in the file gci_call.c.
 * If the current runtime system of Rexx can fetch/define such a buffer
 * from somewhere without access to stack variables in GCI_JUMP_GETVAR, this
 * may be omitted.
 */
#define GCI_JUMPVAR(name)

/*
 * GCI_JUMP_GETVAR is a macro which returns a longjmp suitable jmp_buf without
 * accessing the stack.
 */
#define GCI_JUMP_GETVAR(name) __regina_get_tsd()->gci_jump

/*
 * GCI_JUMP_SETVAR is a macro which sets a longjmp suitable jmp_buf
 * to a reenterable state.
 * This function must block each other thread from entering GCI_JUMP_SETVAR
 * until GCI_JUMP is issued.
 */
#define GCI_JUMP_SETVAR(hidden,name) setjmp( ((tsd_t *) (hidden))->gci_jump )

/*
 * GCI_JUMP is a macro which does a longjmp to a place you may taken from
 * the argument to GCI_JUMP, but keep in mind that it MUST be static and
 * isn't taken from the stack.
 * This macro unblocks the lock that GCI_JUMP_SETVAR has created in
 * multi-threading systems.
 */
#define GCI_JUMP(jumpentry,status) longjmp( jumpentry, status )

/*
 * Regina has some special function in gci_reginabridge.c.
 */
int GCI_Dispatcher( tsd_t *TSD,
                    PFN func,
                    void *treeinfo,
                    int Params,
                    const PRXSTRING params,
                    PRXSTRING retstr );

int GCI_checkDefinition( tsd_t *TSD,
                         const streng *stem_name,
                         void **tree );
