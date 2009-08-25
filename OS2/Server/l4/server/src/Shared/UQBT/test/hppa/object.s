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
	.IMPORT __tf3ios,CODE
	.IMPORT _throw_failure__C3ios,CODE
	.IMPORT fail__C3ios,CODE
	.IMPORT clear__3iosi,CODE
	.IMPORT sync_with_stdio__3iosi,CODE
	.IMPORT __tf9streambuf,CODE
	.IMPORT _IO_free_backup_area,CODE
	.IMPORT in_backup__9streambuf,CODE
	.IMPORT _IO_seekoff,CODE
	.IMPORT _IO_seekpos,CODE
	.IMPORT base__C9streambuf,CODE
	.IMPORT unbuffered__9streambuf,CODE
	.IMPORT doallocate__9streambuf,CODE
	.IMPORT doallocbuf__9streambuf,CODE
	.IMPORT xsputn__9streambufPCci,CODE
	.IMPORT _IO_padn,CODE
	.IMPORT _IO_sgetn,CODE
	.IMPORT _IO_putc,CODE
	.IMPORT _IO_getc,CODE
	.IMPORT __underflow,CODE
	.IMPORT sgetc__9streambuf,CODE
	.IMPORT __tf7filebuf,CODE
	.IMPORT is_open__C7filebuf,CODE
	.IMPORT eback__C9streambuf,CODE
	.IMPORT egptr__C9streambuf,CODE
	.IMPORT is_reading__7filebuf,CODE
	.IMPORT gptr__C9streambuf,CODE
	.IMPORT pptr__C9streambuf,CODE
	.IMPORT eGptr__9streambuf,CODE
	.IMPORT _vt$3ios,DATA
	.IMPORT init__3iosP9streambufP7ostream,CODE
	.IMPORT __builtin_vec_delete,CODE
	.IMPORT __builtin_delete,CODE
	.IMPORT __tf7ostream,CODE
	.IMPORT __3iosP9streambufP7ostream,CODE
	.IMPORT _$_3ios,CODE
	.IMPORT __terminate,CODE
	.IMPORT _vt$7ostream$3ios,DATA
	.IMPORT __sjthrow,CODE
	.IMPORT good__C3ios,CODE
	.IMPORT flush__7ostream,CODE
	.IMPORT flags__C3ios,CODE
	.IMPORT do_osfx__7ostream,CODE
	.IMPORT sputc__9streambufi,CODE
	.IMPORT write__7ostreamPCci,CODE
	.IMPORT __ls__7ostreamc,CODE
	.IMPORT __ls__7ostreamPCc,CODE
	.IMPORT __ls__7ostreami,CODE
	.IMPORT __ls__7ostreamUi,CODE
	.IMPORT __ls__7ostreamd,CODE
	.IMPORT __tf7istream,CODE
	.IMPORT _vt$7istream$3ios,DATA
	.IMPORT get__7istreamPcic,CODE
	.IMPORT get__7istreamRc,CODE
	.IMPORT getline__7istreamPcic,CODE
	.IMPORT read__7istreamPci,CODE
	.IMPORT set__3iosi,CODE
	.IMPORT rdbuf__C3ios,CODE
	.IMPORT in_avail__9streambuf,CODE
	.IMPORT _skip_ws__7istream,CODE
	.IMPORT ipfx1__7istream,CODE
	.IMPORT sbumpc__9streambuf,CODE
	.IMPORT isfx__7istream,CODE
	.IMPORT sputbackc__9streambufc,CODE
	.IMPORT sungetc__9streambuf,CODE
	.IMPORT __rs__7istreamPc,CODE
	.IMPORT __rs__7istreamRc,CODE
	.IMPORT __tf8iostream,CODE
	.IMPORT __7istreami,CODE
	.IMPORT _$_7istream,CODE
	.IMPORT __7ostreami,CODE
	.IMPORT _$_7ostream,CODE
	.IMPORT _vt$8iostream$3ios,DATA
	.IMPORT __tf22_IO_istream_withassign,CODE
	.IMPORT __as__22_IO_istream_withassignR7istream,CODE
	.IMPORT __tf22_IO_ostream_withassign,CODE
	.IMPORT __as__22_IO_ostream_withassignR7ostream,CODE
	.IMPORT setf__3iosUlUl,CODE
	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT __5intob,ENTRY,PRIV_LEV=3,ARGW0=GR,RTNVAL=GR
