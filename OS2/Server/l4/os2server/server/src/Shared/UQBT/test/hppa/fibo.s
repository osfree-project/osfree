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
	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT fib,ENTRY,PRIV_LEV=3,ARGW0=GR,RTNVAL=GR
fib
	.PROC
	.CALLINFO FRAME=64,CALLS,SAVE_RP,SAVE_SP,ENTRY_GR=4
	.ENTRY
	stw %r2,-20(%r30)
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	stw %r4,8(%r3)
	stw %r26,-36(%r3)
	ldw -36(%r3),%r19
	comib,>=,n 1,%r19,L$0003
	ldw -36(%r3),%r20
	ldo -1(%r20),%r19
	copy %r19,%r26
	.CALL ARGW0=GR
	bl fib,%r2
	nop
	copy %r28,%r4
	ldw -36(%r3),%r20
	ldo -2(%r20),%r19
	copy %r19,%r26
	.CALL ARGW0=GR
	bl fib,%r2
	nop
	copy %r28,%r20
	addl %r4,%r20,%r19
	copy %r19,%r28
	b,n L$0002
	b,n L$0004
L$0003
	ldw -36(%r3),%r19
	copy %r19,%r28
	b,n L$0002
L$0004
L$0002
	ldw -20(%r3),%r2
	ldw 8(%r3),%r4
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.IMPORT __main,CODE
	.IMPORT printf,CODE
	.SPACE $TEXT$
	.SUBSPA $LIT$

	.align 4
L$C0000
	.STRING "Input number: \x00"
	.IMPORT scanf,CODE
	.align 4
L$C0001
	.STRING "%d\x00"
	.align 4
L$C0002
	.STRING "fibonacci(%d) = %d\x0a\x00"
	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT main,ENTRY,PRIV_LEV=3,RTNVAL=GR
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
	ldil LR'L$C0000,%r19
	ldo RR'L$C0000(%r19),%r26
	.CALL ARGW0=GR
	bl printf,%r2
	nop
	ldil LR'L$C0001,%r19
	ldo RR'L$C0001(%r19),%r26
	ldo 8(%r3),%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl scanf,%r2
	nop
	ldw 8(%r3),%r26
	.CALL ARGW0=GR
	bl fib,%r2
	nop
	copy %r28,%r19
	stw %r19,12(%r3)
	ldil LR'L$C0002,%r19
	ldo RR'L$C0002(%r19),%r26
	ldw 8(%r3),%r25
	ldw 12(%r3),%r24
	.CALL ARGW0=GR,ARGW1=GR,ARGW2=GR
	bl printf,%r2
	nop
	ldi 0,%r28
	b,n L$0005
L$0005
	ldw -20(%r3),%r2
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
