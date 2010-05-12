! Test for various sparc patterns: call/add, struct_call

!!!!!!!!!!!!
! Call/add !
!!!!!!!!!!!!
! Example from /opt/local/gnu/bin/bash:
! 3b990:  40 00 07 65        call         0x3d724
! 3b994:  9e 03 e0 38        add          %o7, 56, %o7

.section	".rodata"
	.align 8
.LLC0:
	.asciz	"Call/add test: result is "
.LLC1:
	.asciz	"Correct!\n"
.LLC2:
    .asciz  "FAIL!\n"
.section	".text"
	.align 4
	.global main
	.type	 main,#function
	.proc	04
main:
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1

!
! We want a path to the error code, so it isn't optimised out
! Do this if 6 parameters
    cmp %i0, 6
    beq labelFail1
    sethi   %hi(.LLC2), %o0

!
! The call/add test proper
!
	sethi %hi(.LLC0),%o0
	or %o0,%lo(.LLC0),%o0
	call printf,0           ! The call
	add  %o7, 8, %o7        ! The add
    ! 8 bytes is the size of a branch and delay instruction
    ba      labelFail1
    sethi   %hi(.LLC2), %o0
!
! If correct behaviour, will end up here
!
    sethi   %hi(.LLC1), %o0
	call printf,0
	or %o0,%lo(.LLC1),%o0
    ba  labelDone1
    nop

labelFail1:
	call printf,0
	or %o0,%lo(.LLC2),%o0

labelDone1:

!!!!!!!!!!!!!!!
! struct_call !
!!!!!!!!!!!!!!!

	sethi %hi(s2),%o0
	or %o0,%lo(s2),%o1
	st %o1,[%sp+64]             ! Set up struct return address
	mov 55,%o0
	call returns_struct,0       ! The CALL
	mov 99,%o1                  ! X (delay slot instr)
	unimp 8                     ! the UNIMP
	sethi %hi(s2),%o1
	sethi %hi(s2),%o0
	mov 4,%o2
	or %o0,%lo(s2),%o0
	add %o2,%o0,%o2
	sethi %hi(.LLelem),%o3
	or %o3,%lo(.LLelem),%o0
	ld [%o1+%lo(s2)],%o1
	call printf,0
	ld [%o2],%o2

!
!   Tests for call/restore/unimp and ordinary call/restore
!
    call    callRestoreUnimpReg,0
    nop

    call    callRestoreUnimpImm,0
    nop

    call    callRestoreImm,0
    nop

    call    callRestoreReg,0
    nop
    
    call    moveCallMove,0
    nop
    
    call    moveXCallMove,0
    nop

!
! Test jmp/restore
!
    call    jmpRestImm,0
    nop

    call    jmpRestReg,0
    nop 

!
! Other callee prologues
!
    call    sameRegWin,0
    nop

    call    newRegWinLarge,0
    nop

    call    sameRegWinLarge,0
    nop

!
! Other callee epilogues
!
    call    retRegVal,0
    sethi   %hi(.LLrrv), %l0
    mov     %o0, %o1
    call    printf,0
    or      %l0, %lo(.LLrrv), %o0

.section	".rodata"
	.align 8
.LLrrv:
	.asciz	"Return with reg+reg in restore: %d\n"

.section        ".text"
    call    retImmVal,0
    sethi   %hi(.LLriv), %l0
! This is not a struct returning call, but we want to test that form of
! the pattern that starts with "jmp %i7+12"
    unimp   20
    mov     %o0, %o1
    call    printf,0
    or      %l0, %lo(.LLriv), %o0

.section	".rodata"
	.align 8
.LLriv:
	.asciz	"Return with reg+imm in restore: %d\n"

.section        ".text"
    sethi   %hi(.LLdone), %o0
    call    printf,0
    add     %o0, %lo(.LLdone), %o0

	mov 0,%i0
	ret
	restore
.LLfe1:
	.size	 main,.LLfe1-main

.section	".rodata"
	.align 8
.LLdone:
	.asciz	"Done\n"

!!
!! End of main
!!

.section	".rodata"
	.align 8
