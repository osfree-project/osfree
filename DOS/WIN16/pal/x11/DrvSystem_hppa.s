;	DrvSystem_hppa.s
;
;  	Assembly code routines needed for HP PA-RISC 
;
;	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
;
; This file is part of Willows Twin.
; 
; This software is distributed under the terms of the Willows Twin
; Public License as published by Willows Software, Inc.  This software
; may not be redistributed without the express written permission of
; Willows Software, Inc.
; 
; This software is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; Willows Twin Public License for more details.
; 
; You should have received a copy of the Willows Twin Public License
; along with this software; if not, contact Willows Software, Inc.,
; 10295 Saratoga Ave, Saratoga, CA 95070 USA.
;
; http://www.willows.com
; (408) 777-1820 -- VOICE
; (408) 777-1825 -- FAX
;
	.LEVEL	1.1

;
;  Code to flush the data and instruction cache for a given
;  address.  The equivalent C-code would look something 
;  like this:
;
;  hpux_flush_cache(char *addr, int len)
;  {
;      while (len > 0)
;      {
;          FDC(addr);
;	   SYNC;
;          FIC(addr);
;          SYNC;
;          addr += 4;
;          len -= 4;
;      }
;  }

	.SPACE	$TEXT$,SORT=8
	.SUBSPA	$CODE$,QUAD=0,ALIGN=8,ACCESS=0x2c,CODE_ONLY,SORT=24
hpux_flush_cache
	.PROC
	.CALLINFO FRAME=0,ARGS_SAVED
        .ENTRY
	LDI	4,%r20
        COMIBF,<,N      0,%r25,$00000002        ;offset 0x0
$00000003
	; First flush the data cache, writing the contents out
	; to memory.  Perform a sync to make sure it completes.
	; Then flush the instruction cache, again making sure the
	; cache lines are invalidated before going on.
	FDC	%r0(%r26)
	SYNC
	FIC,M	%r20(%r26)	; Use r26, then increment by 4.
	SYNC

        ADDIBF,<=       -4,%r25,$00000003       ;offset 0x4
	NOP
$00000002
        .EXIT
        BV,N    %r0(%r2)        ;offset 0xc
	.PROCEND	;in=25,26;out=28;


;
;  Code to switch stacks.  This routine sets the stack pointer to
;  the new value, and then calls the completion routine.  That routine
;  will never return to us, so don't worry about any cleanup.
;
;  The equivalent psuedo-C code would be as follows:
;
;  TWIN_SwitchStackAssem(char *new_stack)
;  {
;      SP = new_stack;
;      TWIN_InitStackEnd();
;  }

	.SPACE	$TEXT$
	.SUBSPA	$CODE$,QUAD=0,ALIGN=8,ACCESS=0x2c,CODE_ONLY,SORT=24
TWIN_SwitchStackAssem
	.PROC
	.CALLINFO CALLER,FRAME=16,SAVE_RP,ARGS_SAVED
        .ENTRY
        STW     %r2,-20(%r30)   ;offset 0x10
        LDO     64(%r30),%r30   ;offset 0x14
        STW     %r19,-32(%r30)  ;offset 0x18
        STW     %r26,-100(%r30) ;offset 0x1c
        STW     %r19,-32(%r30)  ;offset 0x20
	;
	;  Set the new stack.  The calling code makes certain it
	;  is properly aligned, and accounts for stack growing
	;  upwards, rather than the more typical downwards.
	;  In addition, we add two empty frames to the stack,
	;  so that routines that save values in previous frames
	;  upon entry will have a place to store them.
	;
	LDO	128(%r26),%r30
        .CALL   RTNVAL=GR       ;out=28;
        BL      TWIN_InitStackEnd,%r2   ;offset 0x24
        NOP             ;offset 0x28
	;
	;  We never actually get to this point.
	;
        LDW     -32(%r30),%r19  ;offset 0x2c
        LDW     -84(%r30),%r2   ;offset 0x30
        BV      %r0(%r2)        ;offset 0x34
        .EXIT
        LDO     -64(%r30),%r30  ;offset 0x38
	.PROCEND	;in=26;out=28;

	.SPACE	$TEXT$
	.SUBSPA	$CODE$
	.SUBSPA	$CODE$
	.SPACE	$PRIVATE$,SORT=16
	.SUBSPA	$DATA$,QUAD=1,ALIGN=8,ACCESS=0x1f,SORT=16
	.SPACE	$TEXT$
	.SUBSPA	$CODE$
	.EXPORT	hpux_flush_cache,ENTRY,PRIV_LEV=3,ARGW0=GR,ARGW1=GR,RTNVAL=GR
	.SUBSPA	$CODE$
	.EXPORT	TWIN_SwitchStackAssem,ENTRY,PRIV_LEV=3,ARGW0=GR,RTNVAL=GR
	.IMPORT	TWIN_InitStackEnd,CODE
	.END
