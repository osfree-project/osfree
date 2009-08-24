00A0: 0000 0001      orib #1, d0
00A4: 4E56 FFF4      link a6, #-12
00A8: 2F03           movel d3, -(a7)
00AA: 486E FFF4      pea.ex -12(a6)
00AE: 486E FFF8      pea.ex -8(a6)
00B2: 486E FFFC      pea.ex -4(a6)
00B6: 4E4F           trap #15
00B8: A08F           sysTrapSysAppStartup
00BA: 3600           movew d0, d3
00BC: 4A43           tstw d3
00BE: 4FEF 000C      lea 12(a7), a7
00C2: 671C           beq 00E0
00C4: 41FA 006A      lea 0130, a0
00C8: 4850           pea (a0)
00CA: 3F3C 0035      movew.ex #53, -(a7)
00CE: 41FA 007C      lea 014C, a0
00D2: 4850           pea (a0)
00D4: 4E4F           trap #15
00D6: A084           sysTrapErrDisplayFileLineMsg
00D8: 7000           moveq #0, d0
00DA: 4FEF 000A      lea 10(a7), a7
00DE: 603C           bra 011C
00E0: 206E FFFC      movel.ex -4(a6), a0
00E4: 3028 0006      movew.ex 6(a0), d0
00E8: 0240 0004      andiw #4, d0
00EC: 6704           beq 00F2
00EE: 4EBA 00D0      jsr.ex 01C0
00F2: 206E FFFC      movel.ex -4(a6), a0
00F6: 3F28 0006      movew.ex 6(a0), -(a7)
00FA: 2F28 0002      movel.ex 2(a0), -(a7)
00FE: 3F10           movew (a0), -(a7)
0100: 4EBA 0C48      jsr.ex 0D4A
0104: 2600           movel d0, d3
0106: 2F2E FFF4      movel.ex -12(a6), -(a7)
010A: 2F2E FFF8      movel.ex -8(a6), -(a7)
010E: 2F2E FFFC      movel.ex -4(a6), -(a7)
0112: 4E4F           trap #15
0114: A090           sysTrapSysAppExit
0116: 2003           movel d3, d0
0118: 4FEF 0014      lea 20(a7), a7
011C: 261F           movel (a7)+, d3
011E: 4E5E           unlk a6
0120: 4E75           rts
0122: 8B5F           ormw d5, (a7)+
0124: 5F53           subqw #7, (a3)
0126: 7461           moveq #97, d2
0128: 7274           moveq #116, d1
012A: 7570           dc.w #30064
012C: 5F5F           subqw #7, (a7)+
012E: 002A           dc.w #42
0130: 4572           dc.w #17778
0132: 726F           moveq #111, d1
0134: 7220           moveq #32, d1
0136: 6C61           bge 0199
0138: 756E           dc.w #30062
013A: 6368           bls 01A4
013C: 696E           bvs 01AC
013E: 6720           beq 0160
0140: 6170           bsr 01B2
0142: 706C           moveq #108, d0
0144: 6963           bvs 01A9
0146: 6174           bsr 01BC
0148: 696F           bvs 01B9
014A: 6E00 5374      bgt 54C0
014E: 6172           bsr 01C2
0150: 7475           moveq #117, d2
0152: 7043           moveq #67, d0
0154: 6F64           ble 01BA
0156: 652E           bcs 0186
0158: 6300 48E7      bls 4A41
015C: 3000           movew d0, d0
015E: 7402           moveq #2, d2
0160: 4EFB 2200      jmp.ex 0(pc,d2.w)
0164: 600A           bra 0170
0166: 4C01           dc.w #19457
0168: 0000           dc.w #0
016A: 4CDF 000C      movem (a7)+, <000c>
016E: 4E75           rts
0170: 2400           movel d0, d2
0172: 4842           swap d2
0174: C4C1           mulu d1, d2
0176: 2601           movel d1, d3
0178: 4843           swap d3
017A: C6C0           mulu d0, d3
017C: D443           addrw d3, d2
017E: 4842           swap d2
0180: 4242           clrw d2
0182: C0C1           mulu d1, d0
0184: D082           addrl d2, d0
0186: 4CDF 000C      movem (a7)+, <000c>
018A: 4E75           rts
018C: 885F           orrw (a7)+, d4
018E: 5F6C 6D75      subqw.ex #7, 28021(a4)
0192: 6C5F           bge 01F3
0194: 5F00           subqb #7, d0
0196: 0000           dc.w #0
0198: 205F           movel (a7)+, a0
019A: 2248           movel a0, a1
019C: D2D8           addaw (a0)+, a1
019E: 5848           addqw #4, a0
01A0: 3218           movew (a0)+, d1
01A2: B058           cmpw (a0)+, d0
01A4: 6604           bne 01AA
01A6: D0D0           addaw (a0), a0
01A8: 4ED0           jmp (a0)
01AA: 5448           addqw #2, a0
01AC: 51C9 FFF4      dbf d1, 65524
01B0: 4ED1           jmp (a1)
01B2: 8A5F           orrw (a7)+, d5
01B4: 5F77 7377      subqw.ex #7, 119(a7,d7.w)
01B8: 7463           moveq #99, d2
01BA: 685F           bvc 021B
01BC: 5F00           subqb #7, d0
01BE: 0000           dc.w #0
01C0: 4E75           rts
01C2: 4E56 0000      link a6, #0
01C6: 2F0A           movel a2, -(a7)
01C8: 246E 0010      movel.ex 16(a6), a2
01CC: 206E 000C      movel.ex 12(a6), a0
01D0: 7000           moveq #0, d0
01D2: 1010           moveb (a0), d0
01D4: ED88           lslil #6, d0
01D6: 4872 0801      pea.ex 1(a2,d0.l)
01DA: 206E 0008      movel.ex 8(a6), a0
01DE: 7000           moveq #0, d0
01E0: 1010           moveb (a0), d0
01E2: ED88           lslil #6, d0
01E4: 4872 0801      pea.ex 1(a2,d0.l)
01E8: 4E4F           trap #15
01EA: A0C8           sysTrapStrCompare
01EC: 504F           addqw #8, a7
01EE: 245F           movel (a7)+, a2
01F0: 4E5E           unlk a6
01F2: 4E75           rts
01F4: 9043           subrw d3, d0
01F6: 6F6D           ble 0265
01F8: 7061           moveq #97, d0
01FA: 7265           moveq #101, d1
01FC: 436F           dc.w #17263
01FE: 756E           dc.w #30062
0200: 7472           moveq #114, d2
0202: 6965           bvs 0269
0204: 7300           dc.w #29440
0206: 0000           dc.w #0
0208: 4E56 0000      link a6, #0
020C: 2F03           movel d3, -(a7)
020E: 1F3C 0005      moveb.ex #5, -(a7)
0212: 4E4F           trap #15
0214: A2D1           sysTrapPrefGetPreference
0216: 1B40 FFDB      moveb.mx d0, -37(a5)
021A: 1F3C 0002      moveb.ex #2, -(a7)
021E: 4E4F           trap #15
0220: A2D1           sysTrapPrefGetPreference
0222: 1B40 FFDC      moveb.mx d0, -36(a5)
0226: 1F3C 0003      moveb.ex #3, -(a7)
022A: 4E4F           trap #15
022C: A2D1           sysTrapPrefGetPreference
022E: 1B40 FFDD      moveb.mx d0, -35(a5)
0232: 1F3C 0004      moveb.ex #4, -(a7)
0236: 4E4F           trap #15
0238: A2D1           sysTrapPrefGetPreference
023A: 1B40 FFD9      moveb.mx d0, -39(a5)
023E: 1F3C 0006      moveb.ex #6, -(a7)
0242: 4E4F           trap #15
0244: A2D1           sysTrapPrefGetPreference
0246: 1B40 FFDA      moveb.mx d0, -38(a5)
024A: 1F3C 0001      moveb.ex #1, -(a7)
024E: 4E4F           trap #15
0250: A2D1           sysTrapPrefGetPreference
0252: 1B40 FFD8      moveb.mx d0, -40(a5)
0256: 3F3C 2710      movew.ex #10000, -(a7)
025A: 2F3C 636E 7479 movel.exl #1668183161, -(a7)
0260: 4E4F           trap #15
0262: A05F           sysTrapDmGetResource
0264: 2B48 FFB2      movel.mx a0, -78(a5)
0268: 4AAD FFB2      tstl.ex -78(a5)
026C: 4FEF 0012      lea 18(a7), a7
0270: 6750           beq 02C2
0272: 2F2D FFB2      movel.ex -78(a5), -(a7)
0276: 4E4F           trap #15
0278: A021           sysTrapMemHandleLock
027A: 2B48 FFAE      movel.mx a0, -82(a5)
027E: 2F2D FFAE      movel.ex -82(a5), -(a7)
0282: 4E4F           trap #15
0284: A016           sysTrapMemPtrSize
0286: EC88           lsril #6, d0
0288: 1B40 FFAD      moveb.mx d0, -83(a5)
028C: 7600           moveq #0, d3
028E: 504F           addqw #8, a7
0290: 600E           bra 02A0
0292: 7000           moveq #0, d0
0294: 1003           moveb d3, d0
0296: 41ED FF94      lea -108(a5), a0
029A: 1183 0000      moveb.mx d3, 0(a0,d0.w)
029E: 5203           addqb #1, d3
02A0: 0C03 0017      cmpib #23, d3
02A4: 63EC           bls 0292
02A6: 2F2D FFAE      movel.ex -82(a5), -(a7)
02AA: 487A FF16      pea.ex 01C2
02AE: 2F3C 0018 0001 movel.exl #1572865, -(a7)
02B4: 486D FF94      pea.ex -108(a5)
02B8: 4E4F           trap #15
02BA: A2B3           sysTrapSysInsertionSort
02BC: 4FEF 0010      lea 16(a7), a7
02C0: 6008           bra 02CA
02C2: 42AD FFAE      clrl.ex -82(a5)
02C6: 422D FFAD      clrb.ex -83(a5)
02CA: 7000           moveq #0, d0
02CC: 261F           movel (a7)+, d3
02CE: 4E5E           unlk a6
02D0: 4E75           rts
02D2: 9053           subrw (a3), d0
02D4: 7461           moveq #97, d2
02D6: 7274           moveq #116, d1
02D8: 4170           dc.w #16752
02DA: 706C           moveq #108, d0
02DC: 6963           bvs 0341
02DE: 6174           bsr 0354
02E0: 696F           bvs 0351
02E2: 6E00 0000      bgt 02E4
02E6: 4E56 0000      link a6, #0
02EA: 4E4F           trap #15
02EC: A1A1           sysTrapFrmCloseAllForms
02EE: 4AAD FFDE      tstl.ex -34(a5)
02F2: 670A           beq 02FE
02F4: 2F2D FFDE      movel.ex -34(a5), -(a7)
02F8: 4E4F           trap #15
02FA: A012           sysTrapMemPtrFree
02FC: 584F           addqw #4, a7
02FE: 2F2D FFB2      movel.ex -78(a5), -(a7)
0302: 4E4F           trap #15
0304: A022           sysTrapMemHandleUnlock
0306: 2F2D FFB2      movel.ex -78(a5), -(a7)
030A: 4E4F           trap #15
030C: A061           sysTrapDmReleaseResource
030E: 7000           moveq #0, d0
0310: 102D FFDB      moveb.ex -37(a5), d0
0314: 2F00           movel d0, -(a7)
0316: 1F3C 0005      moveb.ex #5, -(a7)
031A: 4E4F           trap #15
031C: A2D2           sysTrapPrefSetPreference
031E: 7000           moveq #0, d0
0320: 102D FFDC      moveb.ex -36(a5), d0
0324: 2F00           movel d0, -(a7)
0326: 1F3C 0002      moveb.ex #2, -(a7)
032A: 4E4F           trap #15
032C: A2D2           sysTrapPrefSetPreference
032E: 7000           moveq #0, d0
0330: 102D FFDD      moveb.ex -35(a5), d0
0334: 2F00           movel d0, -(a7)
0336: 1F3C 0003      moveb.ex #3, -(a7)
033A: 4E4F           trap #15
033C: A2D2           sysTrapPrefSetPreference
033E: 7000           moveq #0, d0
0340: 102D FFD9      moveb.ex -39(a5), d0
0344: 2F00           movel d0, -(a7)
0346: 1F3C 0004      moveb.ex #4, -(a7)
034A: 4E4F           trap #15
034C: A2D2           sysTrapPrefSetPreference
034E: 7000           moveq #0, d0
0350: 102D FFDA      moveb.ex -38(a5), d0
0354: 2F00           movel d0, -(a7)
0356: 1F3C 0006      moveb.ex #6, -(a7)
035A: 4E4F           trap #15
035C: A2D2           sysTrapPrefSetPreference
035E: 7000           moveq #0, d0
0360: 102D FFD8      moveb.ex -40(a5), d0
0364: 2F00           movel d0, -(a7)
0366: 1F3C 0001      moveb.ex #1, -(a7)
036A: 4E4F           trap #15
036C: A2D2           sysTrapPrefSetPreference
036E: 4E5E           unlk a6
0370: 4E75           rts
0372: 8F53           ormw d7, (a3)
0374: 746F           moveq #111, d2
0376: 7041           moveq #65, d0
0378: 7070           moveq #112, d0
037A: 6C69           bge 03E5
037C: 6361           bls 03DF
037E: 7469           moveq #105, d2
0380: 6F6E           ble 03F0
0382: 0000           dc.w #0
0384: 4E56 0000      link a6, #0
0388: 2F05           movel d5, -(a7)
038A: 206E 0008      movel.ex 8(a6), a0
038E: 1A2E 000C      moveb.ex 12(a6), d5
0392: 342E 000E      movew.ex 14(a6), d2
0396: 7200           moveq #0, d1
0398: 6002           bra 039C
039A: 5241           addqw #1, d1
039C: 7000           moveq #0, d0
039E: 3001           movew d1, d0
03A0: BA30 0800      cmpb.ex 0(a0,d0.l), d5
03A4: 6704           beq 03AA
03A6: B242           cmpw d2, d1
03A8: 65F0           bcs 039A
03AA: B242           cmpw d2, d1
03AC: 6506           bcs 03B4
03AE: 302E 0010      movew.ex 16(a6), d0
03B2: 6002           bra 03B6
03B4: 3001           movew d1, d0
03B6: 2A1F           movel (a7)+, d5
03B8: 4E5E           unlk a6
03BA: 4E75           rts
03BC: 8D4D           dc.w #36173
03BE: 6170           bsr 0430
03C0: 546F 506F      addqw.ex #2, 20591(a7)
03C4: 7369           dc.w #29545
03C6: 7469           moveq #105, d2
03C8: 6F6E           ble 0438
03CA: 0000           dc.w #0
03CC: 4E56 0000      link a6, #0
03D0: 2F0A           movel a2, -(a7)
03D2: 7000           moveq #0, d0
03D4: 302E 0008      movew.ex 8(a6), d0
03D8: 7234           moveq #52, d1
03DA: 4EBA FD7E      jsr.ex 015A
03DE: 206D FFDE      movel.ex -34(a5), a0
03E2: D1C0           addal d0, a0
03E4: 2448           movel a0, a2
03E6: 206E 000A      movel.ex 10(a6), a0
03EA: 3F28 0002      movew.ex 2(a0), -(a7)
03EE: 3F10           movew (a0), -(a7)
03F0: 2F0A           movel a2, -(a7)
03F2: 4E4F           trap #15
03F4: A0C7           sysTrapStrLen
03F6: 584F           addqw #4, a7
03F8: 3F00           movew d0, -(a7)
03FA: 2F0A           movel a2, -(a7)
03FC: 4E4F           trap #15
03FE: A220           sysTrapWinDrawChars
0400: 4FEF 000A      lea 10(a7), a7
0404: 245F           movel (a7)+, a2
0406: 4E5E           unlk a6
0408: 4E75           rts
040A: 9550           submw d2, (a0)
040C: 616E           bsr 047C
040E: 656C           bcs 047C
0410: 5069 636B      addqw.ex #8, 25451(a1)
0414: 4C69           dc.w #19561
0416: 7374           dc.w #29556
0418: 4472 6177      negw.ex 119(a2,d6.w)
041C: 4974           dc.w #18804
041E: 656D           bcs 048D
0420: 0000           dc.w #0
0422: 4E56 FFFA      link a6, #-6
0426: 48E7 1030      movem <1030>, -(a7)
042A: 266E 0008      movel.ex 8(a6), a3
042E: 246E 000C      movel.ex 12(a6), a2
0432: 486D FFE2      pea.ex -30(a5)
0436: 486D FFE6      pea.ex -26(a5)
043A: 4E4F           trap #15
043C: A2DC           sysTrapSysCreatePanelList
043E: 4A6D FFE6      tstw.ex -26(a5)
0442: 504F           addqw #8, a7
0444: 670E           beq 0454
0446: 2F2D FFE2      movel.ex -30(a5), -(a7)
044A: 4E4F           trap #15
044C: A021           sysTrapMemHandleLock
044E: 2B48 FFDE      movel.mx a0, -34(a5)
0452: 584F           addqw #4, a7
0454: 3F2D FFE6      movew.ex -26(a5), -(a7)
0458: 42A7           clrl -(a7)
045A: 2F0A           movel a2, -(a7)
045C: 4E4F           trap #15
045E: A1B8           sysTrapLstSetListChoices
0460: 3F2D FFE6      movew.ex -26(a5), -(a7)
0464: 2F0A           movel a2, -(a7)
0466: 4E4F           trap #15
0468: A1B6           sysTrapLstSetHeight
046A: 2F2E 0010      movel.ex 16(a6), -(a7)
046E: 2F0A           movel a2, -(a7)
0470: 4E4F           trap #15
0472: A1B0           sysTrapLstSetDrawFunction
0474: 486E FFFA      pea.ex -6(a6)
0478: 486E FFFE      pea.ex -2(a6)
047C: 4E4F           trap #15
047E: A0AC           sysTrapSysCurAppDatabase
0480: 7600           moveq #0, d3
0482: 4FEF 0020      lea 32(a7), a7
0486: 6042           bra 04CA
0488: 7034           moveq #52, d0
048A: C1C3           muls d3, d0
048C: 206D FFDE      movel.ex -34(a5), a0
0490: 222E FFFA      movel.ex -6(a6), d1
0494: B2B0 082A      cmpl.ex 42(a0,d0.l), d1
0498: 662E           bne 04C8
049A: 7034           moveq #52, d0
049C: C1C3           muls d3, d0
049E: 322E FFFE      movew.ex -2(a6), d1
04A2: B270 082E      cmpw.ex 46(a0,d0.l), d1
04A6: 6620           bne 04C8
04A8: 3F03           movew d3, -(a7)
04AA: 2F0A           movel a2, -(a7)
04AC: 4E4F           trap #15
04AE: A1B7           sysTrapLstSetSelection
04B0: 7034           moveq #52, d0
04B2: C1C3           muls d3, d0
04B4: 206D FFDE      movel.ex -34(a5), a0
04B8: 4870 0800      pea.ex 0(a0,d0.l)
04BC: 2F0B           movel a3, -(a7)
04BE: 4E4F           trap #15
04C0: A114           sysTrapCtlSetLabel
04C2: 4FEF 000E      lea 14(a7), a7
04C6: 6008           bra 04D0
04C8: 5243           addqw #1, d3
04CA: B66D FFE6      cmpw.ex -26(a5), d3
04CE: 65B8           bcs 0488
04D0: 4CDF 0C08      movem (a7)+, <0c08>
04D4: 4E5E           unlk a6
04D6: 4E75           rts
04D8: 9343           subxrw d3, d1
04DA: 7265           moveq #101, d1
04DC: 6174           bsr 0552
04DE: 6550           bcs 0530
04E0: 616E           bsr 0550
04E2: 656C           bcs 0550
04E4: 5069 636B      addqw.ex #8, 25451(a1)
04E8: 4C69           dc.w #19561
04EA: 7374           dc.w #29556
04EC: 0000           dc.w #0
04EE: 4E56 0000      link a6, #0
04F2: 2F03           movel d3, -(a7)
04F4: 362E 0008      movew.ex 8(a6), d3
04F8: 7000           moveq #0, d0
04FA: 3003           movew d3, d0
04FC: ED88           lslil #6, d0
04FE: 206D FFAE      movel.ex -82(a5), a0
0502: 1B70 0818 FFDB moveb.emx 24(a0,d0.l), -37(a5)
0508: 7000           moveq #0, d0
050A: 3003           movew d3, d0
050C: ED88           lslil #6, d0
050E: 1B70 0815 FFDC moveb.emx 21(a0,d0.l), -36(a5)
0514: 7000           moveq #0, d0
0516: 3003           movew d3, d0
0518: ED88           lslil #6, d0
051A: 1B70 0816 FFDD moveb.emx 22(a0,d0.l), -35(a5)
0520: 7000           moveq #0, d0
0522: 3003           movew d3, d0
0524: ED88           lslil #6, d0
0526: 1B70 0817 FFD9 moveb.emx 23(a0,d0.l), -39(a5)
052C: 7000           moveq #0, d0
052E: 3003           movew d3, d0
0530: ED88           lslil #6, d0
0532: 1B70 0819 FFDA moveb.emx 25(a0,d0.l), -38(a5)
0538: 7000           moveq #0, d0
053A: 102D FFDB      moveb.ex -37(a5), d0
053E: 2F00           movel d0, -(a7)
0540: 1F3C 0005      moveb.ex #5, -(a7)
0544: 4E4F           trap #15
0546: A2D2           sysTrapPrefSetPreference
0548: 5C4F           addqw #6, a7
054A: 261F           movel (a7)+, d3
054C: 4E5E           unlk a6
054E: 4E75           rts
0550: 9753           submw d3, (a3)
0552: 6574           bcs 05C8
0554: 5072 6566      addqw.ex #8, 102(a2,d6.w)
0558: 6572           bcs 05CC
055A: 656E           bcs 05CA
055C: 6365           bls 05C3
055E: 7342           dc.w #29506
0560: 7943           dc.w #31043
0562: 6F75           ble 05D9
0564: 6E74           bgt 05DA
0566: 7279           moveq #121, d1
0568: 0000           dc.w #0
056A: 4E56 FFF8      link a6, #-8
056E: 48E7 1E30      movem <1e30>, -(a7)
0572: 266E 0008      movel.ex 8(a6), a3
0576: 246E 000C      movel.ex 12(a6), a2
057A: 3A2E 0010      movew.ex 16(a6), d5
057E: 3C2E 0012      movew.ex 18(a6), d6
0582: 2F0B           movel a3, -(a7)
0584: 2F0A           movel a2, -(a7)
0586: 4E4F           trap #15
0588: A0C8           sysTrapStrCompare
058A: 4A40           tstw d0
058C: 504F           addqw #8, a7
058E: 6700 0088      beq 0618
0592: 3F06           movew d6, -(a7)
0594: 3F05           movew d5, -(a7)
0596: 2F0A           movel a2, -(a7)
0598: 4E4F           trap #15
059A: A0C7           sysTrapStrLen
059C: 584F           addqw #4, a7
059E: 3F00           movew d0, -(a7)
05A0: 2F0A           movel a2, -(a7)
05A2: 4E4F           trap #15
05A4: A220           sysTrapWinDrawChars
05A6: 2F0A           movel a2, -(a7)
05A8: 4E4F           trap #15
05AA: A0C7           sysTrapStrLen
05AC: 584F           addqw #4, a7
05AE: 3F00           movew d0, -(a7)
05B0: 2F0A           movel a2, -(a7)
05B2: 4E4F           trap #15
05B4: A16B           sysTrapFntCharsWidth
05B6: 3800           movew d0, d4
05B8: 2F0B           movel a3, -(a7)
05BA: 4E4F           trap #15
05BC: A0C7           sysTrapStrLen
05BE: 4A40           tstw d0
05C0: 4FEF 0014      lea 20(a7), a7
05C4: 6716           beq 05DC
05C6: 2F0B           movel a3, -(a7)
05C8: 4E4F           trap #15
05CA: A0C7           sysTrapStrLen
05CC: 584F           addqw #4, a7
05CE: 3F00           movew d0, -(a7)
05D0: 2F0B           movel a3, -(a7)
05D2: 4E4F           trap #15
05D4: A16B           sysTrapFntCharsWidth
05D6: 3600           movew d0, d3
05D8: 5C4F           addqw #6, a7
05DA: 6006           bra 05E2
05DC: 363C 00A0      movew.ex #160, d3
05E0: 9645           subrw d5, d3
05E2: B644           cmpw d4, d3
05E4: 6F28           ble 060E
05E6: 3005           movew d5, d0
05E8: D044           addrw d4, d0
05EA: 3D40 FFF8      movew.mx d0, -8(a6)
05EE: 3D46 FFFA      movew.mx d6, -6(a6)
05F2: 3003           movew d3, d0
05F4: 9044           subrw d4, d0
05F6: 3D40 FFFC      movew.mx d0, -4(a6)
05FA: 4E4F           trap #15
05FC: A168           sysTrapFntLineHeight
05FE: 3D40 FFFE      movew.mx d0, -2(a6)
0602: 4267           clrw -(a7)
0604: 486E FFF8      pea.ex -8(a6)
0608: 4E4F           trap #15
060A: A219           sysTrapWinEraseRectangle
060C: 5C4F           addqw #6, a7
060E: 2F0A           movel a2, -(a7)
0610: 2F0B           movel a3, -(a7)
0612: 4E4F           trap #15
0614: A0C5           sysTrapStrCopy
0616: 504F           addqw #8, a7
0618: 4CDF 0C78      movem (a7)+, <0c78>
061C: 4E5E           unlk a6
061E: 4E75           rts
0620: 9752           submw d3, (a2)
0622: 6564           bcs 0688
0624: 7261           moveq #97, d1
0626: 7753           dc.w #30547
0628: 7472           moveq #114, d2
062A: 696E           bvs 069A
062C: 6749           beq 0677
062E: 6644           bne 0674
0630: 6966           bvs 0698
0632: 6665           bne 0699
0634: 7265           moveq #101, d1
0636: 6E74           bgt 06AC
0638: 0000           dc.w #0
063A: 4E56 FFE8      link a6, #-24
063E: 486E FFF2      pea.ex -14(a6)
0642: 4E4F           trap #15
0644: A0F5           sysTrapTimGetSeconds
0646: 2F00           movel d0, -(a7)
0648: 4E4F           trap #15
064A: A0FC           sysTrapTimSecondsToDateTime
064C: 486E FFE8      pea.ex -24(a6)
0650: 1F2D FFDB      moveb.ex -37(a5), -(a7)
0654: 1F2E FFF5      moveb.ex -11(a6), -(a7)
0658: 1F2E FFF7      moveb.ex -9(a6), -(a7)
065C: 4E4F           trap #15
065E: A268           sysTrapTimeToAscii
0660: 2F3C 0059 0034 movel.exl #5832756, -(a7)
0666: 486E FFE8      pea.ex -24(a6)
066A: 486D FFCF      pea.ex -49(a5)
066E: 4EBA FEFA      jsr.ex 056A
0672: 4E5E           unlk a6
0674: 4E75           rts
0676: 9546           subxrw d6, d2
0678: 6F72           ble 06EC
067A: 6D61           blt 06DD
067C: 7473           moveq #115, d2
067E: 466F 726D      notw.ex 29293(a7)
0682: 5570 6461      subqw.ex #2, 97(a0,d6.w)
0686: 7465           moveq #101, d2
0688: 5469 6D65      addqw.ex #2, 28005(a1)
068C: 0000           dc.w #0
068E: 4E56 FFE2      link a6, #-30
0692: 486E FFF2      pea.ex -14(a6)
0696: 4E4F           trap #15
0698: A0F5           sysTrapTimGetSeconds
069A: 2F00           movel d0, -(a7)
069C: 4E4F           trap #15
069E: A0FC           sysTrapTimSecondsToDateTime
06A0: 486E FFE2      pea.ex -30(a6)
06A4: 1F2D FFDC      moveb.ex -36(a5), -(a7)
06A8: 3F2E FFFC      movew.ex -4(a6), -(a7)
06AC: 1F2E FFF9      moveb.ex -7(a6), -(a7)
06B0: 1F2E FFFB      moveb.ex -5(a6), -(a7)
06B4: 4E4F           trap #15
06B6: A266           sysTrapDateToAscii
06B8: 2F3C 0059 004C movel.exl #5832780, -(a7)
06BE: 486E FFE2      pea.ex -30(a6)
06C2: 486D FFC6      pea.ex -58(a5)
06C6: 4EBA FEA2      jsr.ex 056A
06CA: 486E FFE2      pea.ex -30(a6)
06CE: 1F2D FFDD      moveb.ex -35(a5), -(a7)
06D2: 3F2E FFFC      movew.ex -4(a6), -(a7)
06D6: 1F2E FFF9      moveb.ex -7(a6), -(a7)
06DA: 1F2E FFFB      moveb.ex -5(a6), -(a7)
06DE: 4E4F           trap #15
06E0: A266           sysTrapDateToAscii
06E2: 4E4F           trap #15
06E4: A168           sysTrapFntLineHeight
06E6: 0640 004D      addiw #77, d0
06EA: 3F00           movew d0, -(a7)
06EC: 3F3C 0059      movew.ex #89, -(a7)
06F0: 486E FFE2      pea.ex -30(a6)
06F4: 486D FFB7      pea.ex -73(a5)
06F8: 4EBA FE70      jsr.ex 056A
06FC: 4E5E           unlk a6
06FE: 4E75           rts
0700: 9546           subxrw d6, d2
0702: 6F72           ble 0776
0704: 6D61           blt 0767
0706: 7473           moveq #115, d2
0708: 466F 726D      notw.ex 29293(a7)
070C: 5570 6461      subqw.ex #2, 97(a0,d6.w)
0710: 7465           moveq #101, d2
0712: 4461           negw -(a1)
0714: 7465           moveq #101, d2
0716: 0000           dc.w #0
0718: 4E56 0000      link a6, #0
071C: 48E7 0038      movem <0038>, -(a7)
0720: 286E 000A      movel.ex 10(a6), a4
0724: 266E 000E      movel.ex 14(a6), a3
0728: 7000           moveq #0, d0
072A: 302E 0008      movew.ex 8(a6), d0
072E: 45ED FF94      lea -108(a5), a2
0732: 7200           moveq #0, d1
0734: 1232 0800      moveb.ex 0(a2,d0.l), d1
0738: ED89           lslil #6, d1
073A: 244B           movel a3, a2
073C: D5C1           addal d1, a2
073E: 528A           addql #1, a2
0740: 3F2C 0002      movew.ex 2(a4), -(a7)
0744: 3F14           movew (a4), -(a7)
0746: 2F0A           movel a2, -(a7)
0748: 4E4F           trap #15
074A: A0C7           sysTrapStrLen
074C: 584F           addqw #4, a7
074E: 3F00           movew d0, -(a7)
0750: 2F0A           movel a2, -(a7)
0752: 4E4F           trap #15
0754: A220           sysTrapWinDrawChars
0756: 4FEF 000A      lea 10(a7), a7
075A: 4CDF 1C00      movem (a7)+, <1c00>
075E: 4E5E           unlk a6
0760: 4E75           rts
0762: 9343           subxrw d3, d1
0764: 6F75           ble 07DB
0766: 6E74           bgt 07DC
0768: 7279           moveq #121, d1
076A: 4C69           dc.w #19561
076C: 7374           dc.w #29556
076E: 4472 6177      negw.ex 119(a2,d6.w)
0772: 4974           dc.w #18804
0774: 656D           bcs 07E3
0776: 0000           dc.w #0
0778: 4E56 FFF8      link a6, #-8
077C: 48E7 1030      movem <1030>, -(a7)
0780: 246E 0008      movel.ex 8(a6), a2
0784: 4A2D FFE9      tstb.ex -23(a5)
0788: 6752           beq 07DC
078A: 3F3C 04C2      movew.ex #1218, -(a7)
078E: 2F0A           movel a2, -(a7)
0790: 4E4F           trap #15
0792: A180           sysTrapFrmGetObjectIndex
0794: 5C4F           addqw #6, a7
0796: 3F00           movew d0, -(a7)
0798: 2F0A           movel a2, -(a7)
079A: 4E4F           trap #15
079C: A184           sysTrapFrmHideObject
079E: 3F3C 04D2      movew.ex #1234, -(a7)
07A2: 2F0A           movel a2, -(a7)
07A4: 4E4F           trap #15
07A6: A180           sysTrapFrmGetObjectIndex
07A8: 5C4F           addqw #6, a7
07AA: 3F00           movew d0, -(a7)
07AC: 2F0A           movel a2, -(a7)
07AE: 4E4F           trap #15
07B0: A185           sysTrapFrmShowObject
07B2: 3F3C 04D1      movew.ex #1233, -(a7)
07B6: 2F0A           movel a2, -(a7)
07B8: 4E4F           trap #15
07BA: A180           sysTrapFrmGetObjectIndex
07BC: 3600           movew d0, d3
07BE: 3F03           movew d3, -(a7)
07C0: 2F0A           movel a2, -(a7)
07C2: 4E4F           trap #15
07C4: A185           sysTrapFrmShowObject
07C6: 486E FFF8      pea.ex -8(a6)
07CA: 3F03           movew d3, -(a7)
07CC: 2F0A           movel a2, -(a7)
07CE: 4E4F           trap #15
07D0: A199           sysTrapFrmGetObjectBounds
07D2: 362E FFFC      movew.ex -4(a6), d3
07D6: 5C43           addqw #6, d3
07D8: 4FEF 0022      lea 34(a7), a7
07DC: 3F3C 04CB      movew.ex #1227, -(a7)
07E0: 2F0A           movel a2, -(a7)
07E2: 4E4F           trap #15
07E4: A180           sysTrapFrmGetObjectIndex
07E6: 5C4F           addqw #6, a7
07E8: 3F00           movew d0, -(a7)
07EA: 2F0A           movel a2, -(a7)
07EC: 4E4F           trap #15
07EE: A183           sysTrapFrmGetObjectPtr
07F0: 2648           movel a0, a3
07F2: 7000           moveq #0, d0
07F4: 102D FFAD      moveb.ex -83(a5), d0
07F8: 3F00           movew d0, -(a7)
07FA: 2F2D FFAE      movel.ex -82(a5), -(a7)
07FE: 2F0B           movel a3, -(a7)
0800: 4E4F           trap #15
0802: A1B8           sysTrapLstSetListChoices
0804: 7000           moveq #0, d0
0806: 102D FFAD      moveb.ex -83(a5), d0
080A: 3F00           movew d0, -(a7)
080C: 2F0B           movel a3, -(a7)
080E: 4E4F           trap #15
0810: A1B6           sysTrapLstSetHeight
0812: 487A FF04      pea.ex 0718
0816: 2F0B           movel a3, -(a7)
0818: 4E4F           trap #15
081A: A1B0           sysTrapLstSetDrawFunction
081C: 4267           clrw -(a7)
081E: 7000           moveq #0, d0
0820: 102D FFAD      moveb.ex -83(a5), d0
0824: 3F00           movew d0, -(a7)
0826: 1F2D FFD8      moveb.ex -40(a5), -(a7)
082A: 486D FF94      pea.ex -108(a5)
082E: 4EBA FB54      jsr.ex 0384
0832: 3600           movew d0, d3
0834: 3F03           movew d3, -(a7)
0836: 2F0B           movel a3, -(a7)
0838: 4E4F           trap #15
083A: A1B7           sysTrapLstSetSelection
083C: 3F03           movew d3, -(a7)
083E: 2F0B           movel a3, -(a7)
0840: 4E4F           trap #15
0842: A1B9           sysTrapLstMakeItemVisible
0844: 7000           moveq #0, d0
0846: 102D FFD8      moveb.ex -40(a5), d0
084A: ED88           lslil #6, d0
084C: 206D FFAE      movel.ex -82(a5), a0
0850: 4870 0801      pea.ex 1(a0,d0.l)
0854: 3F3C 04CA      movew.ex #1226, -(a7)
0858: 2F0A           movel a2, -(a7)
085A: 4E4F           trap #15
085C: A180           sysTrapFrmGetObjectIndex
085E: 5C4F           addqw #6, a7
0860: 3F00           movew d0, -(a7)
0862: 2F0A           movel a2, -(a7)
0864: 4E4F           trap #15
0866: A183           sysTrapFrmGetObjectPtr
0868: 5C4F           addqw #6, a7
086A: 2F08           movel a0, -(a7)
086C: 4E4F           trap #15
086E: A114           sysTrapCtlSetLabel
0870: 3F3C 04C7      movew.ex #1223, -(a7)
0874: 2F0A           movel a2, -(a7)
0876: 4E4F           trap #15
0878: A180           sysTrapFrmGetObjectIndex
087A: 5C4F           addqw #6, a7
087C: 3F00           movew d0, -(a7)
087E: 2F0A           movel a2, -(a7)
0880: 4E4F           trap #15
0882: A183           sysTrapFrmGetObjectPtr
0884: 2648           movel a0, a3
0886: 2F3C 0005 0000 movel.exl #327680, -(a7)
088C: 1F2D FFDB      moveb.ex -37(a5), -(a7)
0890: 486D FFEA      pea.ex -22(a5)
0894: 4EBA FAEE      jsr.ex 0384
0898: 3600           movew d0, d3
089A: 3F03           movew d3, -(a7)
089C: 2F0B           movel a3, -(a7)
089E: 4E4F           trap #15
08A0: A1B7           sysTrapLstSetSelection
08A2: 3F03           movew d3, -(a7)
08A4: 2F0B           movel a3, -(a7)
08A6: 4E4F           trap #15
08A8: A1B4           sysTrapLstGetSelectionText
08AA: 5C4F           addqw #6, a7
08AC: 2F08           movel a0, -(a7)
08AE: 3F3C 04C6      movew.ex #1222, -(a7)
08B2: 2F0A           movel a2, -(a7)
08B4: 4E4F           trap #15
08B6: A180           sysTrapFrmGetObjectIndex
08B8: 5C4F           addqw #6, a7
08BA: 3F00           movew d0, -(a7)
08BC: 2F0A           movel a2, -(a7)
08BE: 4E4F           trap #15
08C0: A183           sysTrapFrmGetObjectPtr
08C2: 5C4F           addqw #6, a7
08C4: 2F08           movel a0, -(a7)
08C6: 4E4F           trap #15
08C8: A114           sysTrapCtlSetLabel
08CA: 3F3C 04C0      movew.ex #1216, -(a7)
08CE: 2F0A           movel a2, -(a7)
08D0: 4E4F           trap #15
08D2: A180           sysTrapFrmGetObjectIndex
08D4: 5C4F           addqw #6, a7
08D6: 3F00           movew d0, -(a7)
08D8: 2F0A           movel a2, -(a7)
08DA: 4E4F           trap #15
08DC: A183           sysTrapFrmGetObjectPtr
08DE: 2648           movel a0, a3
08E0: 2F3C 0007 0000 movel.exl #458752, -(a7)
08E6: 1F2D FFDC      moveb.ex -36(a5), -(a7)
08EA: 486D FFF0      pea.ex -16(a5)
08EE: 4EBA FA94      jsr.ex 0384
08F2: 3600           movew d0, d3
08F4: 3F03           movew d3, -(a7)
08F6: 2F0B           movel a3, -(a7)
08F8: 4E4F           trap #15
08FA: A1B7           sysTrapLstSetSelection
08FC: 3F03           movew d3, -(a7)
08FE: 2F0B           movel a3, -(a7)
0900: 4E4F           trap #15
0902: A1B4           sysTrapLstGetSelectionText
0904: 5C4F           addqw #6, a7
0906: 2F08           movel a0, -(a7)
0908: 3F3C 04BF      movew.ex #1215, -(a7)
090C: 2F0A           movel a2, -(a7)
090E: 4E4F           trap #15
0910: A180           sysTrapFrmGetObjectIndex
0912: 5C4F           addqw #6, a7
0914: 3F00           movew d0, -(a7)
0916: 2F0A           movel a2, -(a7)
0918: 4E4F           trap #15
091A: A183           sysTrapFrmGetObjectPtr
091C: 5C4F           addqw #6, a7
091E: 2F08           movel a0, -(a7)
0920: 4E4F           trap #15
0922: A114           sysTrapCtlSetLabel
0924: 3F3C 04B6      movew.ex #1206, -(a7)
0928: 2F0A           movel a2, -(a7)
092A: 4E4F           trap #15
092C: A180           sysTrapFrmGetObjectIndex
092E: 5C4F           addqw #6, a7
0930: 3F00           movew d0, -(a7)
0932: 2F0A           movel a2, -(a7)
0934: 4E4F           trap #15
0936: A183           sysTrapFrmGetObjectPtr
0938: 2648           movel a0, a3
093A: 7000           moveq #0, d0
093C: 102D FFD9      moveb.ex -39(a5), d0
0940: 3F00           movew d0, -(a7)
0942: 2F0B           movel a3, -(a7)
0944: 4E4F           trap #15
0946: A1B7           sysTrapLstSetSelection
0948: 7000           moveq #0, d0
094A: 102D FFD9      moveb.ex -39(a5), d0
094E: 3F00           movew d0, -(a7)
0950: 2F0B           movel a3, -(a7)
0952: 4E4F           trap #15
0954: A1B4           sysTrapLstGetSelectionText
0956: 5C4F           addqw #6, a7
0958: 2F08           movel a0, -(a7)
095A: 3F3C 04B4      movew.ex #1204, -(a7)
095E: 2F0A           movel a2, -(a7)
0960: 4E4F           trap #15
0962: A180           sysTrapFrmGetObjectIndex
0964: 5C4F           addqw #6, a7
0966: 3F00           movew d0, -(a7)
0968: 2F0A           movel a2, -(a7)
096A: 4E4F           trap #15
096C: A183           sysTrapFrmGetObjectPtr
096E: 5C4F           addqw #6, a7
0970: 2F08           movel a0, -(a7)
0972: 4E4F           trap #15
0974: A114           sysTrapCtlSetLabel
0976: 3F3C 04CF      movew.ex #1231, -(a7)
097A: 2F0A           movel a2, -(a7)
097C: 4E4F           trap #15
097E: A180           sysTrapFrmGetObjectIndex
0980: 5C4F           addqw #6, a7
0982: 3F00           movew d0, -(a7)
0984: 2F0A           movel a2, -(a7)
0986: 4E4F           trap #15
0988: A183           sysTrapFrmGetObjectPtr
098A: 2648           movel a0, a3
098C: 7000           moveq #0, d0
098E: 102D FFDA      moveb.ex -38(a5), d0
0992: 3F00           movew d0, -(a7)
0994: 2F0B           movel a3, -(a7)
0996: 4E4F           trap #15
0998: A1B7           sysTrapLstSetSelection
099A: 7000           moveq #0, d0
099C: 102D FFDA      moveb.ex -38(a5), d0
09A0: 3F00           movew d0, -(a7)
09A2: 2F0B           movel a3, -(a7)
09A4: 4E4F           trap #15
09A6: A1B4           sysTrapLstGetSelectionText
09A8: 5C4F           addqw #6, a7
09AA: 2F08           movel a0, -(a7)
09AC: 3F3C 04CE      movew.ex #1230, -(a7)
09B0: 2F0A           movel a2, -(a7)
09B2: 4E4F           trap #15
09B4: A180           sysTrapFrmGetObjectIndex
09B6: 5C4F           addqw #6, a7
09B8: 3F00           movew d0, -(a7)
09BA: 2F0A           movel a2, -(a7)
09BC: 4E4F           trap #15
09BE: A183           sysTrapFrmGetObjectPtr
09C0: 5C4F           addqw #6, a7
09C2: 2F08           movel a0, -(a7)
09C4: 4E4F           trap #15
09C6: A114           sysTrapCtlSetLabel
09C8: 4FEF 00A0      lea 160(a7), a7
09CC: 4CDF 0C08      movem (a7)+, <0c08>
09D0: 4E5E           unlk a6
09D2: 4E75           rts
09D4: 8F46           dc.w #36678
09D6: 6F72           ble 0A4A
09D8: 6D61           blt 0A3B
09DA: 7473           moveq #115, d2
09DC: 466F 726D      notw.ex 29293(a7)
09E0: 496E           dc.w #18798
09E2: 6974           bvs 0A58
09E4: 0000           dc.w #0
09E6: 4E56 FFFE      link a6, #-2
09EA: 42A7           clrl -(a7)
09EC: 486E FFFE      pea.ex -2(a6)
09F0: 4E4F           trap #15
09F2: A20C           sysTrapWinGetWindowExtent
09F4: 3F3C 0022      movew.ex #34, -(a7)
09F8: 3F2E FFFE      movew.ex -2(a6), -(a7)
09FC: 4878 0022      pea.ex (0022).w
0A00: 4E4F           trap #15
0A02: A213           sysTrapWinDrawLine
0A04: 4EBA FC34      jsr.ex 063A
0A08: 4EBA FC84      jsr.ex 068E
0A0C: 4E5E           unlk a6
0A0E: 4E75           rts
0A10: 9346           subxrw d6, d1
0A12: 6F72           ble 0A86
0A14: 6D61           blt 0A77
0A16: 7473           moveq #115, d2
0A18: 466F 726D      notw.ex 29293(a7)
0A1C: 4472 6177      negw.ex 119(a2,d6.w)
0A20: 466F 726D      notw.ex 29293(a7)
0A24: 0000           dc.w #0
0A26: 4E56 0000      link a6, #0
0A2A: 422D FFCF      clrb.ex -49(a5)
0A2E: 422D FFC6      clrb.ex -58(a5)
0A32: 422D FFB7      clrb.ex -73(a5)
0A36: 4EBA FFAE      jsr.ex 09E6
0A3A: 4E5E           unlk a6
0A3C: 4E75           rts
0A3E: 9846           subrw d6, d4
0A40: 6F72           ble 0AB4
0A42: 6D61           blt 0AA5
0A44: 7473           moveq #115, d2
0A46: 466F 726D      notw.ex 29293(a7)
0A4A: 466F 7263      notw.ex 29283(a7)
0A4E: 6544           bcs 0A94
0A50: 7261           moveq #97, d1
0A52: 7746           dc.w #30534
0A54: 6F72           ble 0AC8
0A56: 6D00 0000      blt 0A58
0A5A: 4E56 FFE2      link a6, #-30
0A5E: 48E7 1020      movem <1020>, -(a7)
0A62: 246E 0008      movel.ex 8(a6), a2
0A66: 7600           moveq #0, d3
0A68: 4A52           tstw (a2)
0A6A: 660E           bne 0A7A
0A6C: 4EBA FBCC      jsr.ex 063A
0A70: 4EBA FC1C      jsr.ex 068E
0A74: 7001           moveq #1, d0
0A76: 6000 01FC      bra 0C74
0A7A: 0C52 0009      cmpiw #9, (a2)
0A7E: 661E           bne 0A9E
0A80: 302A 0008      movew.ex 8(a2), d0
0A84: 0440 04D1      subiw #1233, d0
0A88: 6702           beq 0A8C
0A8A: 6012           bra 0A9E
0A8C: 3D7C 0016 FFE2 movew.emx #22, -30(a6)
0A92: 486E FFE2      pea.ex -30(a6)
0A96: 4E4F           trap #15
0A98: A11B           sysTrapEvtAddEventToQueue
0A9A: 7601           moveq #1, d3
0A9C: 584F           addqw #4, a7
0A9E: 0C52 000E      cmpiw #14, (a2)
0AA2: 6600 016C      bne 0C10
0AA6: 302A 000E      movew.ex 14(a2), d0
0AAA: 4EBA F6EC      jsr.ex 0198
0AAE: 01C4           bset d0, d4
0AB0: 04B6 04CF 0005 04B6 subil.ex #80674821, -74(a6,d0.w)
0AB8: 0148 04C0      movep.l (04c0, a0), d0
0ABC: 010C 04C3      movep.w (04c3, a4), d0
0AC0: 000E           dc.w #14
0AC2: 04C7           dc.w #1223
0AC4: 00C8           dc.w #200
0AC6: 04CB           dc.w #1227
0AC8: 007A           dc.w #122
0ACA: 04CF           dc.w #1231
0ACC: 013C 486E      btst d0, Nonzero high byte
0AD0: FFFA           dc.w #65530
0AD2: 486E FFFE      pea.ex -2(a6)
0AD6: 4E4F           trap #15
0AD8: A0AC           sysTrapSysCurAppDatabase
0ADA: 7000           moveq #0, d0
0ADC: 302A 0014      movew.ex 20(a2), d0
0AE0: 7234           moveq #52, d1
0AE2: 4EBA F676      jsr.ex 015A
0AE6: 206D FFDE      movel.ex -34(a5), a0
0AEA: 222E FFFA      movel.ex -6(a6), d1
0AEE: B2B0 082A      cmpl.ex 42(a0,d0.l), d1
0AF2: 504F           addqw #8, a7
0AF4: 6616           bne 0B0C
0AF6: 7000           moveq #0, d0
0AF8: 302A 0014      movew.ex 20(a2), d0
0AFC: 7234           moveq #52, d1
0AFE: 4EBA F65A      jsr.ex 015A
0B02: 322E FFFE      movew.ex -2(a6), d1
0B06: B270 082E      cmpw.ex 46(a0,d0.l), d1
0B0A: 6730           beq 0B3C
0B0C: 42A7           clrl -(a7)
0B0E: 4267           clrw -(a7)
0B10: 7000           moveq #0, d0
0B12: 302A 0014      movew.ex 20(a2), d0
0B16: 7234           moveq #52, d1
0B18: 4EBA F640      jsr.ex 015A
0B1C: 206D FFDE      movel.ex -34(a5), a0
0B20: 2F30 082A      movel.ex 42(a0,d0.l), -(a7)
0B24: 7000           moveq #0, d0
0B26: 302A 0014      movew.ex 20(a2), d0
0B2A: 7234           moveq #52, d1
0B2C: 4EBA F62C      jsr.ex 015A
0B30: 3F30 082E      movew.ex 46(a0,d0.l), -(a7)
0B34: 4E4F           trap #15
0B36: A0A7           sysTrapSysUIAppSwitch
0B38: 4FEF 000C      lea 12(a7), a7
0B3C: 7601           moveq #1, d3
0B3E: 6000 0132      bra 0C72
0B42: 7000           moveq #0, d0
0B44: 302A 0014      movew.ex 20(a2), d0
0B48: 41ED FF94      lea -108(a5), a0
0B4C: 122D FFD8      moveb.ex -40(a5), d1
0B50: B230 0800      cmpb.ex 0(a0,d0.l), d1
0B54: 6730           beq 0B86
0B56: 7000           moveq #0, d0
0B58: 302A 0014      movew.ex 20(a2), d0
0B5C: 1B70 0800 FFD8 moveb.emx 0(a0,d0.l), -40(a5)
0B62: 7000           moveq #0, d0
0B64: 302A 0014      movew.ex 20(a2), d0
0B68: 7200           moveq #0, d1
0B6A: 1230 0800      moveb.ex 0(a0,d0.l), d1
0B6E: 3F01           movew d1, -(a7)
0B70: 4EBA F97C      jsr.ex 04EE
0B74: 4E4F           trap #15
0B76: A173           sysTrapFrmGetActiveForm
0B78: 2448           movel a0, a2
0B7A: 2F0A           movel a2, -(a7)
0B7C: 4EBA FBFA      jsr.ex 0778
0B80: 4EBA FEA4      jsr.ex 0A26
0B84: 5C4F           addqw #6, a7
0B86: 7601           moveq #1, d3
0B88: 6000 00E8      bra 0C72
0B8C: 7000           moveq #0, d0
0B8E: 302A 0014      movew.ex 20(a2), d0
0B92: 41ED FFEA      lea -22(a5), a0
0B96: 122D FFDB      moveb.ex -37(a5), d1
0B9A: B230 0800      cmpb.ex 0(a0,d0.l), d1
0B9E: 6700 00D2      beq 0C72
0BA2: 7000           moveq #0, d0
0BA4: 302A 0014      movew.ex 20(a2), d0
0BA8: 1B70 0800 FFDB moveb.emx 0(a0,d0.l), -37(a5)
0BAE: 4EBA FA8A      jsr.ex 063A
0BB2: 7000           moveq #0, d0
0BB4: 102D FFDB      moveb.ex -37(a5), d0
0BB8: 2F00           movel d0, -(a7)
0BBA: 1F3C 0005      moveb.ex #5, -(a7)
0BBE: 4E4F           trap #15
0BC0: A2D2           sysTrapPrefSetPreference
0BC2: 5C4F           addqw #6, a7
0BC4: 6000 00AC      bra 0C72
0BC8: 7000           moveq #0, d0
0BCA: 302A 0014      movew.ex 20(a2), d0
0BCE: 41ED FFF0      lea -16(a5), a0
0BD2: 122D FFDC      moveb.ex -36(a5), d1
0BD6: B230 0800      cmpb.ex 0(a0,d0.l), d1
0BDA: 6700 0096      beq 0C72
0BDE: 7000           moveq #0, d0
0BE0: 302A 0014      movew.ex 20(a2), d0
0BE4: 1B70 0800 FFDC moveb.emx 0(a0,d0.l), -36(a5)
0BEA: 7000           moveq #0, d0
0BEC: 302A 0014      movew.ex 20(a2), d0
0BF0: 41ED FFF8      lea -8(a5), a0
0BF4: 1B70 0800 FFDD moveb.emx 0(a0,d0.l), -35(a5)
0BFA: 4EBA FA92      jsr.ex 068E
0BFE: 6072           bra 0C72
0C00: 1B6A 0015 FFD9 moveb.emx 21(a2), -39(a5)
0C06: 606A           bra 0C72
0C08: 1B6A 0015 FFDA moveb.emx 21(a2), -38(a5)
0C0E: 6062           bra 0C72
0C10: 0C52 0018      cmpiw #24, (a2)
0C14: 665C           bne 0C72
0C16: 4E4F           trap #15
0C18: A173           sysTrapFrmGetActiveForm
0C1A: 2448           movel a0, a2
0C1C: 2F0A           movel a2, -(a7)
0C1E: 4EBA FB58      jsr.ex 0778
0C22: 2F0A           movel a2, -(a7)
0C24: 4E4F           trap #15
0C26: A171           sysTrapFrmDrawForm
0C28: 4EBA FDFC      jsr.ex 0A26
0C2C: 4A2D FFE9      tstb.ex -23(a5)
0C30: 504F           addqw #8, a7
0C32: 663C           bne 0C70
0C34: 487A F796      pea.ex 03CC
0C38: 3F3C 04C3      movew.ex #1219, -(a7)
0C3C: 2F0A           movel a2, -(a7)
0C3E: 4E4F           trap #15
0C40: A180           sysTrapFrmGetObjectIndex
0C42: 5C4F           addqw #6, a7
0C44: 3F00           movew d0, -(a7)
0C46: 2F0A           movel a2, -(a7)
0C48: 4E4F           trap #15
0C4A: A183           sysTrapFrmGetObjectPtr
0C4C: 5C4F           addqw #6, a7
0C4E: 2F08           movel a0, -(a7)
0C50: 3F3C 04C2      movew.ex #1218, -(a7)
0C54: 2F0A           movel a2, -(a7)
0C56: 4E4F           trap #15
0C58: A180           sysTrapFrmGetObjectIndex
0C5A: 5C4F           addqw #6, a7
0C5C: 3F00           movew d0, -(a7)
0C5E: 2F0A           movel a2, -(a7)
0C60: 4E4F           trap #15
0C62: A183           sysTrapFrmGetObjectPtr
0C64: 5C4F           addqw #6, a7
0C66: 2F08           movel a0, -(a7)
0C68: 4EBA F7B8      jsr.ex 0422
0C6C: 4FEF 000C      lea 12(a7), a7
0C70: 7601           moveq #1, d3
0C72: 1003           moveb d3, d0
0C74: 4CDF 0408      movem (a7)+, <0408>
0C78: 4E5E           unlk a6
0C7A: 4E75           rts
0C7C: 9646           subrw d6, d3
0C7E: 6F72           ble 0CF2
0C80: 6D61           blt 0CE3
0C82: 7473           moveq #115, d2
0C84: 466F 726D      notw.ex 29293(a7)
0C88: 4861           dc.w #18529
0C8A: 6E64           bgt 0CF0
0C8C: 6C65           bge 0CF3
0C8E: 4576           dc.w #17782
0C90: 656E           bcs 0D00
0C92: 7400           moveq #0, d2
0C94: 0000           dc.w #0
0C96: 4E56 0000      link a6, #0
0C9A: 48E7 1020      movem <1020>, -(a7)
0C9E: 246E 0008      movel.ex 8(a6), a2
0CA2: 0C52 0017      cmpiw #23, (a2)
0CA6: 662E           bne 0CD6
0CA8: 362A 0008      movew.ex 8(a2), d3
0CAC: 3F03           movew d3, -(a7)
0CAE: 4E4F           trap #15
0CB0: A16F           sysTrapFrmInitForm
0CB2: 2448           movel a0, a2
0CB4: 2F0A           movel a2, -(a7)
0CB6: 4E4F           trap #15
0CB8: A174           sysTrapFrmSetActiveForm
0CBA: 5C4F           addqw #6, a7
0CBC: 3003           movew d3, d0
0CBE: 0440 04B0      subiw #1200, d0
0CC2: 6702           beq 0CC6
0CC4: 600C           bra 0CD2
0CC6: 487A FD92      pea.ex 0A5A
0CCA: 2F0A           movel a2, -(a7)
0CCC: 4E4F           trap #15
0CCE: A19F           sysTrapFrmSetEventHandler
0CD0: 504F           addqw #8, a7
0CD2: 7001           moveq #1, d0
0CD4: 6002           bra 0CD8
0CD6: 7000           moveq #0, d0
0CD8: 4CDF 0408      movem (a7)+, <0408>
0CDC: 4E5E           unlk a6
0CDE: 4E75           rts
0CE0: 9641           subrw d1, d3
0CE2: 7070           moveq #112, d0
0CE4: 6C69           bge 0D4F
0CE6: 6361           bls 0D49
0CE8: 7469           moveq #105, d2
0CEA: 6F6E           ble 0D5A
0CEC: 4861           dc.w #18529
0CEE: 6E64           bgt 0D54
0CF0: 6C65           bge 0D57
0CF2: 4576           dc.w #17782
0CF4: 656E           bcs 0D64
0CF6: 7400           moveq #0, d2
0CF8: 0000           dc.w #0
0CFA: 4E56 FFE8      link a6, #-24
0CFE: 4878 0064      pea.ex (0064).w
0D02: 486E FFE8      pea.ex -24(a6)
0D06: 4E4F           trap #15
0D08: A11D           sysTrapEvtGetEvent
0D0A: 486E FFE8      pea.ex -24(a6)
0D0E: 4E4F           trap #15
0D10: A0A9           sysTrapSysHandleEvent
0D12: 4A00           tstb d0
0D14: 4FEF 000C      lea 12(a7), a7
0D18: 6618           bne 0D32
0D1A: 486E FFE8      pea.ex -24(a6)
0D1E: 4EBA FF76      jsr.ex 0C96
0D22: 4A00           tstb d0
0D24: 584F           addqw #4, a7
0D26: 660A           bne 0D32
0D28: 486E FFE8      pea.ex -24(a6)
0D2C: 4E4F           trap #15
0D2E: A1A0           sysTrapFrmDispatchEvent
0D30: 584F           addqw #4, a7
0D32: 0C6E 0016 FFE8 cmpiw.ex #22, -24(a6)
0D38: 66C4           bne 0CFE
0D3A: 4E5E           unlk a6
0D3C: 4E75           rts
0D3E: 8945           dc.w #35141
0D40: 7665           moveq #101, d3
0D42: 6E74           bgt 0DB8
0D44: 4C6F           dc.w #19567
0D46: 6F70           ble 0DB8
0D48: 0000           dc.w #0
0D4A: 4E56 FFD2      link a6, #-46
0D4E: 48E7 1800      movem <1800>, -(a7)
0D52: 382E 0008      movew.ex 8(a6), d4
0D56: 3F2E 000E      movew.ex 14(a6), -(a7)
0D5A: 2F3C 0300 0000 movel.exl #50331648, -(a7)
0D60: 4EBA 00C8      jsr.ex 0E2A
0D64: 3600           movew d0, d3
0D66: 4A43           tstw d3
0D68: 5C4F           addqw #6, a7
0D6A: 6708           beq 0D74
0D6C: 7000           moveq #0, d0
0D6E: 3003           movew d3, d0
0D70: 6000 00A4      bra 0E16
0D74: 4A44           tstw d4
0D76: 6716           beq 0D8E
0D78: 0C44 000D      cmpiw #13, d4
0D7C: 6710           beq 0D8E
0D7E: 0C44 000E      cmpiw #14, d4
0D82: 670A           beq 0D8E
0D84: 203C 0000 0502 movel.exl #1282, d0
0D8A: 6000 008A      bra 0E16
0D8E: 0C44 000D      cmpiw #13, d4
0D92: 57C0           seq d0
0D94: 4400           negb d0
0D96: 4880           extw d0
0D98: 1B40 FFE9      moveb.mx d0, -23(a5)
0D9C: 4A2D FFE9      tstb.ex -23(a5)
0DA0: 6656           bne 0DF8
0DA2: 2D7C 6672 6D74 FFF8 movel.emxl #1718775156, -8(a6)
0DAA: 486E FFFC      pea.ex -4(a6)
0DAE: 486E FFF6      pea.ex -10(a6)
0DB2: 1F3C 0001      moveb.ex #1, -(a7)
0DB6: 2F3C 7072 6566 movel.exl #1886545254, -(a7)
0DBC: 2F3C 6170 706C movel.exl #1634758764, -(a7)
0DC2: 486E FFD6      pea.ex -42(a6)
0DC6: 1F3C 0001      moveb.ex #1, -(a7)
0DCA: 4E4F           trap #15
0DCC: A078           sysTrapDmGetNextDatabaseByTypeCreator
0DCE: 4AAE FFFC      tstl.ex -4(a6)
0DD2: 4FEF 0018      lea 24(a7), a7
0DD6: 6720           beq 0DF8
0DD8: 486E FFD2      pea.ex -46(a6)
0DDC: 486E FFF8      pea.ex -8(a6)
0DE0: 2F3C 0000 8001 movel.exl #32769, -(a7)
0DE6: 2F2E FFFC      movel.ex -4(a6), -(a7)
0DEA: 3F2E FFF6      movew.ex -10(a6), -(a7)
0DEE: 4E4F           trap #15
0DF0: A08E           sysTrapSysAppLaunch
0DF2: 3600           movew d0, d3
0DF4: 4FEF 0012      lea 18(a7), a7
0DF8: 4EBA F40E      jsr.ex 0208
0DFC: 3600           movew d0, d3
0DFE: 3F3C 04B0      movew.ex #1200, -(a7)
0E02: 4E4F           trap #15
0E04: A19B           sysTrapFrmGotoForm
0E06: 4A43           tstw d3
0E08: 544F           addqw #2, a7
0E0A: 6604           bne 0E10
0E0C: 4EBA FEEC      jsr.ex 0CFA
0E10: 4EBA F4D4      jsr.ex 02E6
0E14: 7000           moveq #0, d0
0E16: 4CDF 0018      movem (a7)+, <0018>
0E1A: 4E5E           unlk a6
0E1C: 4E75           rts
0E1E: 8950           ormw d4, (a0)
0E20: 696C           bvs 0E8E
0E22: 6F74           ble 0E98
0E24: 4D61           dc.w #19809
0E26: 696E           bvs 0E96
0E28: 0000           dc.w #0
0E2A: 4E56 FFD6      link a6, #-42
0E2E: 2F03           movel d3, -(a7)
0E30: 362E 000C      movew.ex 12(a6), d3
0E34: 486E FFFC      pea.ex -4(a6)
0E38: 3F3C 0001      movew.ex #1, -(a7)
0E3C: 2F3C 7073 7973 movel.exl #1886615923, -(a7)
0E42: 4E4F           trap #15
0E44: A27B           sysTrapFtrGet
0E46: 202E FFFC      movel.ex -4(a6), d0
0E4A: B0AE 0008      cmpl.ex 8(a6), d0
0E4E: 4FEF 000A      lea 10(a7), a7
0E52: 646A           bcc 0EBE
0E54: 3003           movew d3, d0
0E56: 0240 000C      andiw #12, d0
0E5A: 0C40 000C      cmpiw #12, d0
0E5E: 6658           bne 0EB8
0E60: 3F3C 03E9      movew.ex #1001, -(a7)
0E64: 4E4F           trap #15
0E66: A192           sysTrapFrmAlert
0E68: 0CAE 0200 0000 FFFC cmpil.ex #33554432, -4(a6)
0E70: 544F           addqw #2, a7
0E72: 6444           bcc 0EB8
0E74: 486E FFF8      pea.ex -8(a6)
0E78: 486E FFF6      pea.ex -10(a6)
0E7C: 1F3C 0001      moveb.ex #1, -(a7)
0E80: 2F3C 7072 6566 movel.exl #1886545254, -(a7)
0E86: 2F3C 6170 706C movel.exl #1634758764, -(a7)
0E8C: 486E FFD6      pea.ex -42(a6)
0E90: 1F3C 0001      moveb.ex #1, -(a7)
0E94: 4E4F           trap #15
0E96: A078           sysTrapDmGetNextDatabaseByTypeCreator
0E98: 4AAE FFF8      tstl.ex -8(a6)
0E9C: 4FEF 0018      lea 24(a7), a7
0EA0: 6716           beq 0EB8
0EA2: 42A7           clrl -(a7)
0EA4: 4267           clrw -(a7)
0EA6: 2F2E FFF8      movel.ex -8(a6), -(a7)
0EAA: 3F2E FFF6      movew.ex -10(a6), -(a7)
0EAE: 4E4F           trap #15
0EB0: A0A7           sysTrapSysUIAppSwitch
0EB2: 3600           movew d0, d3
0EB4: 4FEF 000C      lea 12(a7), a7
0EB8: 303C 050C      movew.ex #1292, d0
0EBC: 6002           bra 0EC0
0EBE: 7000           moveq #0, d0
0EC0: 261F           movel (a7)+, d3
0EC2: 4E5E           unlk a6
0EC4: 4E75           rts
0EC6: 9452           subrw (a2), d2
0EC8: 6F6D           ble 0F37
0ECA: 5665           addqw #3, -(a5)
0ECC: 7273           moveq #115, d1
0ECE: 696F           bvs 0F3F
0ED0: 6E43           bgt 0F15
0ED2: 6F6D           ble 0F41
0ED4: 7061           moveq #97, d0
0ED6: 7469           moveq #105, d2
0ED8: 626C           bhi 0F46
0EDA: 6500 0000      bcs 0EDC