.LLelem:
	.asciz	"Elements are %d and %d\n"

.section        ".text"
        .align 4
        .global returns_struct
        .type    returns_struct,#function
        .proc   010
returns_struct:
        !#PROLOGUE# 0
        save %sp,-112,%sp
        !#PROLOGUE# 1
        ld [%fp+64],%o0
        st %i0,[%fp+68]
        st %i1,[%fp+72]
        sethi %hi(s1),%o1
        ld [%fp+68],%o2
        st %o2,[%o1+%lo(s1)]
        sethi %hi(s1),%o1
        mov 4,%o2
        or %o1,%lo(s1),%o3
        add %o2,%o3,%o1
        ld [%fp+72],%o2
        st %o2,[%o1]
        sethi %hi(s1),%o1
        ld [%o1+%lo(s1)],%o2
        st %o2,[%o0]
        mov 4,%o2
        or %o1,%lo(s1),%o3
        add %o2,%o3,%o1
        ld [%o1],%o2
        st %o2,[%o0+4]
        b .LL1
        nop
.LL1:
        mov %o0,%i0
        jmp %i7+12
        restore
.LLfe9:
        .size    returns_struct,.LLfe9-returns_struct

        ! Declare the global structs s1 and s2
	    .common	s1,8,4
	    .common	s2,8,4

!
! Proc for call/restore/unimp, register version
!
.section        ".text"
	.align 4
	.global callRestoreUnimpReg
	.type	 callRestoreUnimpReg,#function
callRestoreUnimpReg:
    save    %sp,-96,%sp
    sethi   %hi(.LLcrur),%i0
    or      %i0,%lo(.LLcrur),%i0
    mov     444, %l4
    mov     555, %l5
    call    printf,0
    restore %l4, %l5, %o1
    unimp

.section	".rodata"
	.align 8
.LLcrur:
	.asciz	"Call/restore/unimp with register: %d\n"

!
! Proc for call/restore/unimp, immediate version
!
.section        ".text"
	.align 4
	.global callRestoreUnimpImm
	.type	 callRestoreUnimpImm,#function
callRestoreUnimpImm:
    save    %sp,-96,%sp
    sethi   %hi(.LLcrui),%i0
    or      %i0,%lo(.LLcrui),%i0
    mov     333, %l4
    call    printf,0
    restore %l4, 444, %o1
    unimp

.section	".rodata"
	.align 8
.LLcrui:
	.asciz	"Call/restore/unimp with immediate: %d\n"


!
! Proc for call/restore with immediate
!
.section        ".text"
	.align 4
	.global callRestoreImm
	.type	 callRestoreImm,#function
callRestoreImm:
    save    %sp,-96,%sp
    sethi   %hi(.LLcri),%i0
    or      %i0,%lo(.LLcri),%i0
    mov     44,%i4
    call    printf,0
    restore %i4, 11, %o1

.section	".rodata"
	.align 8
.LLcri:
	.asciz	"Call/restore with immediate: %d\n"


!
! Proc for call/restore with register
!
.section        ".text"
	.align 4
	.global callRestoreReg
	.type	 callRestoreReg,#function
callRestoreReg:
    save    %sp,-96,%sp
    sethi   %hi(.LLcrr),%i0
    or      %i0,%lo(.LLcrr),%i0
    mov     44, %i4
    mov     55, %i5
    call    printf,0
    restore %i4, %i5, %o1

.section	".rodata"
	.align 8
.LLcrr:
	.asciz	"Call/restore with register: %d\n"

!
! Proc for move/call/move
!
.section        ".text"
	.align 4
	.global moveCallMove
	.type	 moveCallMove,#function
moveCallMove:
    sethi   %hi(.LLmcm),%o0
    or      %o0,%lo(.LLmcm),%o0
    mov     %o7,%g3
    call    printf,0
    mov     %g3,%o7

.section	".rodata"
	.align 8
.LLmcm:
	.asciz	"Move/call/move\n"


!
! Proc for move/x/call/move
!
.section        ".text"
	.align 4
	.global moveXCallMove
	.type	 moveXCallMove,#function
