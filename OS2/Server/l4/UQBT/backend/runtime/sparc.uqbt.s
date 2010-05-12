! ==============================================================================
!  FILE:       sparc.uqbt.s
!  OVERVIEW:   Provides assembler routines for the UQBT binary translator's
! 			  	generated code for Sparc targets
! 
!  (C) 2000 The University of Queensland, BT group
! ==============================================================================
.section    ".text"
!  prototyped as __reg_call(...)
! __translate_addr will read native address from global _uqbt_nat
! Number of parameters does not matter: not stored
! We use the global _uqbt_nat rather than a parameter to this code, because
! if we passed a real parameter, we'd have to move all the other parameters
! "down by one". Well, by 2 since we'd also have to pass the number of params.
! This way is much more efficient, and less hairy assembler to go wrong
! We need the save/restore with Sparc because __translate_addr is allowed to
! trash all the parameters.
! We call the C function __translate_addr from assembler, rather than the other
! way around, because we can't be sure what stack frame __translate_addr will
! use. We don't make this function C so we get control over the stack frame.
.proc
.global __reg_call
__reg_call:
    save   %sp, -96, %sp           ! Make new register window; saves %o0-%o5
    ! Note that the C code may use space this stack frame, so -96 is needed
    call   __translate_addr        ! Translate to host
    nop                            ! Wasted delay slot
    jmp    %o0                     ! Jump to dest
    restore                        ! Restore %o0-%o5

.section    ".data"
.global _uqbt_nat
.align  4
_uqbt_nat:   .byte 0,0,0,0
  
