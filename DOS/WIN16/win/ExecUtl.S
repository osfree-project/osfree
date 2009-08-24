#
#	ExecUtl.s
#
#	@(#)ExecUtl.S	1.8 2/20/96 20:21:44 /users/sccs/src/win/s.ExecUtl.S
#
#	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
# 
 
#ifdef ELF
	.globl	_gsb_1
_GetSelectorBase:
	pushl %ebp
	movl %esp,%ebp
	pushl %ebx
	call .L2
.L2:
	popl %ebx
	addl $_GLOBAL_OFFSET_TABLE_+[.-.L2],%ebx
	movl 8(%ebp),%eax
	pushl %eax
	call GetSelectorBase@PLT
	addl $4,%esp
_gsb_1:

	movl -4(%ebp),%ebx
	movl %ebp,%esp
	popl %ebp
	ret

_LogProcName:
	pushl %ebp
	movl %esp,%ebp
	pushl %ebx
	call .L3
.L3:
	popl %ebx
	addl $_GLOBAL_OFFSET_TABLE_+[.-.L3],%ebx
	movl 16(%ebp),%eax
	pushl %eax
	movl 12(%ebp),%eax
	pushl %eax
	movl 8(%ebp),%eax
	pushl %eax
	call LogProcName@PLT
	addl $12,%esp

	movl -4(%ebp),%ebx
	movl %ebp,%esp
	popl %ebp
	ret

/* Caution to all who pass here. */
/* This is safe to call with 32bit SS */
_GetEnvpGlobal32:
	pushl %ebp
	movl %esp,%ebp
	pushl %ebx
	call .L4
.L4:
	popl %ebx
	addl $_GLOBAL_OFFSET_TABLE_+[.-.L4],%ebx
	call GetEnvpGlobal@PLT
	movl -4(%ebp),%ebx
	movl %ebp,%esp
	popl %ebp
	ret


_TWIN_GetReturnThunkSel:
	pushl %ebp
	movl %esp,%ebp
	pushl %ebx
	call .L7
.L7:
	popl %ebx
	addl $_GLOBAL_OFFSET_TABLE_+[.-.L7],%ebx
	call TWIN_GetReturnThunkSel@PLT
	movl -4(%ebp),%ebx
	movl %ebp,%esp
	popl %ebp
	ret
#else
#define	_LogProcName LogProcName
#define _TWIN_GetReturnThunkSel TWIN_GetReturnThunkSel
#define _GetEnvpGlobal32 GetEnvpGlobal
#define _GetSelectorBase GetSelectorBase
#endif

/* ENV structure */
DS	= 0x00
ES	= 0x04
SS	= 0x08
EFLAGS	= 0x0C
EAX	= 0x10
EBX	= 0x14
ECX	= 0x18
EDX	= 0x1C
ESI	= 0x20
EDI	= 0x24
EBP	= 0x28
ESP	= 0x2C
CS	= 0x30
FS	= 0x34
GS	= 0x38
TRANS_ADDR	= 0x3C
RETURN_ADDR	= 0x40
MACHINE_STACK	= 0x44

/* DSCR structure */
SELBASE	= 0x00

/* SEGIMAGE structure */
CONV	= 0x00
TARG	= 0x04

	.text
	.align	4
	.globl	invoke_native
	.globl	_invoke_native

invoke_native:
_invoke_native:
	pushw	%ds		/* -2 push 16-bit ds onto 16-bit stack */
	pushw	%bx		/* -4 push bx onto 16-bit stack */
	pushw	%gs		/* -6 */

	movl	%esp,%ebx
	andl	$0xffff,%ebx	
	movl	%ss:10(%ebx),%ebx	/* 32-bit caller cs */
	movw	%bx,%gs
	movl	%gs:4,%ebx		/* 32-bit ds */
	popw	%gs		/* -4 only bx,ds... */

	movw	%bx,%ds		/* 32-bit ds */

	pushl   %eax		/* -8 16bit ax */

	call 	.L41		/* -12 */