__5intob
	.PROC
	.CALLINFO FRAME=64,NO_CALLS,SAVE_SP,ENTRY_GR=3
	.ENTRY
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	copy %r26,%r19
	ldi 100,%r20
	stw %r20,0(%r19)
	ldi 3,%r20
	stw %r20,4(%r19)
L$0286
	copy %r19,%r28
	b,n L$0285
L$0285
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.IMPORT cout,DATA
	.SPACE $TEXT$
	.SUBSPA $LIT$

	.align 4
L$C0000
	.STRING "Ha! intob destructed!\x0a\x00"
	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT _$_5intob,ENTRY,PRIV_LEV=3,ARGW0=GR,ARGW1=GR
_$_5intob
	.PROC
	.CALLINFO FRAME=64,CALLS,SAVE_RP,SAVE_SP,ENTRY_GR=4
	.ENTRY
	stw %r2,-20(%r30)
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	stw %r4,8(%r3)
	copy %r26,%r4
	stw %r25,-40(%r3)
	addil LR'cout-$global$,%r27
	ldo RR'cout-$global$(%r1),%r26
	ldil LR'L$C0000,%r19
	ldo RR'L$C0000(%r19),%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl __ls__7ostreamPCc,%r2
	nop
L$0288
	ldw -40(%r3),%r20
	extru %r20,31,1,%r19
	comib,=,n 0,%r19,L$0290
	copy %r4,%r26
	.CALL ARGW0=GR
	bl __builtin_delete,%r2
	nop
	b,n L$0290
L$0289
L$0290
L$0287
	ldw -20(%r3),%r2
	ldw 8(%r3),%r4
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT math__C5intob,ENTRY,PRIV_LEV=3,ARGW0=GR,RTNVAL=GR
math__C5intob
	.PROC
	.CALLINFO FRAME=64,NO_CALLS,SAVE_SP,ENTRY_GR=3
	.ENTRY
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	copy %r26,%r19
	ldw 0(%r19),%r26
	ldw 4(%r19),%r25
	.IMPORT $$divI,MILLICODE
	bl $$divI,%r31
	nop
	copy %r29,%r20
	copy %r20,%r28
	b,n L$0291
L$0291
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.IMPORT __tf6thinob,CODE
	.IMPORT __tf5fatob,CODE
	.IMPORT _vt$6thinob,DATA
	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT __6thinob,ENTRY,PRIV_LEV=3,ARGW0=GR,RTNVAL=GR
__6thinob
	.PROC
	.CALLINFO FRAME=64,NO_CALLS,SAVE_SP,ENTRY_GR=3
	.ENTRY
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	copy %r26,%r19
	addil LR'_vt$6thinob-$global$,%r27
	ldo RR'_vt$6thinob-$global$(%r1),%r20
	stw %r20,8(%r19)
	ldi 100,%r20
	stw %r20,0(%r19)
	ldi 3,%r20
	stw %r20,4(%r19)
L$0293
	copy %r19,%r28
	b,n L$0292
L$0292
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT math__C6thinob,ENTRY,PRIV_LEV=3,ARGW0=GR,RTNVAL=GR
math__C6thinob
	.PROC
	.CALLINFO FRAME=64,NO_CALLS,SAVE_SP,ENTRY_GR=3
	.ENTRY
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	copy %r26,%r19
	ldw 0(%r19),%r26
	ldw 4(%r19),%r25
	bl $$divI,%r31
	nop
	copy %r29,%r20
	copy %r20,%r28
	b,n L$0294
L$0294
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.IMPORT _vt$5fatob,DATA
	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT __5fatob,ENTRY,PRIV_LEV=3,ARGW0=GR,RTNVAL=GR
