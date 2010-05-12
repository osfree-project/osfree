! This file was generated with
! % gcc -S test/source/thirds.c
! and was then modified to load a double register with two single word load
! instructions
! Tests overlapping register code

	.file	"thirds.c"
gcc2_compiled.:
.section	".rodata"
	.align 8
.LLC1:
	.asciz	"Two thirds is %.12f\n"
	.align 8
.LLC0:
	.uaword	0x3fd55555 ! ~3.33333333333333314830e-1
	.uaword	0x55555555
.section	".text"
	.align 4
	.global main
	.type	 main,#function
	.proc	04
main:
	!#PROLOGUE# 0
	save	%sp, -128, %sp
	!#PROLOGUE# 1
	sethi	%hi(.LLC0), %o1
	or	%o1, %lo(.LLC0), %o0
	ldd	[%o0], %o2
	std	%o2, [%fp-24]
!	ldd	[%fp-24], %f2
    ! Want to test the overlapping register code
   ld [%fp-24], %f2
   ld [%fp-20], %f3
	std	%f2, [%fp-32]
	ldd	[%fp-32], %f2
	ldd	[%fp-32], %f4
	faddd	%f2, %f4, %f2
	std	%f2, [%fp-16]
	ldd	[%fp-16], %o2
	sethi	%hi(.LLC1), %o1
	or	%o1, %lo(.LLC1), %o0
	mov	%o2, %o1
	mov	%o3, %o2
	call	printf, 0
	 nop
	mov	0, %i0
	b	.LL2
	 nop
.LL2:
	ret
	restore
.LLfe1:
	.size	 main,.LLfe1-main
	.ident	"GCC: (GNU) 2.95.2.1 19991024 (release)"
