00000000	53746172746572           	DC.B	'Starter'
00000007	00                       	DC.B	#0
00000008	00000000                 	DC.W	#0,#0
0000000c	00000000                 	DC.W	#0,#0
00000010	00000000                 	DC.W	#0,#0
00000014	00000000                 	DC.W	#0,#0
00000018	00000000                 	DC.W	#0,#0
0000001c	00000000                 	DC.W	#0,#0
00000020	00010001                 	DC.W	#1,#1
00000024	b1a59e0f                 	DC.W	#-20059,#-25073
00000028	b1a59e0f                 	DC.W	#-20059,#-25073
0000002c	00000000                 	DC.W	#0,#0
00000030	00000000                 	DC.W	#0,#0
00000034	00000000                 	DC.W	#0,#0
00000038	00000000                 	DC.W	#0,#0
0000003c	6170706c73747274         	DC.B	'applstrt'
00000044	00000000                 	DC.W	#0,#0
00000048	00000000                 	DC.W	#0,#0
0000004c	0008636f                 	DC.W	#8,#25455
00000050	64650001                 	DC.W	#25701,#1
00000054	000000a0                 	DC.W	#0,#160
00000058	4d424152                 	DC.W	#19778,#16722
0000005c	03e80000                 	DC.W	#1000,#0
00000060	051e7446                 	DC.W	#1310,#29766
00000064	524d03e8                 	DC.W	#21069,#1000
00000068	00000582                 	DC.W	#0,#1410
0000006c	54616c74                 	DC.W	#21601,#27764
00000070	03e90000                 	DC.W	#1001,#0
00000074	05e46461                 	DC.W	#1508,#25697
00000078	74610000                 	DC.W	#29793,#0
0000007c	000006                   	DC.B	#0,#0,#6
0000007f	46636f6465               	DC.B	'Fcode'
00000084	00000000                 	DC.W	#0,#0
00000088	06                       	DC.B	#6
00000089	717441494e               	DC.B	'qtAIN'
0000008e	03e8                     	DC.W	#1000
00000090	00000689                 	ORI.B	#-119!-$77,D0
00000094	7476                     	MOVEQ	#118,D2
00000096	6572                     	BCS	L2
00000098	03e80000                 	BSET.B	D1,0(A0)
0000009c	069173680000             	ADDI.L	#1936195584!$73680000,(A1)
000000a2	0001487a                 	ORI.B	#122!$7a,D1
000000a6	00040697                 	ORI.B	#-105!-$69,D4
000000aa	000003c0                 	ORI.B	#-64!-$40,D0
000000ae	4e75                     	RTS
000000b0	4e56ffd6            L1   	LINK	A6,#-42
000000b4	48e71c00                 	MOVEM.L	D3-D5,-(A7)
000000b8	2a2e0008                 	MOVE.L	8(A6),D5
000000bc	382e000c                 	MOVE.W	12(A6),D4
000000c0	486efff8                 	PEA	-8(A6)
000000c4	3f3c0001                 	MOVE.W	#1,-(A7)
000000c8	2f3c70737973             	MOVE.L	#1886615923!$70737973,-(A7)
000000ce	4e4f                     	TRAP	#15
000000d0	a27b                     	DC.W	sysTrapFtrGet
000000d2	baaefff8                 	CMP.L	D5,-8(A6)
000000d6	4fef000a                 	LEA	10(A7),A7
000000da	630000aa                 	BLS	L5
000000de	3004                     	MOVE.W	D4,D0
000000e0	0240000c                 	ANDI.W	#12!$c,D0
000000e4	0c40000c                 	CMPI.W	#12!$c,D0
000000e8	66000096                 	BNE	L4
000000ec	3f3c03e9                 	MOVE.W	#1001!$3e9,-(A7)
000000f0	4e4f                     	TRAP	#15
000000f2	a192                     	DC.W	sysTrapFrmAlert
000000f4	0cae02003000fff8         	CMPI.L	#33566720!$2003000,-8(A6)
000000fc	544f                     	ADDQ.W	#2,A7
000000fe	64000080                 	BCC	L4
00000102	486efffc                 	PEA	-4(A6)
00000106	486efff6                 	PEA	-10(A6)
0000010a	1f3c0001            L2   	MOVE.B	#1,-(A7)
0000010e	2f3c70726566             	MOVE.L	#1886545254!$70726566,-(A7)
00000114	2f3c6170706c             	MOVE.L	#1634758764!$6170706c,-(A7)
0000011a	486effd6                 	PEA	-42(A6)
0000011e	1f3c0001                 	MOVE.B	#1,-(A7)
00000122	4e4f                     	TRAP	#15
00000124	a078                     	DC.W	sysTrapDmGetNextDatabaseByTypeCreator
00000126	4aaefffc                 	TST.L	-4(A6)
0000012a	4fef0018                 	LEA	24(A7),A7
0000012e	6618                     	BNE	L3
00000130	41fa0076                 	LEA	L7,A0
00000134	4850                     	PEA	(A0)
00000136	3f3c0090                 	MOVE.W	#144!$90,-(A7)
0000013a	41fa0080                 	LEA	L8,A0
0000013e	4850                     	PEA	(A0)
00000140	4e4f                     	TRAP	#15
00000142	a084                     	DC.W	sysTrapErrDisplayFileLineMsg
00000144	4fef000a                 	LEA	10(A7),A7
00000148	4aaefffc            L3   	TST.L	-4(A6)
0000014c	6732                     	BEQ	L4
0000014e	42a7                     	CLR.L	-(A7)
00000150	4267                     	CLR.W	-(A7)
00000152	2f2efffc                 	MOVE.L	-4(A6),-(A7)
00000156	3f2efff6                 	MOVE.W	-10(A6),-(A7)
0000015a	4e4f                     	TRAP	#15
0000015c	a0a7                     	DC.W	sysTrapSysUIAppSwitch
0000015e	3600                     	MOVE.W	D0,D3
00000160	4a43                     	TST.W	D3
00000162	4fef000c                 	LEA	12(A7),A7
00000166	6718                     	BEQ	L4
00000168	41fa005c                 	LEA	L9,A0
0000016c	4850                     	PEA	(A0)
0000016e	3f3c0090                 	MOVE.W	#144!$90,-(A7)
00000172	41fa0068                 	LEA	L10,A0
00000176	4850                     	PEA	(A0)
00000178	4e4f                     	TRAP	#15
0000017a	a084                     	DC.W	sysTrapErrDisplayFileLineMsg
0000017c	4fef000a                 	LEA	10(A7),A7
00000180	303c050c            L4   	MOVE.W	#1292!$50c,D0
00000184	6002                     	BRA	L6
00000186	7000                L5   	MOVEQ	#0,D0
00000188	4cdf0038            L6   	MOVEM.L	(A7)+,D3-D5
0000018c	4e5e                     	UNLK	A6
0000018e	4e75                     	RTS
00000190	94                       	DC.B	#148
00000191	526f6d56657273696f6e436f6d70617469626c65     	DC.B	'RomVersionCompatible'
000001a5	00003e                   	DC.B	#0,#0,#62
000001a8	436f756c64206e6f742066696e6420617070L7   	DC.B	'Could not find app'
000001ba	0000                     	DC.W	#0
000001bc	537461727465722e63  L8   	DC.B	'Starter.c'
000001c5	00                       	DC.B	#0
000001c6	436f756c64206e6f74206c61756e636820617070L9   	DC.B	'Could not launch app'
000001da	0000                     	DC.W	#0
000001dc	537461727465722e63  L10  	DC.B	'Starter.c'
000001e5	00                       	DC.B	#0
000001e6	4e560000            L11  	LINK	A6,#0
000001ea	4e5e                     	UNLK	A6
000001ec	4e75                     	RTS
000001ee	8c                       	DC.B	#140
000001ef	4d61696e466f726d496e6974     	DC.B	'MainFormInit'
000001fb	000000                   	DC.B	#0,#0,#0
000001fe	4e560000            L12  	LINK	A6,#0
00000202	2f03                     	MOVE.L	D3,-(A7)
00000204	7600                     	MOVEQ	#0,D3
00000206	302e0008                 	MOVE.W	8(A6),D0
0000020a	044003e8                 	SUBI.W	#1000!$3e8,D0
0000020e	6702                     	BEQ	L13
00000210	6014                     	BRA	L14
00000212	42a7                L13  	CLR.L	-(A7)
00000214	4e4f                     	TRAP	#15
00000216	a1c1                     	DC.W	sysTrapMenuEraseStatus
00000218	2f3c73747274             	MOVE.L	#1937011316!$73747274,-(A7)
0000021e	4e4f                     	TRAP	#15
00000220	a2af                     	DC.W	sysTrapAbtShowAbout
00000222	7601                     	MOVEQ	#1,D3
00000224	504f                     	ADDQ.W	#8,A7
00000226	1003                L14  	MOVE.B	D3,D0
00000228	261f                     	MOVE.L	(A7)+,D3
0000022a	4e5e                     	UNLK	A6
0000022c	4e75                     	RTS
0000022e	91                       	DC.B	#145
0000022f	4d61696e466f726d446f436f6d6d616e64     	DC.B	'MainFormDoCommand'
00000240	0000                     	DC.W	#0
00000242	4e560000            L15  	LINK	A6,#0
00000246	48e71030                 	MOVEM.L	D3/A2/A3,-(A7)
0000024a	266e0008                 	MOVEA.L	8(A6),A3
0000024e	7600                     	MOVEQ	#0,D3
00000250	3013                     	MOVE.W	(A3),D0
00000252	04400015                 	SUBI.W	#21!$15,D0
00000256	6706                     	BEQ	L16
00000258	5740                     	SUBQ.W	#3,D0
0000025a	670e                     	BEQ	L17
0000025c	6022                     	BRA	L18
0000025e	3f2b0008            L16  	MOVE.W	8(A3),-(A7)
00000262	4ebaff9a                 	JSR	L12
00000266	544f                     	ADDQ.W	#2,A7
00000268	6018                     	BRA	L19
0000026a	4e4f                L17  	TRAP	#15
0000026c	a173                     	DC.W	sysTrapFrmGetActiveForm
0000026e	2448                     	MOVEA.L	A0,A2
00000270	2f0a                     	MOVE.L	A2,-(A7)
00000272	4ebaff72                 	JSR	L11
00000276	2f0a                     	MOVE.L	A2,-(A7)
00000278	4e4f                     	TRAP	#15
0000027a	a171                     	DC.W	sysTrapFrmDrawForm
0000027c	7601                     	MOVEQ	#1,D3
0000027e	504f                     	ADDQ.W	#8,A7
00000280	1003                L18  	MOVE.B	D3,D0
00000282	4cdf0c08            L19  	MOVEM.L	(A7)+,D3/A2/A3
00000286	4e5e                     	UNLK	A6
00000288	4e75                     	RTS
0000028a	93                       	DC.B	#147
0000028b	4d61696e466f726d48616e646c654576656e74     	DC.B	'MainFormHandleEvent'
0000029e	0000                     	DC.W	#0
000002a0	4e560000            L20  	LINK	A6,#0
000002a4	48e71030                 	MOVEM.L	D3/A2/A3,-(A7)
000002a8	266e0008                 	MOVEA.L	8(A6),A3
000002ac	0c530017                 	CMPI.W	#23!$17,(A3)
000002b0	662e                     	BNE	L23
000002b2	362b0008                 	MOVE.W	8(A3),D3
000002b6	3f03                     	MOVE.W	D3,-(A7)
000002b8	4e4f                     	TRAP	#15
000002ba	a16f                     	DC.W	sysTrapFrmInitForm
000002bc	2448                     	MOVEA.L	A0,A2
000002be	2f0a                     	MOVE.L	A2,-(A7)
000002c0	4e4f                     	TRAP	#15
000002c2	a174                     	DC.W	sysTrapFrmSetActiveForm
000002c4	5c4f                     	ADDQ.W	#6,A7
000002c6	3003                     	MOVE.W	D3,D0
000002c8	044003e8                 	SUBI.W	#1000!$3e8,D0
000002cc	6702                     	BEQ	L21
000002ce	600c                     	BRA	L22
000002d0	487aff70            L21  	PEA	L15
000002d4	2f0a                     	MOVE.L	A2,-(A7)
000002d6	4e4f                     	TRAP	#15
000002d8	a19f                     	DC.W	sysTrapFrmSetEventHandler
000002da	504f                     	ADDQ.W	#8,A7
000002dc	7001                L22  	MOVEQ	#1,D0
000002de	6002                     	BRA	L24
000002e0	7000                L23  	MOVEQ	#0,D0
000002e2	4cdf0c08            L24  	MOVEM.L	(A7)+,D3/A2/A3
000002e6	4e5e                     	UNLK	A6
000002e8	4e75                     	RTS
000002ea	8e                       	DC.B	#142
000002eb	41707048616e646c654576656e74     	DC.B	'AppHandleEvent'
000002f9	000000                   	DC.B	#0,#0,#0
000002fc	4e56ffe6            L25  	LINK	A6,#-26
00000300	4878ffff            L26  	PEA	$ffffffff.W
00000304	486effe8                 	PEA	-24(A6)
00000308	4e4f                     	TRAP	#15
0000030a	a11d                     	DC.W	sysTrapEvtGetEvent
0000030c	486effe8                 	PEA	-24(A6)
00000310	4e4f                     	TRAP	#15
00000312	a0a9                     	DC.W	sysTrapSysHandleEvent
00000314	4a00                     	TST.B	D0
00000316	4fef000c                 	LEA	12(A7),A7
0000031a	662e                     	BNE	L27
0000031c	486effe6                 	PEA	-26(A6)
00000320	486effe8                 	PEA	-24(A6)
00000324	42a7                     	CLR.L	-(A7)
00000326	4e4f                     	TRAP	#15
00000328	a1bf                     	DC.W	sysTrapMenuHandleEvent
0000032a	4a00                     	TST.B	D0
0000032c	4fef000c                 	LEA	12(A7),A7
00000330	6618                     	BNE	L27
00000332	486effe8                 	PEA	-24(A6)
00000336	4ebaff68                 	JSR	L20
0000033a	4a00                     	TST.B	D0
0000033c	584f                     	ADDQ.W	#4,A7
0000033e	660a                     	BNE	L27
00000340	486effe8                 	PEA	-24(A6)
00000344	4e4f                     	TRAP	#15
00000346	a1a0                     	DC.W	sysTrapFrmDispatchEvent
00000348	584f                     	ADDQ.W	#4,A7
0000034a	0c6e0016ffe8        L27  	CMPI.W	#22!$16,-24(A6)
00000350	66ae                     	BNE	L26
00000352	4e5e                     	UNLK	A6
00000354	4e75                     	RTS
00000356	8c                       	DC.B	#140
00000357	4170704576656e744c6f6f70     	DC.B	'AppEventLoop'
00000363	000000                   	DC.B	#0,#0,#0
00000366	4e56fffc            L28  	LINK	A6,#-4
0000036a	3d7c0002fffe             	MOVE.W	#2,-2(A6)
00000370	1f3c0001                 	MOVE.B	#1,-(A7)
00000374	486efffe                 	PEA	-2(A6)
00000378	486efffc                 	PEA	-4(A6)
0000037c	4267                     	CLR.W	-(A7)
0000037e	2f3c73747274             	MOVE.L	#1937011316!$73747274,-(A7)
00000384	4e4f                     	TRAP	#15
00000386	a2d3                     	DC.W	#-23853
00000388	7000                     	MOVEQ	#0,D0
0000038a	4e5e                     	UNLK	A6
0000038c	4e75                     	RTS
0000038e	88                       	DC.B	#136
0000038f	4170705374617274         	DC.B	'AppStart'
00000397	000000                   	DC.B	#0,#0,#0
0000039a	4e56fffe            L29  	LINK	A6,#-2
0000039e	1f3c0001                 	MOVE.B	#1,-(A7)
000003a2	3f3c0002                 	MOVE.W	#2,-(A7)
000003a6	486efffe                 	PEA	-2(A6)
000003aa	48780001                 	PEA	$0001.W
000003ae	2f3c73747274             	MOVE.L	#1937011316!$73747274,-(A7)
000003b4	4e4f                     	TRAP	#15
000003b6	a2d4                     	DC.W	#-23852
000003b8	4e5e                     	UNLK	A6
000003ba	4e75                     	RTS
000003bc	87                       	DC.B	#135
000003bd	41707053746f70           	DC.B	'AppStop'
000003c4	0000                     	DC.W	#0
000003c6	4e560000            L30  	LINK	A6,#0
000003ca	48e71c00                 	MOVEM.L	D3-D5,-(A7)
000003ce	3a2e0008                 	MOVE.W	8(A6),D5
000003d2	382e000e                 	MOVE.W	14(A6),D4
000003d6	3f04                     	MOVE.W	D4,-(A7)
000003d8	2f3c02003000             	MOVE.L	#33566720!$2003000,-(A7)
000003de	4ebafcd0                 	JSR	L1
000003e2	3600                     	MOVE.W	D0,D3
000003e4	4a43                     	TST.W	D3
000003e6	5c4f                     	ADDQ.W	#6,A7
000003e8	6706                     	BEQ	L31
000003ea	3043                     	MOVEA.W	D3,A0
000003ec	2008                     	MOVE.L	A0,D0
000003ee	602a                     	BRA	L35
000003f0	3005                L31  	MOVE.W	D5,D0
000003f2	6702                     	BEQ	L32
000003f4	6022                     	BRA	L34
000003f6	4ebaff6e            L32  	JSR	L28
000003fa	3600                     	MOVE.W	D0,D3
000003fc	4a43                     	TST.W	D3
000003fe	6706                     	BEQ	L33
00000400	3043                     	MOVEA.W	D3,A0
00000402	2008                     	MOVE.L	A0,D0
00000404	6014                     	BRA	L35
00000406	3f3c03e8            L33  	MOVE.W	#1000!$3e8,-(A7)
0000040a	4e4f                     	TRAP	#15
0000040c	a19b                     	DC.W	sysTrapFrmGotoForm
0000040e	4ebafeec                 	JSR	L25
00000412	4ebaff86                 	JSR	L29
00000416	544f                     	ADDQ.W	#2,A7
00000418	7000                L34  	MOVEQ	#0,D0
0000041a	4cdf0038            L35  	MOVEM.L	(A7)+,D3-D5
0000041e	4e5e                     	UNLK	A6
00000420	4e75                     	RTS
00000422	90                       	DC.B	#144
00000423	5374617274657250696c6f744d61696e     	DC.B	'StarterPilotMain'
00000433	000000                   	DC.B	#0,#0,#0
00000436	4e560000                 	LINK	A6,#0
0000043a	48e71820                 	MOVEM.L	D3/D4/A2,-(A7)
0000043e	382e0008                 	MOVE.W	8(A6),D4
00000442	246e000a                 	MOVEA.L	10(A6),A2
00000446	362e000e                 	MOVE.W	14(A6),D3
0000044a	3f03                     	MOVE.W	D3,-(A7)
0000044c	2f0a                     	MOVE.L	A2,-(A7)
0000044e	3f04                     	MOVE.W	D4,-(A7)
00000450	4ebaff74                 	JSR	L30
00000454	504f                     	ADDQ.W	#8,A7
00000456	4cdf0418                 	MOVEM.L	(A7)+,D3/D4/A2
0000045a	4e5e                     	UNLK	A6
0000045c	4e75                     	RTS
0000045e	89                       	DC.B	#137
0000045f	50696c6f744d61696e       	DC.B	'PilotMain'
00000468	0000                     	DC.W	#0
0000046a	4e56fff4                 	LINK	A6,#-12
0000046e	48e71800                 	MOVEM.L	D3/D4,-(A7)
00000472	486efff4                 	PEA	-12(A6)
00000476	486efff8                 	PEA	-8(A6)
0000047a	486efffc                 	PEA	-4(A6)
0000047e	4e4f                     	TRAP	#15
00000480	a08f                     	DC.W	sysTrapSysAppStartup
00000482	3800                     	MOVE.W	D0,D4
00000484	4a44                     	TST.W	D4
00000486	4fef000c                 	LEA	12(A7),A7
0000048a	671c                     	BEQ	L36
0000048c	41fa0066                 	LEA	L40,A0
00000490	4850                     	PEA	(A0)
00000492	3f3c005c                 	MOVE.W	#92!$5c,-(A7)
00000496	41fa0078                 	LEA	L41,A0
0000049a	4850                     	PEA	(A0)
0000049c	4e4f                     	TRAP	#15
0000049e	a084                     	DC.W	sysTrapErrDisplayFileLineMsg
000004a0	7000                     	MOVEQ	#0,D0
000004a2	4fef000a                 	LEA	10(A7),A7
000004a6	6036                     	BRA	L39
000004a8	206efffc            L36  	MOVEA.L	-4(A6),A0
000004ac	3f280006                 	MOVE.W	6(A0),-(A7)
000004b0	2f280002                 	MOVE.L	2(A0),-(A7)
000004b4	3f10                     	MOVE.W	(A0),-(A7)
000004b6	487a000e                 	PEA	L38
000004ba	487a0004                 	PEA	L37
000004be	0697ffffff76             	ADDI.L	#-138!-$8a,(A7)
000004c4	4e75                     	RTS
000004c6	2600                L38  	MOVE.L	D0,D3
000004c8	2f2efff4                 	MOVE.L	-12(A6),-(A7)
000004cc	2f2efff8                 	MOVE.L	-8(A6),-(A7)
000004d0	2f2efffc                 	MOVE.L	-4(A6),-(A7)
000004d4	4e4f                     	TRAP	#15
000004d6	a090                     	DC.W	sysTrapSysAppExit
000004d8	2003                     	MOVE.L	D3,D0
000004da	4fef0014                 	LEA	20(A7),A7
000004de	4cdf0018            L39  	MOVEM.L	(A7)+,D3/D4
000004e2	4e5e                     	UNLK	A6
000004e4	4e75                     	RTS
000004e6	8b                       	DC.B	#139
000004e7	5f5f537461727475705f5f     	DC.B	'__Startup__'
000004f2	002a                     	DC.W	#42
000004f4	4572726f72206c61756e6368696e67206170706c69636174696f6eL40  	DC.B	'Error launching application'
0000050f	00                       	DC.B	#0
00000510	53746172747570436f64652e63L41  	DC.B	'StartupCode.c'
0000051d	00                       	DC.B	#0
0000051e	00000000                 	DC.W	#0,#0
00000522	00000000                 	DC.W	#0,#0
00000526	00000000                 	DC.W	#0,#0
0000052a	00000000                 	DC.W	#0,#0
0000052e	00000000                 	DC.W	#0,#0
00000532	ffff0000                 	DC.W	#-1,#0
00000536	00000001                 	DC.W	#0,#1
0000053a	056df81c                 	DC.W	#1389,#-2020
0000053e	00000000                 	DC.W	#0,#0
00000542	0006000e                 	DC.W	#6,#14
00000546	0064000b                 	DC.W	#100,#11
0000054a	00000000                 	DC.W	#0,#0
0000054e	00040000                 	DC.W	#4,#0
00000552	002f000c                 	DC.W	#47,#12
00000556	0000004a                 	DC.W	#0,#74
0000055a	00010000                 	DC.W	#1,#0
0000055e	004203e8                 	DC.W	#66,#1000
00000562	00000000                 	DC.W	#0,#0
00000566	00                       	DC.B	#0
00000567	524f7074696f6e73         	DC.B	'ROptions'
0000056f	00                       	DC.B	#0
00000570	41626f7574205374617274657220417070     	DC.B	'About Starter App'
00000581	00                       	DC.B	#0
00000582	00000000                 	DC.W	#0,#0
00000586	00000000                 	DC.W	#0,#0
0000058a	12000000                 	DC.W	#4608,#0
0000058e	000000a0                 	DC.W	#0,#160
00000592	00a00000                 	DC.W	#160,#0
00000596	00000000                 	DC.W	#0,#0
0000059a	00000000                 	DC.W	#0,#0
0000059e	00000000                 	DC.W	#0,#0
000005a2	00000000                 	DC.W	#0,#0
000005a6	00000000                 	DC.W	#0,#0
000005aa	03e88000                 	DC.W	#1000,#-32768
000005ae	00000000                 	DC.W	#0,#0
000005b2	00000000                 	DC.W	#0,#0
000005b6	00000000                 	DC.W	#0,#0
000005ba	00000000                 	DC.W	#0,#0
000005be	03e80001                 	DC.W	#1000,#1
000005c2	056df7d8                 	DC.W	#1389,#-2088
000005c6	09e80000                 	DC.W	#2536,#0
000005ca	004a0000                 	DC.W	#74,#0
000005ce	00000000                 	DC.W	#0,#0
000005d2	00000000                 	DC.W	#0,#0
000005d6	0000                     	DC.W	#0
000005d8	53746172746572417070     	DC.B	'StarterApp'
000005e2	00540003                 	DC.W	#84,#3
000005e6	00000001                 	DC.W	#0,#1
000005ea	0000                     	DC.W	#0
000005ec	53797374656d20496e636f6d70617469626c65     	DC.B	'System Incompatible'
000005ff	00                       	DC.B	#0
00000600	53797374656d2056657273696f6e20322e30206f72206772656174657220697320726571756972656420746f2072756e2074686973206170706c69636174696f6e2e     	DC.B	'System Version 2.0 or greater is required to run this application.'
00000642	004f4b00                 	DC.W	#79,#19200
00000646	0000001f                 	DC.W	#0,#31
0000064a	00000000                 	DC.W	#0,#0
0000064e	00000000                 	DC.W	#0,#0
00000652	28000000                 	DC.W	#10240,#0
00000656	00280000                 	DC.W	#40,#0
0000065a	00000000                 	DC.W	#0,#0
0000065e	00000000                 	DC.W	#0,#0
00000662	00000000                 	DC.W	#0,#0
00000666	00000000                 	DC.W	#0,#0
0000066a	00000000                 	DC.W	#0,#0
0000066e	00000000                 	DC.W	#0,#0
00000672	00003000                 	DC.W	#0,#12288
00000676	00000800                 	DC.W	#0,#2048
0000067a	00000800                 	DC.W	#0,#2048
0000067e	00002000                 	DC.W	#0,#8192
00000682	003f3c00                 	DC.W	#63,#15360
00000686	01a9f0                   	DC.B	#1,#169,#240
00000689	53746172746572           	DC.B	'Starter'
00000690	00312e30                 	DC.W	#49,#11824
00000694	00                       	DC.B	#0