__5fatob
	.PROC
	.CALLINFO FRAME=64,CALLS,SAVE_RP,SAVE_SP,ENTRY_GR=4
	.ENTRY
	stw %r2,-20(%r30)
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	stw %r4,8(%r3)
	copy %r26,%r4
	copy %r4,%r26
	.CALL ARGW0=GR
	bl __6thinob,%r2
	nop
	addil LR'_vt$5fatob-$global$,%r27
	ldo RR'_vt$5fatob-$global$(%r1),%r19
	stw %r19,8(%r4)
	ldi 2,%r19
	stw %r19,12(%r4)
L$0296
	copy %r4,%r28
	b,n L$0295
L$0295
	ldw -20(%r3),%r2
	ldw 8(%r3),%r4
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT math__C5fatob,ENTRY,PRIV_LEV=3,ARGW0=GR,RTNVAL=GR
math__C5fatob
	.PROC
	.CALLINFO FRAME=64,NO_CALLS,SAVE_SP,ENTRY_GR=3
	.ENTRY
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	copy %r26,%r19
	ldw 0(%r19),%r26
	ldw 4(%r19),%r25
	bl $$divI,%r31
	nop
	copy %r29,%r20
	copy %r20,%r26
	ldw 12(%r19),%r25
	bl $$divI,%r31
	nop
	copy %r29,%r20
	copy %r20,%r28
	b,n L$0297
L$0297
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.EXPORT i_global,DATA
	.SPACE $PRIVATE$
	.SUBSPA $DATA$

	.align 4
i_global
	.blockz 8
	.IMPORT __main,CODE
	.IMPORT __builtin_new,CODE
	.IMPORT endl__FR7ostream,CODE
	.IMPORT __ls__7ostreamPFR7ostream_R7ostream,CODE
	.SPACE $TEXT$
	.SUBSPA $LIT$

	.align 4
L$C0001
	.STRING "Quotient is \x00"
	.align 4
L$C0003
	.STRING "Math is \x00"
	.IMPORT __get_eh_context,CODE
	.SPACE $PRIVATE$
	.SUBSPA $DATA$

	.align 4
L$C0002
	.word P%endl__FR7ostream
	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT main,ENTRY,PRIV_LEV=3,RTNVAL=GR
main
	.PROC
	.CALLINFO FRAME=320,CALLS,SAVE_RP,SAVE_SP,ENTRY_GR=18,ENTRY_FR=21
	.ENTRY
	stw %r2,-20(%r30)
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,320(%r30)
	stw %r18,88(%r3)
	stw %r17,92(%r3)
	stw %r16,96(%r3)
	stw %r15,100(%r3)
	stw %r14,104(%r3)
	stw %r13,108(%r3)
	stw %r12,112(%r3)
	stw %r11,116(%r3)
	stw %r10,120(%r3)
	stw %r9,124(%r3)
	stw %r8,128(%r3)
	stw %r7,132(%r3)
	stw %r6,136(%r3)
	stw %r5,140(%r3)
	stw %r4,144(%r3)
	ldo 152(%r3),%r1
	fstds,ma %fr21,8(%r1)
	fstds,ma %fr20,8(%r1)
	fstds,ma %fr19,8(%r1)
	fstds,ma %fr18,8(%r1)
	fstds,ma %fr17,8(%r1)
	fstds,ma %fr16,8(%r1)
	fstds,ma %fr15,8(%r1)
	fstds,ma %fr14,8(%r1)
	fstds,ma %fr13,8(%r1)
	fstds,ma %fr12,8(%r1)
	.CALL 
	bl __get_eh_context,%r2
	nop
	copy %r28,%r19
	copy %r19,%r20
	stw %r20,-16(%r30)
	fldws -16(%r30),%fr22L
	fcpy,sgl %fr22L,%fr23L
	fstws %fr23L,-16(%r30)
	ldw -16(%r30),%r21
	stw %r21,72(%r3)
	.CALL 
	bl __main,%r2
	nop
	ldi 16,%r26
	.CALL ARGW0=GR
	bl __builtin_new,%r2
	nop
	copy %r28,%r19
	stw %r19,64(%r3)
	ldi 1,%r19
	stw %r19,68(%r3)
	ldw 72(%r3),%r20
	ldo 4(%r20),%r19
	ldw 0(%r19),%r20
	stw %r20,16(%r3)
	stw %r0,20(%r3)
	ldo 24(%r3),%r20
	ldo 8(%r3),%r21
	stw %r21,0(%r20)
	ldil LR'L$0302,%r22
	ldo RR'L$0302(%r22),%r21
	stw %r21,4(%r20)
	stw %r30,8(%r20)
	ldi 0,%r20
	b,n L$0301
