	.file	"ninths.c"
	.version	"01.01"
	.ident	"BE built: Nov 13 1997 17:02:05 GMT"
/ASM
	.ident	" cpp: Software Generation Utilities (SGU) SunOS/SVR4"
/ASMEND
	.xstabs	".stab.index","Xs ; O ; V=3.1 ; R=WorkShop Compilers 4.2 30 Oct 1996 C 4.2",0x3c,0,0,0
	.xstabs	".stab.index","/export/home/emmerik/mve; /opt/SUNWspro/SC4.2/bin/cc -Xs -YP,:/usr/ucblib:/opt/SUNWspro/SC4.2/bin/../lib:/opt/SUNWspro/SC4.2/bin:/usr/ccs/lib:/usr/lib -xO4 -xinline= -S -I/usr/ucbinclude -lucb -lsocket -lnsl -lelf -laio  test/source/ninths.c -W0,-xp",0x34,0,0,0
/ASM
	.ident	"@(#)stdio.h	1.8	96/05/22 SMI"
/ASMEND
/ASM
	.ident	"@(#)va_list.h	1.6	96/01/26 SMI"
/ASMEND
	.xstabs	".stab.index","main",0x2a,0,0,0
/ASM
	.ident	"acomp: WorkShop Compilers 4.2 30 Oct 1996 C 4.2"
/ASMEND
	.ident	"hc2.8r -O0 -386 -fstrict -on SHAREABLE test/source/ninths.c"
	.section	.text
	.align	4
.L1_.text:

/====================
/ myDivide
/--------------------
	.align	4
	.globl	myDivide
myDivide:
	subl	$16,%esp
	fldl	20(%esp)
	fdivl	32(%esp)
	fstpl	8(%esp)
	fldl	8(%esp)
	fwait	
	addl	$16,%esp
	ret	
	.type	myDivide,@function
	.size	myDivide,.-myDivide
/ASM
	.globl	__fsr_init_value
	__fsr_init_value = 0x0
/ASMEND
	.section	.data
	.align	4
.L2_.data:
	.align	4
.L01:	.string	"Two ninths is %.12f\n\000"
	.set	.,.+0x2
	.section	.text
	.section	.rodata,"a"
	.align	4
.L5_.lit:
	.align	8
.L02:	.4byte	0x0,0x40000000	/ 2.0
	.section	.text

/====================
/ main
/--------------------
	.align	4
	.globl	main
main:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$16,%esp
	pushl	$1075970048
	pushl	$0
	pushl	$0
	pushl	$1072693248
	pushl	$0
	call	myDivide
	fstpl	-8(%ebp)
	fldl	-8(%ebp)
	fmull	.L02
	subl	$8,%esp
	fstpl	(%esp)
	fwait	
	pushl	$.L01
	call	printf
	addl	$32,%esp
	subl	%eax,%eax
	leave	
	ret	
	.align	4
	.type	main,@function
	.size	main,.-main
	.stabs	"",0x62,0,0,0x0
