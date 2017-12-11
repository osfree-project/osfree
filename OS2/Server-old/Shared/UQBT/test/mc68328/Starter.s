00A0: 0000 0001      orib #1, d0
00A4: 487A 0004      pea.ex 00AA
00A8: 0697 0000 03C0 addil #960, (a7)
00AE: 4E75           rts
00B0: 4E56 FFD6      link a6, #-42
00B4: 48E7 1C00      movem <1c00>, -(a7)
00B8: 2A2E 0008      movel.ex 8(a6), d5
00BC: 382E 000C      movew.ex 12(a6), d4
00C0: 486E FFF8      pea.ex -8(a6)
00C4: 3F3C 0001      movew.ex #1, -(a7)
00C8: 2F3C 7073 7973 movel.exl #1886615923, -(a7)
00CE: 4E4F           trap #15
00D0: A27B           sysTrapFtrGet
00D2: BAAE FFF8      cmpl.ex -8(a6), d5
00D6: 4FEF 000A      lea 10(a7), a7
00DA: 6300 00AA      bls 0186
00DE: 3004           movew d4, d0
00E0: 0240 000C      andiw #12, d0
00E4: 0C40 000C      cmpiw #12, d0
00E8: 6600 0096      bne 0180
00EC: 3F3C 03E9      movew.ex #1001, -(a7)
00F0: 4E4F           trap #15
00F2: A192           sysTrapFrmAlert
00F4: 0CAE 0200 3000 FFF8 cmpil.ex #33566720, -8(a6)
00FC: 544F           addqw #2, a7
00FE: 6400 0080      bcc 0180
0102: 486E FFFC      pea.ex -4(a6)
0106: 486E FFF6      pea.ex -10(a6)
010A: 1F3C 0001      moveb.ex #1, -(a7)
010E: 2F3C 7072 6566 movel.exl #1886545254, -(a7)
0114: 2F3C 6170 706C movel.exl #1634758764, -(a7)
011A: 486E FFD6      pea.ex -42(a6)
011E: 1F3C 0001      moveb.ex #1, -(a7)
0122: 4E4F           trap #15
0124: A078           sysTrapDmGetNextDatabaseByTypeCreator
0126: 4AAE FFFC      tstl.ex -4(a6)
012A: 4FEF 0018      lea 24(a7), a7
012E: 6618           bne 0148
0130: 41FA 0076      lea 01A8, a0
0134: 4850           pea (a0)
0136: 3F3C 0090      movew.ex #144, -(a7)
013A: 41FA 0080      lea 01BC, a0
013E: 4850           pea (a0)
0140: 4E4F           trap #15
0142: A084           sysTrapErrDisplayFileLineMsg
0144: 4FEF 000A      lea 10(a7), a7
0148: 4AAE FFFC      tstl.ex -4(a6)
014C: 6732           beq 0180
014E: 42A7           clrl -(a7)
0150: 4267           clrw -(a7)
0152: 2F2E FFFC      movel.ex -4(a6), -(a7)
0156: 3F2E FFF6      movew.ex -10(a6), -(a7)
015A: 4E4F           trap #15
015C: A0A7           sysTrapSysUIAppSwitch
015E: 3600           movew d0, d3
0160: 4A43           tstw d3
0162: 4FEF 000C      lea 12(a7), a7
0166: 6718           beq 0180
0168: 41FA 005C      lea 01C6, a0
016C: 4850           pea (a0)
016E: 3F3C 0090      movew.ex #144, -(a7)
0172: 41FA 0068      lea 01DC, a0
0176: 4850           pea (a0)
0178: 4E4F           trap #15
017A: A084           sysTrapErrDisplayFileLineMsg
017C: 4FEF 000A      lea 10(a7), a7
0180: 303C 050C      movew.ex #1292, d0
0184: 6002           bra 0188
0186: 7000           moveq #0, d0
0188: 4CDF 0038      movem (a7)+, <0038>
018C: 4E5E           unlk a6
018E: 4E75           rts
0190: 9452           subrw (a2), d2
0192: 6F6D           ble 0201
0194: 5665           addqw #3, -(a5)
0196: 7273           moveq #115, d1
0198: 696F           bvs 0209
019A: 6E43           bgt 01DF
019C: 6F6D           ble 020B
019E: 7061           moveq #97, d0
01A0: 7469           moveq #105, d2
01A2: 626C           bhi 0210
01A4: 6500 003E      bcs 01E4
01A8: 436F           dc.w #17263
01AA: 756C           dc.w #30060
01AC: 6420           bcc 01CE
01AE: 6E6F           bgt 021F
01B0: 7420           moveq #32, d2
01B2: 6669           bne 021D
01B4: 6E64           bgt 021A
01B6: 2061           movel -(a1), a0
01B8: 7070           moveq #112, d0
01BA: 0000           dc.w #0
01BC: 5374 6172      subqw.ex #1, 114(a4,d6.w)
01C0: 7465           moveq #101, d2
01C2: 722E           moveq #46, d1
01C4: 6300 436F      bls 4535
01C8: 756C           dc.w #30060
01CA: 6420           bcc 01EC
01CC: 6E6F           bgt 023D
01CE: 7420           moveq #32, d2
01D0: 6C61           bge 0233
01D2: 756E           dc.w #30062
01D4: 6368           bls 023E
01D6: 2061           movel -(a1), a0
01D8: 7070           moveq #112, d0
01DA: 0000           dc.w #0
01DC: 5374 6172      subqw.ex #1, 114(a4,d6.w)
01E0: 7465           moveq #101, d2
01E2: 722E           moveq #46, d1
01E4: 6300 4E56      bls 503C
01E8: 0000           dc.w #0
01EA: 4E5E           unlk a6
01EC: 4E75           rts
01EE: 8C4D           dc.w #35917
01F0: 6169           bsr 025B
01F2: 6E46           bgt 023A
01F4: 6F72           ble 0268
01F6: 6D49           blt 0241
01F8: 6E69           bgt 0263
01FA: 7400           moveq #0, d2
01FC: 0000           dc.w #0
01FE: 4E56 0000      link a6, #0
0202: 2F03           movel d3, -(a7)
0204: 7600           moveq #0, d3
0206: 302E 0008      movew.ex 8(a6), d0
020A: 0440 03E8      subiw #1000, d0
020E: 6702           beq 0212
0210: 6014           bra 0226
0212: 42A7           clrl -(a7)
0214: 4E4F           trap #15
0216: A1C1           sysTrapMenuEraseStatus
0218: 2F3C 7374 7274 movel.exl #1937011316, -(a7)
021E: 4E4F           trap #15
0220: A2AF           sysTrapAbtShowAbout
0222: 7601           moveq #1, d3
0224: 504F           addqw #8, a7
0226: 1003           moveb d3, d0
0228: 261F           movel (a7)+, d3
022A: 4E5E           unlk a6
022C: 4E75           rts
022E: 914D           subxmw -(a5), -(a0)
0230: 6169           bsr 029B
0232: 6E46           bgt 027A
0234: 6F72           ble 02A8
0236: 6D44           blt 027C
0238: 6F43           ble 027D
023A: 6F6D           ble 02A9
023C: 6D61           blt 029F
023E: 6E64           bgt 02A4
0240: 0000           dc.w #0
0242: 4E56 0000      link a6, #0
0246: 48E7 1030      movem <1030>, -(a7)
024A: 266E 0008      movel.ex 8(a6), a3
024E: 7600           moveq #0, d3
0250: 3013           movew (a3), d0
0252: 0440 0015      subiw #21, d0
0256: 6706           beq 025E
0258: 5740           subqw #3, d0
025A: 670E           beq 026A
025C: 6022           bra 0280
025E: 3F2B 0008      movew.ex 8(a3), -(a7)
0262: 4EBA FF9A      jsr.ex 01FE
0266: 544F           addqw #2, a7
0268: 6018           bra 0282
026A: 4E4F           trap #15
026C: A173           sysTrapFrmGetActiveForm
026E: 2448           movel a0, a2
0270: 2F0A           movel a2, -(a7)
0272: 4EBA FF72      jsr.ex 01E6
0276: 2F0A           movel a2, -(a7)
0278: 4E4F           trap #15
027A: A171           sysTrapFrmDrawForm
027C: 7601           moveq #1, d3
027E: 504F           addqw #8, a7
0280: 1003           moveb d3, d0
0282: 4CDF 0C08      movem (a7)+, <0c08>
0286: 4E5E           unlk a6
0288: 4E75           rts
028A: 934D           subxmw -(a5), -(a1)
028C: 6169           bsr 02F7
028E: 6E46           bgt 02D6
0290: 6F72           ble 0304
0292: 6D48           blt 02DC
0294: 616E           bsr 0304
0296: 646C           bcc 0304
0298: 6545           bcs 02DF
029A: 7665           moveq #101, d3
029C: 6E74           bgt 0312
029E: 0000           dc.w #0
02A0: 4E56 0000      link a6, #0
02A4: 48E7 1030      movem <1030>, -(a7)
02A8: 266E 0008      movel.ex 8(a6), a3
02AC: 0C53 0017      cmpiw #23, (a3)
02B0: 662E           bne 02E0
02B2: 362B 0008      movew.ex 8(a3), d3
02B6: 3F03           movew d3, -(a7)
02B8: 4E4F           trap #15
02BA: A16F           sysTrapFrmInitForm
02BC: 2448           movel a0, a2
02BE: 2F0A           movel a2, -(a7)
02C0: 4E4F           trap #15
02C2: A174           sysTrapFrmSetActiveForm
02C4: 5C4F           addqw #6, a7
02C6: 3003           movew d3, d0
02C8: 0440 03E8      subiw #1000, d0
02CC: 6702           beq 02D0
02CE: 600C           bra 02DC
02D0: 487A FF70      pea.ex 0242
02D4: 2F0A           movel a2, -(a7)
02D6: 4E4F           trap #15
02D8: A19F           sysTrapFrmSetEventHandler
02DA: 504F           addqw #8, a7
02DC: 7001           moveq #1, d0
02DE: 6002           bra 02E2
02E0: 7000           moveq #0, d0
02E2: 4CDF 0C08      movem (a7)+, <0c08>
02E6: 4E5E           unlk a6
02E8: 4E75           rts
02EA: 8E41           orrw d1, d7
02EC: 7070           moveq #112, d0
02EE: 4861           dc.w #18529
02F0: 6E64           bgt 0356
02F2: 6C65           bge 0359
02F4: 4576           dc.w #17782
02F6: 656E           bcs 0366
02F8: 7400           moveq #0, d2
02FA: 0000           dc.w #0
02FC: 4E56 FFE6      link a6, #-26
0300: 4878 FFFF      pea.ex (ffff).w
0304: 486E FFE8      pea.ex -24(a6)
0308: 4E4F           trap #15
030A: A11D           sysTrapEvtGetEvent
030C: 486E FFE8      pea.ex -24(a6)
0310: 4E4F           trap #15
0312: A0A9           sysTrapSysHandleEvent
0314: 4A00           tstb d0
0316: 4FEF 000C      lea 12(a7), a7
031A: 662E           bne 034A
031C: 486E FFE6      pea.ex -26(a6)
0320: 486E FFE8      pea.ex -24(a6)
0324: 42A7           clrl -(a7)
0326: 4E4F           trap #15
0328: A1BF           sysTrapMenuHandleEvent
032A: 4A00           tstb d0
032C: 4FEF 000C      lea 12(a7), a7
0330: 6618           bne 034A
0332: 486E FFE8      pea.ex -24(a6)
0336: 4EBA FF68      jsr.ex 02A0
033A: 4A00           tstb d0
033C: 584F           addqw #4, a7
033E: 660A           bne 034A
0340: 486E FFE8      pea.ex -24(a6)
0344: 4E4F           trap #15
0346: A1A0           sysTrapFrmDispatchEvent
0348: 584F           addqw #4, a7
034A: 0C6E 0016 FFE8 cmpiw.ex #22, -24(a6)
0350: 66AE           bne 0300
0352: 4E5E           unlk a6
0354: 4E75           rts
0356: 8C41           orrw d1, d6
0358: 7070           moveq #112, d0
035A: 4576           dc.w #17782
035C: 656E           bcs 03CC
035E: 744C           moveq #76, d2
0360: 6F6F           ble 03D1
0362: 7000           moveq #0, d0
0364: 0000           dc.w #0
0366: 4E56 FFFC      link a6, #-4
036A: 3D7C 0002 FFFE movew.emx #2, -2(a6)
0370: 1F3C 0001      moveb.ex #1, -(a7)
0374: 486E FFFE      pea.ex -2(a6)
0378: 486E FFFC      pea.ex -4(a6)
037C: 4267           clrw -(a7)
037E: 2F3C 7374 7274 movel.exl #1937011316, -(a7)
0384: 4E4F           trap #15
0386: A2D3           sysTrapPrefGetAppPreferences
0388: 7000           moveq #0, d0
038A: 4E5E           unlk a6
038C: 4E75           rts
038E: 8841           orrw d1, d4
0390: 7070           moveq #112, d0
0392: 5374 6172      subqw.ex #1, 114(a4,d6.w)
0396: 7400           moveq #0, d2
0398: 0000           dc.w #0
039A: 4E56 FFFE      link a6, #-2
039E: 1F3C 0001      moveb.ex #1, -(a7)
03A2: 3F3C 0002      movew.ex #2, -(a7)
03A6: 486E FFFE      pea.ex -2(a6)
03AA: 4878 0001      pea.ex (0001).w
03AE: 2F3C 7374 7274 movel.exl #1937011316, -(a7)
03B4: 4E4F           trap #15
03B6: A2D4           sysTrapPrefSetAppPreferences
03B8: 4E5E           unlk a6
03BA: 4E75           rts
03BC: 8741           dc.w #34625
03BE: 7070           moveq #112, d0
03C0: 5374 6F70      subqw.ex #1, 112(a4,d6.l)
03C4: 0000           dc.w #0
03C6: 4E56 0000      link a6, #0
03CA: 48E7 1C00      movem <1c00>, -(a7)
03CE: 3A2E 0008      movew.ex 8(a6), d5
03D2: 382E 000E      movew.ex 14(a6), d4
03D6: 3F04           movew d4, -(a7)
03D8: 2F3C 0200 3000 movel.exl #33566720, -(a7)
03DE: 4EBA FCD0      jsr.ex 00B0
03E2: 3600           movew d0, d3
03E4: 4A43           tstw d3
03E6: 5C4F           addqw #6, a7
03E8: 6706           beq 03F0
03EA: 3043           movew d3, a0
03EC: 2008           movel a0, d0
03EE: 602A           bra 041A
03F0: 3005           movew d5, d0
03F2: 6702           beq 03F6
03F4: 6022           bra 0418
03F6: 4EBA FF6E      jsr.ex 0366
03FA: 3600           movew d0, d3
03FC: 4A43           tstw d3
03FE: 6706           beq 0406
0400: 3043           movew d3, a0
0402: 2008           movel a0, d0
0404: 6014           bra 041A
0406: 3F3C 03E8      movew.ex #1000, -(a7)
040A: 4E4F           trap #15
040C: A19B           sysTrapFrmGotoForm
040E: 4EBA FEEC      jsr.ex 02FC
0412: 4EBA FF86      jsr.ex 039A
0416: 544F           addqw #2, a7
0418: 7000           moveq #0, d0
041A: 4CDF 0038      movem (a7)+, <0038>
041E: 4E5E           unlk a6
0420: 4E75           rts
0422: 9053           subrw (a3), d0
0424: 7461           moveq #97, d2
0426: 7274           moveq #116, d1
0428: 6572           bcs 049C
042A: 5069 6C6F      addqw.ex #8, 27759(a1)
042E: 744D           moveq #77, d2
0430: 6169           bsr 049B
0432: 6E00 0000      bgt 0434
0436: 4E56 0000      link a6, #0
043A: 48E7 1820      movem <1820>, -(a7)
043E: 382E 0008      movew.ex 8(a6), d4
0442: 246E 000A      movel.ex 10(a6), a2
0446: 362E 000E      movew.ex 14(a6), d3
044A: 3F03           movew d3, -(a7)
044C: 2F0A           movel a2, -(a7)
044E: 3F04           movew d4, -(a7)
0450: 4EBA FF74      jsr.ex 03C6
0454: 504F           addqw #8, a7
0456: 4CDF 0418      movem (a7)+, <0418>
045A: 4E5E           unlk a6
045C: 4E75           rts
045E: 8950           ormw d4, (a0)
0460: 696C           bvs 04CE
0462: 6F74           ble 04D8
0464: 4D61           dc.w #19809
0466: 696E           bvs 04D6
0468: 0000           dc.w #0
046A: 4E56 FFF4      link a6, #-12
046E: 48E7 1800      movem <1800>, -(a7)
0472: 486E FFF4      pea.ex -12(a6)
0476: 486E FFF8      pea.ex -8(a6)
047A: 486E FFFC      pea.ex -4(a6)
047E: 4E4F           trap #15
0480: A08F           sysTrapSysAppStartup
0482: 3800           movew d0, d4
0484: 4A44           tstw d4
0486: 4FEF 000C      lea 12(a7), a7
048A: 671C           beq 04A8
048C: 41FA 0066      lea 04F4, a0
0490: 4850           pea (a0)
0492: 3F3C 005C      movew.ex #92, -(a7)
0496: 41FA 0078      lea 0510, a0
049A: 4850           pea (a0)
049C: 4E4F           trap #15
049E: A084           sysTrapErrDisplayFileLineMsg
04A0: 7000           moveq #0, d0
04A2: 4FEF 000A      lea 10(a7), a7
04A6: 6036           bra 04DE
04A8: 206E FFFC      movel.ex -4(a6), a0
04AC: 3F28 0006      movew.ex 6(a0), -(a7)
04B0: 2F28 0002      movel.ex 2(a0), -(a7)
04B4: 3F10           movew (a0), -(a7)
04B6: 487A 000E      pea.ex 04C6
04BA: 487A 0004      pea.ex 04C0
04BE: 0697 FFFF FF76 addil #-138, (a7)
04C4: 4E75           rts
04C6: 2600           movel d0, d3
04C8: 2F2E FFF4      movel.ex -12(a6), -(a7)
04CC: 2F2E FFF8      movel.ex -8(a6), -(a7)
04D0: 2F2E FFFC      movel.ex -4(a6), -(a7)
04D4: 4E4F           trap #15
04D6: A090           sysTrapSysAppExit
04D8: 2003           movel d3, d0
04DA: 4FEF 0014      lea 20(a7), a7
04DE: 4CDF 0018      movem (a7)+, <0018>
04E2: 4E5E           unlk a6
04E4: 4E75           rts
04E6: 8B5F           ormw d5, (a7)+
04E8: 5F53           subqw #7, (a3)
04EA: 7461           moveq #97, d2
04EC: 7274           moveq #116, d1
04EE: 7570           dc.w #30064
04F0: 5F5F           subqw #7, (a7)+
04F2: 002A           dc.w #42
04F4: 4572           dc.w #17778
04F6: 726F           moveq #111, d1
04F8: 7220           moveq #32, d1
04FA: 6C61           bge 055D
04FC: 756E           dc.w #30062
04FE: 6368           bls 0568
0500: 696E           bvs 0570
0502: 6720           beq 0524
0504: 6170           bsr 0576
0506: 706C           moveq #108, d0
0508: 6963           bvs 056D
050A: 6174           bsr 0580
050C: 696F           bvs 057D
050E: 6E00 5374      bgt 5884
0512: 6172           bsr 0586
0514: 7475           moveq #117, d2
0516: 7043           moveq #67, d0
0518: 6F64           ble 057E
051A: 652E           bcs 054A
051C: 6300 0000      bls 051E

