00F4: 0000 0001      orib #1, d0
00F8: 4E56 FFF4      link a6, #-12
00FC: 48E7 1F00      movem <1f00>, -(a7)
0100: 2E0C           movel a4, d7
0102: 486E FFF4      pea.ex -12(a6)
0106: 486E FFF8      pea.ex -8(a6)
010A: 486E FFFC      pea.ex -4(a6)
010E: 4E4F           trap #15
0110: A08F           sysTrapSysAppStartup
0112: DEFC 000C      addaw.ex #12, a7
0116: 4A40           tstw d0
0118: 6710           beq 16
011A: 1F3C 0003      moveb.ex #3, -(a7)
011E: 4E4F           trap #15
0120: A234           sysTrapSndPlaySystemSound
0122: 2847           movel d7, a4
0124: 70FF           moveq #255, d0
0126: 6000 005C      bra 92
012A: 206E FFFC      movel.ex -4(a6), a0
012E: 3C10           movew (a0), d6
0130: 2A28 0002      movel.ex 2(a0), d5
0134: 3828 0006      movew.ex 6(a0), d4
0138: 0804 0002      btst #2, d4
013C: 6710           beq 16
013E: 284D           movel a5, a4
0140: 99FC 0000 0128 subal.ex #296, a4
0146: 6100 0076      bsr 118
014A: 6000 0004      bra 4
014E: 99CC           subal a4, a4
0150: 3F04           movew d4, -(a7)
0152: 2F05           movel d5, -(a7)
0154: 3F06           movew d6, -(a7)
0156: 6100 0180      bsr 384
015A: 3F04           movew d4, -(a7)
015C: 2F05           movel d5, -(a7)
015E: 3F06           movew d6, -(a7)
0160: 6100 02F2      bsr 754
0164: 2600           movel d0, d3
0166: 3F04           movew d4, -(a7)
0168: 2F05           movel d5, -(a7)
016A: 3F06           movew d6, -(a7)
016C: 6100 01B8      bsr 440
0170: 2F2E FFF4      movel.ex -12(a6), -(a7)
0174: 2F2E FFF8      movel.ex -8(a6), -(a7)
0178: 2F2E FFFC      movel.ex -4(a6), -(a7)
017C: 4E4F           trap #15
017E: A090           sysTrapSysAppExit
0180: 2847           movel d7, a4
0182: 2003           movel d3, d0
0184: 4CEE 00F8 FFE0 movem -32(a6), <00f8>
018A: 4E5E           unlk a6
018C: 4E75           rts
018E: 756E           dc.w #30062
0190: 6B6E           bmi 110
0192: 6F77           ble 119
0194: 6E20           bgt 32
0196: 7265           moveq #101, d1
0198: 6C6F           bge 111
019A: 632E           bls 46
019C: 7479           moveq #121, d2
019E: 7065           moveq #101, d0
01A0: 0063 7274      oriw #29300, -(a3)
01A4: 302E 6300      movew.ex 25344(a6), d0
01A8: 556E 6B6E      subqw.ex #2, 27502(a6)
01AC: 6F77           ble 119
01AE: 6E20           bgt 32
01B0: 7265           moveq #101, d1
01B2: 6C6F           bge 111
01B4: 632E           bls 46
01B6: 7365           dc.w #29541
01B8: 6374           bls 116
01BA: 696F           bvs 111
01BC: 6E00 4E56      bgt 20054
01C0: FFF4           dc.w #65524
01C2: 48E7 1F30      movem <1f30>, -(a7)
01C6: 43EC 0000      lea 0(a4), d1
01CA: 2D49 FFFC      movel.mx a1, -4(a6)
01CE: 43EC 0020      lea 32(a4), d1
01D2: 2D49 FFF8      movel.mx a1, -8(a6)
01D6: 43FA FF20      lea 29952(pc), d1
01DA: 2D49 FFF4      movel.mx a1, -12(a6)
01DE: 41EC 0000      lea 0(a4), d0
01E2: 4A50           tstw (a0)
01E4: 6600 00E8      bne 232
01E8: 30BC 0001      movew.ex #1, (a0)
01EC: 93FC 0001 0000 subal.ex #65536, a1
01F2: 2D49 FFF4      movel.mx a1, -12(a6)
01F6: 04AE 0000 0020 FFF8 subil.ex #32, -8(a6)
01FE: 04AE 0000 0000 FFFC subil.ex #0, -4(a6)
0206: 4267           clrw -(a7)
0208: 2F3C 726C 6F63 movel.exl #1919709027, -(a7)
020E: 4E4F           trap #15
0210: A060           sysTrapDmGet1Resource
0212: 2E08           movel a0, d7
0214: 5C4F           addqw #6, a7
0216: 6700 00B6      beq 182
021A: 2F07           movel d7, -(a7)
021C: 4E4F           trap #15
021E: A021           sysTrapMemHandleLock
0220: 3658           movew (a0)+, a3
0222: 2448           movel a0, a2
0224: 4245           clrw d5
0226: 584F           addqw #4, a7
0228: BA4B           cmpw a3, d5
022A: 6400 0096      bcc 150
022E: 7C00           moveq #0, d6
0230: 4244           clrw d4
0232: 3C05           movew d5, d6
0234: 2606           movel d6, d3
0236: E783           aslil #3, d3
0238: 0C32 00BE 3800 cmpib.ex #190, 0(a2,d3.l)
023E: 6718           beq 24
0240: 43FA FF4C      lea 30102(pc), d1
0244: 2F09           movel a1, -(a7)
0246: 3F3C 006B      movew.ex #107, -(a7)
024A: 43FA FF55      lea 30121(pc), d1
024E: 2F09           movel a1, -(a7)
0250: 4E4F           trap #15
0252: A084           sysTrapErrDisplayFileLineMsg
0254: DEFC 000A      addaw.ex #10, a7
0258: 4281           clrl d1
025A: 3232 3802      movew.ex 2(a2,d3.l), d1
025E: 43EC 0000      lea 0(a4), d1
0262: 2009           movel a1, d0
0264: 2041           movel d1, a0
0266: D1C0           addal d0, a0
0268: 1832 3801      moveb.ex 1(a2,d3.l), d4
026C: 0C44 0064      cmpiw #100, d4
0270: 671C           beq 28
0272: 6E0A           bgt 10
0274: 0C44 0062      cmpiw #98, d4
0278: 671E           beq 30
027A: 6000 0026      bra 38
027E: 0C44 0074      cmpiw #116, d4
0282: 661E           bne 30
0284: 242E FFF4      movel.ex -12(a6), d2
0288: D590           addml d2, (a0)
028A: 6000 002E      bra 46
028E: 242E FFFC      movel.ex -4(a6), d2
0292: D590           addml d2, (a0)
0294: 6000 0024      bra 36
0298: 242E FFF8      movel.ex -8(a6), d2
029C: D590           addml d2, (a0)
029E: 6000 001A      bra 26
02A2: 43FA FF04      lea 30128(pc), d1
02A6: 2F09           movel a1, -(a7)
02A8: 3F3C 007E      movew.ex #126, -(a7)
02AC: 43FA FEF3      lea 30121(pc), d1
02B0: 2F09           movel a1, -(a7)
02B2: 4E4F           trap #15
02B4: A084           sysTrapErrDisplayFileLineMsg
02B6: DEFC 000A      addaw.ex #10, a7
02BA: 5245           addqw #1, d5
02BC: BA4B           cmpw a3, d5
02BE: 6500 FF72      bcs 65394
02C2: 2F07           movel d7, -(a7)
02C4: 4E4F           trap #15
02C6: A022           sysTrapMemHandleUnlock
02C8: 2F07           movel d7, -(a7)
02CA: 4E4F           trap #15
02CC: A061           sysTrapDmReleaseResource
02CE: 4CEE 0CF8 FFD8 movem -40(a6), <0cf8>
02D4: 4E5E           unlk a6
02D6: 4E75           rts
02D8: 4E56 FFFC      link a6, #-4
02DC: 48E7 1C30      movem <1c30>, -(a7)
02E0: 3A2E 0008      movew.ex 8(a6), d5
02E4: 282E 000A      movel.ex 10(a6), d4
02E8: 362E 000E      movew.ex 14(a6), d3
02EC: 43FA FE0A      lea 29952(pc), d1
02F0: 93FC 0001 0000 subal.ex #65536, a1
02F6: 45FA 77D0      lea -4400(pc), d2
02FA: 47FA 77CC      lea -4400(pc), d3
02FE: B7CA           cmpal a2, a3
0300: 631A           bls 26
0302: 205A           movel (a2)+, a0
0304: D1C9           addal a1, a0
0306: 3F03           movew d3, -(a7)
0308: 2F04           movel d4, -(a7)
030A: 3F05           movew d5, -(a7)
030C: 2D49 FFFC      movel.mx a1, -4(a6)
0310: 4E90           jsr (a0)
0312: 504F           addqw #0, a7
0314: 226E FFFC      movel.ex -4(a6), a1
0318: B7CA           cmpal a2, a3
031A: 62E6           bhi -26
031C: 4CEE 0C38 FFE8 movem -24(a6), <0c38>
0322: 4E5E           unlk a6
0324: 4E75           rts
0326: 4E56 0000      link a6, #0
032A: 48E7 1E30      movem <1e30>, -(a7)
032E: 3C2E 0008      movew.ex 8(a6), d6
0332: 2A2E 000A      movel.ex 10(a6), d5
0336: 382E 000E      movew.ex 14(a6), d4
033A: 47FA FDBC      lea 29952(pc), d3
033E: 97FC 0001 0000 subal.ex #65536, a3
0344: 43FA 7782      lea -4400(pc), d1
0348: 2609           movel a1, d3
034A: 45FA 777C      lea -4400(pc), d2
034E: B68A           cmpl a2, d3
0350: 6412           bcc 18
0352: 2062           movel -(a2), a0
0354: D1CB           addal a3, a0
0356: 3F04           movew d4, -(a7)
0358: 2F05           movel d5, -(a7)
035A: 3F06           movew d6, -(a7)
035C: 4E90           jsr (a0)
035E: 504F           addqw #0, a7
0360: B68A           cmpl a2, d3
0362: 65EE           bcs -18
0364: 4CEE 0C78 FFE8 movem -24(a6), <0c78>
036A: 4E5E           unlk a6
036C: 4E75           rts
036E: 0000           dc.w #0
0370: 4E56 0000      link a6, #0
0374: 426C 009E      clrw.ex 158(a4)
0378: 426C 010C      clrw.ex 268(a4)
037C: 426C 006A      clrw.ex 106(a4)
0380: 397C 0001 00E4 movew.emx #1, 228(a4)
0386: 426C 0090      clrw.ex 144(a4)
038A: 426C 00D6      clrw.ex 214(a4)
038E: 41EC 0038      lea 56(a4), d0
0392: 4290           clrl (a0)
0394: 42A8 0004      clrl.ex 4(a0)
0398: 397C 004B 006E movew.emx #75, 110(a4)
039E: 397C 004B 0054 movew.emx #75, 84(a4)
03A4: 397C 0050 006C movew.emx #80, 108(a4)
03AA: 397C 0001 0098 movew.emx #1, 152(a4)
03B0: 426C 0124      clrw.ex 292(a4)
03B4: 426C 00DC      clrw.ex 220(a4)
03B8: 42AC 0048      clrl.ex 72(a4)
03BC: 426C 0122      clrw.ex 290(a4)
03C0: 3F3C 001E      movew.ex #30, -(a7)
03C4: 486C 00B0      pea.ex 176(a4)
03C8: 6100 65BA      bsr 26042
03CC: 6100 000E      bsr 14
03D0: 486C 0108      pea.ex 264(a4)
03D4: 6100 0EAC      bsr 3756
03D8: 4E5E           unlk a6
03DA: 4E75           rts
03DC: 4E56 0000      link a6, #0
03E0: 2F0A           movel a2, -(a7)
03E2: 426C 0030      clrw.ex 48(a4)
03E6: 426C 0042      clrw.ex 66(a4)
03EA: 45EC 004C      lea 76(a4), d2
03EE: 3F3C 0020      movew.ex #32, -(a7)
03F2: 6100 0FDE      bsr 4062
03F6: 0640 0030      addiw #48, d0
03FA: 3480           movew d0, (a2)
03FC: 397C 002C 009C movew.emx #44, 156(a4)
0402: 397C 0004 0068 movew.emx #4, 104(a4)
0408: 397C 0004 00D4 movew.emx #4, 212(a4)
040E: 3F3C 0001      movew.ex #1, -(a7)
0412: 6100 0FBE      bsr 4030
0416: 3940 0090      movew.mx d0, 144(a4)
041A: 584F           addqw #4, a7
041C: 660A           bne 10
041E: 397C 0001 008E movew.emx #1, 142(a4)
0424: 6000 0008      bra 8
0428: 397C FFFF 008E movew.emx #65535, 142(a4)
042E: 3F3C 0001      movew.ex #1, -(a7)
0432: 6100 0F9E      bsr 3998
0436: 3940 0090      movew.mx d0, 144(a4)
043A: 660A           bne 10
043C: 397C 0001 00A0 movew.emx #1, 160(a4)
0442: 6000 0008      bra 8
0446: 397C FFFF 00A0 movew.emx #65535, 160(a4)
044C: 246E FFFC      movel.ex -4(a6), a2
0450: 4E5E           unlk a6
0452: 4E75           rts
0454: 4E56 0000      link a6, #0
0458: 4A6E 0008      tstw.ex 8(a6)
045C: 661A           bne 26
045E: 6100 001E      bsr 30
0462: 4A40           tstw d0
0464: 670A           beq 10
0466: 0280 0000 FFFF andil #65535, d0
046C: 6000 000C      bra 12
0470: 6100 003A      bsr 58
0474: 6100 0012      bsr 18
0478: 7000           moveq #0, d0
047A: 4E5E           unlk a6
047C: 4E75           rts
047E: 4E56 0000      link a6, #0
0482: 4240           clrw d0
0484: 4E5E           unlk a6
0486: 4E75           rts
0488: 4E56 0000      link a6, #0
048C: 4E5E           unlk a6
048E: 4E75           rts
0490: 456E           dc.w #17774
0492: 6420           bcc 32
0494: 7468           moveq #104, d2
0496: 6973           bvs 115
0498: 2047           movel d7, a0
049A: 616D           bsr 109
049C: 653F           bcs 63
049E: 0050 6C61      oriw #27745, (a0)
04A2: 7920           dc.w #31008
04A4: 4167           dc.w #16743
04A6: 6169           bsr 105
04A8: 6E3F           bgt 63
04AA: 0000           dc.w #0
04AC: 4E56 FFF8      link a6, #-8
04B0: 48E7 1E30      movem <1e30>, -(a7)
04B4: 4E4F           trap #15
04B6: A0F7           sysTrapTimGetTicks
04B8: 47EC 00D0      lea 208(a4), d3
04BC: 2680           movel d0, (a3)
04BE: 4E4F           trap #15
04C0: A0F7           sysTrapTimGetTicks
04C2: 45EC 0064      lea 100(a4), d2
04C6: 2480           movel d0, (a2)
04C8: 4E4F           trap #15
04CA: A0F7           sysTrapTimGetTicks
04CC: 2940 002C      movel.mx d0, 44(a4)
04D0: 4E4F           trap #15
04D2: A0F7           sysTrapTimGetTicks
04D4: 2940 0070      movel.mx d0, 112(a4)
04D8: 2452           movel (a2), a2
04DA: 95D3           subal (a3), a2
04DC: 294A 0094      movel.mx a2, 148(a4)
04E0: 397C 03E9 0120 movew.emx #1001, 288(a4)
04E6: 397C 03E9 0062 movew.emx #1001, 98(a4)
04EC: 3F3C 03E9      movew.ex #1001, -(a7)
04F0: 4E4F           trap #15
04F2: A19B           sysTrapFrmGotoForm
04F4: 45EC 00F0      lea 240(a4), d2
04F8: 34BC 0017      movew.ex #23, (a2)
04FC: 544F           addqw #2, a7
04FE: 4878 0001      pea.ex (0001).w
0502: 2F0A           movel a2, -(a7)
0504: 4E4F           trap #15
0506: A11D           sysTrapEvtGetEvent
0508: 2F0A           movel a2, -(a7)
050A: 4E4F           trap #15
050C: A0A9           sysTrapSysHandleEvent
050E: DEFC 000C      addaw.ex #12, a7
0512: 4A00           tstb d0
0514: 6600 0CA4      bne 3236
0518: 486C 00D6      pea.ex 214(a4)
051C: 2F0A           movel a2, -(a7)
051E: 42A7           clrl -(a7)
0520: 4E4F           trap #15
0522: A1BF           sysTrapMenuHandleEvent
0524: DEFC 000C      addaw.ex #12, a7
0528: 4A00           tstb d0
052A: 670A           beq 10
052C: 397C 0001 009E movew.emx #1, 158(a4)
0532: 6000 0C86      bra 3206
0536: 4E4F           trap #15
0538: A173           sysTrapFrmGetActiveForm
053A: 2948 005C      movel.mx a0, 92(a4)
053E: 3012           movew (a2), d0
0540: 0C40 0001      cmpiw #1, d0
0544: 6708           beq 8
0546: 0C40 0003      cmpiw #3, d0
054A: 6600 0232      bne 562
054E: 41EC 009E      lea 158(a4), d0
0552: 4A50           tstw (a0)
0554: 6702           beq 2
0556: 4250           clrw (a0)
0558: 302A 0006      movew.ex 6(a2), d0
055C: 0640 FF81      addiw #-127, d0
0560: 0C40 001A      cmpiw #26, d0
0564: 6200 0218      bhi 536
0568: 302A 0004      movew.ex 4(a2), d0
056C: 0640 FFEB      addiw #-21, d0
0570: 0C40 0016      cmpiw #22, d0
0574: 624C           bhi 76
0576: 0C6C 03EA 0120 cmpiw.ex #1002, 288(a4)
057C: 6644           bne 68
057E: 4A6C 010C      tstw.ex 268(a4)
0582: 663E           bne 62
0584: 41EC 00DC      lea 220(a4), d0
0588: 0C50 0003      cmpiw #3, (a0)
058C: 6700 01F0      beq 496
0590: 30BC 0003      movew.ex #3, (a0)
0594: 41EC 0054      lea 84(a4), d0
0598: 3F10           movew (a0), -(a7)
059A: 47EC 006E      lea 110(a4), d3
059E: 3F13           movew (a3), -(a7)
05A0: 45EC 0124      lea 292(a4), d2
05A4: 3F12           movew (a2), -(a7)
05A6: 2D48 FFFC      movel.mx a0, -4(a6)
05AA: 6100 1370      bsr 4976
05AE: 34BC 0003      movew.ex #3, (a2)
05B2: 206E FFFC      movel.ex -4(a6), a0
05B6: 3F10           movew (a0), -(a7)
05B8: 3F13           movew (a3), -(a7)
05BA: 3F3C 0003      movew.ex #3, -(a7)
05BE: 6000 01B2      bra 434
05C2: 41EC 00F0      lea 240(a4), d0
05C6: 3028 0006      movew.ex 6(a0), d0
05CA: 0640 FF81      addiw #-127, d0
05CE: 0C40 001A      cmpiw #26, d0
05D2: 6200 01AA      bhi 426
05D6: 3028 0004      movew.ex 4(a0), d0
05DA: 0640 FFD4      addiw #-44, d0
05DE: 0C40 0016      cmpiw #22, d0
05E2: 624C           bhi 76
05E4: 0C6C 03EA 0120 cmpiw.ex #1002, 288(a4)
05EA: 6644           bne 68
05EC: 4A6C 010C      tstw.ex 268(a4)
05F0: 663E           bne 62
05F2: 41EC 00DC      lea 220(a4), d0
05F6: 0C50 0004      cmpiw #4, (a0)
05FA: 6700 0182      beq 386
05FE: 30BC 0004      movew.ex #4, (a0)
0602: 41EC 0054      lea 84(a4), d0
0606: 3F10           movew (a0), -(a7)
0608: 47EC 006E      lea 110(a4), d3
060C: 3F13           movew (a3), -(a7)
060E: 45EC 0124      lea 292(a4), d2
0612: 3F12           movew (a2), -(a7)
0614: 2D48 FFFC      movel.mx a0, -4(a6)
0618: 6100 1302      bsr 4866
061C: 34BC 0004      movew.ex #4, (a2)
0620: 206E FFFC      movel.ex -4(a6), a0
0624: 3F10           movew (a0), -(a7)
0626: 3F13           movew (a3), -(a7)
0628: 3F3C 0004      movew.ex #4, -(a7)
062C: 6000 0144      bra 324
0630: 41EC 00F0      lea 240(a4), d0
0634: 3028 0006      movew.ex 6(a0), d0
0638: 0640 FF81      addiw #-127, d0
063C: 0C40 001A      cmpiw #26, d0
0640: 6200 013C      bhi 316
0644: 3028 0004      movew.ex 4(a0), d0
0648: 0640 FFBD      addiw #-67, d0
064C: 0C40 0016      cmpiw #22, d0
0650: 624C           bhi 76
0652: 0C6C 03EA 0120 cmpiw.ex #1002, 288(a4)
0658: 6644           bne 68
065A: 4A6C 010C      tstw.ex 268(a4)
065E: 663E           bne 62
0660: 41EC 00DC      lea 220(a4), d0
0664: 0C50 0001      cmpiw #1, (a0)
0668: 6700 0114      beq 276
066C: 30BC 0001      movew.ex #1, (a0)
0670: 41EC 0054      lea 84(a4), d0
0674: 3F10           movew (a0), -(a7)
0676: 47EC 006E      lea 110(a4), d3
067A: 3F13           movew (a3), -(a7)
067C: 45EC 0124      lea 292(a4), d2
0680: 3F12           movew (a2), -(a7)
0682: 2D48 FFFC      movel.mx a0, -4(a6)
0686: 6100 1294      bsr 4756
068A: 34BC 0001      movew.ex #1, (a2)
068E: 206E FFFC      movel.ex -4(a6), a0
0692: 3F10           movew (a0), -(a7)
0694: 3F13           movew (a3), -(a7)
0696: 3F3C 0001      movew.ex #1, -(a7)
069A: 6000 00D6      bra 214
069E: 41EC 00F0      lea 240(a4), d0
06A2: 3028 0006      movew.ex 6(a0), d0
06A6: 0640 FF81      addiw #-127, d0
06AA: 0C40 001A      cmpiw #26, d0
06AE: 6200 00CE      bhi 206
06B2: 3028 0004      movew.ex 4(a0), d0
06B6: 0640 FFA6      addiw #-90, d0
06BA: 0C40 0016      cmpiw #22, d0
06BE: 624C           bhi 76
06C0: 0C6C 03EA 0120 cmpiw.ex #1002, 288(a4)
06C6: 6644           bne 68
06C8: 4A6C 010C      tstw.ex 268(a4)
06CC: 663E           bne 62
06CE: 41EC 00DC      lea 220(a4), d0
06D2: 0C50 0006      cmpiw #6, (a0)
06D6: 6700 00A6      beq 166
06DA: 30BC 0006      movew.ex #6, (a0)
06DE: 41EC 0054      lea 84(a4), d0
06E2: 3F10           movew (a0), -(a7)
06E4: 47EC 006E      lea 110(a4), d3
06E8: 3F13           movew (a3), -(a7)
06EA: 45EC 0124      lea 292(a4), d2
06EE: 3F12           movew (a2), -(a7)
06F0: 2D48 FFFC      movel.mx a0, -4(a6)
06F4: 6100 1226      bsr 4646
06F8: 34BC 0006      movew.ex #6, (a2)
06FC: 206E FFFC      movel.ex -4(a6), a0
0700: 3F10           movew (a0), -(a7)
0702: 3F13           movew (a3), -(a7)
0704: 3F3C 0006      movew.ex #6, -(a7)
0708: 6000 0068      bra 104
070C: 41EC 00F0      lea 240(a4), d0
0710: 3028 0006      movew.ex 6(a0), d0
0714: 0640 FF81      addiw #-127, d0
0718: 0C40 001A      cmpiw #26, d0
071C: 6260           bhi 96
071E: 3028 0004      movew.ex 4(a0), d0
0722: 0640 FF8F      addiw #-113, d0
0726: 0C40 0017      cmpiw #23, d0
072A: 6252           bhi 82
072C: 0C6C 03EA 0120 cmpiw.ex #1002, 288(a4)
0732: 664A           bne 74
0734: 4A6C 010C      tstw.ex 268(a4)
0738: 6644           bne 68
073A: 41EC 00DC      lea 220(a4), d0
073E: 0C50 0002      cmpiw #2, (a0)
0742: 673A           beq 58
0744: 30BC 0002      movew.ex #2, (a0)
0748: 41EC 0054      lea 84(a4), d0
074C: 3F10           movew (a0), -(a7)
074E: 47EC 006E      lea 110(a4), d3
0752: 3F13           movew (a3), -(a7)
0754: 45EC 0124      lea 292(a4), d2
0758: 3F12           movew (a2), -(a7)
075A: 2D48 FFFC      movel.mx a0, -4(a6)
075E: 6100 11BC      bsr 4540
0762: 34BC 0002      movew.ex #2, (a2)
0766: 206E FFFC      movel.ex -4(a6), a0
076A: 3F10           movew (a0), -(a7)
076C: 3F13           movew (a3), -(a7)
076E: 3F3C 0002      movew.ex #2, -(a7)
0772: 6100 10F8      bsr 4344
0776: 426C 006A      clrw.ex 106(a4)
077A: DEFC 000C      addaw.ex #12, a7
077E: 0C6C 0002 00F0 cmpiw.ex #2, 240(a4)
0784: 6600 0140      bne 320
0788: 0C6C 03EA 0120 cmpiw.ex #1002, 288(a4)
078E: 6600 0136      bne 310
0792: 4A6C 010C      tstw.ex 268(a4)
0796: 6700 00EE      beq 238
079A: 43EC 0124      lea 292(a4), d1
079E: 2A09           movel a1, d5
07A0: 3011           movew (a1), d0
07A2: 0C40 0001      cmpiw #1, d0
07A6: 6668           bne 104
07A8: 43EC 009C      lea 156(a4), d1
07AC: 2809           movel a1, d4
07AE: 3F11           movew (a1), -(a7)
07B0: 43EC 004C      lea 76(a4), d1
07B4: 2609           movel a1, d3
07B6: 3F11           movew (a1), -(a7)
07B8: 6100 0FF6      bsr 4086
07BC: 47EC 0054      lea 84(a4), d3
07C0: 3F13           movew (a3), -(a7)
07C2: 45EC 006E      lea 110(a4), d2
07C6: 3F12           movew (a2), -(a7)
07C8: 2245           movel d5, a1
07CA: 3F11           movew (a1), -(a7)
07CC: 6100 114E      bsr 4430
07D0: 2245           movel d5, a1
07D2: 32BC 0005      movew.ex #5, (a1)
07D6: 3F13           movew (a3), -(a7)
07D8: 3F12           movew (a2), -(a7)
07DA: 3F3C 0005      movew.ex #5, -(a7)
07DE: 6100 108C      bsr 4236
07E2: 397C 0005 00DC movew.emx #5, 220(a4)
07E8: 2243           movel d3, a1
07EA: 32BC 006F      movew.ex #111, (a1)
07EE: 2244           movel d4, a1
07F0: 32BC 0058      movew.ex #88, (a1)
07F4: 426C 00D4      clrw.ex 212(a4)
07F8: 397C 0005 0068 movew.emx #5, 104(a4)
07FE: 397C 0001 008E movew.emx #1, 142(a4)
0804: 3F3C 0058      movew.ex #88, -(a7)
0808: 3F3C 006F      movew.ex #111, -(a7)
080C: 6000 006C      bra 108
0810: 0C40 0007      cmpiw #7, d0
0814: 6670           bne 112
0816: 43EC 009C      lea 156(a4), d1
081A: 2809           movel a1, d4
081C: 3F11           movew (a1), -(a7)
081E: 43EC 004C      lea 76(a4), d1
0822: 2609           movel a1, d3
0824: 3F11           movew (a1), -(a7)
0826: 6100 0F88      bsr 3976
082A: 47EC 0054      lea 84(a4), d3
082E: 3F13           movew (a3), -(a7)
0830: 45EC 006E      lea 110(a4), d2
0834: 3F12           movew (a2), -(a7)
0836: 2245           movel d5, a1
0838: 3F11           movew (a1), -(a7)
083A: 6100 10E0      bsr 4320
083E: 2245           movel d5, a1
0840: 32BC 0008      movew.ex #8, (a1)
0844: 3F13           movew (a3), -(a7)
0846: 3F12           movew (a2), -(a7)
0848: 3F3C 0008      movew.ex #8, -(a7)
084C: 6100 101E      bsr 4126
0850: 397C 0008 00DC movew.emx #8, 220(a4)
0856: 2243           movel d3, a1
0858: 32BC 005C      movew.ex #92, (a1)
085C: 2244           movel d4, a1
085E: 32BC 003E      movew.ex #62, (a1)
0862: 426C 00D4      clrw.ex 212(a4)
0866: 397C 0005 0068 movew.emx #5, 104(a4)
086C: 397C 0001 008E movew.emx #1, 142(a4)
0872: 3F3C 003E      movew.ex #62, -(a7)
0876: 3F3C 005C      movew.ex #92, -(a7)
087A: 6100 0E78      bsr 3704
087E: DEFC 0014      addaw.ex #20, a7
0882: 6000 003E      bra 62
0886: 41EC 00DC      lea 220(a4), d0
088A: 4A50           tstw (a0)
088C: 6734           beq 52
088E: 4250           clrw (a0)
0890: 41EC 0054      lea 84(a4), d0
0894: 3F10           movew (a0), -(a7)
0896: 47EC 006E      lea 110(a4), d3
089A: 3F13           movew (a3), -(a7)
089C: 45EC 0124      lea 292(a4), d2
08A0: 3F12           movew (a2), -(a7)
08A2: 2D48 FFFC      movel.mx a0, -4(a6)
08A6: 6100 1074      bsr 4212
08AA: 4252           clrw (a2)
08AC: 206E FFFC      movel.ex -4(a6), a0
08B0: 3F10           movew (a0), -(a7)
08B2: 3F13           movew (a3), -(a7)
08B4: 4267           clrw -(a7)
08B6: 6100 0FB4      bsr 4020
08BA: 426C 006A      clrw.ex 106(a4)
08BE: DEFC 000C      addaw.ex #12, a7
08C2: 426C 010C      clrw.ex 268(a4)
08C6: 41EC 00F0      lea 240(a4), d0
08CA: 3010           movew (a0), d0
08CC: 0C40 0007      cmpiw #7, d0
08D0: 6600 0088      bne 136
08D4: 0C68 0BC7 0008 cmpiw.ex #3015, 8(a0)
08DA: 6616           bne 22
08DC: 45EC 0120      lea 288(a4), d2
08E0: 0C52 1F40      cmpiw #8000, (a2)
08E4: 670C           beq 12
08E6: 41EC 0062      lea 98(a4), d0
08EA: 3490           movew (a0), (a2)
08EC: 3F10           movew (a0), -(a7)
08EE: 6000 022E      bra 558
08F2: 41EC 00F0      lea 240(a4), d0
08F6: 3028 0008      movew.ex 8(a0), d0
08FA: 0C40 0BC6      cmpiw #3014, d0
08FE: 6612           bne 18
0900: 6100 FA6E      bsr 64110
0904: 397C 03EA 0120 movew.emx #1002, 288(a4)
090A: 3F3C 03EA      movew.ex #1002, -(a7)
090E: 6000 020E      bra 526
0912: 0C40 0BC7      cmpiw #3015, d0
0916: 661C           bne 28
0918: 45EC 0120      lea 288(a4), d2
091C: 0C52 1F40      cmpiw #8000, (a2)
0920: 6612           bne 18
0922: 6100 5A3C      bsr 23100
0926: 4A40           tstw d0
0928: 6600 021A      bne 538
092C: 34BC 03E9      movew.ex #1001, (a2)
0930: 6000 01E8      bra 488
0934: 41EC 00F0      lea 240(a4), d0
0938: 0C68 1F44 0008 cmpiw.ex #8004, 8(a0)
093E: 6600 0204      bne 516
0942: 6100 5C1A      bsr 23578
0946: 4A40           tstw d0
0948: 6700 01FA      beq 506
094C: 41EC 0062      lea 98(a4), d0
0950: 3950 0120      movew.mx (a0), 288(a4)
0954: 3F10           movew (a0), -(a7)
0956: 6000 01C6      bra 454
095A: 0C40 0008      cmpiw #8, d0
095E: 6700 01E4      beq 484
0962: 0C40 0009      cmpiw #9, d0
0966: 6700 01DC      beq 476
096A: 0C40 0017      cmpiw #23, d0
096E: 6614           bne 20
0970: 3F28 0008      movew.ex 8(a0), -(a7)
0974: 4E4F           trap #15
0976: A16F           sysTrapFrmInitForm
0978: 2F08           movel a0, -(a7)
097A: 4E4F           trap #15
097C: A174           sysTrapFrmSetActiveForm
097E: 5C4F           addqw #6, a7
0980: 6000 01C2      bra 450
0984: 0C40 0018      cmpiw #24, d0
0988: 6600 010E      bne 270
098C: 47EC 0120      lea 288(a4), d3
0990: 0C53 1F40      cmpiw #8000, (a3)
0994: 6656           bne 86
0996: 3F3C 001E      movew.ex #30, -(a7)
099A: 43EC 00B0      lea 176(a4), d1
099E: 2609           movel a1, d3
09A0: 2F03           movel d3, -(a7)
09A2: 6100 5FE0      bsr 24544
09A6: 3F3C 001E      movew.ex #30, -(a7)
09AA: 2F03           movel d3, -(a7)
09AC: 6100 56E2      bsr 22242
09B0: 3F3C 1F42      movew.ex #8002, -(a7)
09B4: 45EC 005C      lea 92(a4), d2
09B8: 2F12           movel (a2), -(a7)
09BA: 4E4F           trap #15
09BC: A180           sysTrapFrmGetObjectIndex
09BE: 3F00           movew d0, -(a7)
09C0: 2F12           movel (a2), -(a7)
09C2: 4E4F           trap #15
09C4: A184           sysTrapFrmHideObject
09C6: 2F03           movel d3, -(a7)
09C8: 3F3C 1F42      movew.ex #8002, -(a7)
09CC: 2F12           movel (a2), -(a7)
09CE: 4E4F           trap #15
09D0: A18C           sysTrapFrmCopyLabel
09D2: DEFC 0022      addaw.ex #34, a7
09D6: 3F3C 1F42      movew.ex #8002, -(a7)
09DA: 2F12           movel (a2), -(a7)
09DC: 4E4F           trap #15
09DE: A180           sysTrapFrmGetObjectIndex
09E0: 3F00           movew d0, -(a7)
09E2: 2F12           movel (a2), -(a7)
09E4: 4E4F           trap #15
09E6: A185           sysTrapFrmShowObject
09E8: DEFC 000C      addaw.ex #12, a7
09EC: 4E4F           trap #15
09EE: A173           sysTrapFrmGetActiveForm
09F0: 2948 005C      movel.mx a0, 92(a4)
09F4: 2F08           movel a0, -(a7)
09F6: 4E4F           trap #15
09F8: A171           sysTrapFrmDrawForm
09FA: 6100 589A      bsr 22682
09FE: 584F           addqw #4, a7
0A00: 4A40           tstw d0
0A02: 6610           bne 16
0A04: 6100 5890      bsr 22672
0A08: 4A40           tstw d0
0A0A: 6674           bne 116
0A0C: 6100 5952      bsr 22866
0A10: 4A40           tstw d0
0A12: 666C           bne 108
0A14: 0C53 03EA      cmpiw #1002, (a3)
0A18: 6600 012A      bne 298
0A1C: 3F2C 006C      movew.ex 108(a4), -(a7)
0A20: 6100 09DE      bsr 2526
0A24: 6100 0B8A      bsr 2954
0A28: 3F2C 0054      movew.ex 84(a4), -(a7)
0A2C: 3F2C 006E      movew.ex 110(a4), -(a7)
0A30: 3F2C 0124      movew.ex 292(a4), -(a7)
0A34: 6100 0E36      bsr 3638
0A38: 3F2C 009C      movew.ex 156(a4), -(a7)
0A3C: 3F2C 004C      movew.ex 76(a4), -(a7)
0A40: 6100 0CB2      bsr 3250
0A44: 45EC 0108      lea 264(a4), d2
0A48: 2F0A           movel a2, -(a7)
0A4A: 6100 0836      bsr 2102
0A4E: 3F3C 1775      movew.ex #6005, -(a7)
0A52: 2F12           movel (a2), -(a7)
0A54: 6100 5E5C      bsr 24156
0A58: 3F3C 1774      movew.ex #6004, -(a7)
0A5C: 2F2C 0048      movel.ex 72(a4), -(a7)
0A60: 6100 5E50      bsr 24144
0A64: 3F3C 1776      movew.ex #6006, -(a7)
0A68: 4280           clrl d0
0A6A: 302C 0122      movew.ex 290(a4), d0
0A6E: 2F00           movel d0, -(a7)
0A70: 6100 5E40      bsr 24128
0A74: DEFC 0022      addaw.ex #34, a7
0A78: 6100 60C2      bsr 24770
0A7C: 6000 00C6      bra 198
0A80: 41EC 0120      lea 288(a4), d0
0A84: 0C50 03E9      cmpiw #1001, (a0)
0A88: 6600 00BA      bne 186
0A8C: 30BC 1F40      movew.ex #8000, (a0)
0A90: 3F3C 1F40      movew.ex #8000, -(a7)
0A94: 6000 0088      bra 136
0A98: 0C40 0015      cmpiw #21, d0
0A9C: 6600 00A6      bne 166
0AA0: 3028 0008      movew.ex 8(a0), d0
0AA4: 0C40 0FA1      cmpiw #4001, d0
0AA8: 661C           bne 28
0AAA: 426C 009E      clrw.ex 158(a4)
0AAE: 6100 57E6      bsr 22502
0AB2: 4A40           tstw d0
0AB4: 6600 008E      bne 142
0AB8: 397C 1F40 0120 movew.emx #8000, 288(a4)
0ABE: 3F3C 1F40      movew.ex #8000, -(a7)
0AC2: 6000 005A      bra 90
0AC6: 0C40 0FA2      cmpiw #4002, d0
0ACA: 6612           bne 18
0ACC: 426C 009E      clrw.ex 158(a4)
0AD0: 397C 03E8 0120 movew.emx #1000, 288(a4)
0AD6: 3F3C 03E8      movew.ex #1000, -(a7)
0ADA: 6000 0042      bra 66
0ADE: 0C40 0FA3      cmpiw #4003, d0
0AE2: 660C           bne 12
0AE4: 426C 009E      clrw.ex 158(a4)
0AE8: 30BC 0016      movew.ex #22, (a0)
0AEC: 6000 0056      bra 86
0AF0: 0C40 0FA4      cmpiw #4004, d0
0AF4: 6632           bne 50
0AF6: 426C 009E      clrw.ex 158(a4)
0AFA: 42A7           clrl -(a7)
0AFC: 42A7           clrl -(a7)
0AFE: 43FA F990      lea 30872(pc), d1
0B02: 2F09           movel a1, -(a7)
0B04: 3F3C 1194      movew.ex #4500, -(a7)
0B08: 4E4F           trap #15
0B0A: A194           sysTrapFrmCustomAlert
0B0C: DEFC 000E      addaw.ex #14, a7
0B10: 4A40           tstw d0
0B12: 6630           bne 48
0B14: 397C 03E9 0120 movew.emx #1001, 288(a4)
0B1A: 3F3C 03E9      movew.ex #1001, -(a7)
0B1E: 4E4F           trap #15
0B20: A19B           sysTrapFrmGotoForm
0B22: 544F           addqw #2, a7
0B24: 6000 001E      bra 30
0B28: 0C40 0FA5      cmpiw #4005, d0
0B2C: 6616           bne 22
0B2E: 426C 009E      clrw.ex 158(a4)
0B32: 41EC 00E4      lea 228(a4), d0
0B36: 4A50           tstw (a0)
0B38: 6706           beq 6
0B3A: 4250           clrw (a0)
0B3C: 6000 0006      bra 6
0B40: 30BC 0001      movew.ex #1, (a0)
0B44: 486C 00F0      pea.ex 240(a4)
0B48: 4E4F           trap #15
0B4A: A173           sysTrapFrmGetActiveForm
0B4C: 2F08           movel a0, -(a7)
0B4E: 4E4F           trap #15
0B50: A17A           sysTrapFrmHandleEvent
0B52: 504F           addqw #0, a7
0B54: 0C6C 03EA 0120 cmpiw.ex #1002, 288(a4)
0B5A: 6600 0604      bne 1540
0B5E: 4A6C 010C      tstw.ex 268(a4)
0B62: 6600 05FC      bne 1532
0B66: 4A6C 009E      tstw.ex 158(a4)
0B6A: 6600 05F4      bne 1524
0B6E: 41EC 006A      lea 106(a4), d0
0B72: 3010           movew (a0), d0
0B74: 3C00           movew d0, d6
0B76: 5246           addqw #1, d6
0B78: 3086           movew d6, (a0)
0B7A: 5240           addqw #1, d0
0B7C: 0C40 003B      cmpiw #59, d0
0B80: 6330           bls 48
0B82: 4250           clrw (a0)
0B84: 41EC 0054      lea 84(a4), d0
0B88: 3F10           movew (a0), -(a7)
0B8A: 47EC 006E      lea 110(a4), d3
0B8E: 3F13           movew (a3), -(a7)
0B90: 45EC 0124      lea 292(a4), d2
0B94: 3F12           movew (a2), -(a7)
0B96: 2D48 FFFC      movel.mx a0, -4(a6)
0B9A: 6100 0D80      bsr 3456
0B9E: 4252           clrw (a2)
0BA0: 206E FFFC      movel.ex -4(a6), a0
0BA4: 3F10           movew (a0), -(a7)
0BA6: 3F13           movew (a3), -(a7)
0BA8: 4267           clrw -(a7)
0BAA: 6100 0CC0      bsr 3264
0BAE: DEFC 000C      addaw.ex #12, a7
0BB2: 4E4F           trap #15
0BB4: A0F7           sysTrapTimGetTicks
0BB6: 2940 0064      movel.mx d0, 100(a4)
0BBA: 45EC 00D0      lea 208(a4), d2
0BBE: 9092           subrl (a2), d0
0BC0: 2940 0094      movel.mx d0, 148(a4)
0BC4: 7C0E           moveq #14, d6
0BC6: BC80           cmpl d0, d6
0BC8: 6400 0596      bcc 1430
0BCC: 4E4F           trap #15
0BCE: A0F7           sysTrapTimGetTicks
0BD0: 2480           movel d0, (a2)
0BD2: 47EC 009C      lea 156(a4), d3
0BD6: 3F13           movew (a3), -(a7)
0BD8: 45EC 004C      lea 76(a4), d2
0BDC: 3F12           movew (a2), -(a7)
0BDE: 6100 0BD0      bsr 3024
0BE2: 302C 0068      movew.ex 104(a4), d0
0BE6: C1EC 008E      muls.ex 142(a4), d0
0BEA: 3212           movew (a2), d1
0BEC: D240           addrw d0, d1
0BEE: 3481           movew d1, (a2)
0BF0: 302C 00D4      movew.ex 212(a4), d0
0BF4: C1EC 00A0      muls.ex 160(a4), d0
0BF8: 3413           movew (a3), d2
0BFA: D440           addrw d0, d2
0BFC: 3682           movew d2, (a3)
0BFE: 584F           addqw #4, a7
0C00: 0C41 0093      cmpiw #147, d1
0C04: 6358           bls 88
0C06: 302C 006C      movew.ex 108(a4), d0
0C0A: B042           cmpw d2, d0
0C0C: 6250           bhi 80
0C0E: 0640 000B      addiw #11, d0
0C12: B042           cmpw d2, d0
0C14: 6548           bcs 72
0C16: 4A6C 0030      tstw.ex 48(a4)
0C1A: 6742           beq 66
0C1C: 45EC 0048      lea 72(a4), d2
0C20: 7C64           moveq #100, d6
0C22: DD92           addml d6, (a2)
0C24: 6100 5242      bsr 21058
0C28: 6100 F7B2      bsr 63410
0C2C: 3F3C 1774      movew.ex #6004, -(a7)
0C30: 2F12           movel (a2), -(a7)
0C32: 6100 5C7E      bsr 23678
0C36: 47EC 0108      lea 264(a4), d3
0C3A: 2012           movel (a2), d0
0C3C: 5C4F           addqw #6, a7
0C3E: B093           cmpl (a3), d0
0C40: 6F00 04B8      ble 1208
0C44: 2680           movel d0, (a3)
0C46: 3F3C 1775      movew.ex #6005, -(a7)
0C4A: 2F00           movel d0, -(a7)
0C4C: 6100 5C64      bsr 23652
0C50: 2F13           movel (a3), -(a7)
0C52: 6100 0588      bsr 1416
0C56: DEFC 000A      addaw.ex #10, a7
0C5A: 6000 049E      bra 1182
0C5E: 0C6C 0095 004C cmpiw.ex #149, 76(a4)
0C64: 6312           bls 18
0C66: 41EC 008E      lea 142(a4), d0
0C6A: 0C50 0001      cmpiw #1, (a0)
0C6E: 6608           bne 8
0C70: 30BC FFFF      movew.ex #65535, (a0)
0C74: 6000 0050      bra 80
0C78: 0C6C 0004 004C cmpiw.ex #4, 76(a4)
0C7E: 620A           bhi 10
0C80: 41EC 008E      lea 142(a4), d0
0C84: 0C50 FFFF      cmpiw #-1, (a0)
0C88: 6738           beq 56
0C8A: 0C6C 0079 009C cmpiw.ex #121, 156(a4)
0C90: 631E           bls 30
0C92: 41EC 00A0      lea 160(a4), d0
0C96: 0C50 0001      cmpiw #1, (a0)
0C9A: 6614           bne 20
0C9C: 30BC FFFF      movew.ex #65535, (a0)
0CA0: 426C 0030      clrw.ex 48(a4)
0CA4: 6100 5338      bsr 21304
0CA8: 6100 5E92      bsr 24210
0CAC: 6000 044C      bra 1100
0CB0: 0C6C 002A 009C cmpiw.ex #42, 156(a4)
0CB6: 621E           bhi 30
0CB8: 41EC 00A0      lea 160(a4), d0
0CBC: 0C50 FFFF      cmpiw #-1, (a0)
0CC0: 6614           bne 20
0CC2: 30BC 0001      movew.ex #1, (a0)
0CC6: 426C 0042      clrw.ex 66(a4)
0CCA: 426C 0030      clrw.ex 48(a4)
0CCE: 6100 530E      bsr 21262
0CD2: 6000 0426      bra 1062
0CD6: 4A6C 0124      tstw.ex 292(a4)
0CDA: 6600 0092      bne 146
0CDE: 302C 004C      movew.ex 76(a4), d0
0CE2: 0640 FFB8      addiw #-72, d0
0CE6: 0C40 0006      cmpiw #6, d0
0CEA: 620E           bhi 14
0CEC: 302C 009C      movew.ex 156(a4), d0
0CF0: 0640 FFB5      addiw #-75, d0
0CF4: 0C40 0006      cmpiw #6, d0
0CF8: 6344           bls 68
0CFA: 4A6C 0124      tstw.ex 292(a4)
0CFE: 666E           bne 110
0D00: 302C 004C      movew.ex 76(a4), d0
0D04: 0640 FFC3      addiw #-61, d0
0D08: 0C40 0020      cmpiw #32, d0
0D0C: 620E           bhi 14
0D0E: 302C 009C      movew.ex 156(a4), d0
0D12: 0640 FFAA      addiw #-86, d0
0D16: 0C40 0012      cmpiw #18, d0
0D1A: 6322           bls 34
0D1C: 4A6C 0124      tstw.ex 292(a4)
0D20: 664C           bne 76
0D22: 302C 004C      movew.ex 76(a4), d0
0D26: 0640 FFBC      addiw #-68, d0
0D2A: 0C40 000E      cmpiw #14, d0
0D2E: 623E           bhi 62
0D30: 302C 009C      movew.ex 156(a4), d0
0D34: 0640 FF98      addiw #-104, d0
0D38: 0C40 0014      cmpiw #20, d0
0D3C: 6230           bhi 48
0D3E: 41EC 0042      lea 66(a4), d0
0D42: 4A50           tstw (a0)
0D44: 6600 03B4      bne 948
0D48: 30BC 0001      movew.ex #1, (a0)
0D4C: 45EC 0122      lea 290(a4), d2
0D50: 5252           addqw #1, (a2)
0D52: 426C 0030      clrw.ex 48(a4)
0D56: 6100 52BC      bsr 21180
0D5A: 3F3C 1776      movew.ex #6006, -(a7)
0D5E: 4280           clrl d0
0D60: 3012           movew (a2), d0
0D62: 2F00           movel d0, -(a7)
0D64: 6100 5B4C      bsr 23372
0D68: 5C4F           addqw #6, a7
0D6A: 6000 038E      bra 910
0D6E: 0C6C 0001 0124 cmpiw.ex #1, 292(a4)
0D74: 6600 00CE      bne 206
0D78: 45EC 004C      lea 76(a4), d2
0D7C: 3012           movew (a2), d0
0D7E: 0640 FFE8      addiw #-24, d0
0D82: 0C40 0010      cmpiw #16, d0
0D86: 6238           bhi 56
0D88: 47EC 009C      lea 156(a4), d3
0D8C: 3013           movew (a3), d0
0D8E: 0640 FFA8      addiw #-88, d0
0D92: 0C40 0009      cmpiw #9, d0
0D96: 6228           bhi 40
0D98: 41EC 010C      lea 268(a4), d0
0D9C: 4A50           tstw (a0)
0D9E: 6600 035A      bne 858
0DA2: 397C 0001 0030 movew.emx #1, 48(a4)
0DA8: 30BC 0001      movew.ex #1, (a0)
0DAC: 34BC 001C      movew.ex #28, (a2)
0DB0: 36BC 005D      movew.ex #93, (a3)
0DB4: 3F3C 005D      movew.ex #93, -(a7)
0DB8: 3F3C 001C      movew.ex #28, -(a7)
0DBC: 6000 0078      bra 120
0DC0: 0C6C 0001 0124 cmpiw.ex #1, 292(a4)
0DC6: 667C           bne 124
0DC8: 43EC 004C      lea 76(a4), d1
0DCC: 2609           movel a1, d3
0DCE: 3011           movew (a1), d0
0DD0: 0640 FFB8      addiw #-72, d0
0DD4: 0C40 0010      cmpiw #16, d0
0DD8: 626A           bhi 106
0DDA: 47EC 009C      lea 156(a4), d3
0DDE: 3013           movew (a3), d0
0DE0: 0640 FFAA      addiw #-86, d0
0DE4: 0C40 0009      cmpiw #9, d0
0DE8: 625A           bhi 90
0DEA: 41EC 0030      lea 48(a4), d0
0DEE: 4A50           tstw (a0)
0DF0: 6600 0308      bne 776
0DF4: 30BC 0001      movew.ex #1, (a0)
0DF8: 41EC 008E      lea 142(a4), d0
0DFC: 0C50 FFFF      cmpiw #-1, (a0)
0E00: 6604           bne 4
0E02: 30BC 0001      movew.ex #1, (a0)
0E06: 397C 0005 0068 movew.emx #5, 104(a4)
0E0C: 45EC 00D4      lea 212(a4), d2
0E10: 3F3C 0001      movew.ex #1, -(a7)
0E14: 6100 05BC      bsr 1468
0E18: 3480           movew d0, (a2)
0E1A: 3F3C 0001      movew.ex #1, -(a7)
0E1E: 6100 05B2      bsr 1458
0E22: 41EC 00A0      lea 160(a4), d0
0E26: 3080           movew d0, (a0)
0E28: 584F           addqw #4, a7
0E2A: 6604           bne 4
0E2C: 30BC FFFF      movew.ex #65535, (a0)
0E30: 3F13           movew (a3), -(a7)
0E32: 2243           movel d3, a1
0E34: 3F11           movew (a1), -(a7)
0E36: 6100 08BC      bsr 2236
0E3A: 6100 50B4      bsr 20660
0E3E: 584F           addqw #4, a7
0E40: 6000 02B8      bra 696
0E44: 0C6C 0004 0124 cmpiw.ex #4, 292(a4)
0E4A: 6600 009E      bne 158
0E4E: 302C 004C      movew.ex 76(a4), d0
0E52: 0640 FFD1      addiw #-47, d0
0E56: 0C40 0013      cmpiw #19, d0
0E5A: 6236           bhi 54
0E5C: 302C 009C      movew.ex 156(a4), d0
0E60: 0640 FFB2      addiw #-78, d0
0E64: 0C40 000E      cmpiw #14, d0
0E68: 6228           bhi 40
0E6A: 41EC 00A0      lea 160(a4), d0
0E6E: 0C50 0001      cmpiw #1, (a0)
0E72: 6600 0286      bne 646
0E76: 30BC FFFF      movew.ex #65535, (a0)
0E7A: 45EC 0068      lea 104(a4), d2
0E7E: 3F3C 0003      movew.ex #3, -(a7)
0E82: 6100 054E      bsr 1358
0E86: 3480           movew d0, (a2)
0E88: 397C 0005 00D4 movew.emx #5, 212(a4)
0E8E: 6000 004A      bra 74
0E92: 0C6C 0004 0124 cmpiw.ex #4, 292(a4)
0E98: 6650           bne 80
0E9A: 302C 004C      movew.ex 76(a4), d0
0E9E: 0640 FFBE      addiw #-66, d0
0EA2: 0C40 000E      cmpiw #14, d0
0EA6: 6242           bhi 66
0EA8: 302C 009C      movew.ex 156(a4), d0
0EAC: 0640 FFAB      addiw #-85, d0
0EB0: 0C40 000B      cmpiw #11, d0
0EB4: 6234           bhi 52
0EB6: 41EC 00A0      lea 160(a4), d0
0EBA: 0C50 0001      cmpiw #1, (a0)
0EBE: 6600 023A      bne 570
0EC2: 30BC FFFF      movew.ex #65535, (a0)
0EC6: 397C 0005 0068 movew.emx #5, 104(a4)
0ECC: 45EC 00D4      lea 212(a4), d2
0ED0: 3F3C 0003      movew.ex #3, -(a7)
0ED4: 6100 04FC      bsr 1276
0ED8: 3480           movew d0, (a2)
0EDA: 397C 0001 0030 movew.emx #1, 48(a4)
0EE0: 6100 500E      bsr 20494
0EE4: 544F           addqw #2, a7
0EE6: 6000 0212      bra 530
0EEA: 0C6C 0003 0124 cmpiw.ex #3, 292(a4)
0EF0: 6600 008E      bne 142
0EF4: 302C 004C      movew.ex 76(a4), d0
0EF8: 0640 FFD1      addiw #-47, d0
0EFC: 0C40 0009      cmpiw #9, d0
0F00: 6234           bhi 52
0F02: 302C 009C      movew.ex 156(a4), d0
0F06: 0640 FFA4      addiw #-92, d0
0F0A: 0C40 0007      cmpiw #7, d0
0F0E: 6226           bhi 38
0F10: 41EC 008E      lea 142(a4), d0
0F14: 0C50 0001      cmpiw #1, (a0)
0F18: 6600 01E0      bne 480
0F1C: 30BC FFFF      movew.ex #65535, (a0)
0F20: 397C FFFF 00A0 movew.emx #65535, 160(a4)
0F26: 397C 0001 0068 movew.emx #1, 104(a4)
0F2C: 397C 0005 00D4 movew.emx #5, 212(a4)
0F32: 6000 01BC      bra 444
0F36: 0C6C 0003 0124 cmpiw.ex #3, 292(a4)
0F3C: 6642           bne 66
0F3E: 302C 004C      movew.ex 76(a4), d0
0F42: 0640 FFD1      addiw #-47, d0
0F46: 0C40 0009      cmpiw #9, d0
0F4A: 6234           bhi 52
0F4C: 302C 009C      movew.ex 156(a4), d0
0F50: 0640 FF9D      addiw #-99, d0
0F54: 0C40 000D      cmpiw #13, d0
0F58: 6226           bhi 38
0F5A: 41EC 008E      lea 142(a4), d0
0F5E: 0C50 0001      cmpiw #1, (a0)
0F62: 6600 0196      bne 406
0F66: 30BC FFFF      movew.ex #65535, (a0)
0F6A: 397C FFFF 00A0 movew.emx #65535, 160(a4)
0F70: 397C 0005 0068 movew.emx #5, 104(a4)
0F76: 397C 0003 00D4 movew.emx #3, 212(a4)
0F7C: 6000 0172      bra 370
0F80: 0C6C 0002 0124 cmpiw.ex #2, 292(a4)
0F86: 6600 008E      bne 142
0F8A: 302C 004C      movew.ex 76(a4), d0
0F8E: 0640 FFA7      addiw #-89, d0
0F92: 0C40 0006      cmpiw #6, d0
0F96: 6234           bhi 52
0F98: 302C 009C      movew.ex 156(a4), d0
0F9C: 0640 FFAF      addiw #-81, d0
0FA0: 0C40 000E      cmpiw #14, d0
0FA4: 6226           bhi 38
0FA6: 41EC 008E      lea 142(a4), d0
0FAA: 0C50 FFFF      cmpiw #-1, (a0)
0FAE: 6600 014A      bne 330
0FB2: 30BC 0001      movew.ex #1, (a0)
0FB6: 397C 0001 00A0 movew.emx #1, 160(a4)
0FBC: 397C 0006 0068 movew.emx #6, 104(a4)
0FC2: 397C 0001 00D4 movew.emx #1, 212(a4)
0FC8: 6000 0126      bra 294
0FCC: 0C6C 0002 0124 cmpiw.ex #2, 292(a4)
0FD2: 6642           bne 66
0FD4: 302C 004C      movew.ex 76(a4), d0
0FD8: 0640 FF9C      addiw #-100, d0
0FDC: 0C40 0006      cmpiw #6, d0
0FE0: 6234           bhi 52
0FE2: 302C 009C      movew.ex 156(a4), d0
0FE6: 0640 FFA1      addiw #-95, d0
0FEA: 0C40 0019      cmpiw #25, d0
0FEE: 6226           bhi 38
0FF0: 41EC 008E      lea 142(a4), d0
0FF4: 0C50 FFFF      cmpiw #-1, (a0)
0FF8: 6600 0100      bn