.L41:
	popl 	%ebx		/* -8 */
	addl 	$_GLOBAL_OFFSET_TABLE_+[.-.L41],%ebx
	movl 	envp_global@GOT(%ebx),%eax
	movl	(%eax), %ebx		/* ebx points to envp_global */

	popl	%eax		/* -4 get 16bit ax again */
	movw	%ax,EAX(%ebx)	/* saved into envp */

/* restore saved bx */
	popw	%ax		/* -2 get 16bit bx again */
	movw	%ax,EBX(%ebx)	
	movw	%cx,ECX(%ebx)	
	movw	%dx,EDX(%ebx)	
	movw	%si,ESI(%ebx)	
	movw	%di,EDI(%ebx)	
/* restore saved 16-bit ds */
	popw	%ax		/* -0  */
	movw	%ax,DS(%ebx)	
	movw	%es,ES(%ebx)	
	movw	%ss,SS(%ebx)	
	movw	%fs,FS(%ebx)	
	movw	%gs,GS(%ebx)	

/* push 32-bit flags */
	pushf			/*  -4 */
/* move them to eax */
	popl	%eax		/*  0 */
/* envp->reg.flags */
	movl	%eax,EFLAGS(%ebx)	
/* return ip to obtain the ordinal number */
	popl	%ecx		/*  +4 */
/* suppress hiword */
	andl	$0xffff,%ecx	
/* return cs - 32-bits */
	popl	%eax		/*   +6 */
/* bogus segment cs */
	movw	%ax,%gs		

/* GetPhysicalAddress(ss) */
/*=======*/
	pushl	%ebx		/* -4 */
	call 	.L44		/* -8 */
.L44:
	popl 	%ebx		/* -4 */
	addl 	$_GLOBAL_OFFSET_TABLE_+[.-.L44],%ebx
	movl 	LDT@GOT(%ebx),%eax
	movl	(%eax), %edx		/* ebx points to LDT */
	popl    %ebx		/* 0 */
/*=======*/
	movw	%ss,%ax		/* short 16-bit ss selector */
	andl	$0xfff8,%eax	/* suppress hiword and lower 3 bits */
	shll	$1,%eax		/* LDT element is 16 bytes long -- 8*2 */
	addl	%edx,%eax	/* offset of ss descriptor */
	movl	SELBASE(%eax),%edx /* physical address of ss data */

	xorl	%eax,%eax	/* suppress hiword */
	movw	%bp,%ax		/* 16-bit bp */
	addl	%edx,%eax	/* long bp */
	movl	%eax,EBP(%ebx)	

	xorl	%eax,%eax
	movw	%sp,%ax	
	movw	%ss:(%eax),%si	/* real 16-bit return ip */
	movw	%ss:2(%eax),%di	/* real 16-bit return cs */
	addl	%edx,%eax
	movl	%eax,ESP(%ebx)	

	shll	$16,%edi	/* move cs to the hiword */
	movw	%si,%di		/* combine cs:ip */
	movl	%edi,RETURN_ADDR(%ebx)	

/* gs override */
	movl	%gs:0,%edx	/* fetch the segimage ptr */
	movl	%edx,%eax
	subl	$8,%ecx		/* this is a correct offset */

	movw	%gs,%di
	shll	$16,%edi	/* move cs to the hiword */
	movw	%cx,%di		/* cs:ip of transfer addr */
	movl	%edi,TRANS_ADDR(%ebx)	

	addl	%ecx,%eax	/* points to our segimage entry */
	movl	TARG(%eax),%ecx	/* segimage->targ */
	movl	CONV(%eax),%eax	/* segimage->conv */

	movl	MACHINE_STACK(%ebx),%esi	
/*----------------*/
/*	GetNativeSS	 native_ss -> %eax */
	pushl	%eax
	pushl	%ebx		/* -4 */
	call 	.L47		/* -8 */