L$0302
	ldo -8(%r3),%r21
	copy %r21,%r3
	ldi 1,%r20
	b,n L$0299
L$0301
	ldo 16(%r3),%r20
	stw %r20,0(%r19)
	ldw 64(%r3),%r26
	.CALL ARGW0=GR
	bl __5fatob,%r2
	nop
	copy %r28,%r19
	stw %r0,68(%r3)
	stw %r19,8(%r3)
	ldw 72(%r3),%r21
	ldo 4(%r21),%r19
	ldw 0(%r19),%r20
	ldw 0(%r20),%r21
	stw %r21,0(%r19)
	ldw 68(%r3),%r19
	comib,=,n 0,%r19,L$0304
	ldw 64(%r3),%r26
	.CALL ARGW0=GR
	bl __builtin_delete,%r2
	nop
	b,n L$0304
L$0303
L$0304
	addil LR'cout-$global$,%r27
	ldo RR'cout-$global$(%r1),%r26
	ldil LR'L$C0001,%r19
	ldo RR'L$C0001(%r19),%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl __ls__7ostreamPCc,%r2
	nop
	stw %r28,76(%r3)
	addil LR'i_global-$global$,%r27
	ldo RR'i_global-$global$(%r1),%r26
	.CALL ARGW0=GR
	bl math__C5intob,%r2
	nop
	copy %r28,%r19
	ldw 76(%r3),%r26
	copy %r19,%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl __ls__7ostreami,%r2
	nop
	copy %r28,%r19
	copy %r19,%r26
	ldil LR'L$C0002,%r20
	ldo RR'L$C0002(%r20),%r19
	ldw 0(%r19),%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl __ls__7ostreamPFR7ostream_R7ostream,%r2
	nop
	addil LR'cout-$global$,%r27
	ldo RR'cout-$global$(%r1),%r26
	ldil LR'L$C0003,%r19
	ldo RR'L$C0003(%r19),%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl __ls__7ostreamPCc,%r2
	nop
	stw %r28,80(%r3)
	ldw 8(%r3),%r19
	ldw 8(%r19),%r20
	ldo 8(%r20),%r21
	stw %r21,84(%r3)
	ldw 84(%r3),%r19
	ldh 0(%r19),%r20
	extrs %r20,31,16,%r19
	ldw 8(%r3),%r20
	addl %r19,%r20,%r19
	ldw 84(%r3),%r21
	ldw 4(%r21),%r20
	copy %r19,%r26
	copy %r20,%r22
	.CALL	ARGW0=GR
	bl $$dyncall,%r31
	copy %r31,%r2
	copy %r28,%r19
	ldw 80(%r3),%r26
	copy %r19,%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl __ls__7ostreami,%r2
	nop
	copy %r28,%r19
	copy %r19,%r26
	ldil LR'L$C0002,%r20
	ldo RR'L$C0002(%r20),%r19
	ldw 0(%r19),%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl __ls__7ostreamPFR7ostream_R7ostream,%r2
	nop
	ldi 0,%r28
	b,n L$0298
	ldi 0,%r28
	b,n L$0298
	b,n L$0311
