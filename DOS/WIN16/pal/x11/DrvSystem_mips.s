 #	DrvSystem_mips.s
 #
 #	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
 #
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
 #  Assembly routine to switch the stack on an SGI MIPS platform.
 #  The MIPS compiler does not support "asm()", so we do this via
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

	.text	
	.align	2
	.globl	TWIN_SwitchStackAssem
	.ent	TWIN_SwitchStackAssem 2
TWIN_SwitchStackAssem:

 # 	Setup the new stack, and save the old stack pointer and
 #	return address.  We should never really have to return
 #	from this routine, but save the info as a safety measure.

	move	$3, $sp		# Save current stack pointer
	move	$sp, $4		# Set sp to passed arg ($4)
	subu	$sp, 16		# Make some room on the stack
	sw	$31, 4($sp)	# Save return address
	sw	$3, 8($sp)	# Save old stack pointer

 #      Call the "finishing" routine.

	jal	TWIN_InitStackEnd

 #	We should never actually get here, but try and do something
 # 	moderately sane if we do.  Just restore the return address
 #	and stack pointer, then return.

	lw	$31, 4($sp)	# Restore return address
	lw	$3, 8($sp)	# Restore old stack pointer
	move	$sp, $3		# Set SP to old value
	j	$31		# Return

	.end	TWIN_SwitchStackAssem