.L47:
	popl 	%ebx		/* -4 */
	addl 	$_GLOBAL_OFFSET_TABLE_+[.-.L47],%ebx
	movl 	native_ss@GOT(%ebx),%eax
	movl	(%eax), %edi		/* edi points to LDT */
	popl    %ebx		/* 0 */
	popl	%eax

/* turn off tracing during stack switch */
	pushf
	pushf
	popl	%edx
	and	$0xFFFFFEFF,%edx
	pushl	%edx
	popf
	popl	%edx

/* switch to 32-bit stack */
	movw	%di,%ss		/* switch to 32-bit stack */
	movl	%esi,%esp	/* switch to 32-bit stack */

/* restore tracing to previous state */
	pushl	%edx
	popf

	movl	8(%esp),%ebp	/* restore ebp from PUSHA frame */
/* set es,fs,gs to ds value for C environment */
	movw	%ds,%di
	movw	%di,%es
	movw	%di,%fs
	movw	%di,%gs

	pushl	%ebp
	movl	%esp,%ebp

/*------------------------------------------------------------*/
/*	DEBUG LOGGING OF API CALLS                            */
/*------------------------------------------------------------*/
#ifdef DEBUG
	pusha
	pushl	$1
	pushl	TRANS_ADDR(%ebx)
	pushl	TRANS_ADDR+2(%ebx)
	call	_LogProcName
	popl	%eax		/* cleanup */
	popl	%eax		/* cleanup */
	popl	%eax		/* cleanup */
	popa
#endif
/*------------------------------------------------------------*/

	pushl	%ecx		/* C calling convention: second parm (targ) */
	pushl	%ebx		/* C calling convention: first parm (envp_global) */

	call	*%eax		/* call to conv */

	popl	%eax
	popl	%eax		/* cleanup after the C call */

	popl	%ebp
	pushl	%ebx

	call	_GetEnvpGlobal32	/* envp_global -> %eax */

/*------------------------------------------------------------*/
/*	DEBUG LOGGING OF API CALL RESULTS                     */
/*------------------------------------------------------------*/
#ifdef DEBUG
	pusha
	pushl	$0
	pushl	EDX(%eax)
	pushl	EAX(%eax)
	call	_LogProcName
	popl	%eax		/* cleanup */
	popl	%eax		/* cleanup */
	popl	%eax		/* cleanup */
	popa
#endif

/*------------------------------------------------------------*/

	movl	RETURN_ADDR(%eax),%ebx
	movl	%ebx,TRANS_ADDR(%eax)
	popl	%ebx
	jmp	return_to_binary

	.type	_invoke_native,@function
	.size	_invoke_native,.-_invoke_native

	.align	4
	.globl	invoke_code16
	.globl	_invoke_code16

invoke_code16:
_invoke_code16:

/* save all registers */
/* these will be popped in return_native */
	pusha			

	.globl	return_to_binary
	.globl	_return_to_binary

return_to_binary:
_return_to_binary:

/* get the global envp pointer, use %ebx throughout... */
	pushl   %eax			/* 16bit ax */

	call .L42
.L42:
	popl 	%ebx
	addl 	$_GLOBAL_OFFSET_TABLE_+[.-.L42],%ebx
	movl 	envp_global@GOT(%ebx),%eax
	movl	(%eax), %ebx		/* ebx points to envp_global */

	popl	%eax			/* get 16bit ax again */

/* save machine_stack in env; it will be picked up in invoke_native */
	movl	%esp,MACHINE_STACK(%ebx)		

	movw	SS(%ebx),%ax		/* short 16-bit ss selector */
	pushl	%eax
	andl	$0xfff8,%eax		/* suppress hiword and DPL bits */
	pushl	%eax			/* %eax holds Stack Selector... */
/* physical address of ss  (currently in %eax) */
	call	_GetSelectorBase	/* GetSelectorBase(%eax) -> %eax */
	popl	%edx			/* cleanup after C call */