moveXCallMove:
    sethi   %hi(.LLmxcm),%o0
    mov     %o7,%g3
    or      %o0,%lo(.LLmxcm),%o0
    call    printf,0
    mov     %g3,%o7

.section	".rodata"
	.align 8
.LLmxcm:
	.asciz	"Move/x/call/move\n"


!
! Proc for jmp/restore with immediate
!
.section        ".text"
	.align 4
	.global jmpRestImm
	.type	 jmpRestImm,#function
jmpRestImm:
    save    %sp,-96,%sp
    sethi   %hi(.LLjri),%i0
    or      %i0,%lo(.LLjri),%i0
    sethi   %hi(printf), %l2
    or      %l2, %lo(printf), %l2
    mov     33, %i4
    jmp     %l2
    restore %i4, 44, %o1

.section	".rodata"
	.align 8
.LLjri:
	.asciz	"Jump Restore with immediate: %d\n"


!
! Proc for jmp/restore with register
!
.section        ".text"
	.align 4
	.global jmpRestReg
	.type	 jmpRestReg,#function
jmpRestReg:
    save    %sp,-96,%sp
    sethi   %hi(.LLjrr),%i0
    or      %i0,%lo(.LLjrr),%i0
    sethi   %hi(printf), %l2
    or      %l2, %lo(printf), %l2
    mov     11, %i4
    mov     55, %i5
    jmp     %l2
    restore %i4, %i5, %o1

.section	".rodata"
	.align 8
.LLjrr:
	.asciz	"Jump Restore with register: %d\n"


!
! Proc for same_reg_win callee prologue
!
.section        ".text"
	.align 4
	.global  sameRegWin
	.type	 sameRegWin,#function
sameRegWin:
    add     %sp,-96,%sp
    mov     %o7, %l7            ! Save return address
    sethi   %hi(.LLsrw),%o0
    call    printf,0
    or      %o0,%lo(.LLsrw),%o0
    mov     %l7, %o7            ! Restore return address
    retl
    add     %sp, 96, %sp        ! Restore the stack!

.section	".rodata"
	.align 8
.LLsrw:
	.asciz	"Same register window\n"



!
! Proc for new_reg_win_large callee prologue
!
.section        ".text"
	.align 4
	.global  newRegWinLarge
	.type	 newRegWinLarge,#function
newRegWinLarge:
    sethi   %hi(-5432), %g1
    add      %g1, %lo(-5432), %g1
    save    %sp, %g1, %sp
    sethi   %hi(.LLnrwl),%o0
    call    printf,0
    or      %o0,%lo(.LLnrwl),%o0
    ret
    restore

.section	".rodata"
	.align 8
.LLnrwl:
	.asciz	"New register window large\n"


!
! Proc for same_reg_win_large callee prologue
!
.section        ".text"
	.align 4
	.global  sameRegWinLarge
	.type	 sameRegWinLarge,#function
sameRegWinLarge:
    sethi   %hi(-8888), %l6
    or      %l6, %lo(-8888), %l6
    add     %sp, %l6 ,%sp
    mov     %o7, %l7            ! Save return address
    sethi   %hi(.LLsrwl),%o0
    call    printf,0
    or      %o0,%lo(.LLsrwl),%o0
    mov     %l7, %o7            ! Restore return address
    retl
    sub     %sp, %l6, %sp       ! Restore the stack!

.section	".rodata"
	.align 8
.LLsrwl:
	.asciz	"Same register window large\n"


!
! Proc for ret_reg_val callee epilogue
!
.section        ".text"
	.align 4
	.global  retRegVal
	.type	 retRegVal,#function
retRegVal:
    save    %sp, -96 ,%sp
    mov     222, %o4
    mov     333, %o5
    ret
    restore %o4, %o5, %o0



!
! Proc for ret_imm_val callee epilogue
!
.section        ".text"
	.align 4
	.global  retImmVal
	.type	 retImmVal,#function
retImmVal:
    save    %sp, -96 ,%sp
    mov     111, %o4
    jmp     %i7+12
    restore %o4, 222, %o0

