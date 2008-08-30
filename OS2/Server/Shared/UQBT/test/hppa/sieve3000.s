	.LEVEL 1.1
	.SPACE $PRIVATE$
	.SUBSPA $DATA$,QUAD=1,ALIGN=8,ACCESS=31
	.SUBSPA $BSS$,QUAD=1,ALIGN=8,ACCESS=31,ZERO,SORT=82
	.SPACE $TEXT$
	.SUBSPA $LIT$,QUAD=0,ALIGN=8,ACCESS=44
	.SUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.IMPORT $global$,DATA
	.IMPORT $$dyncall,MILLICODE
; gcc_compiled.:
	.IMPORT __main,CODE
	.IMPORT printf,CODE
	.SPACE $TEXT$
	.SUBSPA $LIT$

	.align 4
L$C0000
	.STRING "%i\x0a\x00"
	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT main,ENTRY,PRIV_LEV=3
main
	.PROC
	.CALLINFO FRAME=128,CALLS,SAVE_RP,SAVE_SP,ENTRY_GR=3
	.ENTRY
	stw %r2,-20(%r30)
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,128(%r30)
	.CALL 
	bl __main,%r2
	nop
	stw %r0,8(%r3)
	ldi 3,%r19
	stw %r19,12(%r3)
L$0003
	ldw 8(%r3),%r19
	ldi 2999,%r20
	comb,>>=,n %r20,%r19,L$0006
	b,n L$0004
L$0006
	ldi 1,%r19
	stw %r19,20(%r3)
	ldi 3,%r19
	stw %r19,16(%r3)
L$0007
	ldw 16(%r3),%r19
	ldw 12(%r3),%r20
	comb,>>,n %r20,%r19,L$0010
	b,n L$0008
L$0010
	ldw 12(%r3),%r19
	copy %r19,%r26
	ldw 16(%r3),%r25
	.IMPORT $$remU,MILLICODE
	bl $$remU,%r31
	nop
	copy %r29,%r19
	comib,<>,n 0,%r19,L$0009
	stw %r0,20(%r3)
L$0011
L$0009
	ldw 16(%r3),%r19
	ldo 2(%r19),%r20
	stw %r20,16(%r3)
	b,n L$0007
L$0008
	ldw 20(%r3),%r19
	comib,=,n 0,%r19,L$0005
	ldil LR'L$C0000,%r19
	ldo RR'L$C0000(%r19),%r26
	ldw 12(%r3),%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl printf,%r2
	nop
	ldw 8(%r3),%r19
	ldo 1(%r19),%r20
	stw %r20,8(%r3)
L$0012
L$0005
	ldw 12(%r3),%r19
	ldo 2(%r19),%r20
	stw %r20,12(%r3)
	b,n L$0003
L$0004
L$0002
	ldw -20(%r3),%r2
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
