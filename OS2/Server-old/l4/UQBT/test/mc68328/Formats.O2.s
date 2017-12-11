0082: 0000 0001      orib #1, d0
0086: 487A 0004      pea.ex 29844(pc)
008A: 0697 0000 0006 addil #6, (a7)
0090: 4E75           rts
0092: 2F03           movel d3, -(a7)
0094: 4FEF FFF4      lea -12(a7), d7
0098: 4857           pea (a7)
009A: 486F 0008      pea.ex 8(a7)
009E: 486F 0010      pea.ex 16(a7)
00A2: 4E4F           trap #15
00A4: A08F           sysTrapSysAppStartup
00A6: 3600           movew d0, d3
00A8: 4A43           tstw d3
00AA: 4FEF 000C      lea 12(a7), d7
00AE: 671A           beq 26
00B0: 486D FFE4      pea.ex -28(a5)
00B4: 3F3C 003C      movew.ex #60, -(a7)
00B8: 486D FFD6      pea.ex -42(a5)
00BC: 4E4F           trap #15
00BE: A084           sysTrapErrDisplayFileLineMsg
00C0: 7000           moveq #0, d0
00C2: 4FEF 0016      lea 22(a7), d7
00C6: 261F           movel (a7)+, d3
00C8: 4E75           rts
00CA: 206F 0008      movel.ex 8(a7), a0
00CE: 3028 0006      movew.ex 6(a0), d0
00D2: 0240 0004      andiw #4, d0
00D6: 6710           beq 16
00D8: 487A 000E      pea.ex 29936(pc)
00DC: 487A 0004      pea.ex 29930(pc)
00E0: 0697 0000 0040 addil #64, (a7)
00E6: 4E75           rts
00E8: 206F 0008      movel.ex 8(a7), a0
00EC: 3F28 0006      movew.ex 6(a0), -(a7)
00F0: 2F28 0002      movel.ex 2(a0), -(a7)
00F4: 3F10           movew (a0), -(a7)
00F6: 487A 000E      pea.ex 29966(pc)
00FA: 487A 0004      pea.ex 29960(pc)
00FE: 0697 0000 0C18 addil #3096, (a7)
0104: 4E75           rts
0106: 2600           movel d0, d3
0108: 2F2F 0008      movel.ex 8(a7), -(a7)
010C: 2F2F 0010      movel.ex 16(a7), -(a7)
0110: 2F2F 0018      movel.ex 24(a7), -(a7)
0114: 4E4F           trap #15
0116: A090           sysTrapSysAppExit
0118: 2003           movel d3, d0
011A: 4FEF 0020      lea 32(a7), d7
011E: 261F           movel (a7)+, d3
0120: 4E75           rts
0122: 4E75           rts
0124: 48E7 3000      movem <3000>, -(a7)
0128: 7402           moveq #2, d2
012A: 4EFB 2200      jmp.ex 0(pc,d2.w)
012E: 600A           bra 10
0130: 4C01           dc.w #19457
0132: 0000           dc.w #0
0134: 4CDF 000C      movem (a7)+, <000c>
0138: 4E75           rts
013A: 2400           movel d0, d2
013C: 4842           swap d2
013E: C4C1           mulu d1, d2
0140: 2601           movel d1, d3
0142: 4843           swap d3
0144: C6C0           mulu d0, d3
0146: D443           addrw d3, d2
0148: 4842           swap d2
014A: 4242           clrw d2
014C: C0C1           mulu d1, d0
014E: D082           addrl d2, d0
0150: 4CDF 000C      movem (a7)+, <000c>
0154: 4E75           rts
0156: 205F           movel (a7)+, a0
0158: 2248           movel a0, a1
015A: D2D8           addaw (a0)+, a1
015C: B058           cmpw (a0)+, d0
015E: 6C02           bge 2
0160: 4ED1           jmp (a1)
0162: B058           cmpw (a0)+, d0
0164: 6F02           ble 2
0166: 4ED1           jmp (a1)
0168: 3218           movew (a0)+, d1
016A: B058           cmpw (a0)+, d0
016C: 6604           bne 4
016E: D0D0           addaw (a0), a0
0170: 4ED0           jmp (a0)
0172: 5448           addqw #2, a0
0174: 51C9 FFF4      dbf d1, 65524
0178: 4ED1           jmp (a1)
017A: 4E56 0000      link a6, #0
017E: 2F0A           movel a2, -(a7)
0180: 246E 0010      movel.ex 16(a6), a2
0184: 206E 000C      movel.ex 12(a6), a0
0188: 7000           moveq #0, d0
018A: 1010           moveb (a0), d0
018C: ED88           lslil #6, d0
018E: 4872 0801      pea.ex 1(a2,d0.l)
0192: 206E 0008      movel.ex 8(a6), a0
0196: 7000           moveq #0, d0
0198: 1010           moveb (a0), d0
019A: ED88           lslil #6, d0
019C: 4872 0801      pea.ex 1(a2,d0.l)
01A0: 4E4F           trap #15
01A2: A0C8           sysTrapStrCompare
01A4: 504F           addqw #0, a7
01A6: 245F           movel (a7)+, a2
01A8: 4E5E           unlk a6
01AA: 4E75           rts
01AC: 9043           subrw d3, d0
01AE: 6F6D           ble 109
01B0: 7061           moveq #97, d0
01B2: 7265           moveq #101, d1
01B4: 436F           dc.w #17263
01B6: 756E           dc.w #30062
01B8: 7472           moveq #114, d2
01BA: 6965           bvs 101
01BC: 7300           dc.w #29440
01BE: 0000           dc.w #0
01C0: 4E56 0000      link a6, #0
01C4: 2F03           movel d3, -(a7)
01C6: 1F3C 0005      moveb.ex #5, -(a7)
01CA: 4E4F           trap #15
01CC: A2D1           sysTrapPrefGetPreference
01CE: 1B40 FF7B      moveb.mx d0, -133(a5)
01D2: 1F3C 0002      moveb.ex #2, -(a7)
01D6: 4E4F           trap #15
01D8: A2D1           sysTrapPrefGetPreference
01DA: 1B40 FF7C      moveb.mx d0, -132(a5)
01DE: 1F3C 0003      moveb.ex #3, -(a7)
01E2: 4E4F           trap #15
01E4: A2D1           sysTrapPrefGetPreference
01E6: 1B40 FF7D      moveb.mx d0, -131(a5)
01EA: 1F3C 0004      moveb.ex #4, -(a7)
01EE: 4E4F           trap #15
01F0: A2D1           sysTrapPrefGetPreference
01F2: 1B40 FF79      moveb.mx d0, -135(a5)
01F6: 1F3C 0006      moveb.ex #6, -(a7)
01FA: 4E4F           trap #15
01FC: A2D1           sysTrapPrefGetPreference
01FE: 1B40 FF7A      moveb.mx d0, -134(a5)
0202: 1F3C 0001      moveb.ex #1, -(a7)
0206: 4E4F           trap #15
0208: A2D1           sysTrapPrefGetPreference
020A: 1B40 FF78      moveb.mx d0, -136(a5)
020E: 3F3C 2710      movew.ex #10000, -(a7)
0212: 2F3C 636E 7479 movel.exl #1668183161, -(a7)
0218: 4E4F           trap #15
021A: A05F           sysTrapDmGetResource
021C: 2B48 FF5A      movel.mx a0, -166(a5)
0220: 4AAD FF5A      tstl.ex -166(a5)
0224: 4FEF 0012      lea 18(a7), d7
0228: 6750           beq 80
022A: 2F2D FF5A      movel.ex -166(a5), -(a7)
022E: 4E4F           trap #15
0230: A021           sysTrapMemHandleLock
0232: 2B48 FF56      movel.mx a0, -170(a5)
0236: 2F2D FF56      movel.ex -170(a5), -(a7)
023A: 4E4F           trap #15
023C: A016           sysTrapMemPtrSize
023E: EC88           lsril #6, d0
0240: 1B40 FF55      moveb.mx d0, -171(a5)
0244: 7600           moveq #0, d3
0246: 504F           addqw #0, a7
0248: 600E           bra 14
024A: 7000           moveq #0, d0
024C: 1003           moveb d3, d0
024E: 41ED FF3C      lea -196(a5), d0
0252: 1183 0000      moveb.mx d3, 0(a0,d0.w)
0256: 5203           addqb #1, d3
0258: 0C03 0017      cmpib #23, d3
025C: 63EC           bls -20
025E: 2F2D FF56      movel.ex -170(a5), -(a7)
0262: 487A FF16      pea.ex 30082(pc)
0266: 2F3C 0018 0001 movel.exl #1572865, -(a7)
026C: 486D FF3C      pea.ex -196(a5)
0270: 4E4F           trap #15
0272: A2B3           sysTrapSysInsertionSort
0274: 4FEF 0010      lea 16(a7), d7
0278: 6008           bra 8
027A: 42AD FF56      clrl.ex -170(a5)
027E: 422D FF55      clrb.ex -171(a5)
0282: 7000           moveq #0, d0
0284: 261F           movel (a7)+, d3
0286: 4E5E           unlk a6
0288: 4E75           rts
028A: 9053           subrw (a3), d0
028C: 7461           moveq #97, d2
028E: 7274           moveq #116, d1
0290: 4170           dc.w #16752
0292: 706C           moveq #108, d0
0294: 6963           bvs 99
0296: 6174           bsr 116
0298: 696F           bvs 111
029A: 6E00 0000      bgt 0
029E: 4E56 0000      link a6, #0
02A2: 4E4F           trap #15
02A4: A1A1           sysTrapFrmCloseAllForms
02A6: 4AAD FF38      tstl.ex -200(a5)
02AA: 670A           beq 10
02AC: 2F2D FF38      movel.ex -200(a5), -(a7)
02B0: 4E4F           trap #15
02B2: A012           sysTrapMemChunkFree
02B4: 584F           addqw #4, a7
02B6: 2F2D FF5A      movel.ex -166(a5), -(a7)
02BA: 4E4F           trap #15
02BC: A022           sysTrapMemHandleUnlock
02BE: 2F2D FF5A      movel.ex -166(a5), -(a7)
02C2: 4E4F           trap #15
02C4: A061           sysTrapDmReleaseResource
02C6: 7000           moveq #0, d0
02C8: 102D FF7B      moveb.ex -133(a5), d0
02CC: 2F00           movel d0, -(a7)
02CE: 1F3C 0005      moveb.ex #5, -(a7)
02D2: 4E4F           trap #15
02D4: A2D2           sysTrapPrefSetPreference
02D6: 7000           moveq #0, d0
02D8: 102D FF7C      moveb.ex -132(a5), d0
02DC: 2F00           movel d0, -(a7)
02DE: 1F3C 0002      moveb.ex #2, -(a7)
02E2: 4E4F           trap #15
02E4: A2D2           sysTrapPrefSetPreference
02E6: 7000           moveq #0, d0
02E8: 102D FF7D      moveb.ex -131(a5), d0
02EC: 2F00           movel d0, -(a7)
02EE: 1F3C 0003      moveb.ex #3, -(a7)
02F2: 4E4F           trap #15
02F4: A2D2           sysTrapPrefSetPreference
02F6: 7000           moveq #0, d0
02F8: 102D FF79      moveb.ex -135(a5), d0
02FC: 2F00           movel d0, -(a7)
02FE: 1F3C 0004      moveb.ex #4, -(a7)
0302: 4E4F           trap #15
0304: A2D2           sysTrapPrefSetPreference
0306: 7000           moveq #0, d0
0308: 102D FF7A      moveb.ex -134(a5), d0
030C: 2F00           movel d0, -(a7)
030E: 1F3C 0006      moveb.ex #6, -(a7)
0312: 4E4F           trap #15
0314: A2D2           sysTrapPrefSetPreference
0316: 7000           moveq #0, d0
0318: 102D FF78      moveb.ex -136(a5), d0
031C: 2F00           movel d0, -(a7)
031E: 1F3C 0001      moveb.ex #1, -(a7)
0322: 4E4F           trap #15
0324: A2D2           sysTrapPrefSetPreference
0326: 4E5E           unlk a6
0328: 4E75           rts
032A: 8F53           ormw d7, (a3)
032C: 746F           moveq #111, d2
032E: 7041           moveq #65, d0
0330: 7070           moveq #112, d0
0332: 6C69           bge 105
0334: 6361           bls 97
0336: 7469           moveq #105, d2
0338: 6F6E           ble 110
033A: 0000           dc.w #0
033C: 4E56 0000      link a6, #0
0340: 2F05           movel d5, -(a7)
0342: 206E 0008      movel.ex 8(a6), a0
0346: 1A2E 000C      moveb.ex 12(a6), d5
034A: 342E 000E      movew.ex 14(a6), d2
034E: 7200           moveq #0, d1
0350: 6002           bra 2
0352: 5241           addqw #1, d1
0354: 7000           moveq #0, d0
0356: 3001           movew d1, d0
0358: BA30 0800      cmpb.ex 0(a0,d0.l), d5
035C: 6704           beq 4
035E: B242           cmpw d2, d1
0360: 65F0           bcs -16
0362: B242           cmpw d2, d1
0364: 6506           bcs 6
0366: 302E 0010      movew.ex 16(a6), d0
036A: 6002           bra 2
036C: 3001           movew d1, d0
036E: 2A1F           movel (a7)+, d5
0370: 4E5E           unlk a6
0372: 4E75           rts
0374: 8D4D           dc.w #36173
0376: 6170           bsr 112
0378: 546F 506F      addqw.ex #2, 20591(a7)
037C: 7369           dc.w #29545
037E: 7469           moveq #105, d2
0380: 6F6E           ble 110
0382: 0000           dc.w #0
0384: 4E56 0000      link a6, #0
0388: 2F0A           movel a2, -(a7)
038A: 7000           moveq #0, d0
038C: 302E 0008      movew.ex 8(a6), d0
0390: 7234           moveq #52, d1
0392: 4EBA FD90      jsr.ex 29996(pc)
0396: 206D FF38      movel.ex -200(a5), a0
039A: D1C0           addal d0, a0
039C: 2448           movel a0, a2
039E: 206E 000A      movel.ex 10(a6), a0
03A2: 3F28 0002      movew.ex 2(a0), -(a7)
03A6: 206E 000A      movel.ex 10(a6), a0
03AA: 3F10           movew (a0), -(a7)
03AC: 2F0A           movel a2, -(a7)
03AE: 4E4F           trap #15
03B0: A0C7           sysTrapStrLen
03B2: 584F           addqw #4, a7
03B4: 3F00           movew d0, -(a7)
03B6: 2F0A           movel a2, -(a7)
03B8: 4E4F           trap #15
03BA: A220           sysTrapWinDrawChars
03BC: 4FEF 000A      lea 10(a7), d7
03C0: 245F           movel (a7)+, a2
03C2: 4E5E           unlk a6
03C4: 4E75           rts
03C6: 9550           submw d2, (a0)
03C8: 616E           bsr 110
03CA: 656C           bcs 108
03CC: 5069 636B      addqw.ex #0, 25451(a1)
03D0: 4C69           dc.w #19561
03D2: 7374           dc.w #29556
03D4: 4472 6177      negw.ex 119(a2,d6.w)
03D8: 4974           dc.w #18804
03DA: 656D           bcs 109
03DC: 0000           dc.w #0
03DE: 4E56 FFFA      link a6, #-6
03E2: 48E7 1030      movem <1030>, -(a7)
03E6: 266E 0008      movel.ex 8(a6), a3
03EA: 246E 000C      movel.ex 12(a6), a2
03EE: 486D FF7E      pea.ex -130(a5)
03F2: 486D FF82      pea.ex -126(a5)
03F6: 4E4F           trap #15
03F8: A2DC           sysTrapSysCreatePanelList
03FA: 4A6D FF82      tstw.ex -126(a5)
03FE: 504F           addqw #0, a7
0400: 670E           beq 14
0402: 2F2D FF7E      movel.ex -130(a5), -(a7)
0406: 4E4F           trap #15
0408: A021           sysTrapMemHandleLock
040A: 2B48 FF38      movel.mx a0, -200(a5)
040E: 584F           addqw #4, a7
0410: 3F2D FF82      movew.ex -126(a5), -(a7)
0414: 42A7           clrl -(a7)
0416: 2F0A           movel a2, -(a7)
0418: 4E4F           trap #15
041A: A1B8           sysTrapLstSetListChoices
041C: 3F2D FF82      movew.ex -126(a5), -(a7)
0420: 2F0A           movel a2, -(a7)
0422: 4E4F           trap #15
0424: A1B6           sysTrapLstSetHeight
0426: 2F2E 0010      movel.ex 16(a6), -(a7)
042A: 2F0A           movel a2, -(a7)
042C: 4E4F           trap #15
042E: A1B0           sysTrapLstSetDrawFunction
0430: 486E FFFA      pea.ex -6(a6)
0434: 486E FFFE      pea.ex -2(a6)
0438: 4E4F           trap #15
043A: A0AC           sysTrapSysCurAppDatabase
043C: 7600           moveq #0, d3
043E: 4FEF 0020      lea 32(a7), d7
0442: 6046           bra 70
0444: 7034           moveq #52, d0
0446: C1C3           muls d3, d0
0448: 206D FF38      movel.ex -200(a5), a0
044C: 222E FFFA      movel.ex -6(a6), d1
0450: B2B0 082A      cmpl.ex 42(a0,d0.l), d1
0454: 6632           bne 50
0456: 7034           moveq #52, d0
0458: C1C3           muls d3, d0
045A: 206D FF38      movel.ex -200(a5), a0
045E: 322E FFFE      movew.ex -2(a6), d1
0462: B270 082E      cmpw.ex 46(a0,d0.l), d1
0466: 6620           bne 32
0468: 3F03           movew d3, -(a7)
046A: 2F0A           movel a2, -(a7)
046C: 4E4F           trap #15
046E: A1B7           sysTrapLstSetSelection
0470: 7034           moveq #52, d0
0472: C1C3           muls d3, d0
0474: 206D FF38      movel.ex -200(a5), a0
0478: 4870 0800      pea.ex 0(a0,d0.l)
047C: 2F0B           movel a3, -(a7)
047E: 4E4F           trap #15
0480: A114           sysTrapCtlSetLabel
0482: 4FEF 000E      lea 14(a7), d7
0486: 6008           bra 8
0488: 5243           addqw #1, d3
048A: B66D FF82      cmpw.ex -126(a5), d3
048E: 65B4           bcs -76
0490: 4CDF 0C08      movem (a7)+, <0c08>
0494: 4E5E           unlk a6
0496: 4E75           rts
0498: 9343           subxrw d3, d1
049A: 7265           moveq #101, d1
049C: 6174           bsr 116
049E: 6550           bcs 80
04A0: 616E           bsr 110
04A2: 656C           bcs 108
04A4: 5069 636B      addqw.ex #0, 25451(a1)
04A8: 4C69           dc.w #19561
04AA: 7374           dc.w #29556
04AC: 0000           dc.w #0
04AE: 4E56 0000      link a6, #0
04B2: 2F03           movel d3, -(a7)
04B4: 362E 0008      movew.ex 8(a6), d3
04B8: 7000           moveq #0, d0
04BA: 3003           movew d3, d0
04BC: ED88           lslil #6, d0
04BE: 206D FF56      movel.ex -170(a5), a0
04C2: 1B70 0818 FF7B moveb.emx 24(a0,d0.l), -133(a5)
04C8: 7000           moveq #0, d0
04CA: 3003           movew d3, d0
04CC: ED88           lslil #6, d0
04CE: 206D FF56      movel.ex -170(a5), a0
04D2: 1B70 0815 FF7C moveb.emx 21(a0,d0.l), -132(a5)
04D8: 7000           moveq #0, d0
04DA: 3003           movew d3, d0
04DC: ED88           lslil #6, d0
04DE: 206D FF56      movel.ex -170(a5), a0
04E2: 1B70 0816 FF7D moveb.emx 22(a0,d0.l), -131(a5)
04E8: 7000           moveq #0, d0
04EA: 3003           movew d3, d0
04EC: ED88           lslil #6, d0
04EE: 206D FF56      movel.ex -170(a5), a0
04F2: 1B70 0817 FF79 moveb.emx 23(a0,d0.l), -135(a5)
04F8: 7000           moveq #0, d0
04FA: 3003           movew d3, d0
04FC: ED88           lslil #6, d0
04FE: 206D FF56      movel.ex -170(a5), a0
0502: 1B70 0819 FF7A moveb.emx 25(a0,d0.l), -134(a5)
0508: 7000           moveq #0, d0
050A: 102D FF7B      moveb.ex -133(a5), d0
050E: 2F00           movel d0, -(a7)
0510: 1F3C 0005      moveb.ex #5, -(a7)
0514: 4E4F           trap #15
0516: A2D2           sysTrapPrefSetPreference
0518: 5C4F           addqw #6, a7
051A: 261F           movel (a7)+, d3
051C: 4E5E           unlk a6
051E: 4E75           rts
0520: 9753           submw d3, (a3)
0522: 6574           bcs 116
0524: 5072 6566      addqw.ex #0, 102(a2,d6.w)
0528: 6572           bcs 114
052A: 656E           bcs 110
052C: 6365           bls 101
052E: 7342           dc.w #29506
0530: 7943           dc.w #31043
0532: 6F75           ble 117
0534: 6E74           bgt 116
0536: 7279           moveq #121, d1
0538: 0000           dc.w #0
053A: 4E56 FFF8      link a6, #-8
053E: 48E7 1E30      movem <1e30>, -(a7)
0542: 266E 0008      movel.ex 8(a6), a3
0546: 246E 000C      movel.ex 12(a6), a2
054A: 3C2E 0010      movew.ex 16(a6), d6
054E: 3A2E 0012      movew.ex 18(a6), d5
0552: 2F0B           movel a3, -(a7)
0554: 2F0A           movel a2, -(a7)
0556: 4E4F           trap #15
0558: A0C8           sysTrapStrCompare
055A: 4A40           tstw d0
055C: 504F           addqw #0, a7
055E: 6772           beq 114
0560: 3F05           movew d5, -(a7)
0562: 3F06           movew d6, -(a7)
0564: 2F0A           movel a2, -(a7)
0566: 4E4F           trap #15
0568: A0C7           sysTrapStrLen
056A: 584F           addqw #4, a7
056C: 3F00           movew d0, -(a7)
056E: 2F0A           movel a2, -(a7)
0570: 4E4F           trap #15
0572: A220           sysTrapWinDrawChars
0574: 2F0B           movel a3, -(a7)
0576: 4E4F           trap #15
0578: A0C7           sysTrapStrLen
057A: 584F           addqw #4, a7
057C: 3F00           movew d0, -(a7)
057E: 2F0B           movel a3, -(a7)
0580: 4E4F           trap #15
0582: A16B           sysTrapFntCharsWidth
0584: 3800           movew d0, d4
0586: 2F0A           movel a2, -(a7)
0588: 4E4F           trap #15
058A: A0C7           sysTrapStrLen
058C: 584F           addqw #4, a7
058E: 3F00           movew d0, -(a7)
0590: 2F0A           movel a2, -(a7)
0592: 4E4F           trap #15
0594: A16B           sysTrapFntCharsWidth
0596: 3600           movew d0, d3
0598: B843           cmpw d3, d4
059A: 4FEF 0016      lea 22(a7), d7
059E: 6F28           ble 40
05A0: 3006           movew d6, d0
05A2: D043           addrw d3, d0
05A4: 3D40 FFF8      movew.mx d0, -8(a6)
05A8: 3D45 FFFA      movew.mx d5, -6(a6)
05AC: 3004           movew d4, d0
05AE: 9043           subrw d3, d0
05B0: 3D40 FFFC      movew.mx d0, -4(a6)
05B4: 4E4F           trap #15
05B6: A168           sysTrapFntLineHeight
05B8: 3D40 FFFE      movew.mx d0, -2(a6)
05BC: 4267           clrw -(a7)
05BE: 486E FFF8      pea.ex -8(a6)
05C2: 4E4F           trap #15
05C4: A219           sysTrapWinEraseRectangle
05C6: 5C4F           addqw #6, a7
05C8: 2F0A           movel a2, -(a7)
05CA: 2F0B           movel a3, -(a7)
05CC: 4E4F           trap #15
05CE: A0C5           sysTrapStrCopy
05D0: 504F           addqw #0, a7
05D2: 4CDF 0C78      movem (a7)+, <0c78>
05D6: 4E5E           unlk a6
05D8: 4E75           rts
05DA: 9752           submw d3, (a2)
05DC: 6564           bcs 100
05DE: 7261           moveq #97, d1
05E0: 7753           dc.w #30547
05E2: 7472           moveq #114, d2
05E4: 696E           bvs 110
05E6: 6749           beq 73
05E8: 6644           bne 68
05EA: 6966           bvs 102
05EC: 6665           bne 101
05EE: 7265           moveq #101, d1
05F0: 6E74           bgt 116
05F2: 0000           dc.w #0
05F4: 4E56 FFE8      link a6, #-24
05F8: 486E FFF2      pea.ex -14(a6)
05FC: 4E4F           trap #15
05FE: A0F5           sysTrapTimGetSeconds
0600: 2F00           movel d0, -(a7)
0602: 4E4F           trap #15
0604: A0FC           sysTrapTimSecondsToDateTime
0606: 486E FFE8      pea.ex -24(a6)
060A: 1F2D FF7B      moveb.ex -133(a5), -(a7)
060E: 1F2E FFF5      moveb.ex -11(a6), -(a7)
0612: 1F2E FFF7      moveb.ex -9(a6), -(a7)
0616: 4E4F           trap #15
0618: A268           sysTrapTimeToAscii
061A: 2F3C 0059 0034 movel.exl #5832756, -(a7)
0620: 486E FFE8      pea.ex -24(a6)
0624: 486D FF6E      pea.ex -146(a5)
0628: 4EBA FF10      jsr.ex 31042(pc)
062C: 4E5E           unlk a6
062E: 4E75           rts
0630: 9546           subxrw d6, d2
0632: 6F72           ble 114
0634: 6D61           blt 97
0636: 7473           moveq #115, d2
0638: 466F 726D      notw.ex 29293(a7)
063C: 5570 6461      subqw.ex #2, 97(a0,d6.w)
0640: 7465           moveq #101, d2
0642: 5469 6D65      addqw.ex #2, 28005(a1)
0646: 0000           dc.w #0
0648: 4E56 FFE2      link a6, #-30
064C: 486E FFF2      pea.ex -14(a6)
0650: 4E4F           trap #15
0652: A0F5           sysTrapTimGetSeconds
0654: 2F00           movel d0, -(a7)
0656: 4E4F           trap #15
0658: A0FC           sysTrapTimSecondsToDateTime
065A: 486E FFE2      pea.ex -30(a6)
065E: 1F2D FF7C      moveb.ex -132(a5), -(a7)
0662: 3F2E FFFC      movew.ex -4(a6), -(a7)
0666: 1F2E FFF9      moveb.ex -7(a6), -(a7)
066A: 1F2E FFFB      moveb.ex -5(a6), -(a7)
066E: 4E4F           trap #15
0670: A266           sysTrapDateToAscii
0672: 2F3C 0059 004C movel.exl #5832780, -(a7)
0678: 486E FFE2      pea.ex -30(a6)
067C: 486D FF5E      pea.ex -162(a5)
0680: 4EBA FEB8      jsr.ex 31042(pc)
0684: 486E FFE2      pea.ex -30(a6)
0688: 1F2D FF7D      moveb.ex -131(a5), -(a7)
068C: 3F2E FFFC      movew.ex -4(a6), -(a7)
0690: 1F2E FFF9      moveb.ex -7(a6), -(a7)
0694: 1F2E FFFB      moveb.ex -5(a6), -(a7)
0698: 4E4F           trap #15
069A: A266           sysTrapDateToAscii
069C: 4E4F           trap #15
069E: A168           sysTrapFntLineHeight
06A0: 0640 004D      addiw #77, d0
06A4: 3F00           movew d0, -(a7)
06A6: 3F3C 0059      movew.ex #89, -(a7)
06AA: 486E FFE2      pea.ex -30(a6)
06AE: 486D FF5E      pea.ex -162(a5)
06B2: 4EBA FE86      jsr.ex 31042(pc)
06B6: 4E5E           unlk a6
06B8: 4E75           rts
06BA: 9546           subxrw d6, d2
06BC: 6F72           ble 114
06BE: 6D61           blt 97
06C0: 7473           moveq #115, d2
06C2: 466F 726D      notw.ex 29293(a7)
06C6: 5570 6461      subqw.ex #2, 97(a0,d6.w)
06CA: 7465           moveq #101, d2
06CC: 4461           negw -(a1)
06CE: 7465           moveq #101, d2
06D0: 0000           dc.w #0
06D2: 4E56 0000      link a6, #0
06D6: 48E7 0038      movem <0038>, -(a7)
06DA: 286E 000A      movel.ex 10(a6), a4
06DE: 266E 000E      movel.ex 14(a6), a3
06E2: 7000           moveq #0, d0
06E4: 302E 0008      movew.ex 8(a6), d0
06E8: 45ED FF3C      lea -196(a5), d2
06EC: 7200           moveq #0, d1
06EE: 1232 0800      moveb.ex 0(a2,d0.l), d1
06F2: ED89           lslil #6, d1
06F4: 244B           movel a3, a2
06F6: D5C1           addal d1, a2
06F8: 528A           addql #1, a2
06FA: 3F2C 0002      movew.ex 2(a4), -(a7)
06FE: 3F14           movew (a4), -(a7)
0700: 2F0A           movel a2, -(a7)
0702: 4E4F           trap #15
0704: A0C7           sysTrapStrLen
0706: 584F           addqw #4, a7
0708: 3F00           movew d0, -(a7)
070A: 2F0A           movel a2, -(a7)
070C: 4E4F           trap #15
070E: A220           sysTrapWinDrawChars
0710: 4FEF 000A      lea 10(a7), d7
0714: 4CDF 1C00      movem (a7)+, <1c00>
0718: 4E5E           unlk a6
071A: 4E75           rts
071C: 9343           subxrw d3, d1
071E: 6F75           ble 117
0720: 6E74           bgt 116
0722: 7279           moveq #121, d1
0724: 4C69           dc.w #19561
0726: 7374           dc.w #29556
0728: 4472 6177      negw.ex 119(a2,d6.w)
072C: 4974           dc.w #18804
072E: 656D           bcs 109
0730: 0000           dc.w #0
0732: 4E56 FFF8      link a6, #-8
0736: 48E7 1030      movem <1030>, -(a7)
073A: 246E 0008      movel.ex 8(a6), a2
073E: 4A2D FF85      tstb.ex -123(a5)
0742: 6752           beq 82
0744: 3F3C 04C2      movew.ex #1218, -(a7)
0748: 2F0A           movel a2, -(a7)
074A: 4E4F           trap #15
074C: A180           sysTrapFrmGetObjectIndex
074E: 5C4F           addqw #6, a7
0750: 3F00           movew d0, -(a7)
0752: 2F0A           movel a2, -(a7)
0754: 4E4F           trap #15
0756: A184           sysTrapFrmHideObject
0758: 3F3C 04D2      movew.ex #1234, -(a7)
075C: 2F0A           movel a2, -(a7)
075E: 4E4F           trap #15
0760: A180           sysTrapFrmGetObjectIndex
0762: 5C4F           addqw #6, a7
0764: 3F00           movew d0, -(a7)
0766: 2F0A           movel a2, -(a7)
0768: 4E4F           trap #15
076A: A185           sysTrapFrmShowObject
076C: 3F3C 04D1      movew.ex #1233, -(a7)
0770: 2F0A           movel a2, -(a7)
0772: 4E4F           trap #15
0774: A180           sysTrapFrmGetObjectIndex
0776: 3600           movew d0, d3
0778: 3F03           movew d3, -(a7)
077A: 2F0A           movel a2, -(a7)
077C: 4E4F           trap #15
077E: A185           sysTrapFrmShowObject
0780: 486E FFF8      pea.ex -8(a6)
0784: 3F03           movew d3, -(a7)
0786: 2F0A           movel a2, -(a7)
0788: 4E4F           trap #15
078A: A199           sysTrapFrmGetObjectBounds
078C: 362E FFFC      movew.ex -4(a6), d3
0790: 5C43           addqw #6, d3
0792: 4FEF 0022      lea 34(a7), d7
0796: 3F3C 04CB      movew.ex #1227, -(a7)
079A: 2F0A           movel a2, -(a7)
079C: 4E4F           trap #15
079E: A180           sysTrapFrmGetObjectIndex
07A0: 5C4F           addqw #6, a7
07A2: 3F00           movew d0, -(a7)
07A4: 2F0A           movel a2, -(a7)
07A6: 4E4F           trap #15
07A8: A183           sysTrapFrmGetObjectPtr
07AA: 2648           movel a0, a3
07AC: 7000           moveq #0, d0
07AE: 102D FF55      moveb.ex -171(a5), d0
07B2: 3F00           movew d0, -(a7)
07B4: 2F2D FF56      movel.ex -170(a5), -(a7)
07B8: 2F0B           movel a3, -(a7)
07BA: 4E4F           trap #15
07BC: A1B8           sysTrapLstSetListChoices
07BE: 7000           moveq #0, d0
07C0: 102D FF55      moveb.ex -171(a5), d0
07C4: 3F00           movew d0, -(a7)
07C6: 2F0B           movel a3, -(a7)
07C8: 4E4F           trap #15
07CA: A1B6           sysTrapLstSetHeight
07CC: 487A FF04      pea.ex 31450(pc)
07D0: 2F0B           movel a3, -(a7)
07D2: 4E4F           trap #15
07D4: A1B0           sysTrapLstSetDrawFunction
07D6: 4267           clrw -(a7)
07D8: 7000           moveq #0, d0
07DA: 102D FF55      moveb.ex -171(a5), d0
07DE: 3F00           movew d0, -(a7)
07E0: 1F2D FF78      moveb.ex -136(a5), -(a7)
07E4: 486D FF3C      pea.ex -196(a5)
07E8: 4EBA FB52      jsr.ex 30532(pc)
07EC: 3600           movew d0, d3
07EE: 3F03           movew d3, -(a7)
07F0: 2F0B           movel a3, -(a7)
07F2: 4E4F           trap #15
07F4: A1B7           sysTrapLstSetSelection
07F6: 3F03           movew d3, -(a7)
07F8: 2F0B           movel a3, -(a7)
07FA: 4E4F           trap #15
07FC: A1B9           sysTrapLstMakeItemVisible
07FE: 7000           moveq #0, d0
0800: 102D FF78      moveb.ex -136(a5), d0
0804: ED88           lslil #6, d0
0806: 206D FF56      movel.ex -170(a5), a0
080A: 4870 0801      pea.ex 1(a0,d0.l)
080E: 3F3C 04CA      movew.ex #1226, -(a7)
0812: 2F0A           movel a2, -(a7)
0814: 4E4F           trap #15
0816: A180           sysTrapFrmGetObjectIndex
0818: 5C4F           addqw #6, a7
081A: 3F00           movew d0, -(a7)
081C: 2F0A           movel a2, -(a7)
081E: 4E4F           trap #15
0820: A183           sysTrapFrmGetObjectPtr
0822: 5C4F           addqw #6, a7
0824: 2F08           movel a0, -(a7)
0826: 4E4F           trap #15
0828: A114           sysTrapCtlSetLabel
082A: 3F3C 04C7      movew.ex #1223, -(a7)
082E: 2F0A           movel a2, -(a7)
0830: 4E4F           trap #15
0832: A180           sysTrapFrmGetObjectIndex
0834: 5C4F           addqw #6, a7
0836: 3F00           movew d0, -(a7)
0838: 2F0A           movel a2, -(a7)
083A: 4E4F           trap #15
083C: A183           sysTrapFrmGetObjectPtr
083E: 2648           movel a0, a3
0840: 2F3C 0005 0000 movel.exl #327680, -(a7)
0846: 1F2D FF7B      moveb.ex -133(a5), -(a7)
084A: 486D FF86      pea.ex -122(a5)
084E: 4EBA FAEC      jsr.ex 30532(pc)
0852: 3600           movew d0, d3
0854: 3F03           movew d3, -(a7)
0856: 2F0B           movel a3, -(a7)
0858: 4E4F           trap #15
085A: A1B7           sysTrapLstSetSelection
085C: 3F03           movew d3, -(a7)
085E: 2F0B           movel a3, -(a7)
0860: 4E4F           trap #15
0862: A1B4           sysTrapLstGetSelectionText
0864: 5C4F           addqw #6, a7
0866: 2F08           movel a0, -(a7)
0868: 3F3C 04C6      movew.ex #1222, -(a7)
086C: 2F0A           movel a2, -(a7)
086E: 4E4F           trap #15
0870: A180           sysTrapFrmGetObjectIndex
0872: 5C4F           addqw #6, a7
0874: 3F00           movew d0, -(a7)
0876: 2F0A           movel a2, -(a7)
0878: 4E4F           trap #15
087A: A183           sysTrapFrmGetObjectPtr
087C: 5C4F           addqw #6, a7
087E: 2F08           movel a0, -(a7)
0880: 4E4F           trap #15
0882: A114           sysTrapCtlSetLabel
0884: 3F3C 04C0      movew.ex #1216, -(a7)
0888: 2F0A           movel a2, -(a7)
088A: 4E4F           trap #15
088C: A180           sysTrapFrmGetObjectIndex
088E: 5C4F           addqw #6, a7
0890: 3F00           movew d0, -(a7)
0892: 2F0A           movel a2, -(a7)
0894: 4E4F           trap #15
0896: A183           sysTrapFrmGetObjectPtr
0898: 2648           movel a0, a3
089A: 2F3C 0007 0000 movel.exl #458752, -(a7)
08A0: 1F2D FF7C      moveb.ex -132(a5), -(a7)
08A4: 486D FF8C      pea.ex -116(a5)
08A8: 4EBA FA92      jsr.ex 30532(pc)
08AC: 3600           movew d0, d3
08AE: 3F03           movew d3, -(a7)
08B0: 2F0B           movel a3, -(a7)
08B2: 4E4F           trap #15
08B4: A1B7           sysTrapLstSetSelection
08B6: 3F03           movew d3, -(a7)
08B8: 2F0B           movel a3, -(a7)
08BA: 4E4F           trap #15
08BC: A1B4           sysTrapLstGetSelectionText
08BE: 5C4F           addqw #6, a7
08C0: 2F08           movel a0, -(a7)
08C2: 3F3C 04BF      movew.ex #1215, -(a7)
08C6: 2F0A           movel a2, -(a7)
08C8: 4E4F           trap #15
08CA: A180           sysTrapFrmGetObjectIndex
08CC: 5C4F           addqw #6, a7
08CE: 3F00           movew d0, -(a7)
08D0: 2F0A           movel a2, -(a7)
08D2: 4E4F           trap #15
08D4: A183           sysTrapFrmGetObjectPtr
08D6: 5C4F           addqw #6, a7
08D8: 2F08           movel a0, -(a7)
08DA: 4E4F           trap #15
08DC: A114           sysTrapCtlSetLabel
08DE: 3F3C 04B6      movew.ex #1206, -(a7)
08E2: 2F0A           movel a2, -(a7)
08E4: 4E4F           trap #15
08E6: A180           sysTrapFrmGetObjectIndex
08E8: 5C4F           addqw #6, a7
08EA: 3F00           movew d0, -(a7)
08EC: 2F0A           movel a2, -(a7)
08EE: 4E4F           trap #15
08F0: A183           sysTrapFrmGetObjectPtr
08F2: 2648           movel a0, a3
08F4: 7000           moveq #0, d0
08F6: 102D FF79      moveb.ex -135(a5), d0
08FA: 3F00           movew d0, -(a7)
08FC: 2F0B           movel a3, -(a7)
08FE: 4E4F           trap #15
0900: A1B7           sysTrapLstSetSelection
0902: 7000           moveq #0, d0
0904: 102D FF79      moveb.ex -135(a5), d0
0908: 3F00           movew d0, -(a7)
090A: 2F0B           movel a3, -(a7)
090C: 4E4F           trap #15
090E: A1B4           sysTrapLstGetSelectionText
0910: 5C4F           addqw #6, a7
0912: 2F08           movel a0, -(a7)
0914: 3F3C 04B4      movew.ex #1204, -(a7)
0918: 2F0A           movel a2, -(a7)
091A: 4E4F           trap #15
091C: A180           sysTrapFrmGetObjectIndex
091E: 5C4F           addqw #6, a7
0920: 3F00           movew d0, -(a7)
0922: 2F0A           movel a2, -(a7)
0924: 4E4F           trap #15
0926: A183           sysTrapFrmGetObjectPtr
0928: 5C4F           addqw #6, a7
092A: 2F08           movel a0, -(a7)
092C: 4E4F           trap #15
092E: A114           sysTrapCtlSetLabel
0930: 3F3C 04CF      movew.ex #1231, -(a7)
0934: 2F0A           movel a2, -(a7)
0936: 4E4F           trap #15
0938: A180           sysTrapFrmGetObjectIndex
093A: 5C4F           addqw #6, a7
093C: 3F00           movew d0, -(a7)
093E: 2F0A           movel a2, -(a7)
0940: 4E4F           trap #15
0942: A183           sysTrapFrmGetObjectPtr
0944: 2648           movel a0, a3
0946: 7000           moveq #0, d0
0948: 102D FF7A      moveb.ex -134(a5), d0
094C: 3F00           movew d0, -(a7)
094E: 2F0B           movel a3, -(a7)
0950: 4E4F           trap #15
0952: A1B7           sysTrapLstSetSelection
0954: 7000           moveq #0, d0
0956: 102D FF7A      moveb.ex -134(a5), d0
095A: 3F00           movew d0, -(a7)
095C: 2F0B           movel a3, -(a7)
095E: 4E4F           trap #15
0960: A1B4           sysTrapLstGetSelectionText
0962: 5C4F           addqw #6, a7
0964: 2F08           movel a0, -(a7)
0966: 3F3C 04CE      movew.ex #1230, -(a7)
096A: 2F0A           movel a2, -(a7)
096C: 4E4F           trap #15
096E: A180           sysTrapFrmGetObjectIndex
0970: 5C4F           addqw #6, a7
0972: 3F00           movew d0, -(a7)
0974: 2F0A           movel a2, -(a7)
0976: 4E4F           trap #15
0978: A183           sysTrapFrmGetObjectPtr
097A: 5C4F           addqw #6, a7
097C: 2F08           movel a0, -(a7)
097E: 4E4F           trap #15
0980: A114           sysTrapCtlSetLabel
0982: 4FEF 00A0      lea 160(a7), d7
0986: 4CDF 0C08      movem (a7)+, <0c08>
098A: 4E5E           unlk a6
098C: 4E75           rts
098E: 8F46           dc.w #36678
0990: 6F72           ble 114
0992: 6D61           blt 97
0994: 7473           moveq #115, d2
0996: 466F 726D      notw.ex 29293(a7)
099A: 496E           dc.w #18798
099C: 6974           bvs 116
099E: 0000           dc.w #0
09A0: 4E56 FFFE      link a6, #-2
09A4: 42A7           clrl -(a7)
09A6: 486E FFFE      pea.ex -2(a6)
09AA: 4E4F           trap #15
09AC: A20C           sysTrapWinGetWindowExtent
09AE: 3F3C 0022      movew.ex #34, -(a7)
09B2: 3F2E FFFE      movew.ex -2(a6), -(a7)
09B6: 4878 0022      pea.ex (0022).w
09BA: 4E4F           trap #15
09BC: A213           sysTrapWinDrawLine
09BE: 4EBA FC34      jsr.ex 31228(pc)
09C2: 4EBA FC84      jsr.ex 31312(pc)
09C6: 4E5E           unlk a6
09C8: 4E75           rts
09CA: 9346           subxrw d6, d1
09CC: 6F72           ble 114
09CE: 6D61           blt 97
09D0: 7473           moveq #115, d2
09D2: 466F 726D      notw.ex 29293(a7)
09D6: 4472 6177      negw.ex 119(a2,d6.w)
09DA: 466F 726D      notw.ex 29293(a7)
09DE: 0000           dc.w #0
09E0: 4E56 0000      link a6, #0
09E4: 422D FF6E      clrb.ex -146(a5)
09E8: 422D FF5E      clrb.ex -162(a5)
09EC: 4EBA FFB2      jsr.ex 32168(pc)
09F0: 4E5E           unlk a6
09F2: 4E75           rts
09F4: 9846           subrw d6, d4
09F6: 6F72           ble 114
09F8: 6D61           blt 97
09FA: 7473           moveq #115, d2
09FC: 466F 726D      notw.ex 29293(a7)
0A00: 466F 7263      notw.ex 29283(a7)
0A04: 6544           bcs 68
0A06: 7261           moveq #97, d1
0A08: 7746           dc.w #30534
0A0A: 6F72           ble 114
0A0C: 6D00 0000      blt 0
0A10: 4E56 FFE2      link a6, #-30
0A14: 48E7 1020      movem <1020>, -(a7)
0A18: 246E 0008      movel.ex 8(a6), a2
0A1C: 7600           moveq #0, d3
0A1E: 4A52           tstw (a2)
0A20: 660E           bne 14
0A22: 4EBA FBD0      jsr.ex 31228(pc)
0A26: 4EBA FC20      jsr.ex 31312(pc)
0A2A: 7001           moveq #1, d0
0A2C: 6000 0214      bra 532
0A30: 0C52 0009      cmpiw #9, (a2)
0A34: 661E           bne 30
0A36: 302A 0008      movew.ex 8(a2), d0
0A3A: 0440 04D1      subiw #1233, d0
0A3E: 6702           beq 2
0A40: 6012           bra 18
0A42: 3D7C 0016 FFE2 movew.emx #22, -30(a6)
0A48: 486E FFE2      pea.ex -30(a6)
0A4C: 4E4F           trap #15
0A4E: A11B           sysTrapEvtAddEventToQueue
0A50: 7601           moveq #1, d3
0A52: 584F           addqw #4, a7
0A54: 0C52 000E      cmpiw #14, (a2)
0A58: 6600 0184      bne 388
0A5C: 302A 000E      movew.ex 14(a2), d0
0A60: 4EBA F6F4      jsr.ex 30046(pc)
0A64: 01DC           bset d0, (a4)+
0A66: 04B6 04CF 0005 04B6 subil.ex #80674821, 182(a6,d0.w)
0A6E: 0160           bchg d0, -(a0)
0A70: 04C0           dc.w #1216
0A72: 0120           btst d0, -(a0)
0A74: 04C3           dc.w #1219
0A76: 000E           dc.w #14
0A78: 04C7           dc.w #1223
0A7A: 00D8           dc.w #216
0A7C: 04CB           dc.w #1227
0A7E: 0082 04CF 0154 oril #80675156, d2
0A84: 486E FFFA      pea.ex -6(a6)
0A88: 486E FFFE      pea.ex -2(a6)
0A8C: 4E4F           trap #15
0A8E: A0AC           sysTrapSysCurAppDatabase
0A90: 7000           moveq #0, d0
0A92: 302A 0014      movew.ex 20(a2), d0
0A96: 7234           moveq #52, d1
0A98: 4EBA F68A      jsr.ex 29996(pc)
0A9C: 206D FF38      movel.ex -200(a5), a0
0AA0: 222E FFFA      movel.ex -6(a6), d1
0AA4: B2B0 082A      cmpl.ex 42(a0,d0.l), d1
0AA8: 504F           addqw #0, a7
0AAA: 661A           bne 26
0AAC: 7000           moveq #0, d0
0AAE: 302A 0014      movew.ex 20(a2), d0
0AB2: 7234           moveq #52, d1
0AB4: 4EBA F66E      jsr.ex 29996(pc)
0AB8: 206D FF38      movel.ex -200(a5), a0
0ABC: 322E FFFE      movew.ex -2(a6), d1
0AC0: B270 082E      cmpw.ex 46(a0,d0.l), d1
0AC4: 6734           beq 52
0AC6: 42A7           clrl -(a7)
0AC8: 4267           clrw -(a7)
0ACA: 7000           moveq #0, d0
0ACC: 302A 0014      movew.ex 20(a2), d0
0AD0: 7234           moveq #52, d1
0AD2: 4EBA F650      jsr.ex 29996(pc)
0AD6: 206D FF38      movel.ex -200(a5), a0
0ADA: 2F30 082A      movel.ex 42(a0,d0.l), -(a7)
0ADE: 7000           moveq #0, d0
0AE0: 302A 0014      movew.ex 20(a2), d0
0AE4: 7234           moveq #52, d1
0AE6: 4EBA F63C      jsr.ex 29996(pc)
0AEA: 206D FF38      movel.ex -200(a5), a0
0AEE: 3F30 082E      movew.ex 46(a0,d0.l), -(a7)
0AF2: 4E4F           trap #15
0AF4: A0A7           sysTrapSysUIAppSwitch
0AF6: 4FEF 000C      lea 12(a7), d7
0AFA: 7601           moveq #1, d3
0AFC: 6000 0142      bra 322
0B00: 7000           moveq #0, d0
0B02: 302A 0014      movew.ex 20(a2), d0
0B06: 41ED FF3C      lea -196(a5), d0
0B0A: 122D FF78      moveb.ex -136(a5), d1
0B0E: B230 0800      cmpb.ex 0(a0,d0.l), d1
0B12: 6738           beq 56
0B14: 7000           moveq #0, d0
0B16: 302A 0014      movew.ex 20(a2), d0
0B1A: 41ED FF3C      lea -196(a5), d0
0B1E: 1B70 0800 FF78 moveb.emx 0(a0,d0.l), -136(a5)
0B24: 7000           moveq #0, d0
0B26: 302A 0014      movew.ex 20(a2), d0
0B2A: 41ED FF3C      lea -196(a5), d0
0B2E: 7200           moveq #0, d1
0B30: 1230 0800      moveb.ex 0(a0,d0.l), d1
0B34: 3F01           movew d1, -(a7)
0B36: 4EBA F976      jsr.ex 30902(pc)
0B3A: 4E4F           trap #15
0B3C: A173           sysTrapFrmGetActiveForm
0B3E: 2448           movel a0, a2
0B40: 2F0A           movel a2, -(a7)
0B42: 4EBA FBEE      jsr.ex 31546(pc)
0B46: 4EBA FE98      jsr.ex 32232(pc)
0B4A: 5C4F           addqw #6, a7
0B4C: 7601           moveq #1, d3
0B4E: 6000 00F0      bra 240
0B52: 7000           moveq #0, d0
0B54: 302A 0014      movew.ex 20(a2), d0
0B58: 41ED FF86      lea -122(a5), d0
0B5C: 122D FF7B      moveb.ex -133(a5), d1
0B60: B230 0800      cmpb.ex 0(a0,d0.l), d1
0B64: 6700 00DA      beq 218
0B68: 7000           moveq #0, d0
0B6A: 302A 0014      movew.ex 20(a2), d0
0B6E: 41ED FF86      lea -122(a5), d0
0B72: 1B70 0800 FF7B moveb.emx 0(a0,d0.l), -133(a5)
0B78: 4EBA FA7A      jsr.ex 31228(pc)
0B7C: 7000           moveq #0, d0
0B7E: 102D FF7B      moveb.ex -133(a5), d0
0B82: 2F00           movel d0, -(a7)
0B84: 1F3C 0005      moveb.ex #5, -(a7)
0B88: 4E4F           trap #15
0B8A: A2D2           sysTrapPrefSetPreference
0B8C: 5C4F           addqw #6, a7
0B8E: 6000 00B0      bra 176
0B92: 7000           moveq #0, d0
0B94: 302A 0014      movew.ex 20(a2), d0
0B98: 41ED FF8C      lea -116(a5), d0
0B9C: 122D FF7C      moveb.ex -132(a5), d1
0BA0: B230 0800      cmpb.ex 0(a0,d0.l), d1
0BA4: 6700 009A      beq 154
0BA8: 7000           moveq #0, d0
0BAA: 302A 0014      movew.ex 20(a2), d0
0BAE: 41ED FF8C      lea -116(a5), d0
0BB2: 1B70 0800 FF7C moveb.emx 0(a0,d0.l), -132(a5)
0BB8: 7000           moveq #0, d0
0BBA: 302A 0014      movew.ex 20(a2), d0
0BBE: 41ED FF94      lea -108(a5), d0
0BC2: 1B70 0800 FF7D moveb.emx 0(a0,d0.l), -131(a5)
0BC8: 4EBA FA7E      jsr.ex 31312(pc)
0BCC: 6072           bra 114
0BCE: 1B6A 0015 FF79 moveb.emx 21(a2), -135(a5)
0BD4: 606A           bra 106
0BD6: 1B6A 0015 FF7A moveb.emx 21(a2), -134(a5)
0BDC: 6062           bra 98
0BDE: 0C52 0018      cmpiw #24, (a2)
0BE2: 665C           bne 92
0BE4: 4E4F           trap #15
0BE6: A173           sysTrapFrmGetActiveForm
0BE8: 2448           movel a0, a2
0BEA: 2F0A           movel a2, -(a7)
0BEC: 4EBA FB44      jsr.ex 31546(pc)
0BF0: 2F0A           movel a2, -(a7)
0BF2: 4E4F           trap #15
0BF4: A171           sysTrapFrmDrawForm
0BF6: 4EBA FDE8      jsr.ex 32232(pc)
0BFA: 4A2D FF85      tstb.ex -123(a5)
0BFE: 504F           addqw #0, a7
0C00: 663C           bne 60
0C02: 487A F780      pea.ex 30604(pc)
0C06: 3F3C 04C3      movew.ex #1219, -(a7)
0C0A: 2F0A           movel a2, -(a7)
0C0C: 4E4F           trap #15
0C0E: A180           sysTrapFrmGetObjectIndex
0C10: 5C4F           addqw #6, a7
0C12: 3F00           movew d0, -(a7)
0C14: 2F0A           movel a2, -(a7)
0C16: 4E4F           trap #15
0C18: A183           sysTrapFrmGetObjectPtr
0C1A: 5C4F           addqw #6, a7
0C1C: 2F08           movel a0, -(a7)
0C1E: 3F3C 04C2      movew.ex #1218, -(a7)
0C22: 2F0A           movel a2, -(a7)
0C24: 4E4F           trap #15
0C26: A180           sysTrapFrmGetObjectIndex
0C28: 5C4F           addqw #6, a7
0C2A: 3F00           movew d0, -(a7)
0C2C: 2F0A           movel a2, -(a7)
0C2E: 4E4F           trap #15
0C30: A183           sysTrapFrmGetObjectPtr
0C32: 5C4F           addqw #6, a7
0C34: 2F08           movel a0, -(a7)
0C36: 4EBA F7A6      jsr.ex 30694(pc)
0C3A: 4FEF 000C      lea 12(a7), d7
0C3E: 7601           moveq #1, d3
0C40: 1003           moveb d3, d0
0C42: 4CDF 0408      movem (a7)+, <0408>
0C46: 4E5E           unlk a6
0C48: 4E75           rts
0C4A: 9646           subrw d6, d3
0C4C: 6F72           ble 114
0C4E: 6D61           blt 97
0C50: 7473           moveq #115, d2
0C52: 466F 726D      notw.ex 29293(a7)
0C56: 4861           dc.w #18529
0C58: 6E64           bgt 100
0C5A: 6C65           bge 101
0C5C: 4576           dc.w #17782
0C5E: 656E           bcs 110
0C60: 7400           moveq #0, d2
0C62: 0000           dc.w #0
0C64: 4E56 0000      link a6, #0
0C68: 48E7 1020      movem <1020>, -(a7)
0C6C: 246E 0008      movel.ex 8(a6), a2
0C70: 0C52 0017      cmpiw #23, (a2)
0C74: 662E           bne 46
0C76: 362A 0008      movew.ex 8(a2), d3
0C7A: 3F03           movew d3, -(a7)
0C7C: 4E4F           trap #15
0C7E: A16F           sysTrapFrmInitForm
0C80: 2448           movel a0, a2
0C82: 2F0A           movel a2, -(a7)
0C84: 4E4F           trap #15
0C86: A174           sysTrapFrmSetActiveForm
0C88: 5C4F           addqw #6, a7
0C8A: 3003           movew d3, d0
0C8C: 0440 04B0      subiw #1200, d0
0C90: 6702           beq 2
0C92: 600C           bra 12
0C94: 487A FD7A      pea.ex 32280(pc)
0C98: 2F0A           movel a2, -(a7)
0C9A: 4E4F           trap #15
0C9C: A19F           sysTrapFrmSetEventHandler
0C9E: 504F           addqw #0, a7
0CA0: 7001           moveq #1, d0
0CA2: 6002           bra 2
0CA4: 7000           moveq #0, d0
0CA6: 4CDF 0408      movem (a7)+, <0408>
0CAA: 4E5E           unlk a6
0CAC: 4E75           rts
0CAE: 9641           subrw d1, d3
0CB0: 7070           moveq #112, d0
0CB2: 6C69           bge 105
0CB4: 6361           bls 97
0CB6: 7469           moveq #105, d2
0CB8: 6F6E           ble 110
0CBA: 4861           dc.w #18529
0CBC: 6E64           bgt 100
0CBE: 6C65           bge 101
0CC0: 4576           dc.w #17782
0CC2: 656E           bcs 110
0CC4: 7400           moveq #0, d2
0CC6: 0000           dc.w #0
0CC8: 4E56 FFE8      link a6, #-24
0CCC: 4878 0064      pea.ex (0064).w
0CD0: 486E FFE8      pea.ex -24(a6)
0CD4: 4E4F           trap #15
0CD6: A11D           sysTrapEvtGetEvent
0CD8: 486E FFE8      pea.ex -24(a6)
0CDC: 4E4F           trap #15
0CDE: A0A9           sysTrapSysHandleEvent
0CE0: 4A00           tstb d0
0CE2: 4FEF 000C      lea 12(a7), d7
0CE6: 6618           bne 24
0CE8: 486E FFE8      pea.ex -24(a6)
0CEC: 4EBA FF76      jsr.ex -32660(pc)
0CF0: 4A00           tstb d0
0CF2: 584F           addqw #4, a7
0CF4: 660A           bne 10
0CF6: 486E FFE8      pea.ex -24(a6)
0CFA: 4E4F           trap #15
0CFC: A1A0           sysTrapFrmDispatchEvent
0CFE: 584F           addqw #4, a7
0D00: 0C6E 0016 FFE8 cmpiw.ex #22, -24(a6)
0D06: 66C4           bne -60
0D08: 4E5E           unlk a6
0D0A: 4E75           rts
0D0C: 8945           dc.w #35141
0D0E: 7665           moveq #101, d3
0D10: 6E74           bgt 116
0D12: 4C6F           dc.w #19567
0D14: 6F70           ble 112
0D16: 0000           dc.w #0
0D18: 4E56 FFD2      link a6, #-46
0D1C: 2F03           movel d3, -(a7)
0D1E: 362E 0008      movew.ex 8(a6), d3
0D22: 4A43           tstw d3
0D24: 6716           beq 22
0D26: 0C43 000D      cmpiw #13, d3
0D2A: 6710           beq 16
0D2C: 0C43 000E      cmpiw #14, d3
0D30: 670A           beq 10
0D32: 203C 0000 0502 movel.exl #1282, d0
0D38: 6000 00BE      bra 190
0D3C: 0C43 000D      cmpiw #13, d3
0D40: 57C0           seq d0
0D42: 4400           negb d0
0D44: 4880           extw d0
0D46: 1B40 FF85      moveb.mx d0, -123(a5)
0D4A: 4A2D FF85      tstb.ex -123(a5)
0D4E: 6600 008A      bne 138
0D52: 2D7C 6672 6D74 FFF8 movel.emxl #1718775156, -8(a6)
0D5A: 486E FFFC      pea.ex -4(a6)
0D5E: 486E FFF6      pea.ex -10(a6)
0D62: 1F3C 0001      moveb.ex #1, -(a7)
0D66: 2F3C 7072 6566 movel.exl #1886545254, -(a7)
0D6C: 2F3C 6170 706C movel.exl #1634758764, -(a7)
0D72: 486E FFD6      pea.ex -42(a6)
0D76: 1F3C 0001      moveb.ex #1, -(a7)
0D7A: 4E4F           trap #15
0D7C: A078           sysTrapDmGetNextDatabaseByTypeCreator
0D7E: 4AAE FFFC      tstl.ex -4(a6)
0D82: 4FEF 0018      lea 24(a7), d7
0D86: 6614           bne 20
0D88: 486D FFAC      pea.ex -84(a5)
0D8C: 3F3C 042E      movew.ex #1070, -(a7)
0D90: 486D FF9C      pea.ex -100(a5)
0D94: 4E4F           trap #15
0D96: A084           sysTrapErrDisplayFileLineMsg
0D98: 4FEF 000A      lea 10(a7), d7
0D9C: 4AAE FFFC      tstl.ex -4(a6)
0DA0: 6738           beq 56
0DA2: 486E FFD2      pea.ex -46(a6)
0DA6: 486E FFF8      pea.ex -8(a6)
0DAA: 2F3C 0000 8001 movel.exl #32769, -(a7)
0DB0: 2F2E FFFC      movel.ex -4(a6), -(a7)
0DB4: 3F2E FFF6      movew.ex -10(a6), -(a7)
0DB8: 4E4F           trap #15
0DBA: A08E           sysTrapSysAppLaunch
0DBC: 3600           movew d0, d3
0DBE: 4A43           tstw d3
0DC0: 4FEF 0012      lea 18(a7), d7
0DC4: 6714           beq 20
0DC6: 486D FFC0      pea.ex -64(a5)
0DCA: 3F3C 042E      movew.ex #1070, -(a7)
0DCE: 486D FF9C      pea.ex -100(a5)
0DD2: 4E4F           trap #15
0DD4: A084           sysTrapErrDisplayFileLineMsg
0DD6: 4FEF 000A      lea 10(a7), d7
0DDA: 4EBA F3E4      jsr.ex 30152(pc)
0DDE: 3600           movew d0, d3
0DE0: 3F3C 04B0      movew.ex #1200, -(a7)
0DE4: 4E4F           trap #15
0DE6: A19B           sysTrapFrmGotoForm
0DE8: 4A43           tstw d3
0DEA: 544F           addqw #2, a7
0DEC: 6604           bne 4
0DEE: 4EBA FED8      jsr.ex -32560(pc)
0DF2: 4EBA F4AA      jsr.ex 30374(pc)
0DF6: 7000           moveq #0, d0
0DF8: 261F           movel (a7)+, d3
0DFA: 4E5E           unlk a6
0DFC: 4E75           rts
0DFE: 8950           ormw d4, (a0)
0E00: 696C           bvs 108
0E02: 6F74           ble 116
0E04: 4D61           dc.w #19809
0E06: 696E           bvs 110
0E08: 0000 0000      orib #0, d0

