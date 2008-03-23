/*    
	invoke_binary.c	2.11
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

#undef CARRY_FLAG	/* well-coordinated work */
#undef ZERO_FLAG
#include "kerndef.h"
#include "BinTypes.h"
#include "DPMI.h"

#include "Log.h"

extern void TWIN_FreeObsoleteCatches(ENV *envp);
extern void invoke_code16(ENV *);

ENV *envp_global;	/* the current global pointer to the x86 environment */

WORD return_thunk_selector;

/***************************************************************
 *	invoke_binary()
 *	
 *	uses the current envp_global environment to call into the binary
 *	world from the native world.
 *
 ****************************************************************/

void invoke_binary()
{
	ENV new_env;
 	ENV *old_env;
	unsigned char *sp;

	/****************************************************
	Creates a new ENV structure, initializes it from the current
	ENV structure, and makes it be the current ENV structure.  The
	new ENV structure has a level field one greater than the previous,
	has a prev_env field that points to the previous, and has 0 in its
	is_catch and active fields.
	****************************************************/

	new_env = *envp_global;
 	new_env.prev_env = envp_global;
 	new_env.active = 0;
 	new_env.is_catch = 0;
	new_env.level = envp_global->level + 1;
 	envp_global = &new_env;

	/****************************************************
 	Sets up a return address by pushing a return-thunk on
 	the simulated stack.
	(the binary function will do a far-return to that 
	address, which will be intercepted by the interpreter)
  	****************************************************/

	sp = (unsigned char *)envp_global->reg.sp;
 	*(sp - 2) = return_thunk_selector & 0xff;
	*(sp - 1) = return_thunk_selector >>8;
 	*(sp - 3) = *(sp - 4) = 0;
 	envp_global->reg.sp = (REGISTER)(sp - 4);
	envp_global->reg.ax = (WORD) GetCurrentDataSelector();

	setjmp(envp_global->jump_buffer);
	if (GetSelectorType(HIWORD(envp_global->trans_addr)) == 
			TRANSFER_CODE16) 
	{
#if !defined(TWIN_INTERP_NOT_SUPPORTED)
		invoke_code16(envp_global);
#else
  		FatalAppExit(0,"invoke_code16 not supported!\n");
#endif
	}

	if (envp_global->is_catch)
	    TWIN_FreeObsoleteCatches(envp_global);

	old_env = envp_global;
 	envp_global = (ENV *)envp_global->prev_env;
	envp_global->reg.ax = old_env->reg.ax;
 	envp_global->reg.dx = old_env->reg.dx;
 	envp_global->reg.sp = old_env->reg.sp; 
}

ENV  *
GetEnvpGlobal(void)
{
	return envp_global;
}

