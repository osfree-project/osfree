0168: 0000 0001      orib #1, d0
016C: 487A 0004      pea.ex 30074(pc)
0170: 0697 0000 0006 addil #6, (a7)
0176: 4E75           rts
0178: 4E56 FFF4      link a6, #-12
017C: 48E7 1800      movem <1800>, -(a7)
0180: 486E FFF4      pea.ex -12(a6)
0184: 486E FFF8      pea.ex -8(a6)
0188: 486E FFFC      pea.ex -4(a6)
018C: 4E4F           trap #15
018E: A08F           sysTrapSysAppStartup
0190: 3800           movew d0, d4
0192: 4A44           tstw d4
0194: 4FEF 000C      lea 12(a7), d7
0198: 671C           beq 28
019A: 41FA 0066      lea 30218(pc), d0
019E: 4850           pea (a0)
01A0: 3F3C 005C      movew.ex #92, -(a7)
01A4: 41FA 0078      lea 30246(pc), d0
01A8: 4850           pea (a0)
01AA: 4E4F           trap #15
01AC: A084           sysTrapErrDisplayFileLineMsg
01AE: 7000           moveq #0, d0
01B0: 4FEF 000A      lea 10(a7), d7
01B4: 6036           bra 54
01B6: 206E FFFC      movel.ex -4(a6), a0
01BA: 3F28 0006      movew.ex 6(a0), -(a7)
01BE: 2F28 0002      movel.ex 2(a0), -(a7)
01C2: 3F10           movew (a0), -(a7)
01C4: 487A 000E      pea.ex 30172(pc)
01C8: 487A 0004      pea.ex 30166(pc)
01CC: 0697 0000 2C58 addil #11352, (a7)
01D2: 4E75           rts
01D4: 2600           movel d0, d3
01D6: 2F2E FFF4      movel.ex -12(a6), -(a7)
01DA: 2F2E FFF8      movel.ex -8(a6), -(a7)
01DE: 2F2E FFFC      movel.ex -4(a6), -(a7)
01E2: 4E4F           trap #15
01E4: A090           sysTrapSysAppExit
01E6: 2003           movel d3, d0
01E8: 4FEF 0014      lea 20(a7), d7
01EC: 4CDF 0018      movem (a7)+, <0018>
01F0: 4E5E           unlk a6
01F2: 4E75           rts
01F4: 8B5F           ormw d5, (a7)+
01F6: 5F53           subqw #7, (a3)
01F8: 7461           moveq #97, d2
01FA: 7274           moveq #116, d1
01FC: 7570           dc.w #30064
01FE: 5F5F           subqw #7, (a7)+
0200: 002A           dc.w #42
0202: 4572           dc.w #17778
0204: 726F           moveq #111, d1
0206: 7220           moveq #32, d1
0208: 6C61           bge 97
020A: 756E           dc.w #30062
020C: 6368           bls 104
020E: 696E           bvs 110
0210: 6720           beq 32
0212: 6170           bsr 112
0214: 706C           moveq #108, d0
0216: 6963           bvs 99
0218: 6174           bsr 116
021A: 696F           bvs 111
021C: 6E00 5374      bgt 21364
0220: 6172           bsr 114
0222: 7475           moveq #117, d2
0224: 7043           moveq #67, d0
0226: 6F64           ble 100
0228: 652E           bcs 46
022A: 6300 4E56      bls 20054
022E: FFA2           dc.w #65442
0230: 48E7 1F00      movem <1f00>, -(a7)
0234: 7600           moveq #0, d3
0236: 42AE FFF4      clrl.ex -12(a6)
023A: 7A00           moveq #0, d5
023C: 486E FFA2      pea.ex -94(a6)
0240: 4E4F           trap #15
0242: A22C           sysTrapPrefGetPreferences
0244: 1B6E FFAE FFE1 moveb.emx -82(a6), -31(a5)
024A: 4A2D FFE1      tstb.ex -31(a5)
024E: 584F           addqw #4, a7
0250: 6704           beq 4
0252: 7803           moveq #3, d4
0254: 6002           bra 2
0256: 7813           moveq #19, d4
0258: 3F04           movew d4, -(a7)
025A: 2F3C 6D65 6D6F movel.exl #1835363695, -(a7)
0260: 2F3C 4441 5441 movel.exl #1145132097, -(a7)
0266: 4E4F           trap #15
0268: A075           sysTrapDmOpenDatabaseByTypeCreator
026A: 2B48 FFF4      movel.mx a0, -12(a5)
026E: 4AAD FFF4      tstl.ex -12(a5)
0272: 4FEF 000A      lea 10(a7), d7
0276: 6600 00E6      bne 230
027A: 4227           clrb -(a7)
027C: 2F3C 4441 5441 movel.exl #1145132097, -(a7)
0282: 2F3C 6D65 6D6F movel.exl #1835363695, -(a7)
0288: 41FA 01CA      lea 30812(pc), d0
028C: 4850           pea (a0)
028E: 4267           clrw -(a7)
0290: 4E4F           trap #15
0292: A041           sysTrapDmCreateDatabase
0294: 3600           movew d0, d3
0296: 4A43           tstw d3
0298: 4FEF 0010      lea 16(a7), d7
029C: 6706           beq 6
029E: 3003           movew d3, d0
02A0: 6000 0196      bra 406
02A4: 3F04           movew d4, -(a7)
02A6: 2F3C 6D65 6D6F movel.exl #1835363695, -(a7)
02AC: 2F3C 4441 5441 movel.exl #1145132097, -(a7)
02B2: 4E4F           trap #15
02B4: A075           sysTrapDmOpenDatabaseByTypeCreator
02B6: 2B48 FFF4      movel.mx a0, -12(a5)
02BA: 4AAD FFF4      tstl.ex -12(a5)
02BE: 4FEF 000A      lea 10(a7), d7
02C2: 6606           bne 6
02C4: 7001           moveq #1, d0
02C6: 6000 0170      bra 368
02CA: 42A7           clrl -(a7)
02CC: 486E FFFE      pea.ex -2(a6)
02D0: 42A7           clrl -(a7)
02D2: 42A7           clrl -(a7)
02D4: 486E FFFA      pea.ex -6(a6)
02D8: 2F2D FFF4      movel.ex -12(a5), -(a7)
02DC: 4E4F           trap #15
02DE: A04C           sysTrapDmOpenDatabaseInfo
02E0: 42A7           clrl -(a7)
02E2: 42A7           clrl -(a7)
02E4: 42A7           clrl -(a7)
02E6: 42A7           clrl -(a7)
02E8: 42A7           clrl -(a7)
02EA: 42A7           clrl -(a7)
02EC: 42A7           clrl -(a7)
02EE: 42A7           clrl -(a7)
02F0: 42A7           clrl -(a7)
02F2: 486E FFF8      pea.ex -8(a6)
02F6: 42A7           clrl -(a7)
02F8: 2F2E FFFA      movel.ex -6(a6), -(a7)
02FC: 3F2E FFFE      movew.ex -2(a6), -(a7)
0300: 4E4F           trap #15
0302: A046           sysTrapDmDatabaseInfo
0304: 006E 0008 FFF8 oriw.ex #8, -8(a6)
030A: 42A7           clrl -(a7)
030C: 42A7           clrl -(a7)
030E: 42A7           clrl -(a7)
0310: 42A7           clrl -(a7)
0312: 42A7           clrl -(a7)
0314: 42A7           clrl -(a7)
0316: 42A7           clrl -(a7)
0318: 42A7           clrl -(a7)
031A: 42A7           clrl -(a7)
031C: 486E FFF8      pea.ex -8(a6)
0320: 42A7           clrl -(a7)
0322: 2F2E FFFA      movel.ex -6(a6), -(a7)
0326: 3F2E FFFE      movew.ex -2(a6), -(a7)
032A: 4E4F           trap #15
032C: A047           sysTrapDmSetDatabaseInfo
032E: 2F2D FFF4      movel.ex -12(a5), -(a7)
0332: 4EBA 2DF6      jsr.ex -23246(pc)
0336: 3600           movew d0, d3
0338: 4A43           tstw d3
033A: 4FEF 0080      lea 128(a7), d7
033E: 671E           beq 30
0340: 2F2D FFF4      movel.ex -12(a5), -(a7)
0344: 4E4F           trap #15
0346: A04A           sysTrapDmCloseDatabase
0348: 2F2E FFFA      movel.ex -6(a6), -(a7)
034C: 3F2E FFFE      movew.ex -2(a6), -(a7)
0350: 4E4F           trap #15
0352: A042           sysTrapDmDeleteDatabase
0354: 3003           movew d3, d0
0356: 4FEF 000A      lea 10(a7), d7
035A: 6000 00DC      bra 220
035E: 486E FFF4      pea.ex -12(a6)
0362: 4EBA 01B8      jsr.ex 31012(pc)
0366: 486D FFE4      pea.ex -28(a5)
036A: 3F2D FFFC      movew.ex -4(a5), -(a7)
036E: 2F2D FFF4      movel.ex -12(a5), -(a7)
0372: 4E4F           trap #15
0374: A104           sysTrapCategoryGetName
0376: 4A2D FFE4      tstb.ex -28(a5)
037A: 4FEF 000E      lea 14(a7), d7
037E: 660C           bne 12
0380: 3B7C 00FF FFFC movew.emx #255, -4(a5)
0386: 1B7C 0001 FFFE moveb.emx #1, -2(a5)
038C: 3F2D FFF8      movew.ex -8(a5), -(a7)
0390: 2F2D FFF4      movel.ex -12(a5), -(a7)
0394: 4E4F           trap #15
0396: A05B           sysTrapDmQueryRecord
0398: 2008           movel a0, d0
039A: 5C4F           addqw #6, a7
039C: 6754           beq 84
039E: 42A7           clrl -(a7)
03A0: 486E FFEE      pea.ex -18(a6)
03A4: 486E FFF2      pea.ex -14(a6)
03A8: 3F2D FFF8      movew.ex -8(a5), -(a7)
03AC: 2F2D FFF4      movel.ex -12(a5), -(a7)
03B0: 4E4F           trap #15
03B2: A050           sysTrapDmRecordInfo
03B4: 7C00           moveq #0, d6
03B6: 202E FFEE      movel.ex -18(a6), d0
03BA: B0AE FFF4      cmpl.ex -12(a6), d0
03BE: 57C0           seq d0
03C0: 4400           negb d0
03C2: 4880           extw d0
03C4: 4FEF 0012      lea 18(a7), d7
03C8: 6726           beq 38
03CA: 7E01           moveq #1, d7
03CC: 102D FFE1      moveb.ex -31(a5), d0
03D0: 57C0           seq d0
03D2: 4400           negb d0
03D4: 4880           extw d0
03D6: 6612           bne 18
03D8: 302E FFF2      movew.ex -14(a6), d0
03DC: 0240 0010      andiw #16, d0
03E0: 57C0           seq d0
03E2: 4400           negb d0
03E4: 4880           extw d0
03E6: 6602           bne 2
03E8: 7E00           moveq #0, d7
03EA: 4A47           tstw d7
03EC: 6702           beq 2
03EE: 7C01           moveq #1, d6
03F0: 1A06           moveb d6, d5
03F2: 4A05           tstb d5
03F4: 6614           bne 20
03F6: 426D FFD2      clrw.ex -46(a5)
03FA: 3B7C FFFF FFF8 movew.emx #65535, -8(a5)
0400: 3B7C 03E8 FFFA movew.emx #1000, -6(a5)
0406: 426D FFD6      clrw.ex -42(a5)
040A: 4A2D FFFE      tstb.ex -2(a5)
040E: 6714           beq 20
0410: 3F3C 00FF      movew.ex #255, -(a7)
0414: 2F2D FFF4      movel.ex -12(a5), -(a7)
0418: 4E4F           trap #15
041A: A071           sysTrapDmNumRecordsInCategory
041C: 3B40 FFE2      movew.mx d0, -30(a5)
0420: 5C4F           addqw #6, a7
0422: 6012           bra 18
0424: 3F2D FFFC      movew.ex -4(a5), -(a7)
0428: 2F2D FFF4      movel.ex -12(a5), -(a7)
042C: 4E4F           trap #15
042E: A071           sysTrapDmNumRecordsInCategory
0430: 3B40 FFE2      movew.mx d0, -30(a5)
0434: 5C4F           addqw #6, a7
0436: 3003           movew d3, d0
0438: 4CDF 00F8      movem (a7)+, <00f8>
043C: 4E5E           unlk a6
043E: 4E75           rts
0440: 9053           subrw (a3), d0
0442: 7461           moveq #97, d2
0444: 7274           moveq #116, d1
0446: 4170           dc.w #16752
0448: 706C           moveq #108, d0
044A: 6963           bvs 99
044C: 6174           bsr 116
044E: 696F           bvs 111
0450: 6E00 0008      bgt 8
0454: 4D65           dc.w #19813
0456: 6D6F           blt 111
0458: 4442           negw d2
045A: 0000           dc.w #0
045C: 4E56 0000      link a6, #0
0460: 48E7 1030      movem <1030>, -(a7)
0464: 7600           moveq #0, d3
0466: 0C6D 044C FFFA cmpiw.ex #1100, -6(a5)
046C: 6634           bne 52
046E: 0C6D FFFF FFF8 cmpiw.ex #-1, -8(a5)
0474: 672C           beq 44
0476: 3F3C 044C      movew.ex #1100, -(a7)
047A: 4E4F           trap #15
047C: A17E           sysTrapFrmGetFormPtr
047E: 2448           movel a0, a2
0480: 3F3C 0455      movew.ex #1109, -(a7)
0484: 2F0A           movel a2, -(a7)
0486: 4E4F           trap #15
0488: A180           sysTrapFrmGetObjectIndex
048A: 5C4F           addqw #6, a7
048C: 3F00           movew d0, -(a7)
048E: 2F0A           movel a2, -(a7)
0490: 4E4F           trap #15
0492: A183           sysTrapFrmGetObjectPtr
0494: 2648           movel a0, a3
0496: 2F0B           movel a3, -(a7)
0498: 4E4F           trap #15
049A: A148           sysTrapFldGetScrollPosition
049C: 3600           movew d0, d3
049E: 4FEF 000C      lea 12(a7), d7
04A2: 4E4F           trap #15
04A4: A1A1           sysTrapFrmCloseAllForms
04A6: 3F03           movew d3, -(a7)
04A8: 4EBA 0158      jsr.ex 31242(pc)
04AC: 2F2D FFF4      movel.ex -12(a5), -(a7)
04B0: 4E4F           trap #15
04B2: A04A           sysTrapDmCloseDatabase
04B4: 5C4F           addqw #6, a7
04B6: 4CDF 0C08      movem (a7)+, <0c08>
04BA: 4E5E           unlk a6
04BC: 4E75           rts
04BE: 8F53           ormw d7, (a3)
04C0: 746F           moveq #111, d2
04C2: 7041           moveq #65, d0
04C4: 7070           moveq #112, d0
04C6: 6C69           bge 105
04C8: 6361           bls 97
04CA: 7469           moveq #105, d2
04CC: 6F6E           ble 110
04CE: 0000           dc.w #0
04D0: 4E56 0000      link a6, #0
04D4: 2F0A           movel a2, -(a7)
04D6: 3F3C 0003      movew.ex #3, -(a7)
04DA: 2F3C 6D65 6D6F movel.exl #1835363695, -(a7)
04E0: 2F3C 4441 5441 movel.exl #1145132097, -(a7)
04E6: 4E4F           trap #15
04E8: A075           sysTrapDmOpenDatabaseByTypeCreator
04EA: 2448           movel a0, a2
04EC: 200A           movel a2, d0
04EE: 4FEF 000A      lea 10(a7), d7
04F2: 670E           beq 14
04F4: 2F0A           movel a2, -(a7)
04F6: 4EBA 2F56      jsr.ex -22442(pc)
04FA: 2F0A           movel a2, -(a7)
04FC: 4E4F           trap #15
04FE: A04A           sysTrapDmCloseDatabase
0500: 504F           addqw #0, a7
0502: 245F           movel (a7)+, a2
0504: 4E5E           unlk a6
0506: 4E75           rts
0508: 9053           subrw (a3), d0
050A: 796E           dc.w #31086
050C: 634E           bls 78
050E: 6F74           ble 116
0510: 6966           bvs 102
0512: 6963           bvs 99
0514: 6174           bsr 116
0516: 696F           bvs 111
0518: 6E00 0000      bgt 0
051C: 4E56 FFE8      link a6, #-24
0520: 48E7 1020      movem <1020>, -(a7)
0524: 246E 0008      movel.ex 8(a6), a2
0528: 3D7C 0016 FFE8 movew.emx #22, -24(a6)
052E: 1F3C 0001      moveb.ex #1, -(a7)
0532: 486E FFE8      pea.ex -24(a6)
0536: 486E FFEA      pea.ex -22(a6)
053A: 4267           clrw -(a7)
053C: 2F3C 6D65 6D6F movel.exl #1835363695, -(a7)
0542: 4E4F           trap #15
0544: A2D3           sysTrapPrefGetAppPreferences
0546: 3600           movew d0, d3
0548: 0C43 0003      cmpiw #3, d3
054C: 4FEF 0010      lea 16(a7), d7
0550: 6F02           ble 2
0552: 76FF           moveq #255, d3
0554: 0C43 FFFF      cmpiw #-1, d3
0558: 6F6E           ble 110
055A: 0C43 0002      cmpiw #2, d3
055E: 6C06           bge 6
0560: 1D6D FFD4 FFFD moveb.emx -44(a5), -3(a6)
0566: 0C43 0003      cmpiw #3, d3
056A: 6C0C           bge 12
056C: 1D6E FFF2 FFFE moveb.emx -14(a6), -2(a6)
0572: 1D6E FFFD FFFF moveb.emx -3(a6), -1(a6)
0578: 3B6E FFEA FFD2 movew.emx -22(a6), -46(a5)
057E: 3B6E FFEC FFF8 movew.emx -20(a6), -8(a5)
0584: 3B6E FFEE FFFA movew.emx -18(a6), -6(a5)
058A: 3B6E FFF0 FFFC movew.emx -16(a6), -4(a5)
0590: 0C2E 0002 FFFE cmpib.ex #2, -2(a6)
0596: 6608           bne 8
0598: 1B7C 0007 FFD5 moveb.emx #7, -43(a5)
059E: 6006           bra 6
05A0: 1B6E FFFE FFD5 moveb.emx -2(a6), -43(a5)
05A6: 3B6E FFF4 FFD6 movew.emx -12(a6), -42(a5)
05AC: 1B6E FFF6 FFFE moveb.emx -10(a6), -2(a5)
05B2: 1B6E FFFC FFFF moveb.emx -4(a6), -1(a5)
05B8: 24AE FFF8      movel.ex -8(a6), (a2)
05BC: 0C43 0002      cmpiw #2, d3
05C0: 6D06           blt 6
05C2: 1B6E FFFF FFD4 moveb.emx -1(a6), -44(a5)
05C8: 0C43 0003      cmpiw #3, d3
05CC: 6718           beq 24
05CE: 41FA 002E      lea 31238(pc), d0
05D2: 4850           pea (a0)
05D4: 3F3C FFFD      movew.ex #65533, -(a7)
05D8: 2F3C 6D65 6D6F movel.exl #1835363695, -(a7)
05DE: 4E4F           trap #15
05E0: A30F           ù„øê
05E2: 4FEF 000A      lea 10(a7), d7
05E6: 4CDF 0408      movem (a7)+, <0408>
05EA: 4E5E           unlk a6
05EC: 4E75           rts
05EE: 8D4D           dc.w #36173
05F0: 656D           bcs 109
05F2: 6F4C           ble 76
05F4: 6F61           ble 97
05F6: 6450           bcc 80
05F8: 7265           moveq #101, d1
05FA: 6673           bne 115
05FC: 0004           dc.w #4
05FE: 7478           moveq #120, d2
0600: 7400           moveq #0, d2
0602: 4E56 FFE6      link a6, #-26
0606: 3D6D FFD2 FFEA movew.emx -46(a5), -22(a6)
060C: 3D6D FFF8 FFEC movew.emx -8(a5), -20(a6)
0612: 3D6D FFFA FFEE movew.emx -6(a5), -18(a6)
0618: 3D6D FFFC FFF0 movew.emx -4(a5), -16(a6)
061E: 1D6D FFD5 FFFE moveb.emx -43(a5), -2(a6)
0624: 1D6D FFFE FFF6 moveb.emx -2(a5), -10(a6)
062A: 3D6E 0008 FFF4 movew.emx 8(a6), -12(a6)
0630: 1D6D FFFF FFFC moveb.emx -1(a5), -4(a6)
0636: 1D6D FFD4 FFFF moveb.emx -44(a5), -1(a6)
063C: 422E FFF2      clrb.ex -14(a6)
0640: 422E FFFD      clrb.ex -3(a6)
0644: 3F2D FFF8      movew.ex -8(a5), -(a7)
0648: 2F2D FFF4      movel.ex -12(a5), -(a7)
064C: 4E4F           trap #15
064E: A05B           sysTrapDmQueryRecord
0650: 2008           movel a0, d0
0652: 5C4F           addqw #6, a7
0654: 6720           beq 32
0656: 42A7           clrl -(a7)
0658: 486E FFE6      pea.ex -26(a6)
065C: 42A7           clrl -(a7)
065E: 3F2D FFF8      movew.ex -8(a5), -(a7)
0662: 2F2D FFF4      movel.ex -12(a5), -(a7)
0666: 4E4F           trap #15
0668: A050           sysTrapDmRecordInfo
066A: 2D6E FFE6 FFF8 movel.emx -26(a6), -8(a6)
0670: 4FEF 0012      lea 18(a7), d7
0674: 6006           bra 6
0676: 70FF           moveq #255, d0
0678: 2D40 FFF8      movel.mx d0, -8(a6)
067C: 1F3C 0001      moveb.ex #1, -(a7)
0680: 3F3C 0016      movew.ex #22, -(a7)
0684: 486E FFEA      pea.ex -22(a6)
0688: 4878 0003      pea.ex (0003).w
068C: 2F3C 6D65 6D6F movel.exl #1835363695, -(a7)
0692: 4E4F           trap #15
0694: A2D4           sysTrapPrefSetAppPreferences
0696: 4E5E           unlk a6
0698: 4E75           rts
069A: 8D4D           dc.w #36173
069C: 656D           bcs 109
069E: 6F53           ble 83
06A0: 6176           bsr 118
06A2: 6550           bcs 80
06A4: 7265           moveq #101, d1
06A6: 6673           bne 115
06A8: 0000           dc.w #0
06AA: 4E56 0000      link a6, #0
06AE: 2F0A           movel a2, -(a7)
06B0: 4E4F           trap #15
06B2: A173           sysTrapFrmGetActiveForm
06B4: 2448           movel a0, a2
06B6: 3F2E 0008      movew.ex 8(a6), -(a7)
06BA: 2F0A           movel a2, -(a7)
06BC: 4E4F           trap #15
06BE: A180           sysTrapFrmGetObjectIndex
06C0: 5C4F           addqw #6, a7
06C2: 3F00           movew d0, -(a7)
06C4: 2F0A           movel a2, -(a7)
06C6: 4E4F           trap #15
06C8: A183           sysTrapFrmGetObjectPtr
06CA: 5C4F           addqw #6, a7
06CC: 245F           movel (a7)+, a2
06CE: 4E5E           unlk a6
06D0: 4E75           rts
06D2: 8C47           orrw d7, d6
06D4: 6574           bcs 116
06D6: 4F62           dc.w #20322
06D8: 6A65           bpl 101
06DA: 6374           bls 116
06DC: 5074 7200      addqw.ex #0, 0(a4,d7.w)
06E0: 0000           dc.w #0
06E2: 4E56 0000      link a6, #0
06E6: 48E7 1020      movem <1020>, -(a7)
06EA: 4E4F           trap #15
06EC: A173           sysTrapFrmGetActiveForm
06EE: 2448           movel a0, a2
06F0: 2F0A           movel a2, -(a7)
06F2: 4E4F           trap #15
06F4: A178           sysTrapFrmGetFocus
06F6: 3600           movew d0, d3
06F8: 0C43 FFFF      cmpiw #-1, d3
06FC: 584F           addqw #4, a7
06FE: 6604           bne 4
0700: 91C8           subal a0, a0
0702: 600A           bra 10
0704: 3F03           movew d3, -(a7)
0706: 2F0A           movel a2, -(a7)
0708: 4E4F           trap #15
070A: A183           sysTrapFrmGetObjectPtr
070C: 5C4F           addqw #6, a7
070E: 4CDF 0408      movem (a7)+, <0408>
0712: 4E5E           unlk a6
0714: 4E75           rts
0716: 9147           subxrw d7, d0
0718: 6574           bcs 116
071A: 466F 6375      notw.ex 25461(a7)
071E: 734F           dc.w #29519
0720: 626A           bhi 106
0722: 6563           bcs 99
0724: 7450           moveq #80, d2
0726: 7472           moveq #114, d2
0728: 0000           dc.w #0
072A: 4E56 0000      link a6, #0
072E: 48E7 1820      movem <1820>, -(a7)
0732: 246E 0008      movel.ex 8(a6), a2
0736: 382E 000C      movew.ex 12(a6), d4
073A: 362E 000E      movew.ex 14(a6), d3
073E: 3F2D FFFC      movew.ex -4(a5), -(a7)
0742: 3F03           movew d3, -(a7)
0744: 3F04           movew d4, -(a7)
0746: 2F0A           movel a2, -(a7)
0748: 2F2D FFF4      movel.ex -12(a5), -(a7)
074C: 4E4F           trap #15
074E: A073           sysTrapDmSeekRecordInCategory
0750: 4E4F           trap #15
0752: A04E           sysTrapDmGetLastErr
0754: 4A40           tstw d0
0756: 4FEF 000E      lea 14(a7), d7
075A: 6704           beq 4
075C: 7000           moveq #0, d0
075E: 6002           bra 2
0760: 7001           moveq #1, d0
0762: 4CDF 0418      movem (a7)+, <0418>
0766: 4E5E           unlk a6
0768: 4E75           rts
076A: 8A53           orrw (a3), d5
076C: 6565           bcs 101
076E: 6B52           bmi 82
0770: 6563           bcs 99
0772: 6F72           ble 114
0774: 6400 0000      bcc 0
0778: 4E56 0000      link a6, #0
077C: 2F03           movel d3, -(a7)
077E: 362E 0008      movew.ex 8(a6), d3
0782: 4A2D FFFE      tstb.ex -2(a5)
0786: 6714           beq 20
0788: 3F3C 00FF      movew.ex #255, -(a7)
078C: 2F2D FFF4      movel.ex -12(a5), -(a7)
0790: 4E4F           trap #15
0792: A071           sysTrapDmNumRecordsInCategory
0794: 3B40 FFE2      movew.mx d0, -30(a5)
0798: 5C4F           addqw #6, a7
079A: 6010           bra 16
079C: 3F03           movew d3, -(a7)
079E: 2F2D FFF4      movel.ex -12(a5), -(a7)
07A2: 4E4F           trap #15
07A4: A071           sysTrapDmNumRecordsInCategory
07A6: 3B40 FFE2      movew.mx d0, -30(a5)
07AA: 5C4F           addqw #6, a7
07AC: 3B43 FFFC      movew.mx d3, -4(a5)
07B0: 426D FFD2      clrw.ex -46(a5)
07B4: 261F           movel (a7)+, d3
07B6: 4E5E           unlk a6
07B8: 4E75           rts
07BA: 8E43           orrw d3, d7
07BC: 6861           bvc 97
07BE: 6E67           bgt 103
07C0: 6543           bcs 67
07C2: 6174           bsr 116
07C4: 6567           bcs 103
07C6: 6F72           ble 114
07C8: 7900           dc.w #30976
07CA: 0000           dc.w #0
07CC: 4E56 FFFA      link a6, #-6
07D0: 48E7 1E30      movem <1e30>, -(a7)
07D4: 246E 0008      movel.ex 8(a6), a2
07D8: 362E 000C      movew.ex 12(a6), d3
07DC: 3C2E 000E      movew.ex 14(a6), d6
07E0: 382E 0010      movew.ex 16(a6), d4
07E4: 3F3C 000A      movew.ex #10, -(a7)
07E8: 2F0A           movel a2, -(a7)
07EA: 4E4F           trap #15
07EC: A0CC           sysTrapStrChr
07EE: 2648           movel a0, a3
07F0: 200B           movel a3, d0
07F2: 5C4F           addqw #6, a7
07F4: 6708           beq 8
07F6: 204B           movel a3, a0
07F8: 91CA           subal a2, a0
07FA: 2A08           movel a0, d5
07FC: 600A           bra 10
07FE: 2F0A           movel a2, -(a7)
0800: 4E4F           trap #15
0802: A0C7           sysTrapStrLen
0804: 3A00           movew d0, d5
0806: 584F           addqw #4, a7
0808: 3D44 FFFC      movew.mx d4, -4(a6)
080C: 3D45 FFFE      movew.mx d5, -2(a6)
0810: 486E FFFB      pea.ex -5(a6)
0814: 486E FFFE      pea.ex -2(a6)
0818: 486E FFFC      pea.ex -4(a6)
081C: 2F0A           movel a2, -(a7)
081E: 4E4F           trap #15
0820: A16D           sysTrapFntCharsInWidth
0822: 4A2E FFFB      tstb.ex -5(a6)
0826: 4FEF 0010      lea 16(a7), d7
082A: 6714           beq 20
082C: 3F06           movew d6, -(a7)
082E: 3F03           movew d3, -(a7)
0830: 3F2E FFFE      movew.ex -2(a6), -(a7)
0834: 2F0A           movel a2, -(a7)
0836: 4E4F           trap #15
0838: A220           sysTrapWinDrawChars
083A: 4FEF 000A      lea 10(a7), d7
083E: 6072           bra 114
0840: 1F3C 002E      moveb.ex #46, -(a7)
0844: 4E4F           trap #15
0846: A16A           sysTrapFntCharWidth
0848: C1FC 0003      muls.ex #3, d0
084C: 9840           subrw d0, d4
084E: 544F           addqw #2, a7
0850: 6016           bra 22
0852: 536E FFFE      subqw.ex #1, -2(a6)
0856: 302E FFFE      movew.ex -2(a6), d0
085A: 1F32 0000      moveb.ex 0(a2,d0.w), -(a7)
085E: 4E4F           trap #15
0860: A16A           sysTrapFntCharWidth
0862: 916E FFFC      submw.ex d0, -4(a6)
0866: 544F           addqw #2, a7
0868: B86E FFFC      cmpw.ex -4(a6), d4
086C: 6DE4           blt -28
086E: 302E FFFE      movew.ex -2(a6), d0
0872: 5340           subqw #1, d0
0874: 0C32 0020 0000 cmpib.ex #32, 0(a2,d0.w)
087A: 67D6           beq -42
087C: 302E FFFE      movew.ex -2(a6), d0
0880: 5340           subqw #1, d0
0882: 0C32 0009 0000 cmpib.ex #9, 0(a2,d0.w)
0888: 67C8           beq -56
088A: 3F06           movew d6, -(a7)
088C: 3F03           movew d3, -(a7)
088E: 3F2E FFFE      movew.ex -2(a6), -(a7)
0892: 2F0A           movel a2, -(a7)
0894: 4E4F           trap #15
0896: A220           sysTrapWinDrawChars
0898: D66E FFFC      addrw.ex -4(a6), d3
089C: 3F06           movew d6, -(a7)
089E: 3F03           movew d3, -(a7)
08A0: 3F3C 0003      movew.ex #3, -(a7)
08A4: 41FA 0024      lea 31954(pc), d0
08A8: 4850           pea (a0)
08AA: 4E4F           trap #15
08AC: A220           sysTrapWinDrawChars
08AE: 4FEF 0014      lea 20(a7), d7
08B2: 4CDF 0C78      movem (a7)+, <0c78>
08B6: 4E5E           unlk a6
08B8: 4E75           rts
08BA: 8D44           dc.w #36164
08BC: 7261           moveq #97, d1
08BE: 774D           dc.w #30541
08C0: 656D           bcs 109
08C2: 6F54           ble 84
08C4: 6974           bvs 116
08C6: 6C65           bge 101
08C8: 0004           dc.w #4
08CA: 2E2E 2E00      movel.ex 11776(a6), d7
08CE: 4E56 FFEC      link a6, #-20
08D2: 48E7 1F38      movem <1f38>, -(a7)
08D6: 266E 0008      movel.ex 8(a6), a3
08DA: 426E FFF4      clrw.ex -12(a6)
08DE: 244B           movel a3, a2
08E0: 3F12           movew (a2), -(a7)
08E2: 2F3C 6D65 6D6F movel.exl #1835363695, -(a7)
08E8: 2F3C 4441 5441 movel.exl #1145132097, -(a7)
08EE: 4E4F           trap #15
08F0: A075           sysTrapDmOpenDatabaseByTypeCreator
08F2: 2848           movel a0, a4
08F4: 200C           movel a4, d0
08F6: 4FEF 000A      lea 10(a7), d7
08FA: 6608           bne 8
08FC: 422B 0004      clrb.ex 4(a3)
0900: 6000 0120      bra 288
0904: 42A7           clrl -(a7)
0906: 486E FFF4      pea.ex -12(a6)
090A: 42A7           clrl -(a7)
090C: 42A7           clrl -(a7)
090E: 486E FFEC      pea.ex -20(a6)
0912: 2F0C           movel a4, -(a7)
0914: 4E4F           trap #15
0916: A04C           sysTrapDmOpenDatabaseInfo
0918: 3F3C 0064      movew.ex #100, -(a7)
091C: 2F3C 7453 5452 movel.exl #1951618130, -(a7)
0922: 4E4F           trap #15
0924: A05F           sysTrapDmGetResource
0926: 2A08           movel a0, d5
0928: 2F05           movel d5, -(a7)
092A: 4E4F           trap #15
092C: A021           sysTrapMemHandleLock
092E: 2E08           movel a0, d7
0930: 2F07           movel d7, -(a7)
0932: 2F0B           movel a3, -(a7)
0934: 4E4F           trap #15
0936: A26D           sysTrapFindDrawHeader
0938: 1600           moveb d0, d3
093A: 2F05           movel d5, -(a7)
093C: 4E4F           trap #15
093E: A022           sysTrapMemHandleUnlock
0940: 4A03           tstb d3
0942: 4FEF 002E      lea 46(a7), d7
0946: 6600 00D2      bne 210
094A: 3D6A 0002 FFFE movew.emx 2(a2), -2(a6)
0950: 302E FFFE      movew.ex -2(a6), d0
0954: 0240 000F      andiw #15, d0
0958: 6618           bne 24
095A: 1F3C 0001      moveb.ex #1, -(a7)
095E: 4E4F           trap #15
0960: A2CD           sysTrapEvtSysEventAvail
0962: 4A00           tstb d0
0964: 544F           addqw #2, a7
0966: 670A           beq 10
0968: 157C 0001 0004 moveb.emx #1, 4(a2)
096E: 6000 00AA      bra 170
0972: 3F3C 00FF      movew.ex #255, -(a7)
0976: 486E FFFE      pea.ex -2(a6)
097A: 2F0C           movel a4, -(a7)
097C: 4E4F           trap #15
097E: A070           sysTrapDmQueryNextInCategory
0980: 2808           movel a0, d4
0982: 4A84           tstl d4
0984: 4FEF 000A      lea 10(a7), d7
0988: 6608           bne 8
098A: 422A 0004      clrb.ex 4(a2)
098E: 6000 008A      bra 138
0992: 2F04           movel d4, -(a7)
0994: 4E4F           trap #15
0996: A021           sysTrapMemHandleLock
0998: 2C08           movel a0, d6
099A: 486E FFF2      pea.ex -14(a6)
099E: 486A 0016      pea.ex 22(a2)
09A2: 2F06           movel d6, -(a7)
09A4: 4E4F           trap #15
09A6: A26A           sysTrapFindStrInStr
09A8: 1D40 FFF1      moveb.mx d0, -15(a6)
09AC: 4A2E FFF1      tstb.ex -15(a6)
09B0: 4FEF 0010      lea 16(a7), d7
09B4: 6750           beq 80
09B6: 2F2E FFEC      movel.ex -20(a6), -(a7)
09BA: 3F2E FFF4      movew.ex -12(a6), -(a7)
09BE: 42A7           clrl -(a7)
09C0: 4267           clrw -(a7)
09C2: 3F2E FFF2      movew.ex -14(a6), -(a7)
09C6: 3F2E FFFE      movew.ex -2(a6), -(a7)
09CA: 2F0B           movel a3, -(a7)
09CC: 4E4F           trap #15
09CE: A26B           sysTrapFindSaveMatch
09D0: 1600           moveb d0, d3
09D2: 4A03           tstb d3
09D4: 4FEF 0014      lea 20(a7), d7
09D8: 662C           bne 44
09DA: 486E FFF6      pea.ex -10(a6)
09DE: 2F0B           movel a3, -(a7)
09E0: 4E4F           trap #15
09E2: A26C           sysTrapFindGetLineBounds
09E4: 302E FFFA      movew.ex -6(a6), d0
09E8: 5540           subqw #2, d0
09EA: 3F00           movew d0, -(a7)
09EC: 3F2E FFF8      movew.ex -8(a6), -(a7)
09F0: 302E FFF6      movew.ex -10(a6), d0
09F4: 5240           addqw #1, d0
09F6: 3F00           movew d0, -(a7)
09F8: 2F06           movel d6, -(a7)
09FA: 4EBA FDD0      jsr.ex 31700(pc)
09FE: 526A 002A      addqw.ex #1, 42(a2)
0A02: 4FEF 0012      lea 18(a7), d7
0A06: 2F04           movel d4, -(a7)
0A08: 4E4F           trap #15
0A0A: A022           sysTrapMemHandleUnlock
0A0C: 4A03           tstb d3
0A0E: 584F           addqw #4, a7
0A10: 6608           bne 8
0A12: 526E FFFE      addqw.ex #1, -2(a6)
0A16: 6000 FF38      bra 65336
0A1A: 2F0C           movel a4, -(a7)
0A1C: 4E4F           trap #15
0A1E: A04A           sysTrapDmCloseDatabase
0A20: 584F           addqw #4, a7
0A22: 4CDF 1CF8      movem (a7)+, <1cf8>
0A26: 4E5E           unlk a6
0A28: 4E75           rts
0A2A: 8653           orrw (a3), d3
0A2C: 6561           bcs 97
0A2E: 7263           moveq #99, d1
0A30: 6800 0000      bvc 0
0A34: 4E56 FFE0      link a6, #-32
0A38: 2F0A           movel a2, -(a7)
0A3A: 246E 0008      movel.ex 8(a6), a2
0A3E: 3D6A 0008 FFE6 movew.emx 8(a2), -26(a6)
0A44: 42A7           clrl -(a7)
0A46: 486E FFE0      pea.ex -32(a6)
0A4A: 486E FFE4      pea.ex -28(a6)
0A4E: 3F2E FFE6      movew.ex -26(a6), -(a7)
0A52: 2F2D FFF4      movel.ex -12(a5), -(a7)
0A56: 4E4F           trap #15
0A58: A050           sysTrapDmRecordInfo
0A5A: 3B6E FFE6 FFD2 movew.emx -26(a6), -46(a5)
0A60: 0C6D 00FF FFFC cmpiw.ex #255, -4(a5)
0A66: 4FEF 0012      lea 18(a7), d7
0A6A: 6710           beq 16
0A6C: 302E FFE4      movew.ex -28(a6), d0
0A70: 0240 000F      andiw #15, d0
0A74: 3F00           movew d0, -(a7)
0A76: 4EBA FD00      jsr.ex 31616(pc)
0A7A: 544F           addqw #2, a7
0A7C: 4A2E 000C      tstb.ex 12(a6)
0A80: 6618           bne 24
0A82: 4E4F           trap #15
0A84: A1A1           sysTrapFrmCloseAllForms
0A86: 486E FFE6      pea.ex -26(a6)
0A8A: 2F2E FFE0      movel.ex -32(a6), -(a7)
0A8E: 2F2D FFF4      movel.ex -12(a5), -(a7)
0A92: 4E4F           trap #15
0A94: A07B           sysTrapDmFindRecordByID
0A96: 4FEF 000C      lea 12(a7), d7
0A9A: 4227           clrb -(a7)
0A9C: 4878 0018      pea.ex (0018).w
0AA0: 486E FFE8      pea.ex -24(a6)
0AA4: 4E4F           trap #15
0AA6: A027           sysTrapMemSet
0AA8: 3D7C 0017 FFE8 movew.emx #23, -24(a6)
0AAE: 3D7C 044C FFF0 movew.emx #1100, -16(a6)
0AB4: 486E FFE8      pea.ex -24(a6)
0AB8: 4E4F           trap #15
0ABA: A11B           sysTrapEvtAddEventToQueue
0ABC: 3D7C 0019 FFE8 movew.emx #25, -24(a6)
0AC2: 3D6E FFE6 FFF2 movew.emx -26(a6), -14(a6)
0AC8: 3D6A 000A FFF4 movew.emx 10(a2), -12(a6)
0ACE: 3D52 FFF6      movew.mx (a2), -10(a6)
0AD2: 3D6A 000C FFF8 movew.emx 12(a2), -8(a6)
0AD8: 3D7C 044C FFF0 movew.emx #1100, -16(a6)
0ADE: 486E FFE8      pea.ex -24(a6)
0AE2: 4E4F           trap #15
0AE4: A11B           sysTrapEvtAddEventToQueue
0AE6: 3B7C 044C FFFA movew.emx #1100, -6(a5)
0AEC: 4FEF 0012      lea 18(a7), d7
0AF0: 245F           movel (a7)+, a2
0AF2: 4E5E           unlk a6
0AF4: 4E75           rts
0AF6: 8847           orrw d7, d4
0AF8: 6F54           ble 84
0AFA: 6F49           ble 73
0AFC: 7465           moveq #101, d2
0AFE: 6D00 0000      blt 0
0B02: 4E56 FFFA      link a6, #-6
0B06: 48E7 1030      movem <1030>, -(a7)
0B0A: 422E FFFB      clrb.ex -5(a6)
0B0E: 7600           moveq #0, d3
0B10: 2F2D FFF4      movel.ex -12(a5), -(a7)
0B14: 4E4F           trap #15
0B16: A04F           sysTrapDmNumRecords
0B18: 3D40 FFFE      movew.mx d0, -2(a6)
0B1C: 4878 0040      pea.ex (0040).w
0B20: 486E FFFE      pea.ex -2(a6)
0B24: 2F2D FFF4      movel.ex -12(a5), -(a7)
0B28: 4E4F           trap #15
0B2A: A055           sysTrapDmNewRecord
0B2C: 2648           movel a0, a3
0B2E: 200B           movel a3, d0
0B30: 4FEF 0010      lea 16(a7), d7
0B34: 6610           bne 16
0B36: 3F3C 271D      movew.ex #10013, -(a7)
0B3A: 4E4F           trap #15
0B3C: A192           sysTrapFrmAlert
0B3E: 7000           moveq #0, d0
0B40: 544F           addqw #2, a7
0B42: 6000 00D6      bra 214
0B46: 2F0B           movel a3, -(a7)
0B48: 4E4F           trap #15
0B4A: A021           sysTrapMemHandleLock
0B4C: 2448           movel a0, a2
0B4E: 4A2E 0008      tstb.ex 8(a6)
0B52: 584F           addqw #4, a7
0B54: 672C           beq 44
0B56: 0C2E 0061 0008 cmpib.ex #97, 8(a6)
0B5C: 6D0E           blt 14
0B5E: 0C2E 007A 0008 cmpib.ex #122, 8(a6)
0B64: 6E06           bgt 6
0B66: 042E 0020 0008 subib.ex #32, 8(a6)
0B6C: 4878 0001      pea.ex (0001).w
0B70: 486E 0008      pea.ex 8(a6)
0B74: 42A7           clrl -(a7)
0B76: 2F0A           movel a2, -(a7)
0B78: 4E4F           trap #15
0B7A: A076           sysTrapDmWrite
0B7C: 5283           addql #1, d3
0B7E: 4FEF 0010      lea 16(a7), d7
0B82: 4878 0001      pea.ex (0001).w
0B86: 486E FFFB      pea.ex -5(a6)
0B8A: 2F03           movel d3, -(a7)
0B8C: 2F0A           movel a2, -(a7)
0B8E: 4E4F           trap #15
0B90: A076           sysTrapDmWrite
0B92: 2F0A           movel a2, -(a7)
0B94: 4E4F           trap #15
0B96: A035           sysTrapMemPtrUnlock
0B98: 42A7           clrl -(a7)
0B9A: 42A7           clrl -(a7)
0B9C: 486E FFFC      pea.ex -4(a6)
0BA0: 3F2E FFFE      movew.ex -2(a6), -(a7)
0BA4: 2F2D FFF4      movel.ex -12(a5), -(a7)
0BA8: 4E4F           trap #15
0BAA: A050           sysTrapDmRecordInfo
0BAC: 026E FFF0 FFFC andiw.ex #-16, -4(a6)
0BB2: 0C6D 00FF FFFC cmpiw.ex #255, -4(a5)
0BB8: 4FEF 0026      lea 38(a7), d7
0BBC: 6708           beq 8
0BBE: 302D FFFC      movew.ex -4(a5), d0
0BC2: 816E FFFC      ormw.ex d0, -4(a6)
0BC6: 42A7           clrl -(a7)
0BC8: 486E FFFC      pea.ex -4(a6)
0BCC: 3F2E FFFE      movew.ex -2(a6), -(a7)
0BD0: 2F