L$0299
	ldw 72(%r3),%r20
	ldo 4(%r20),%r19
	ldw 0(%r19),%r20
	stw %r20,40(%r3)
	stw %r0,44(%r3)
	ldo 48(%r3),%r20
	ldo 8(%r3),%r21
	stw %r21,0(%r20)
	ldil LR'L$0308,%r22
	ldo RR'L$0308(%r22),%r21
	stw %r21,4(%r20)
	stw %r30,8(%r20)
	ldi 0,%r20
	b,n L$0307
L$0308
	ldo -8(%r3),%r21
	copy %r21,%r3
	ldi 1,%r20
	b,n L$0305
L$0307
	ldo 40(%r3),%r20
	stw %r20,0(%r19)
	ldw 68(%r3),%r21
	comib,=,n 0,%r21,L$0310
	ldw 64(%r3),%r26
	.CALL ARGW0=GR
	bl __builtin_delete,%r2
	nop
	b,n L$0310
L$0309
L$0310
	ldw 72(%r3),%r20
	ldo 4(%r20),%r19
	ldw 0(%r19),%r20
	ldw 0(%r20),%r21
	stw %r21,0(%r19)
	.CALL 
	bl __sjthrow,%r2
	nop
L$0305
	.CALL 
	bl __terminate,%r2
	nop
L$0311
L$0298
	ldw -20(%r3),%r2
	ldw 88(%r3),%r18
	ldw 92(%r3),%r17
	ldw 96(%r3),%r16
	ldw 100(%r3),%r15
	ldw 104(%r3),%r14
	ldw 108(%r3),%r13
	ldw 112(%r3),%r12
	ldw 116(%r3),%r11
	ldw 120(%r3),%r10
	ldw 124(%r3),%r9
	ldw 128(%r3),%r8
	ldw 132(%r3),%r7
	ldw 136(%r3),%r6
	ldw 140(%r3),%r5
	ldw 144(%r3),%r4
	ldo 152(%r3),%r1
	fldds,ma 8(%r1),%fr21
	fldds,ma 8(%r1),%fr20
	fldds,ma 8(%r1),%fr19
	fldds,ma 8(%r1),%fr18
	fldds,ma 8(%r1),%fr17
	fldds,ma 8(%r1),%fr16
	fldds,ma 8(%r1),%fr15
	fldds,ma 8(%r1),%fr14
	fldds,ma 8(%r1),%fr13
	fldds,ma 8(%r1),%fr12
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.EXPORT _vt$5fatob,DATA
	.SPACE $PRIVATE$
	.SUBSPA $DATA$

	.align 8
_vt$5fatob
	.half 0
	.half 0
	.word P%__tf5fatob
	.half 0
	.half 0
	.word P%math__C5fatob
	.blockz 8
	.EXPORT _vt$6thinob,DATA
	.align 8
_vt$6thinob
	.half 0
	.half 0
	.word P%__tf6thinob
	.half 0
	.half 0
	.word P%math__C6thinob
	.blockz 8
	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.PARAM __static_initialization_and_destruction_0,ARGW0=GR,ARGW1=GR
__static_initialization_and_destruction_0
	.PROC
	.CALLINFO FRAME=64,CALLS,SAVE_RP,SAVE_SP,ENTRY_GR=3
	.ENTRY
	stw %r2,-20(%r30)
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	stw %r26,-36(%r3)
	stw %r25,-40(%r3)
	ldw -40(%r3),%r19
	zdepi -1,31,16,%r20
	comb,<>,n %r20,%r19,L$0313
	ldw -36(%r3),%r19
	comib,=,n 0,%r19,L$0314
	addil LR'i_global-$global$,%r27
	ldo RR'i_global-$global$(%r1),%r26
	.CALL ARGW0=GR
	bl __5intob,%r2
	nop
	b,n L$0313
L$0314
	addil LR'i_global-$global$,%r27
	ldo RR'i_global-$global$(%r1),%r26
	ldi 2,%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl _$_5intob,%r2
	nop
L$0315
L$0313
L$0312
	ldw -20(%r3),%r2
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.SPACE $PRIVATE$
	.SUBSPA $BSS$

