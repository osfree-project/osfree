# This program is to test the switch statement from switch_epc, withoug having
# to translate the whole run time library
# The epc switch statement is special in that there is no subtract for the
# lower bound (here 2)
# The input is argc, the number of parameters, e.g.
# switch_epc2 a b c
# Four!
	.file	"test_epc2.s"
gcc2_compiled.:
.section	".rodata"
	.align 8
.LLC0:
    .asciz  "Other!\n"
.LLC2:
	.asciz	"Two\n"
.LLC3:
	.asciz	"Three!\n"
.LLC4:
	.asciz	"Four!\n"
.LLC5:
	.asciz	"Five!\n"
.LLC6:
	.asciz	"Six!\n"
.LLC7:
	.asciz	"Seven!\n"
.section	".text"
	.align 4
	.global main
	.type	 main,#function
	.proc	04
main:
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1

    mov    %i0, %o0         ! Copy argc to %o0

    cmp    %o0, 2
    bl     other
    cmp    %o0, 7
    bg,a   other
    nop
    sethi  %hi(table-2*4), %o1
    or     %o1, %lo(table-2*4), %o1
    sll    %o0, 2, %o0
    ld     [%o0 + %o1], %o0
    jmp    %o0
    nop

table:
    .uaword  case2, case3, case4, case5, case6, case7

other:
	sethi %hi(.LLC0),%o0
    ba  print
	or %o0,%lo(.LLC0),%o0

case2:
	sethi %hi(.LLC2),%o0
    ba  print
	or %o0,%lo(.LLC2),%o0

case3:
	sethi %hi(.LLC3),%o0
    ba  print
	or %o0,%lo(.LLC3),%o0

case4:
	sethi %hi(.LLC4),%o0
    ba  print
	or %o0,%lo(.LLC4),%o0

case5:
	sethi %hi(.LLC5),%o0
    ba  print
	or %o0,%lo(.LLC5),%o0

case6:
	sethi %hi(.LLC6),%o0
    ba  print
	or %o0,%lo(.LLC6),%o0

case7:
	sethi %hi(.LLC7),%o0
    ba  print
	or %o0,%lo(.LLC7),%o0

print:
	call printf,0
	nop
	mov 0,%i0
	ret
	restore
.LLfe1:
	.size	 main,.LLfe1-main
	.ident	"GCC: (GNU) 2.8.1"
