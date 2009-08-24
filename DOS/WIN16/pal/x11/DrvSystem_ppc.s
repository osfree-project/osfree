#
#	DrvSystem_ppc.s
#
#	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
#
# This file is part of Willows Twin.
# 
# This software is distributed under the terms of the Willows Twin
# Public License as published by Willows Software, Inc.  This software
# may not be redistributed without the express written permission of
# Willows Software, Inc.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# Willows Twin Public License for more details.
# 
# You should have received a copy of the Willows Twin Public License
# along with this software; if not, contact Willows Software, Inc.,
# 10295 Saratoga Ave, Saratoga, CA 95070 USA.
#
# http://www.willows.com
# (408) 777-1820 -- VOICE
# (408) 777-1825 -- FAX
#
#  Assembly routine to switch the stack on an AIX PowerPC platform.
#  The AIX compiler does not support "asm()", so we do this via
#  a separate assembly module.  The equivalent C code would look
#  something like this:
#
#  TWIN_SwitchStackAssem(volatile int new_sp)
#  {
#      asm("move $sp, $4");  /* move new_sp to sp register */
#      TWIN_InitStackEnd();
#  }
#
#  All we do is set the new stack pointer, then call the "cleanup"
#  function, which will save the current context, and restore to
#  the pre-switched context.  The TWIN_InitStackEnd function will
#  never return to us.

.set SP,1
.set r3,3

	.globl	.TWIN_SwitchStackAssem  
	.extern	.TWIN_InitStackEnd{PR}   

# .text section

	.csect
.TWIN_SwitchStackAssem:

# Minimal code: store the passed argument as the new SP, then
# call the followup routine.  It will never return to us.
	mr	SP,r3
	bl	.TWIN_InitStackEnd{PR}
	nop
	nop
	nop

