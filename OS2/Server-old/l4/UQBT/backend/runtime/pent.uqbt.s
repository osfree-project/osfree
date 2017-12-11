! ==============================================================================
!  FILE:       pent.uqbt.s
!  OVERVIEW:   Provides assembler routines for the UQBT binary translator's
!              generated code for Pentium targets
!
! Copyright (C) 2000, The University of Queensland, BT group
! ==============================================================================
    .section    ".text"
!  prototyped as __reg_call(...)
! __translate_addr will read native address from global _uqbt_nat
! Number of parameters does not matter: not stored
    .proc
.global __reg_call
__reg_call:
    call   __translate_addr        ! Translate to host
    jmp    %eax                    ! Jump to dest

.section    ".data"
.global _uqbt_nat
.align  4
_uqbt_nat:   .byte 0,0,0,0