/* %di = 16-bit ss */
	popl	%edi			
	andl	$0xffff,%edi
	movl	%eax,%edx		/* phys. addr. of ss data */
	
	movl	TRANS_ADDR(%ebx),%eax
	xorl	%esi,%esi
	movw	%ax,%si			/* ip of trans_addr */
	shrl	$16,%eax		/* cs of trans_addr */
	movl	%eax,%ecx		/* LOWORD(cx) - trans cs --- */
	pushl	%ecx			/* save trans cs */
	pushl	%edx			/* save phys(ss) */

	call	_TWIN_GetReturnThunkSel	/* _return_thunk_selector -> %eax */
	movw	%ax,%gs

/* physical address of gs */
	pushl	%eax
	call	_GetSelectorBase	/* %eax holds return_thunk.. */
	popl	%ecx			/* cleanup (%eax popped) */

	popl	%edx			/* restore %edx - phys(ss) */
	popl	%ecx			/* restore trans cs --- */

/* constructing long address for indirect jmp at gs:0x10 */
	movw	%si,0x10(%eax)
	movw	%cx,0x12(%eax)

	movl	ESP(%ebx),%eax		/* long sp */
	subl	%edx,%eax		/* short sp the 16bit sp is in eax */


/* turn off tracing, it will be restored when EFLAGS are loaded */
 	pushf
 	popl	%ecx
	movl	%ecx,%esi
	andw	$0x0100,%cx
	orw	%cx,EFLAGS(%ebx)
	and	$0xFFFFFEFF,%esi
	pushl	%esi
 	popf

/* stack switch */
	xorl	%esp,%esp
	movw	%di,%ss			/* switch to 16-bit stack */
	movw	%ax,%sp

	movl	EBP(%ebx),%eax		/* long bp */
	subl	%edx,%eax		/* short bp */
	xorl	%ebp,%ebp
	movw	%ax,%bp			/* bp set */

/* push short bx */
	pushw	EBX(%ebx)		
	movw	EAX(%ebx),%ax		/* set */
	movw	ECX(%ebx),%cx		/* set */
	movw	EDX(%ebx),%dx		/* set */
	movw	ESI(%ebx),%si		/* set */
	movw	EDI(%ebx),%di		/* set */
/* push short flags */
	pushw	EFLAGS(%ebx)		

/* used to be popfw on ncr */
	.byte	0x66
	popf			
/* flags set 16-bit pop */

	movw	ES(%ebx),%es
	movw	FS(%ebx),%fs
	movw	DS(%ebx),%ds

	popw	%bx			/* bx set */

/**************************************************/
	.globl	WarpNexus
	.globl	_WarpNexus
/**************************************************/
/*      Last chance to break before warping out...*/
/**************************************************/
_WarpNexus:
WarpNexus:

/* operand & address overrides */
	.byte	0x66			
	.byte	0x67

/*	ljmp	gs:[0x10] */
/*       gs override */
	.byte	0x65			
	.byte	0xff
	.byte	0x2e
	.byte	0x10
	.byte	0x00

	.type	_invoke_code16,@function
	.size	_invoke_code16,.-_invoke_code16

	.align	4
	.globl	return_to_native
	.globl	_return_to_native

return_to_native:
_return_to_native:
	popl	%ebx
	popl	%ebx		/* 32-bit caller cs */
	movw	%bx,%gs
	movl	%gs:8,%ebx	/* 32-bit native ds */
	movw	%bx,%ds		/* switch to it */


	pushl   %eax			/* 16bit ax  */

	call .L43
.L43:
	popl    %ebx
	addl    $_GLOBAL_OFFSET_TABLE_+[.-.L43],%ebx
	movl    envp_global@GOT(%ebx),%eax
	movl	(%eax), %ebx		/* ebx points to envp_global */

	popl	%eax			/* get 16bit ax again */

	movw	%ax,EAX(%ebx)
	movw	%dx,EDX(%ebx)

/*-----------------------*/
/* GetPhysicalAddress(ss) */
	pushl	%ebx		/* -4 */
	call 	.L45		/* -8 */
