	.file	"returnparam.c"
	.version	"01.01"
gcc2_compiled.:
.section	.rodata
.LC0:
	.string	"Hello"
.LC1:
	.string	"s.a = '%s' %d\n"
	.align 4	/* Needed until the alignment issue is solved */
.LC2:
	.string	"World"
.LC3:
	.string	"s.b = '%s' %d\n"
.LC4:
	.string	"Elements are %s and %s\n"
.text
	.align 4
.globl getFirstStr
	.type	 getFirstStr,@function
getFirstStr:
	pushl %ebp
	movl %esp,%ebp
	movl 8(%ebp),%eax
	leave
	ret
.Lfe1:
	.size	 getFirstStr,.Lfe1-getFirstStr
	.align 4
.globl main
	.type	 main,@function
main:
	pushl %ebp
	movl %esp,%ebp
	subl $32,%esp
	pushl %esi
	pushl %ebx
	movl .LC0,%eax
	movl %eax,-32(%ebp)
	movw .LC0+4,%ax
	movw %ax,-28(%ebp)
	pushl $99
	leal -32(%ebp),%esi
	pushl %esi
	pushl $.LC1
	call printf
	movl .LC2,%eax
	movl %eax,-16(%ebp)
	movw .LC2+4,%ax
	movw %ax,-12(%ebp)
	pushl $99
	leal -16(%ebp),%ebx
	pushl %ebx
	pushl $.LC3
	call printf
	pushl %ebx
	pushl %esi
	call getFirstStr
	addl $4,%esp
	pushl %eax
	pushl $.LC4
	call printf
	xorl %eax,%eax
	leal -40(%ebp),%esp
	popl %ebx
	popl %esi
	leave
	ret
.Lfe2:
	.size	 main,.Lfe2-main
	.ident	"GCC: (GNU) 2.8.1"
