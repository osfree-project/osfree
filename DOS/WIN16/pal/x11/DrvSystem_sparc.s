!
!	DrvSystem_sparc.s
!
!	Copyright (c) 1995-1996, Willows Software, Inc.  All rights reserved.
!
! This file is part of Willows Twin.
! 
! This software is distributed under the terms of the Willows Twin
! Public License as published by Willows Software, Inc.  This software
! may not be redistributed without the express written permission of
! Willows Software, Inc.
! 
! This software is distributed in the hope that it will be useful,
! but WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
! Willows Twin Public License for more details.
! 
! You should have received a copy of the Willows Twin Public License
! along with this software; if not, contact Willows Software, Inc.,
! 10295 Saratoga Ave, Saratoga, CA 95070 USA.
!
! http://www.willows.com
! (408) 777-1820 -- VOICE
! (408) 777-1825 -- FAX
!
!  Assembly language routine to switch the stack on any sparc machine.
!
	.text
        .align  8
        .skip   16

	.global _TWIN_SwitchStackAssem

_TWIN_SwitchStackAssem:
	ta   0x3
	save %i0,-96,%sp
	nop
	nop
	nop
	nop
	call _TWIN_InitStackEnd,1
	nop
	nop
	nop
	nop
