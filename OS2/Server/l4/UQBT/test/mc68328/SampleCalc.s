00BE: 0000 0001      orib #1, d0
00C2: 487A 0004      pea.ex 29904(pc)
00C6: 0697 0000 0006 addil #6, (a7)
00CC: 4E75           rts
00CE: 4E56 FFF4      link a6, #-12
00D2: 48E7 1800      movem <1800>, -(a7)
00D6: 486E FFF4      pea.ex -12(a6)
00DA: 486E FFF8      pea.ex -8(a6)
00DE: 486E FFFC      pea.ex -4(a6)
00E2: 4E4F           trap #15
00E4: A08F           sysTrapSysAppStartup
00E6: 3800           movew d0, d4
00E8: 4A44           tstw d4
00EA: 4FEF 000C      lea 12(a7), d7
00EE: 671C           beq 28
00F0: 41FA 0066      lea 30048(pc), d0
00F4: 4850           pea (a0)
00F6: 3F3C 005C      movew.ex #92, -(a7)
00FA: 41FA 0078      lea 30076(pc), d0
00FE: 4850           pea (a0)
0100: 4E4F           trap #15
0102: A084           sysTrapErrDisplayFileLineMsg
0104: 7000           moveq #0, d0
0106: 4FEF 000A      lea 10(a7), d7
010A: 6036           bra 54
010C: 206E FFFC      movel.ex -4(a6), a0
0110: 3F28 0006      movew.ex 6(a0), -(a7)
0114: 2F28 0002      movel.ex 2(a0), -(a7)
0118: 3F10           movew (a0), -(a7)
011A: 487A 000E      pea.ex 30002(pc)
011E: 487A 0004      pea.ex 29996(pc)
0122: 0697 0000 0E5E addil #3678, (a7)
0128: 4E75           rts
012A: 2600           movel d0, d3
012C: 2F2E FFF4      movel.ex -12(a6), -(a7)
0130: 2F2E FFF8      movel.ex -8(a6), -(a7)
0134: 2F2E FFFC      movel.ex -4(a6), -(a7)
0138: 4E4F           trap #15
013A: A090           sysTrapSysAppExit
013C: 2003           movel d3, d0
013E: 4FEF 0014      lea 20(a7), d7
0142: 4CDF 0018      movem (a7)+, <0018>
0146: 4E5E           unlk a6
0148: 4E75           rts
014A: 8B5F           ormw d5, (a7)+
014C: 5F53           subqw #7, (a3)
014E: 7461           moveq #97, d2
0150: 7274           moveq #116, d1
0152: 7570           dc.w #30064
0154: 5F5F           subqw #7, (a7)+
0156: 002A           dc.w #42
0158: 4572           dc.w #17778
015A: 726F           moveq #111, d1
015C: 7220           moveq #32, d1
015E: 6C61           bge 97
0160: 756E           dc.w #30062
0162: 6368           bls 104
0164: 696E           bvs 110
0166: 6720           beq 32
0168: 6170           bsr 112
016A: 706C           moveq #108, d0
016C: 6963           bvs 99
016E: 6174           bsr 116
0170: 696F           bvs 111
0172: 6E00 5374      bgt 21364
0176: 6172           bsr 114
0178: 7475           moveq #117, d2
017A: 7043           moveq #67, d0
017C: 6F64           ble 100
017E: 652E           bcs 46
0180: 6300 4E56      bls 20054
0184: 0000           dc.w #0
0186: 4A2E 0008      tstb.ex 8(a6)
018A: 660C           bne 12
018C: 4A6D FF7C      tstw.ex -132(a5)
0190: 662E           bne 46
0192: 4A2D FF67      tstb.ex -153(a5)
0196: 6728           beq 40
0198: 2B6D FF4C FF44 movel.emx -180(a5), -188(a5)
019E: 2B6D FF50 FF48 movel.emx -176(a5), -184(a5)
01A4: 2B6D FF54 FF4C movel.emx -172(a5), -180(a5)
01AA: 2B6D FF58 FF50 movel.emx -168(a5), -176(a5)
01B0: 2B6D FF5C FF54 movel.emx -164(a5), -172(a5)
01B6: 2B6D FF60 FF58 movel.emx -160(a5), -168(a5)
01BC: 422D FF67      clrb.ex -153(a5)
01C0: 4E5E           unlk a6
01C2: 4E75           rts
01C4: 8E53           orrw (a3), d7
01C6: 7461           moveq #97, d2
01C8: 636B           bls 107
01CA: 4C69           dc.w #19561
01CC: 6674           bne 116
01CE: 5075 7368      addqw.ex #0, 104(a5,d7.w)
01D2: 5800           addqb #4, d0
01D4: 0000           dc.w #0
01D6: 4E56 FFF8      link a6, #-8
01DA: 2F0A           movel a2, -(a7)
01DC: 246E 0008      movel.ex 8(a6), a2
01E0: 2D6D FF54 FFF8 movel.emx -172(a5), -8(a6)
01E6: 2D6D FF58 FFFC movel.emx -168(a5), -4(a6)
01EC: 2B6D FF4C FF54 movel.emx -180(a5), -172(a5)
01F2: 2B6D FF50 FF58 movel.emx -176(a5), -168(a5)
01F8: 2B6D FF44 FF4C movel.emx -188(a5), -180(a5)
01FE: 2B6D FF48 FF50 movel.emx -184(a5), -176(a5)
0204: 24AE FFF8      movel.ex -8(a6), (a2)
0208: 256E FFFC 0004 movel.emx -4(a6), 4(a2)
020E: 245F           movel (a7)+, a2
0210: 4E5E           unlk a6
0212: 4E75           rts
0214: 8D53           ormw d6, (a3)
0216: 7461           moveq #97, d2
0218: 636B           bls 107
021A: 4472 6F70      negw.ex 112(a2,d6.l)
021E: 506F 7059      addqw.ex #0, 28761(a7)
0222: 0000           dc.w #0
0224: 4E56 0000      link a6, #0
0228: 48E7 1E00      movem <1e00>, -(a7)
022C: 1C2E 0008      moveb.ex 8(a6), d6
0230: 4A2D FF69      tstb.ex -151(a5)
0234: 675C           beq 92
0236: 0C6D 0002 FF7A cmpiw.ex #2, -134(a5)
023C: 6416           bcc 22
023E: 302D FF7A      movew.ex -134(a5), d0
0242: 526D FF7A      addqw.ex #1, -134(a5)
0246: 7200           moveq #0, d1
0248: 3200           movew d0, d1
024A: 41ED FF6A      lea -150(a5), d0
024E: 1186 1800      moveb.mx d6, 0(a0,d1.l)
0252: 603A           bra 58
0254: 7601           moveq #1, d3
0256: 601C           bra 28
0258: 7000           moveq #0, d0
025A: 3003           movew d3, d0
025C: 41ED FF6A      lea -150(a5), d0
0260: 3203           movew d3, d1
0262: 0641 FFFF      addiw #-1, d1
0266: 7400           moveq #0, d2
0268: 3401           movew d1, d2
026A: 2248           movel a0, a1
026C: 13B0 0800 2800 moveb.emx 0(a0,d0.l), 0(a1,d2.l)
0272: 5243           addqw #1, d3
0274: B66D FF7A      cmpw.ex -134(a5), d3
0278: 65DE           bcs -34
027A: 302D FF7A      movew.ex -134(a5), d0
027E: 0640 FFFF      addiw #-1, d0
0282: 7200           moveq #0, d1
0284: 3200           movew d0, d1
0286: 41ED FF6A      lea -150(a5), d0
028A: 1186 1800      moveb.mx d6, 0(a0,d1.l)
028E: 7801           moveq #1, d4
0290: 6044           bra 68
0292: 4A2D FF68      tstb.ex -152(a5)
0296: 6704           beq 4
0298: 7A09           moveq #9, d5
029A: 6002           bra 2
029C: 7A08           moveq #8, d5
029E: BA6D FF7C      cmpw.ex -132(a5), d5
02A2: 6330           bls 48
02A4: 4A6D FF7C      tstw.ex -132(a5)
02A8: 6608           bne 8
02AA: 4227           clrb -(a7)
02AC: 4EBA FED4      jsr.ex 30090(pc)
02B0: 544F           addqw #2, a7
02B2: 302D FF7C      movew.ex -132(a5), d0
02B6: 526D FF7C      addqw.ex #1, -132(a5)
02BA: 7200           moveq #0, d1
02BC: 3200           movew d0, d1
02BE: 41ED FF6E      lea -146(a5), d0
02C2: 1186 1800      moveb.mx d6, 0(a0,d1.l)
02C6: 7000           moveq #0, d0
02C8: 302D FF7C      movew.ex -132(a5), d0
02CC: 4230 0800      clrb.ex 0(a0,d0.l)
02D0: 7801           moveq #1, d4
02D2: 6002           bra 2
02D4: 7800           moveq #0, d4
02D6: 1004           moveb d4, d0
02D8: 4CDF 0078      movem (a7)+, <0078>
02DC: 4E5E           unlk a6
02DE: 4E75           rts
02E0: 9844           subrw d4, d4
02E2: 6973           bvs 115
02E4: 706C           moveq #108, d0
02E6: 6179           bsr 121
02E8: 5374 7269      subqw.ex #1, 105(a4,d7.w)
02EC: 6E67           bgt 103
02EE: 4170           dc.w #16752
02F0: 7065           moveq #101, d0
02F2: 6E64           bgt 100
02F4: 4469 6769      negw.ex 26473(a1)
02F8: 7400           moveq #0, d2
02FA: 0000           dc.w #0
02FC: 4E56 0000      link a6, #0
0300: 2F03           movel d3, -(a7)
0302: 4A2D FF69      tstb.ex -151(a5)
0306: 6658           bne 88
0308: 4A2D FF68      tstb.ex -152(a5)
030C: 6652           bne 82
030E: 0C6D 0008 FF7C cmpiw.ex #8, -132(a5)
0314: 644A           bcc 74
0316: 4A6D FF7C      tstw.ex -132(a5)
031A: 661E           bne 30
031C: 4227           clrb -(a7)
031E: 4EBA FE62      jsr.ex 30090(pc)
0322: 302D FF7C      movew.ex -132(a5), d0
0326: 526D FF7C      addqw.ex #1, -132(a5)
032A: 7200           moveq #0, d1
032C: 3200           movew d0, d1
032E: 41ED FF6E      lea -146(a5), d0
0332: 11BC 0030 1800 moveb.emx #48, 0(a0,d1.l)
0338: 544F           addqw #2, a7
033A: 1B7C 0001 FF68 moveb.emx #1, -152(a5)
0340: 3B6D FF7C FF64 movew.emx -132(a5), -156(a5)
0346: 302D FF7C      movew.ex -132(a5), d0
034A: 526D FF7C      addqw.ex #1, -132(a5)
034E: 7200           moveq #0, d1
0350: 3200           movew d0, d1
0352: 41ED FF6E      lea -146(a5), d0
0356: 11BC 002E 1800 moveb.emx #46, 0(a0,d1.l)
035C: 7601           moveq #1, d3
035E: 6002           bra 2
0360: 7600           moveq #0, d3
0362: 1003           moveb d3, d0
0364: 261F           movel (a7)+, d3
0366: 4E5E           unlk a6
0368: 4E75           rts
036A: 9A44           subrw d4, d5
036C: 6973           bvs 115
036E: 706C           moveq #108, d0
0370: 6179           bsr 121
0372: 5374 7269      subqw.ex #1, 105(a4,d7.w)
0376: 6E67           bgt 103
0378: 4170           dc.w #16752
037A: 7065           moveq #101, d0
037C: 6E64           bgt 100
037E: 4465           negw -(a5)
0380: 6369           bls 105
0382: 6D61           blt 97
0384: 6C00 0000      bge 0
0388: 4E56 0000      link a6, #0
038C: 2F05           movel d5, -(a7)
038E: 7201           moveq #1, d1
0390: 4A2D FF69      tstb.ex -151(a5)
0394: 6712           beq 18
0396: 4A2D FF78      tstb.ex -136(a5)
039A: 6704           beq 4
039C: 7400           moveq #0, d2
039E: 6002           bra 2
03A0: 742D           moveq #45, d2
03A2: 1B42 FF78      moveb.mx d2, -136(a5)
03A6: 6034           bra 52
03A8: 4A6D FF7C      tstw.ex -132(a5)
03AC: 6712           beq 18
03AE: 4A2D FF79      tstb.ex -135(a5)
03B2: 6704           beq 4
03B4: 7A00           moveq #0, d5
03B6: 6002           bra 2
03B8: 7A2D           moveq #45, d5
03BA: 1B45 FF79      moveb.mx d5, -135(a5)
03BE: 601C           bra 28
03C0: 202D FF5C      movel.ex -164(a5), d0
03C4: 0280 7FFF FFFF andil #2147483647, d0
03CA: 80AD FF60      orrl.ex -160(a5), d0
03CE: 670A           beq 10
03D0: 0AAD 8000 0000 FF5C eoril.ex #-2147483648, -164(a5)
03D8: 6002           bra 2
03DA: 7200           moveq #0, d1
03DC: 1001           moveb d1, d0
03DE: 2A1F           movel (a7)+, d5
03E0: 4E5E           unlk a6
03E2: 4E75           rts
03E4: 9344           subxrw d4, d1
03E6: 6973           bvs 115
03E8: 706C           moveq #108, d0
03EA: 6179           bsr 121
03EC: 5374 7269      subqw.ex #1, 105(a4,d7.w)
03F0: 6E67           bgt 103
03F2: 4E65           move a5, usp
03F4: 6761           beq 97
03F6: 7465           moveq #101, d2
03F8: 0000           dc.w #0
03FA: 4E56 0000      link a6, #0
03FE: 2F03           movel d3, -(a7)
0400: 4A2D FF69      tstb.ex -151(a5)
0404: 662E           bne 46
0406: 1B7C 0001 FF69 moveb.emx #1, -151(a5)
040C: 7601           moveq #1, d3
040E: 4A6D FF7C      tstw.ex -132(a5)
0412: 6622           bne 34
0414: 4227           clrb -(a7)
0416: 4EBA FD6A      jsr.ex 30090(pc)
041A: 302D FF7C      movew.ex -132(a5), d0
041E: 526D FF7C      addqw.ex #1, -132(a5)
0422: 7200           moveq #0, d1
0424: 3200           movew d0, d1
0426: 41ED FF6E      lea -146(a5), d0
042A: 11BC 0031 1800 moveb.emx #49, 0(a0,d1.l)
0430: 544F           addqw #2, a7
0432: 6002           bra 2
0434: 7600           moveq #0, d3
0436: 1003           moveb d3, d0
0438: 261F           movel (a7)+, d3
043A: 4E5E           unlk a6
043C: 4E75           rts
043E: 9844           subrw d4, d4
0440: 6973           bvs 115
0442: 706C           moveq #108, d0
0444: 6179           bsr 121
0446: 5374 7269      subqw.ex #1, 105(a4,d7.w)
044A: 6E67           bgt 103
044C: 4164           dc.w #16740
044E: 6445           bcc 69
0450: 7870           moveq #112, d4
0452: 6F6E           ble 110
0454: 656E           bcs 110
0456: 7400           moveq #0, d2
0458: 0000           dc.w #0
045A: 4E56 0000      link a6, #0
045E: 2F03           movel d3, -(a7)
0460: 4A6D FF7C      tstw.ex -132(a5)
0464: 674E           beq 78
0466: 7601           moveq #1, d3
0468: 4A2D FF69      tstb.ex -151(a5)
046C: 671E           beq 30
046E: 4A6D FF7A      tstw.ex -134(a5)
0472: 6706           beq 6
0474: 536D FF7A      subqw.ex #1, -134(a5)
0478: 605C           bra 92
047A: 4A2D FF78      tstb.ex -136(a5)
047E: 6706           beq 6
0480: 422D FF78      clrb.ex -136(a5)
0484: 6050           bra 80
0486: 422D FF69      clrb.ex -151(a5)
048A: 604A           bra 74
048C: 536D FF7C      subqw.ex #1, -132(a5)
0490: 4A6D FF7C      tstw.ex -132(a5)
0494: 6606           bne 6
0496: 422D FF79      clrb.ex -135(a5)
049A: 603A           bra 58
049C: 7000           moveq #0, d0
049E: 302D FF7C      movew.ex -132(a5), d0
04A2: 41ED FF6E      lea -146(a5), d0
04A6: 0C30 002E 0800 cmpib.ex #46, 0(a0,d0.l)
04AC: 6628           bne 40
04AE: 422D FF68      clrb.ex -152(a5)
04B2: 6022           bra 34
04B4: 202D FF5C      movel.ex -164(a5), d0
04B8: 0280 7FFF FFFF andil #2147483647, d0
04BE: 80AD FF60      orrl.ex -160(a5), d0
04C2: 6710           beq 16
04C4: 42AD FF5C      clrl.ex -164(a5)
04C8: 42AD FF60      clrl.ex -160(a5)
04CC: 422D FF67      clrb.ex -153(a5)
04D0: 7601           moveq #1, d3
04D2: 6002           bra 2
04D4: 7600           moveq #0, d3
04D6: 1003           moveb d3, d0
04D8: 261F           movel (a7)+, d3
04DA: 4E5E           unlk a6
04DC: 4E75           rts
04DE: 9644           subrw d4, d3
04E0: 6973           bvs 115
04E2: 706C           moveq #108, d0
04E4: 6179           bsr 121
04E6: 5374 7269      subqw.ex #1, 105(a4,d7.w)
04EA: 6E67           bgt 103
04EC: 4261           clrw -(a1)
04EE: 636B           bls 107
04F0: 7370           dc.w #29552
04F2: 6163           bsr 99
04F4: 6500 0000      bcs 0
04F8: 4E56 0000      link a6, #0
04FC: 426D FF7C      clrw.ex -132(a5)
0500: 426D FF7A      clrw.ex -134(a5)
0504: 422D FF79      clrb.ex -135(a5)
0508: 422D FF78      clrb.ex -136(a5)
050C: 1B7C 0030 FF6E moveb.emx #48, -146(a5)
0512: 1B7C 002E FF6F moveb.emx #46, -145(a5)
0518: 3B7C 0001 FF64 movew.emx #1, -156(a5)
051E: 422D FF68      clrb.ex -152(a5)
0522: 7202           moveq #2, d1
0524: 6010           bra 16
0526: 7000           moveq #0, d0
0528: 3001           movew d1, d0
052A: 41ED FF6E      lea -146(a5), d0
052E: 11BC 0030 0800 moveb.emx #48, 0(a0,d0.l)
0534: 5241           addqw #1, d1
0536: 0C41 0008      cmpiw #8, d1
053A: 65EA           bcs -22
053C: 7000           moveq #0, d0
053E: 3001           movew d1, d0
0540: 41ED FF6E      lea -146(a5), d0
0544: 4230 0800      clrb.ex 0(a0,d0.l)
0548: 7200           moveq #0, d1
054A: 6010           bra 16
054C: 7000           moveq #0, d0
054E: 3001           movew d1, d0
0550: 41ED FF6A      lea -150(a5), d0
0554: 11BC 0030 0800 moveb.emx #48, 0(a0,d0.l)
055A: 5241           addqw #1, d1
055C: 0C41 0002      cmpiw #2, d1
0560: 65EA           bcs -22
0562: 7000           moveq #0, d0
0564: 3001           movew d1, d0
0566: 41ED FF6A      lea -150(a5), d0
056A: 4230 0800      clrb.ex 0(a0,d0.l)
056E: 422D FF69      clrb.ex -151(a5)
0572: 4E5E           unlk a6
0574: 4E75           rts
0576: 9244           subrw d4, d1
0578: 6973           bvs 115
057A: 706C           moveq #108, d0
057C: 6179           bsr 121
057E: 5374 7269      subqw.ex #1, 105(a4,d7.w)
0582: 6E67           bgt 103
0584: 5265           addqw #1, -(a5)
0586: 7365           dc.w #29541
0588: 7400           moveq #0, d2
058A: 0000           dc.w #0
058C: 4E56 FFF0      link a6, #-16
0590: 48E7 1800      movem <1800>, -(a7)
0594: 7600           moveq #0, d3
0596: 4A2D FF79      tstb.ex -135(a5)
059A: 6712           beq 18
059C: 3003           movew d3, d0
059E: 5243           addqw #1, d3
05A0: 7200           moveq #0, d1
05A2: 3200           movew d0, d1
05A4: 41EE FFF0      lea -16(a6), d0
05A8: 11AD FF79 1800 moveb.emx -135(a5), 0(a0,d1.l)
05AE: 7800           moveq #0, d4
05B0: 601C           bra 28
05B2: 7000           moveq #0, d0
05B4: 3004           movew d4, d0
05B6: 41ED FF6E      lea -146(a5), d0
05BA: 3203           movew d3, d1
05BC: 5243           addqw #1, d3
05BE: 7400           moveq #0, d2
05C0: 3401           movew d1, d2
05C2: 43EE FFF0      lea -16(a6), d1
05C6: 13B0 0800 2800 moveb.emx 0(a0,d0.l), 0(a1,d2.l)
05CC: 5244           addqw #1, d4
05CE: B86D FF7C      cmpw.ex -132(a5), d4
05D2: 65DE           bcs -34
05D4: 4A2D FF69      tstb.ex -151(a5)
05D8: 674C           beq 76
05DA: 3003           movew d3, d0
05DC: 5243           addqw #1, d3
05DE: 7200           moveq #0, d1
05E0: 3200           movew d0, d1
05E2: 41EE FFF0      lea -16(a6), d0
05E6: 11BC 0065 1800 moveb.emx #101, 0(a0,d1.l)
05EC: 4A2D FF78      tstb.ex -136(a5)
05F0: 670E           beq 14
05F2: 3003           movew d3, d0
05F4: 5243           addqw #1, d3
05F6: 7200           moveq #0, d1
05F8: 3200           movew d0, d1
05FA: 11AD FF78 1800 moveb.emx -136(a5), 0(a0,d1.l)
0600: 7800           moveq #0, d4
0602: 601C           bra 28
0604: 7000           moveq #0, d0
0606: 3004           movew d4, d0
0608: 41ED FF6A      lea -150(a5), d0
060C: 3203           movew d3, d1
060E: 5243           addqw #1, d3
0610: 7400           moveq #0, d2
0612: 3401           movew d1, d2
0614: 43EE FFF0      lea -16(a6), d1
0618: 13B0 0800 2800 moveb.emx 0(a0,d0.l), 0(a1,d2.l)
061E: 5244           addqw #1, d4
0620: B86D FF7A      cmpw.ex -134(a5), d4
0624: 65DE           bcs -34
0626: 3003           movew d3, d0
0628: 5243           addqw #1, d3
062A: 7200           moveq #0, d1
062C: 3200           movew d0, d1
062E: 41EE FFF0      lea -16(a6), d0
0632: 4230 1800      clrb.ex 0(a0,d1.l)
0636: 486E FFF0      pea.ex -16(a6)
063A: 486D FF5C      pea.ex -164(a5)
063E: 7402           moveq #2, d2
0640: 4E4F           trap #15
0642: A305           sysTrapFlpDispatch
0644: 4EBA FEB2      jsr.ex 30976(pc)
0648: 504F           addqw #0, a7
064A: 4CDF 0018      movem (a7)+, <0018>
064E: 4E5E           unlk a6
0650: 4E75           rts
0652: 9A44           subrw d4, d5
0654: 6973           bvs 115
0656: 706C           moveq #108, d0
0658: 6179           bsr 121
065A: 5374 7269      subqw.ex #1, 105(a4,d7.w)
065E: 6E67           bgt 103
0660: 436F           dc.w #17263
0662: 6E76           bgt 118
0664: 6572           bcs 114
0666: 7454           moveq #84, d2
0668: 6F58           ble 88
066A: 5265           addqw #1, -(a5)
066C: 6700 0000      beq 0
0670: 4E56 FFD2      link a6, #-46
0674: 48E7 1F30      movem <1f30>, -(a7)
0678: 3D7C 000A FFEA movew.emx #10, -22(a6)
067E: 3D7C 0003 FFEC movew.emx #3, -20(a6)
0684: 3D7C 0087 FFEE movew.emx #135, -18(a6)
068A: 3D7C 001B FFF0 movew.emx #27, -16(a6)
0690: 4267           clrw -(a7)
0692: 486E FFEA      pea.ex -22(a6)
0696: 4E4F           trap #15
0698: A219           sysTrapWinEraseRectangle
069A: 1F3C 0006      moveb.ex #6, -(a7)
069E: 4E4F           trap #15
06A0: A164           sysTrapFntSetFont
06A2: 4A6D FF7C      tstw.ex -132(a5)
06A6: 504F           addqw #0, a7
06A8: 6600 01D8      bne 472
06AC: 42AE FFE6      clrl.ex -26(a6)
06B0: 426E FFFE      clrw.ex -2(a6)
06B4: 426E FFE4      clrw.ex -28(a6)
06B8: 7A00           moveq #0, d5
06BA: 7600           moveq #0, d3
06BC: 7C00           moveq #0, d6
06BE: 486E FFE4      pea.ex -28(a6)
06C2: 486E FFFE      pea.ex -2(a6)
06C6: 486E FFE6      pea.ex -26(a6)
06CA: 2F2D FF60      movel.ex -160(a5), -(a7)
06CE: 2F2D FF5C      movel.ex -164(a5), -(a7)
06D2: 7400           moveq #0, d2
06D4: 4E4F           trap #15
06D6: A305           sysTrapFlpDispatch
06D8: 3D40 FFD2      movew.mx d0, -46(a6)
06DC: 0C40 0681      cmpiw #1665, d0
06E0: 4FEF 0014      lea 20(a7), d7
06E4: 661A           bne 26
06E6: 3F3C 000A      movew.ex #10, -(a7)
06EA: 2F3C 0003 004A movel.exl #196682, -(a7)
06F0: 486D FF80      pea.ex -128(a5)
06F4: 4E4F           trap #15
06F6: A220           sysTrapWinDrawChars
06F8: 4FEF 000A      lea 10(a7), d7
06FC: 6000 0268      bra 616
0700: 4AAE FFE6      tstl.ex -26(a6)
0704: 661A           bne 26
0706: 486D FF84      pea.ex -124(a5)
070A: 486E FFDA      pea.ex -38(a6)
070E: 4E4F           trap #15
0710: A0C5           sysTrapStrCopy
0712: 3D7C FFF9 FFFE movew.emx #65529, -2(a6)
0718: 426E FFE4      clrw.ex -28(a6)
071C: 504F           addqw #0, a7
071E: 600E           bra 14
0720: 2F2E FFE6      movel.ex -26(a6), -(a7)
0724: 486E FFDA      pea.ex -38(a6)
0728: 4E4F           trap #15
072A: A0C9           sysTrapStrIToA
072C: 504F           addqw #0, a7
072E: 4A6E FFE4      tstw.ex -28(a6)
0732: 6716           beq 22
0734: 3F3C 000A      movew.ex #10, -(a7)
0738: 2F3C 0001 000A movel.exl #65546, -(a7)
073E: 486D FF8E      pea.ex -114(a5)
0742: 4E4F           trap #15
0744: A220           sysTrapWinDrawChars
0746: 4FEF 000A      lea 10(a7), d7
074A: 4A6E FFFE      tstw.ex -2(a6)
074E: 6E18           bgt 24
0750: 70F8           moveq #248, d0
0752: 906D FF7E      subrw.ex -130(a5), d0
0756: B06E FFFE      cmpw.ex -2(a6), d0
075A: 6C0C           bge 12
075C: 3A2E FFFE      movew.ex -2(a6), d5
0760: 5045           addqw #0, d5
0762: 426E FFFE      clrw.ex -2(a6)
0766: 6006           bra 6
0768: 7A01           moveq #1, d5
076A: 5E6E FFFE      addqw.ex #7, -2(a6)
076E: 0C45 0001      cmpiw #1, d5
0772: 6C2A           bge 42
0774: 3003           movew d3, d0
0776: 5243           addqw #1, d3
0778: 41EE FFF2      lea -14(a6), d0
077C: 11BC 0030 0000 moveb.emx #48, 0(a0,d0.w)
0782: 3003           movew d3, d0
0784: 5243           addqw #1, d3
0786: 11BC 002E 0000 moveb.emx #46, 0(a0,d0.w)
078C: 6010           bra 16
078E: 3003           movew d3, d0
0790: 5243           addqw #1, d3
0792: 41EE FFF2      lea -14(a6), d0
0796: 11BC 0030 0000 moveb.emx #48, 0(a0,d0.w)
079C: 5246           addqw #1, d6
079E: 3005           movew d5, d0
07A0: D046           addrw d6, d0
07A2: 4A40           tstw d0
07A4: 6DE8           blt -24
07A6: 45EE FFDA      lea -38(a6), d2
07AA: 47EE FFF2      lea -14(a6), d3
07AE: 6022           bra 34
07B0: 3003           movew d3, d0
07B2: 5243           addqw #1, d3
07B4: 41EE FFF2      lea -14(a6), d0
07B8: 119A 0000      moveb.mx (a2)+, 0(a0,d0.w)
07BC: B645           cmpw d5, d3
07BE: 660C           bne 12
07C0: 3003           movew d3, d0
07C2: 5243           addqw #1, d3
07C4: 11BC 002E 0000 moveb.emx #46, 0(a0,d0.w)
07CA: 6006           bra 6
07CC: B645           cmpw d5, d3
07CE: 6F02           ble 2
07D0: 5246           addqw #1, d6
07D2: 0C43 0009      cmpiw #9, d3
07D6: 6C06           bge 6
07D8: BC6D FF7E      cmpw.ex -130(a5), d6
07DC: 6DD2           blt -46
07DE: 3003           movew d3, d0
07E0: 5243           addqw #1, d3
07E2: 41EE FFF2      lea -14(a6), d0
07E6: 4230 0000      clrb.ex 0(a0,d0.w)
07EA: 2F3C 0012 000A movel.exl #1179658, -(a7)
07F0: 486E FFF2      pea.ex -14(a6)
07F4: 4E4F           trap #15
07F6: A0C7           sysTrapStrLen
07F8: 584F           addqw #4, a7
07FA: 3F00           movew d0, -(a7)
07FC: 486E FFF2      pea.ex -14(a6)
0800: 4E4F           trap #15
0802: A220           sysTrapWinDrawChars
0804: 4A6E FFFE      tstw.ex -2(a6)
0808: 4FEF 000A      lea 10(a7), d7
080C: 6700 0158      beq 344
0810: 4A6E FFFE      tstw.ex -2(a6)
0814: 6C20           bge 32
0816: 3F3C 000A      movew.ex #10, -(a7)
081A: 2F3C 0001 0071 movel.exl #65649, -(a7)
0820: 486D FF8E      pea.ex -114(a5)
0824: 4E4F           trap #15
0826: A220           sysTrapWinDrawChars
0828: 302E FFFE      movew.ex -2(a6), d0
082C: 4440           negw d0
082E: 3D40 FFFE      movew.mx d0, -2(a6)
0832: 4FEF 000A      lea 10(a7), d7
0836: 306E FFFE      movew.ex -2(a6), a0
083A: 2008           movel a0, d0
083C: 81FC 000A      divs.ex #10, d0
0840: 4840           swap d0
0842: 0640 0030      addiw #48, d0
0846: 1D40 FFD9      moveb.mx d0, -39(a6)
084A: 3008           movew a0, d0
084C: 48C0           extl d0
084E: 81FC 000A      divs.ex #10, d0
0852: 3D40 FFFE      movew.mx d0, -2(a6)
0856: 3040           movew d0, a0
0858: 2008           movel a0, d0
085A: 81FC 000A      divs.ex #10, d0
085E: 4840           swap d0
0860: 0640 0030      addiw #48, d0
0864: 1D40 FFD8      moveb.mx d0, -40(a6)
0868: 3F3C 000A      movew.ex #10, -(a7)
086C: 2F3C 0002 0079 movel.exl #131193, -(a7)
0872: 486E FFD8      pea.ex -40(a6)
0876: 4E4F           trap #15
0878: A220           sysTrapWinDrawChars
087A: 4FEF 000A      lea 10(a7), d7
087E: 6000 00E6      bra 230
0882: 4A2D FF79      tstb.ex -135(a5)
0886: 6716           beq 22
0888: 3F3C 000A      movew.ex #10, -(a7)
088C: 2F3C 0001 000A movel.exl #65546, -(a7)
0892: 486D FF79      pea.ex -135(a5)
0896: 4E4F           trap #15
0898: A220           sysTrapWinDrawChars
089A: 4FEF 000A      lea 10(a7), d7
089E: 4A6D FF7C      tstw.ex -132(a5)
08A2: 672C           beq 44
08A4: 4A6D FF7C      tstw.ex -132(a5)
08A8: 6708           beq 8
08AA: 3D6D FF7C FFD4 movew.emx -132(a5), -44(a6)
08B0: 6006           bra 6
08B2: 3D7C 0008 FFD4 movew.emx #8, -44(a6)
08B8: 2F3C 0012 000A movel.exl #1179658, -(a7)
08BE: 3F2E FFD4      movew.ex -44(a6), -(a7)
08C2: 486D FF6E      pea.ex -146(a5)
08C6: 4E4F           trap #15
08C8: A220           sysTrapWinDrawChars
08CA: 4FEF 000A      lea 10(a7), d7
08CE: 6016           bra 22
08D0: 3F3C 000A      movew.ex #10, -(a7)
08D4: 2F3C 0006 0012 movel.exl #393234, -(a7)
08DA: 486D FF90      pea.ex -112(a5)
08DE: 4E4F           trap #15
08E0: A220           sysTrapWinDrawChars
08E2: 4FEF 000A      lea 10(a7), d7
08E6: 4A2D FF69      tstb.ex -151(a5)
08EA: 677A           beq 122
08EC: 7E00           moveq #0, d7
08EE: 382D FF7A      movew.ex -134(a5), d4
08F2: 6014           bra 20
08F4: 3007           movew d7, d0
08F6: 5247           addqw #1, d7
08F8: 7200           moveq #0, d1
08FA: 3200           movew d0, d1
08FC: 41EE FFD6      lea -42(a6), d0
0900: 11BC 0030 1800 moveb.emx #48, 0(a0,d1.l)
0906: 5244           addqw #1, d4
0908: 0C44 0002      cmpiw #2, d4
090C: 65E6           bcs -26
090E: 7800           moveq #0, d4
0910: 601C           bra 28
0912: 7000           moveq #0, d0
0914: 3004           movew d4, d0
0916: 41ED FF6A      lea -150(a5), d0
091A: 3207           movew d7, d1
091C: 5247           addqw #1, d7
091E: 7400           moveq #0, d2
0920: 3401           movew d1, d2
0922: 43EE FFD6      lea -42(a6), d1
0926: 13B0 0800 2800 moveb.emx 0(a0,d0.l), 0(a1,d2.l)
092C: 5244           addqw #1, d4
092E: B86D FF7A      cmpw.ex -134(a5), d4
0932: 65DE           bcs -34
0934: 4A2D FF78      tstb.ex -136(a5)
0938: 6716           beq 22
093A: 3F3C 000A      movew.ex #10, -(a7)
093E: 2F3C 0001 0071 movel.exl #65649, -(a7)
0944: 486D FF78      pea.ex -136(a5)
0948: 4E4F           trap #15
094A: A220           sysTrapWinDrawChars
094C: 4FEF 000A      lea 10(a7), d7
0950: 3F3C 000A      movew.ex #10, -(a7)
0954: 2F3C 0002 0079 movel.exl #131193, -(a7)
095A: 486E FFD6      pea.ex -42(a6)
095E: 4E4F           trap #15
0960: A220           sysTrapWinDrawChars
0962: 4FEF 000A      lea 10(a7), d7
0966: 4CDF 0CF8      movem (a7)+, <0cf8>
096A: 4E5E           unlk a6
096C: 4E75           rts
096E: 8D44           dc.w #36164
0970: 6973           bvs 115
0972: 706C           moveq #108, d0
0974: 6179           bsr 121
0976: 5570 6461      subqw.ex #2, 97(a0,d6.w)
097A: 7465           moveq #101, d2
097C: 0000           dc.w #0
097E: 4E56 0000      link a6, #0
0982: 4A6D FF7C      tstw.ex -132(a5)
0986: 6704           beq 4
0988: 4EBA FC02      jsr.ex 31124(pc)
098C: 1F3C 0001      moveb.ex #1, -(a7)
0990: 4EBA F7F0      jsr.ex 30090(pc)
0994: 7001           moveq #1, d0
0996: 4E5E           unlk a6
0998: 4E75           rts
099A: 8D46           dc.w #36166
099C: 756E           dc.w #30062
099E: 6374           bls 116
09A0: 696F           bvs 111
09A2: 6E50           bgt 80
09A4: 7573           dc.w #30067
09A6: 6858           bvc 88
09A8: 0000           dc.w #0
09AA: 4E56 FFD0      link a6, #-48
09AE: 4A6D FF7C      tstw.ex -132(a5)
09B2: 6704           beq 4
09B4: 4EBA FBD6      jsr.ex 31124(pc)
09B8: 42AE FFD0      clrl.ex -48(a6)
09BC: 42AE FFD4      clrl.ex -44(a6)
09C0: 7000           moveq #0, d0
09C2: 102E 0008      moveb.ex 8(a6), d0
09C6: 0C40 0005      cmpiw #5, d0
09CA: 6200 00EE      bhi 238
09CE: D040           addrw d0, d0
09D0: 303B 0006      movew.ex 6(pc,d0.w), d0
09D4: 4EFB 0002      jmp.ex 2(pc,d0.w)
09D8: 00E2           dc.w #226
09DA: 000C           dc.w #12
09DC: 003A           dc.w #58
09DE: 0068 0092 00BC oriw.ex #146, 188(a0)
09E4: 4267           clrw -(a7)
09E6: 2F2D FF60      movel.ex -160(a5), -(a7)
09EA: 2F2D FF5C      movel.ex -164(a5), -(a7)
09EE: 486E FFF0      pea.ex -16(a6)
09F2: 4EBA F7E2      jsr.ex 30174(pc)
09F6: 584F           addqw #4, a7
09F8: 2F2E FFF4      movel.ex -12(a6), -(a7)
09FC: 2F2E FFF0      movel.ex -16(a6), -(a7)
0A00: 486D FF5C      pea.ex -164(a5)
0A04: 7403           moveq #3, d2
0A06: 4E4F           trap #15
0A08: A305           sysTrapFlpDispatch
0A0A: 4FEF 0016      lea 22(a7), d7
0A0E: 6000 00BE      bra 190
0A12: 4267           clrw -(a7)
0A14: 2F2D FF60      movel.ex -160(a5), -(a7)
0A18: 2F2D FF5C      movel.ex -164(a5), -(a7)
0A1C: 486E FFE8      pea.ex -24(a6)
0A20: 4EBA F7B4      jsr.ex 30174(pc)
0A24: 584F           addqw #4, a7
0A26: 2F2E FFEC      movel.ex -20(a6), -(a7)
0A2A: 2F2E FFE8      movel.ex -24(a6), -(a7)
0A2E: 486D FF5C      pea.ex -164(a5)
0A32: 7404           moveq #4, d2
0A34: 4E4F           trap #15
0A36: A305           sysTrapFlpDispatch
0A38: 4FEF 0016      lea 22(a7), d7
0A3C: 6000 0090      bra 144
0A40: 2F2D FF60      movel.ex -160(a5), -(a7)
0A44: 2F2D FF5C      movel.ex -164(a5), -(a7)
0A48: 486E FFE0      pea.ex -32(a6)
0A4C: 4EBA F788      jsr.ex 30174(pc)
0A50: 584F           addqw #4, a7
0A52: 2F2E FFE4      movel.ex -28(a6), -(a7)
0A56: 2F2E FFE0      movel.ex -32(a6), -(a7)
0A5A: 486D FF5C      pea.ex -164(a5)
0A5E: 7434           moveq #52, d2
0A60: 4E4F           trap #15
0A62: A306           sysTrapFlpEmDispatch
0A64: 4FEF 0014      lea 20(a7), d7
0A68: 6064           bra 100
0A6A: 2F2D FF60      movel.ex -160(a5), -(a7)
0A6E: 2F2D FF5C      movel.ex -164(a5), -(a7)
0A72: 486E FFD8      pea.ex -40(a6)
0A76: 4EBA F75E      jsr.ex 30174(pc)
0A7A: 584F           addqw #4, a7
0A7C: 2F2E FFDC      movel.ex -36(a6), -(a7)
0A80: 2F2E FFD8      movel.ex -40(a6), -(a7)
0A84: 486D FF5C      pea.ex -164(a5)
0A88: 7436           moveq #54, d2
0A8A: 4E4F           trap #15
0A8C: A306           sysTrapFlpEmDispatch
0A8E: 4FEF 0014      lea 20(a7), d7
0A92: 603A           bra 58
0A94: 2D6D FF5C FFF8 movel.emx -164(a5), -8(a6)
0A9A: 2D6D FF60 FFFC movel.emx -160(a5), -4(a6)
0AA0: 2B6D FF54 FF5C movel.emx -172(a5), -164(a5)
0AA6: 2B6D FF58 FF60 movel.emx -168(a5), -160(a5)
0AAC: 2B6E FFF8 FF54 movel.emx -8(a6), -172(a5)
0AB2: 2B6E FFFC FF58 movel.emx -4(a6), -168(a5)
0AB8: 6014           bra 20
0ABA: 486D FFA6      pea.ex -90(a5)
0ABE: 3F3C 02AF      movew.ex #687, -(a7)
0AC2: 486D FF98      pea.ex -104(a5)
0AC6: 4E4F           trap #15
0AC8: A084           sysTrapErrDisplayFileLineMsg
0ACA: 4FEF 000A      lea 10(a7), d7
0ACE: 1B7C 0001 FF67 moveb.emx #1, -153(a5)
0AD4: 7001           moveq #1, d0
0AD6: 4E5E           unlk a6
0AD8: 4E75           rts
0ADA: 9146           subxrw d6, d0
0ADC: 756E           dc.w #30062
0ADE: 6374           bls 116
0AE0: 696F           bvs 111
0AE2: 6E43           bgt 67
0AE4: 616C           bsr 108
0AE6: 6375           bls 117
0AE8: 6C61           bge 97
0AEA: 7465           moveq #101, d2
0AEC: 0000           dc.w #0
0AEE: 4E56 0000      link a6, #0
0AF2: 2F0A           movel a2, -(a7)
0AF4: 302E 0008      movew.ex 8(a6), d0
0AF8: 0440 03E8      subiw #1000, d0
0AFC: 6702           beq 2
0AFE: 6020           bra 32
0B00: 42A7           clrl -(a7)
0B02: 4E4F           trap #15
0B04: A1C1           sysTrapMenuEraseStatus
0B06: 3F3C 044C      movew.ex #1100, -(a7)
0B0A: 4E4F           trap #15
0B0C: A16F           sysTrapFrmInitForm
0B0E: 2448           movel a0, a2
0B10: 2F0A           movel a2, -(a7)
0B12: 4E4F           trap #15
0B14: A193           sysTrapFrmDoDialog
0B16: 2F0A           movel a2, -(a7)
0B18: 4E4F           trap #15
0B1A: A170           sysTrapFrmDeleteForm
0B1C: 4FEF 000E      lea 14(a7), d7
0B20: 245F           movel (a7)+, a2
0B22: 4E5E           unlk a6
0B24: 4E75           rts
0B26: 914D           subxmw -(a5), -(a0)
0B28: 6169           bsr 105
0B2A: 6E46           bgt 70
0B2C: 6F72           ble 114
0B2E: 6D44           blt 68
0B30: 6F43           ble 67
0B32: 6F6D           ble 109
0B34: 6D61           blt 97
0B36: 6E64           bgt 100
0B38: 0000           dc.w #0
0B3A: 4E56 FFFE      link a6, #-2
0B3E: 48E7 1F30      movem <1f30>, -(a7)
0B42: 246E 0008      movel.ex 8(a6), a2
0B46: 7C00           moveq #0, d6
0B48: 7600           moveq #0, d3
0B4A: 7A00           moveq #0, d5
0B4C: 7800           moveq #0, d4
0B4E: 3012           movew (a2), d0
0B50: 5940           subqw #4, d0
0B52: 6700 0148      beq 328
0B56: 5B40           subqw #5, d0
0B58: 6732           beq 50
0B5A: 0440 000C      subiw #12, d0
0B5E: 6708           beq 8
0B60: 5740           subqw #3, d0
0B62: 6714           beq 20
0B64: 6000 01D2      bra 466
0B68: 3F2A 0008      movew.ex 8(a2), -(a7)
0B6C: 4EBA FF80      jsr.ex 32502(pc)
0B70: 7C01           moveq #1, d6
0B72: 544F           addqw #2, a7
0B74: 6000 01C2      bra 450
0B78: 4E4F           trap #15
0B7A: A173           sysTrapFrmGetActiveForm
0B7C: 2648           movel a0, a3
0B7E: 2F0B           movel a3, -(a7)
0B80: 4E4F           trap #15
0B82: A171           sysTrapFrmDrawForm
0B84: 7C01           moveq #1, d6
0B86: 584F           addqw #4, a7
0B88: 6000 01AE      bra 430
0B8C: 7C01           moveq #1, d6
0B8E: 302A 0008      movew.ex 8(a2), d0
0B92: 0440 03F5      subiw #1013, d0
0B96: 0C40 0027      cmpiw #39, d0
0B9A: 6200 00C0      bhi 192
0B9E: D040           addrw d0, d0
0BA0: 303B 0006      movew.ex 6(pc,d0.w), d0
0BA4: 4EFB 0002      jmp.ex 2(pc,d0.w)
0BA8: 0050 0078      oriw #120, (a0)
0BAC: 00A0 00B4 00B4 oril #11796660, -(a0)
0BB2: 00B4 00B4 00B4 0098 oril.ex #11796660, 152(a4,d0.w)
0BBA: 0054 0058      oriw #88, (a4)
0BBE: 005C 00A4      oriw #164, (a4)+
0BC2: 00B4 00B4 00B4 00B4 oril.ex #11796660, 180(a4,d0.w)
0BCA: 00B4 0088 0060 0064 oril.ex #8912992, 100(a4,d0.w)
0BD2: 0068 00A8 00B4 oriw.ex #168, 180(a0)
0BD8: 00B4 00B4 00B4 00B4 oril.ex #11796660, 180(a4,d0.w)
0BE0: 0080 006C 0070 oril #7078000, d0
0BE6: 0074 00AC 00B4 oriw.ex #172, 180(a4,d0.w)
0BEC: 00B4 00B4 00B4 00B4 oril.ex #11796660, 180(a4,d0.w)
0BF4: 0090 00B0 7A30 oril #11565616, (a0)
0BFA: 6062           bra 98
0BFC: 7A31           moveq #49, d5
0BFE: 605E           bra 94
0C00: 7A32           moveq #50, d5
0C02: 605A           bra 90
0C04: 7A33           moveq #51, d5
0C06: 6056           bra 86
0C08: 7A34           moveq #52, d5
0C0A: 6052           bra 82
0C0C: 7A35           moveq #53, d5
0C0E: 604E           bra 78
0C10: 7A36           moveq #54, d5
0C12: 604A           bra 74
0C14: 7A37           moveq #55, d5
0C16: 6046           bra 70
0C18: 7A38           moveq #56, d5
0C1A: 6042           bra 66
0C1C: 7A39           moveq #57, d5
0C1E: 603E           bra 62
0C20: 4EBA F6DA      jsr.ex 30468(pc)
0C24: 1600           moveb d0, d3
0C26: 6036           bra 54
0C28: 4EBA F75E      jsr.ex 30608(pc)
0C2C: 1600           moveb d0, d3
0C2E: 602E           bra 46
0C30: 4EBA F7C8      jsr.ex 30722(pc)
0C34: 1600           moveb d0, d3
0C36: 6026           bra 38
0C38: 4EBA F820      jsr.ex 30818(pc)
0C3C: 1600           moveb d0, d3
0C3E: 601E           bra 30
0C40: 4EBA FD3C      jsr.ex 32134(pc)
0C44: 1600           moveb d0, d3
0C46: 6016           bra 22
0C48: 7801           moveq #1, d4
0C4A: 6012           bra 18
0C4C: 7802           moveq #2, d4
0C4E: 600E           bra 14
0C50: 7803           moveq #3, d4
0C52: 600A           bra 10
0C54: 7804           moveq #4, d4
0C56: 6006           bra 6
0C58: 7805           moveq #5, d4
0C5A: 6002           bra 2
0C5C: 7C00           moveq #0, d6
0C5E: 4A05           tstb d5
0C60: 670C           beq 12
0C62: 1F05           moveb d5, -(a7)
0C64: 4EBA F5BE      jsr.ex 30252(pc)
0C68: 1600           moveb d0, d3
0C6A: 544F           addqw #2, a7
0C6C: 600E           bra 14
0C6E: 4A04           tstb d4
0C70: 670A           beq 10
0C72: 1F04           moveb d4, -(a7)
0C74: 4EBA FD34      jsr.ex 32178(pc)
0C78: 1600           moveb d0, d3
0C7A: 544F           addqw #2, a7
0C7C: 4A03           tstb d3
0C7E: 6708           beq 8
0C80: 1D7C 0007 FFFF moveb.emx #7, -1(a6)
0C86: 6006           bra 6
0C88: 1D7C 0007 FFFF moveb.emx #7, -1(a6)
0C8E: 1F2E FFFF      moveb.ex -1(a6), -(a7)
0C92: 4E4F           trap #15
0C94: A234           sysTrapSndPlaySystemSound
0C96: 544F           addqw #2, a7
0C98: 6000 009E      bra 158
0C9C: 1E2A 0009      moveb.ex 9(a2), d7
0CA0: 0C07 0030      cmpib #48, d7
0CA4: 6D14           blt 20
0CA6: 0C07 0039      cmpib #57, d7
0CAA: 6E0E           bgt 14
0CAC: 7C01           moveq #1, d6
0CAE: 1F07           moveb d7, -(a7)
0CB0: 4EBA F572      jsr.ex 30252(pc)
0CB4: 1600           moveb d0, d3
0CB6: 544F           addqw #2, a7
0CB8: 6070           bra 112
0CBA: 7C01           moveq #1, d6
0CBC: 1007           moveb d7, d0
0CBE: 4880           extw d0
0CC0: 5D40           subqw #6, d0
0CC2: 674C           beq 76
0CC4: 5540           subqw #2, d0
0CC6: 6740           beq 64
0CC8: 0440 0022      subiw #34, d0
0CCC: 6752           beq 82
0CCE: 5340           subqw #1, d0
0CD0: 6746           beq 70
0CD2: 5540           subqw #2, d0
0CD4: 6746           beq 70
0CD6: 5340           subqw #1, d0
0CD8: 6716           beq 22
0CDA: 5340           subqw #1, d0
0CDC: 6746           beq 70
0CDE: 0440 000E      subiw #14, d0
0CE2: 6714           beq 20
0CE4: 5140           subqw #0, d0
0CE6: 6718           beq 24
0CE8: 0440 0020      subiw #32, d0
0CEC: 6712           beq 18
0CEE: 6038           bra 56
0CF0: 4EBA F60A      jsr.ex 30468(pc)
0CF4: 1600           moveb d0, d3
0CF6: 6032           bra 50
0CF8: 4EBA F68E      jsr.ex 30608(pc)
0CFC: 1600           moveb d0, d3
0CFE: 602A           bra 42
0D00: 4EBA F6F8      jsr.ex 30722(pc)
0D04: 1600           moveb d0, d3
0D06: 6022           bra 34
0D08: 4EBA F750      jsr.ex 30818(pc)
0D0C: 1600           moveb d0, d3
0D0E: 601A           bra 26
0D10: 4EBA FC6C      jsr.ex 32134(pc)
0D14: 1600           moveb d0, d3
0D16: 6012           bra 18
0D18: 7801           moveq #1, d4
0D1A: 600E           bra 14
0D1C: 7802           moveq #2, d4
0D1E: 600A           bra 10
0D20: 7803           moveq #3, d4
0D22: 6006           bra 6
0D24: 7804           moveq #4, d4
0D26: 6002           bra 2
0D28: 7C00           moveq #0, d6
0D2A: 4A04           tstb d4
0D2C: 670A           beq 10
0D2E: 1F04           moveb d4, -(a7)
0D30: 4EBA FC78      jsr.ex 32178(pc)
0D34: 1600           moveb d0, d3
0D36: 544F           addqw #2, a7
0D38: 4A03           tstb d3
0D3A: 6704           beq 4
0D3C: 4EBA F932      jsr.ex 31352(pc)
0D40: 1006           moveb d6, d0
0D42: 4CDF 0CF8      movem (a7)+, <0cf8>
0D46: 4E5E           unlk a6
0D48: 4E75           rts
0D4A: 934D           subxmw -(a5), -(a1)
0D4C: 6169           bsr 105
0D4E: 6E46           bgt 70
0D50: 6F72           ble 114
0D52: 6D48           blt 72
0D54: 616E           bsr 110
0D56: 646C           bcc 108
0D58: 6545           bcs 69
0D5A: 7665           moveq #101, d3
0D5C: 6E74           bgt 116
0D5E: 0000           dc.w #0
0D60: 4E56 0000      link a6, #0
0D64: 48E7 1030      movem <1030>, -(a7)
0D68: 266E 0008      movel.ex 8(a6), a3
0D6C: 0C53 0017      cmpiw #23, (a3)
0D70: 662E           bne 46
0D72: 362B 0008      movew.ex 8(a3), d3
0D76: 3F03           movew d3, -(a7)
0D78: 4E4F           trap #15
0D7A: A16F           sysTrapFrmInitForm
0D7C: 2448           movel a0, a2
0D7E: 2F0A           movel a2, -(a7)
0D80: 4E4F           trap #15
0D82: A174           sysTrapFrmSetActiveForm
0D84: 5C4F           addqw #6, a7
0D86: 3003           movew d3, d0
0D88: 0440 03E8      subiw #1000, d0
0D8C: 6702           beq 2
0D8E: 600C           bra 12
0D90: 487A FDA8      pea.ex 32578(pc)
0D94: 2F0A           movel a2, -(a7)
0D96: 4E4F           trap #15
0D98: A19F           sysTrapFrmSetEventHandler
0D9A: 504F           addqw #0, a7
0D9C: 7001           moveq #1, d0
0D9E: 6002           bra 2
0DA0: 7000           moveq #0, d0
0DA2: 4CDF 0C08      movem (a7)+, <0c08>
0DA6: 4E5E           unlk a6
0DA8: 4E75           rts
0DAA: 8E41           orrw d1, d7
0DAC: 7070           moveq #112, d0
0DAE: 4861           dc.w #18529
0DB0: 6E64           bgt 100
0DB2: 6C65           bge 101
0DB4: 4576           dc.w #17782
0DB6: 656E           bcs 110
0DB8: 7400           moveq #0, d2
0DBA: 0000           dc.w #0
0DBC: 4E56 FFE6      link a6, #-26
0DC0: 4878 FFFF      pea.ex (ffff).w
0DC4: 486E FFE8      pea.ex -24(a6)
0DC8: 4E4F           trap #15
0DCA: A11D           sysTrapEvtGetEvent
0DCC: 486E FFE8      pea.ex -24(a6)
0DD0: 4E4F           trap #15
0DD2: A0A9           sysTrapSysHandleEvent
0DD4: 4A00           tstb d0
0DD6: 4FEF 000C      lea 12(a7), d7
0DDA: 662E           bne 46
0DDC: 486E FFE6      pea.ex -26(a6)
0DE0: 486E FFE8      pea.ex -24(a6)
0DE4: 42A7           clrl -(a7)
0DE6: 4E4F           trap #15
0DE8: A1BF           sysTrapMenuHandleEvent
0DEA: 4A00           tstb d0
0DEC: 4FEF 000C      lea 12(a7), d7
0DF0: 6618           bne 24
0DF2: 486E FFE8      pea.ex -24(a6)
0DF6: 4EBA FF68      jsr.ex -32408(pc)
0DFA: 4A00           tstb d0
0DFC: 584F           addqw #4, a7
0DFE: 660A           bne 10
0E00: 486E FFE8      pea.ex -24(a6)
0E04: 4E4F           trap #15
0E06: A1A0           sysTrapFrmDispatchEvent
0E08: 584F           addqw #4, a7
0E0A: 0C6E 0016 FFE8 cmpiw.ex #22, -24(a6)
0E10: 66AE           bne -82
0E12: 4E5E           unlk a6
0E14: 4E75           rts
0E16: 8945           dc.w #35141
0E18: 7665           moveq #101, d3
0E1A: 6E74           bgt 116
0E1C: 4C6F           dc.w #19567
0E1E: 6F70           ble 112
0E20: 0000           dc.w #0
0E22: 4E56 0000      link a6, #0
0E26: 3F3C 03E8      movew.ex #1000, -(a7)
0E2A: 4E4F           trap #15
0E2C: A19B           sysTrapFrmGotoForm
0E2E: 4EBA F6C8      jsr.ex 30976(pc)
0E32: 3B7C 0005 FF7E movew.emx #5, -130(a5)
0E38: 42AD FF5C      clrl.ex -164(a5)
0E3C: 42AD FF60      clrl.ex -160(a5)
0E40: 42AD FF54      clrl.ex -172(a5)
0E44: 42AD FF58      clrl.ex -168(a5)
0E48: 42AD FF4C      clrl.ex -180(a5)
0E4C: 42AD FF50      clrl.ex -176(a5)
0E50: 42AD FF44      clrl.ex -188(a5)
0E54: 42AD FF48      clrl.ex -184(a5)
0E58: 422D FF67      clrb.ex -153(a5)
0E5C: 7000           moveq #0, d0
0E5E: 4E5E           unlk a6
0E60: 4E75           rts
0E62: 9053           subrw (a3), d0
0E64: 7461           moveq #97, d2
0E66: 7274           moveq #116, d1
0E68: 4170           dc.w #16752
0E6A: 706C           moveq #108, d0
0E6C: 6963           bvs 99
0E6E: 6174           bsr 116
0E70: 696F           bvs 111
0E72: 6E00 0000      bgt 0
0E76: 4E56 0000      link a6, #0
0E7A: 4E4F           trap #15
0E7C: A1A1           sysTrapFrmCloseAllForms
0E7E: 4E5E           unlk a6
0E80: 4E75           rts
0E82: 8F53           ormw d7, (a3)
0E84: 746F           moveq #111, d2
0E86: 7041           moveq #65, d0
0E88: 7070           moveq #112, d0
0E8A: 6C69           bge 105
0E8C: 6361           bls 97
0E8E: 7469           moveq #105, d2
0E90: 6F6E           ble 110
0E92: 0000           dc.w #0
0E94: 4E56 FFD6      link a6, #-42
0E98: 48E7 1C00      movem <1c00>, -(a7)
0E9C: 2A2E 0008      movel.ex 8(a6), d5
0EA0: 382E 000C      movew.ex 12(a6), d4
0EA4: 486E FFF8      pea.ex -8(a6)
0EA8: 3F3C 0001      movew.ex #1, -(a7)
0EAC: 2F3C 7073 7973 movel.exl #1886615923, -(a7)
0EB2: 4E4F           trap #15
0EB4: A27B           sysTrapFtrGet
0EB6: BAAE FFF8      cmpl.ex -8(a6), d5
0EBA: 4FEF 000A      lea 10(a7), d7
0EBE: 6300 00A0      bls 160
0EC2: 3004           movew d4, d0
0EC4: 0240 000C      andiw #12, d0
0EC8: 0C40 000C      cmpiw #12, d0
0ECC: 6600 008C      bne 140
0ED0: 3F3C 03E9      movew.ex #1001, -(a7)
0ED4: 4E4F           trap #15
0ED6: A192           sysTrapFrmAlert
0ED8: 0CAE 0200 0000 FFF8 cmpil.ex #33554432, -8(a6)
0EE0: 544F           addqw #2, a7
0EE2: 6476           bcc 118
0EE4: 486E FFFC      pea.ex -4(a6)
0EE8: 486E FFF6      pea.ex -10(a6)
0EEC: 1F3C 0001      moveb.ex #1, -(a7)
0EF0: 2F3C 7072 6566 movel.exl #1886545254, -(a7)
0EF6: 2F3C 6170 706C movel.exl #1634758764, -(a7)
0EFC: 486E FFD6      pea.ex -42(a6)
0F00: 1F3C 0001      moveb.ex #1, -(a7)
0F04: 4E4F           trap #15
0F06: A078           sysTrapDmGetNextDatabaseByTypeCreator
0F08: 4AAE FFFC      tstl.ex -4(a6)
0F0C: 4FEF 0018      lea 24(a7), d7
0F10: 6614           bne 20
0F12: 486D FFD6      pea.ex -42(a5)
0F16: 3F3C 043F      movew.ex #1087, -(a7)
0F1A: 486D FF98      pea.ex -104(a5)
0F1E: 4E4F           trap #15
0F20: A084           sysTrapErrDisplayFileLineMsg
0F22: 4FEF 000A      lea 10(a7), d7
0F26: 4AAE FFFC      tstl.ex -4(a6)
0F2A: 672E           beq 46
0F2C: 42A7           clrl -(a7)
0F2E: 4267           clrw -(a7)
0F30: 2F2E FFFC      movel.ex -4(a6), -(a7)
0F34: 3F2E FFF6      movew.ex -10(a6), -(a7)
0F38: 4E4F           trap #15
0F3A: A0A7           sysTrapSysUIAppSwitch
0F3C: 3600           movew d0, d3
0F3E: 4A43           tstw d3
0F40: 4FEF 000C      lea 12(a7), d7
0F44: 6714           beq 20
0F46: 486D FFEA      pea.ex -22(a5)
0F4A: 3F3C 043F      movew.ex #1087, -(a7)
0F4E: 486D FF98      pea.ex -104(a5)
0F52: 4E4F           trap #15
0F54: A084           sysTrapErrDisplayFileLineMsg
0F56: 4FEF 000A      lea 10(a7), d7
0F5A: 303C 050C      movew.ex #1292, d0
0F5E: 6002           bra 2
0F60: 7000           moveq #0, d0
0F62: 4CDF 0038      movem (a7)+, <0038>
0F66: 4E5E           unlk a6
0F68: 4E75           rts
0F6A: 9452           subrw (a2), d2
0F6C: 6F6D           ble 109
0F6E: 5665           addqw #3, -(a5)
0F70: 7273           moveq #115, d1
0F72: 696F           bvs 111
0F74: 6E43           bgt 67
0F76: 6F6D           ble 109
0F78: 7061           moveq #97, d0
0F7A: 7469           moveq #105, d2
0F7C: 626C           bhi 108
0F7E: 6500 0000      bcs 0
0F82: 4E56 0000      link a6, #0
0F86: 48E7 1C00      movem <1c00>, -(a7)
0F8A: 3A2E 0008      movew.ex 8(a6), d5
0F8E: 382E 000E      movew.ex 14(a6), d4
0F92: 3F04           movew d4, -(a7)
0F94: 2F3C 0200 0000 movel.exl #33554432, -(a7)
0F9A: 4EBA FEF8      jsr.ex -32100(pc)
0F9E: 3600           movew d0, d3
0FA0: 4A43           tstw d3
0FA2: 5C4F           addqw #6, a7
0FA4: 6706           beq 6
0FA6: 7000           moveq #0, d0
0FA8: 3003           movew d3, d0
0FAA: 601A           bra 26
0FAC: 4A45           tstw d5
0FAE: 6708           beq 8
0FB0: 203C 0000 0502 movel.exl #1282, d0
0FB6: 600E           bra 14
0FB8: 4EBA FE68      jsr.ex -32214(pc)
0FBC: 4EBA FDFE      jsr.ex -32316(pc)
0FC0: 4EBA FEB4      jsr.ex -32130(pc)
0FC4: 7000           moveq #0, d0
0FC6: 4CDF 0038      movem (a7)+, <0038>
0FCA: 4E5E           unlk a6
0FCC: 4E75           rts
0FCE: 8950           ormw d4, (a0)
0FD0: 696C           bvs 108
0FD2: 6F74           ble 116
0FD4: 4D61           dc.w #19809
0FD6: 696E           bvs 110
0FD8: 0000 0000      orib #0, d0