.L45:
	popl 	%ebx		/* -4 */
	addl 	$_GLOBAL_OFFSET_TABLE_+[.-.L45],%ebx
	movl 	LDT@GOT(%ebx),%eax
	movl	(%eax), %edx		/* ebx points to LDT */
	popl    %ebx		/* 0 */
/*-----------------------*/
	movw	%ss,%ax		/* short 16-bit ss selector */
	andl	$0xfff8,%eax	/* suppress hiword and lower 3 bits */
	shll	$1,%eax		/* LDT element is 16 bytes long -- >>3<<4 */
	addl	%edx,%eax	/* offset of ss descriptor */
	movl	SELBASE(%eax),%edx	/* physical address of ss data */

	xorl	%eax,%eax
	movw	%sp,%ax	
	addl	%edx,%eax
	movl	%eax,ESP(%ebx)	

/*-----------------*/
/* GetNativeSS	*/
	pushl	%ebx		/* -4 */
	call 	.L46		/* -8 */
.L46:
	popl 	%ebx		/* -4 */
	addl 	$_GLOBAL_OFFSET_TABLE_+[.-.L46],%ebx
	movl 	native_ss@GOT(%ebx),%eax
	movl	(%eax), %edx		/* ebx points to LDT */
	popl    %ebx		/* 0 */
/*-----------------*/
	movl	MACHINE_STACK(%ebx),%eax
	movw	%dx,%ss
	movl	%eax,%esp	/* switch to hard-coded 32-bit stack */
	pushw	%ds
	popw	%es

/* restore native environment */
	popa			
	ret

	.type	_return_to_native,@function
	.size	_return_to_native,.-_return_to_native

	.align	4
	.globl  get_sel_limit
	.globl  _get_sel_limit

get_sel_limit:
_get_sel_limit:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%ecx

	xorl	%eax,%eax
	movw 	8(%ebp),%ax
	lsl	%eax,%ecx
	jnz	err
okay:
	movl	%ecx,%eax
	popl	%ecx
	leave
	ret	
err:
	xorl	%eax,%eax
	popl	%ecx
	leave
	ret	

	.type	_get_sel_limit,@function
	.size	_get_sel_limit,.-_get_sel_limit


	.align	4
	.globl	get_native_cs
	.globl	_get_native_cs

get_native_cs:
_get_native_cs:
	xorl	%eax,%eax
	movw	%cs,%ax
	ret

	.type	_get_native_cs,@function
	.size	_get_native_cs,.-_get_native_cs

	.align	4
	.globl	get_native_ds
	.globl	_get_native_ds

get_native_ds:
_get_native_ds:
	xorl	%eax,%eax
	movw	%ds,%ax
	ret

	.type	_get_native_ds,@function
	.size	_get_native_ds,.-_get_native_ds

	.align	4
	.globl	get_native_ss
	.globl	_get_native_ss

get_native_ss:
_get_native_ss:
	xorl	%eax,%eax
	movw	%ss,%ax
	ret

	.type	_get_native_ss,@function
	.size	_get_native_ss,.-_get_native_ss

/*====================*/
/* read_seg           */
/*--------------------*/
	.align	4
	.globl	read_seg
	.globl	_read_seg
read_seg:
_read_seg:
	pushl	%ebp
	movl	%esp,%ebp

	pushl	%ebx
	pushl	%ecx
	push	%es

	xorl	%eax,%eax
	movw 	8(%ebp),%ax

	movw	%ax, %es

	xorl	%ebx,%ebx
	movw	12(%ebp),%bx

	xorl	%eax,%eax
	movb 	%es:(%ebx),%al

	pop	%es
	popl	%ecx
	popl	%ebx

	leave
	ret	
	.align	4
	.type	_read_seg,@function
	.size	_read_seg,.-_read_seg


/*====================*/
/* debuggerbreak      */
/*--------------------*/
	.align	4
	.globl	debuggerbreak

debuggerbreak:
	int	$3
	ret
	.align	4
	.type	debuggerbreak,@function
	.size	debuggerbreak,.-debuggerbreak