__ti6thinob	.comm 8
	.IMPORT __rtti_user,CODE
	.SPACE $TEXT$
	.SUBSPA $LIT$

	.align 4
L$C0004
	.STRING "6thinob\x00"
	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT __tf6thinob,ENTRY,PRIV_LEV=3,RTNVAL=GR
__tf6thinob
	.PROC
	.CALLINFO FRAME=64,CALLS,SAVE_RP,SAVE_SP,ENTRY_GR=3
	.ENTRY
	stw %r2,-20(%r30)
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	addil LR'__ti6thinob-$global$,%r27
	ldo RR'__ti6thinob-$global$(%r1),%r19
	ldw 0(%r19),%r20
	comib,<>,n 0,%r20,L$0317
	addil LR'__ti6thinob-$global$,%r27
	ldo RR'__ti6thinob-$global$(%r1),%r26
	ldil LR'L$C0004,%r19
	ldo RR'L$C0004(%r19),%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl __rtti_user,%r2
	nop
L$0317
	addil LR'__ti6thinob-$global$,%r27
	ldo RR'__ti6thinob-$global$(%r1),%r28
	b,n L$0316
L$0316
	ldw -20(%r3),%r2
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.SPACE $PRIVATE$
	.SUBSPA $BSS$

__ti5fatob	.comm 12
	.IMPORT __rtti_si,CODE
	.SPACE $TEXT$
	.SUBSPA $LIT$

	.align 4
L$C0005
	.STRING "5fatob\x00"
	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT __tf5fatob,ENTRY,PRIV_LEV=3,RTNVAL=GR
__tf5fatob
	.PROC
	.CALLINFO FRAME=64,CALLS,SAVE_RP,SAVE_SP,ENTRY_GR=3
	.ENTRY
	stw %r2,-20(%r30)
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	addil LR'__ti5fatob-$global$,%r27
	ldo RR'__ti5fatob-$global$(%r1),%r19
	ldw 0(%r19),%r20
	comib,<>,n 0,%r20,L$0319
	.CALL 
	bl __tf6thinob,%r2
	nop
	copy %r28,%r19
	addil LR'__ti5fatob-$global$,%r27
	ldo RR'__ti5fatob-$global$(%r1),%r26
	ldil LR'L$C0005,%r19
	ldo RR'L$C0005(%r19),%r25
	addil LR'__ti6thinob-$global$,%r27
	ldo RR'__ti6thinob-$global$(%r1),%r24
	.CALL ARGW0=GR,ARGW1=GR,ARGW2=GR
	bl __rtti_si,%r2
	nop
L$0319
	addil LR'__ti5fatob-$global$,%r27
	ldo RR'__ti5fatob-$global$(%r1),%r28
	b,n L$0318
L$0318
	ldw -20(%r3),%r2
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT _GLOBAL_$I$__5intob,ENTRY,PRIV_LEV=3
_GLOBAL_$I$__5intob
	.PROC
	.CALLINFO FRAME=64,CALLS,SAVE_RP,SAVE_SP,ENTRY_GR=3
	.ENTRY
	stw %r2,-20(%r30)
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	ldi 1,%r26
	zdepi -1,31,16,%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl __static_initialization_and_destruction_0,%r2
	nop
L$0320
	ldw -20(%r3),%r2
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
	.align 4
	.NSUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.EXPORT _GLOBAL_$D$__5intob,ENTRY,PRIV_LEV=3
_GLOBAL_$D$__5intob
	.PROC
	.CALLINFO FRAME=64,CALLS,SAVE_RP,SAVE_SP,ENTRY_GR=3
	.ENTRY
	stw %r2,-20(%r30)
	copy %r3,%r1
	copy %r30,%r3
	stwm %r1,64(%r30)
	ldi 0,%r26
	zdepi -1,31,16,%r25
	.CALL ARGW0=GR,ARGW1=GR
	bl __static_initialization_and_destruction_0,%r2
	nop
L$0321
	ldw -20(%r3),%r2
	ldo 64(%r3),%r30
	ldwm -64(%r30),%r3
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
