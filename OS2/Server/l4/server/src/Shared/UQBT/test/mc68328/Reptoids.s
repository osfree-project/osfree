02C6: 0000 0001      orib #1, d0
02CA: 487A 0004      pea.ex 30424(pc)
02CE: 0697 0000 0006 addil #6, (a7)
02D4: 4E75           rts
02D6: 4E56 FFF4      link a6, #-12
02DA: 48E7 1800      movem <1800>, -(a7)
02DE: 486E FFF4      pea.ex -12(a6)
02E2: 486E FFF8      pea.ex -8(a6)
02E6: 486E FFFC      pea.ex -4(a6)
02EA: 4E4F           trap #15
02EC: A08F           sysTrapSysAppStartup
02EE: 3800           movew d0, d4
02F0: 4A44           tstw d4
02F2: 4FEF 000C      lea 12(a7), d7
02F6: 671C           beq 28
02F8: 41FA 0066      lea 30568(pc), d0
02FC: 4850           pea (a0)
02FE: 3F3C 005C      movew.ex #92, -(a7)
0302: 41FA 0078      lea 30596(pc), d0
0306: 4850           pea (a0)
0308: 4E4F           trap #15
030A: A084           sysTrapErrDisplayFileLineMsg
030C: 7000           moveq #0, d0
030E: 4FEF 000A      lea 10(a7), d7
0312: 6036           bra 54
0314: 206E FFFC      movel.ex -4(a6), a0
0318: 3F28 0006      movew.ex 6(a0), -(a7)
031C: 2F28 0002      movel.ex 2(a0), -(a7)
0320: 3F10           movew (a0), -(a7)
0322: 487A 000E      pea.ex 30522(pc)
0326: 487A 0004      pea.ex 30516(pc)
032A: 0697 0000 6A72 addil #27250, (a7)
0330: 4E75           rts
0332: 2600           movel d0, d3
0334: 2F2E FFF4      movel.ex -12(a6), -(a7)
0338: 2F2E FFF8      movel.ex -8(a6), -(a7)
033C: 2F2E FFFC      movel.ex -4(a6), -(a7)
0340: 4E4F           trap #15
0342: A090           sysTrapSysAppExit
0344: 2003           movel d3, d0
0346: 4FEF 0014      lea 20(a7), d7
034A: 4CDF 0018      movem (a7)+, <0018>
034E: 4E5E           unlk a6
0350: 4E75           rts
0352: 8B5F           ormw d5, (a7)+
0354: 5F53           subqw #7, (a3)
0356: 7461           moveq #97, d2
0358: 7274           moveq #116, d1
035A: 7570           dc.w #30064
035C: 5F5F           subqw #7, (a7)+
035E: 002A           dc.w #42
0360: 4572           dc.w #17778
0362: 726F           moveq #111, d1
0364: 7220           moveq #32, d1
0366: 6C61           bge 97
0368: 756E           dc.w #30062
036A: 6368           bls 104
036C: 696E           bvs 110
036E: 6720           beq 32
0370: 6170           bsr 112
0372: 706C           moveq #108, d0
0374: 6963           bvs 99
0376: 6174           bsr 116
0378: 696F           bvs 111
037A: 6E00 5374      bgt 21364
037E: 6172           bsr 114
0380: 7475           moveq #117, d2
0382: 7043           moveq #67, d0
0384: 6F64           ble 100
0386: 652E           bcs 46
0388: 6300 48E7      bls 18663
038C: 3000           movew d0, d0
038E: 7402           moveq #2, d2
0390: 4EFB 2200      jmp.ex 0(pc,d2.w)
0394: 600A           bra 10
0396: 4C01           dc.w #19457
0398: 0000           dc.w #0
039A: 4CDF 000C      movem (a7)+, <000c>
039E: 4E75           rts
03A0: 2400           movel d0, d2
03A2: 4842           swap d2
03A4: C4C1           mulu d1, d2
03A6: 2601           movel d1, d3
03A8: 4843           swap d3
03AA: C6C0           mulu d0, d3
03AC: D443           addrw d3, d2
03AE: 4842           swap d2
03B0: 4242           clrw d2
03B2: C0C1           mulu d1, d0
03B4: D082           addrl d2, d0
03B6: 4CDF 000C      movem (a7)+, <000c>
03BA: 4E75           rts
03BC: 885F           orrw (a7)+, d4
03BE: 5F6C 6D75      subqw.ex #7, 28021(a4)
03C2: 6C5F           bge 95
03C4: 5F00           subqb #7, d0
03C6: 0000           dc.w #0
03C8: 48E7 3000      movem <3000>, -(a7)
03CC: 7402           moveq #2, d2
03CE: 4EFB 2200      jmp.ex 0(pc,d2.w)
03D2: 600A           bra 10
03D4: 4C41           dc.w #19521
03D6: 0000           dc.w #0
03D8: 4CDF 000C      movem (a7)+, <000c>
03DC: 4E75           rts
03DE: 2401           movel d1, d2
03E0: 4242           clrw d2
03E2: 4842           swap d2
03E4: 661C           bne 28
03E6: 3600           movew d0, d3
03E8: 4240           clrw d0
03EA: 4840           swap d0
03EC: 6706           beq 6
03EE: 80C1           divu d1, d0
03F0: 3400           movew d0, d2
03F2: 4842           swap d2
03F4: 3003           movew d3, d0
03F6: 80C1           divu d1, d0
03F8: 3400           movew d0, d2
03FA: 2002           movel d2, d0
03FC: 4CDF 000C      movem (a7)+, <000c>
0400: 4E75           rts
0402: 3400           movew d0, d2
0404: 4240           clrw d0
0406: 4840           swap d0
0408: 4842           swap d2
040A: 2601           movel d1, d3
040C: 720F           moveq #15, d1
040E: D482           addrl d2, d2
0410: D180           addxrl d0, d0
0412: B083           cmpl d3, d0
0414: 6504           bcs 4
0416: 9083           subrl d3, d0
0418: 5202           addqb #1, d2
041A: 51C9 FFF2      dbf d1, 65522
041E: 2002           movel d2, d0
0420: 4CDF 000C      movem (a7)+, <000c>
0424: 4E75           rts
0426: 895F           ormw d4, (a7)+
0428: 5F6C 6469      subqw.ex #7, 25705(a4)
042C: 7675           moveq #117, d3
042E: 5F5F           subqw #7, (a7)+
0430: 0000           dc.w #0
0432: 4A80           tstl d0
0434: 6C0C           bge 12
0436: 4480           negl d0
0438: 4A81           tstl d1
043A: 6C10           bge 16
043C: 4481           negl d1
043E: 4EFA FF88      jmp.ex 30672(pc)
0442: 4A81           tstl d1
0444: 6D04           blt 4
0446: 4EFA FF80      jmp.ex 30672(pc)
044A: 4481           negl d1
044C: 4EBA FF7A      jsr.ex 30672(pc)
0450: 4480           negl d0
0452: 4E75           rts
0454: 885F           orrw (a7)+, d4
0456: 5F6C 6469      subqw.ex #7, 25705(a4)
045A: 765F           moveq #95, d3
045C: 5F00           subqb #7, d0
045E: 0000           dc.w #0
0460: 4E56 0000      link a6, #0
0464: 2F03           movel d3, -(a7)
0466: 0C2D 0001 F414 cmpib.ex #1, -3052(a5)
046C: 6714           beq 20
046E: 4A2D F414      tstb.ex -3052(a5)
0472: 670E           beq 14
0474: 0C2D 0007 F414 cmpib.ex #7, -3052(a5)
047A: 6706           beq 6
047C: 4A2D F41E      tstb.ex -3042(a5)
0480: 6704           beq 4
0482: 70FF           moveq #255, d0
0484: 6014           bra 20
0486: 4E4F           trap #15
0488: A0F7           sysTrapTimGetTicks
048A: 222D F41A      movel.ex -3046(a5), d1
048E: 9280           subrl d0, d1
0490: 2601           movel d1, d3
0492: 4A83           tstl d3
0494: 6C02           bge 2
0496: 7600           moveq #0, d3
0498: 2003           movel d3, d0
049A: 261F           movel (a7)+, d3
049C: 4E5E           unlk a6
049E: 4E75           rts
04A0: 9454           subrw (a4), d2
04A2: 696D           bvs 109
04A4: 6555           bcs 85
04A6: 6E74           bgt 116
04A8: 696C           bvs 108
04AA: 6C4E           bge 78
04AC: 6578           bcs 120
04AE: 7450           moveq #80, d2
04B0: 6572           bcs 114
04B2: 696F           bvs 111
04B4: 6400 0000      bcc 0
04B8: 4E56 FFF4      link a6, #-12
04BC: 48E7 1020      movem <1020>, -(a7)
04C0: 1F3C 0009      moveb.ex #9, -(a7)
04C4: 4E4F           trap #15
04C6: A2D1           sysTrapPrefGetPreference
04C8: 5380           subql #1, d0
04CA: 544F           addqw #2, a7
04CC: 6712           beq 18
04CE: 486D F214      pea.ex -3564(a5)
04D2: 42A7           clrl -(a7)
04D4: 42A7           clrl -(a7)
04D6: 4E4F           trap #15
04D8: A232           sysTrapSndGetDefaultVolume
04DA: 4FEF 000C      lea 12(a7), d7
04DE: 6004           bra 4
04E0: 426D F214      clrw.ex -3564(a5)
04E4: 486D F212      pea.ex -3566(a5)
04E8: 486D F210      pea.ex -3568(a5)
04EC: 486D F20E      pea.ex -3570(a5)
04F0: 486D F20C      pea.ex -3572(a5)
04F4: 4227           clrb -(a7)
04F6: 4E4F           trap #15
04F8: A2A4           sysTrapKeyRates
04FA: 4E4F           trap #15
04FC: A1FE           sysTrapWinGetDrawWindow
04FE: 2448           movel a0, a2
0500: 7600           moveq #0, d3
0502: 4FEF 0012      lea 18(a7), d7
0506: 6000 00DC      bra 220
050A: 3003           movew d3, d0
050C: 0640 03E9      addiw #1001, d0
0510: 3F00           movew d0, -(a7)
0512: 2F3C 5462 6D70 movel.exl #1415736688, -(a7)
0518: 4E4F           trap #15
051A: A05F           sysTrapDmGetResource
051C: 3243           movew d3, a1
051E: 2009           movel a1, d0
0520: E588           lslil #2, d0
0522: 43ED F37C      lea -3204(a5), d1
0526: 2388 0800      movel.mx a0, 0(a1,d0.l)
052A: 3043           movew d3, a0
052C: 2008           movel a0, d0
052E: E588           lslil #2, d0
0530: 2049           movel a1, a0
0532: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
0536: 4E4F           trap #15
0538: A021           sysTrapMemHandleLock
053A: 3243           movew d3, a1
053C: 2009           movel a1, d0
053E: E588           lslil #2, d0
0540: 43ED F2E4      lea -3356(a5), d1
0544: 2388 0800      movel.mx a0, 0(a1,d0.l)
0548: 0C43 03E7      cmpiw #999, d3
054C: 4FEF 000A      lea 10(a7), d7
0550: 6612           bne 18
0552: 3043           movew d3, a0
0554: 2008           movel a0, d0
0556: E588           lslil #2, d0
0558: 41ED F24C      lea -3508(a5), d0
055C: 42B0 0800      clrl.ex 0(a0,d0.l)
0560: 6000 0080      bra 128
0564: 486E FFF4      pea.ex -12(a6)
0568: 4227           clrb -(a7)
056A: 3043           movew d3, a0
056C: 2008           movel a0, d0
056E: E588           lslil #2, d0
0570: 2049           movel a1, a0
0572: 2070 0800      movel.ex 0(a0,d0.l), a0
0576: 3F28 0002      movew.ex 2(a0), -(a7)
057A: 3043           movew d3, a0
057C: 2008           movel a0, d0
057E: E588           lslil #2, d0
0580: 2049           movel a1, a0
0582: 2070 0800      movel.ex 0(a0,d0.l), a0
0586: 3F10           movew (a0), -(a7)
0588: 4E4F           trap #15
058A: A1F7           sysTrapWinCreateOffscreenWindow
058C: 3243           movew d3, a1
058E: 2009           movel a1, d0
0590: E588           lslil #2, d0
0592: 43ED F24C      lea -3508(a5), d1
0596: 2388 0800      movel.mx a0, 0(a1,d0.l)
059A: 4A6E FFF4      tstw.ex -12(a6)
059E: 4FEF 000A      lea 10(a7), d7
05A2: 6714           beq 20
05A4: 486D FDF2      pea.ex -526(a5)
05A8: 3F3C 04BB      movew.ex #1211, -(a7)
05AC: 486D FDEA      pea.ex -534(a5)
05B0: 4E4F           trap #15
05B2: A084           sysTrapErrDisplayFileLineMsg
05B4: 4FEF 000A      lea 10(a7), d7
05B8: 3043           movew d3, a0
05BA: 2008           movel a0, d0
05BC: E588           lslil #2, d0
05BE: 41ED F24C      lea -3508(a5), d0
05C2: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
05C6: 4E4F           trap #15
05C8: A1FD           sysTrapWinSetDrawWindow
05CA: 42A7           clrl -(a7)
05CC: 3043           movew d3, a0
05CE: 2008           movel a0, d0
05D0: E588           lslil #2, d0
05D2: 41ED F2E4      lea -3356(a5), d0
05D6: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
05DA: 4E4F           trap #15
05DC: A226           sysTrapWinDrawBitmap
05DE: 4FEF 000C      lea 12(a7), d7
05E2: 5243           addqw #1, d3
05E4: 0C43 0026      cmpiw #38, d3
05E8: 6D00 FF20      blt 65312
05EC: 422D F246      clrb.ex -3514(a5)
05F0: 422D F247      clrb.ex -3513(a5)
05F4: 422D F248      clrb.ex -3512(a5)
05F8: 422D F249      clrb.ex -3511(a5)
05FC: 3D7C 00CC FFF6 movew.emx #204, -10(a6)
0602: 1F3C 0001      moveb.ex #1, -(a7)
0606: 486E FFF6      pea.ex -10(a6)
060A: 486D F0B0      pea.ex -3920(a5)
060E: 4267           clrw -(a7)
0610: 2F3C 5250 5444 movel.exl #1380996164, -(a7)
0616: 4E4F           trap #15
0618: A2D3           sysTrapPrefGetAppPreferences
061A: 5240           addqw #1, d0
061C: 4FEF 0010      lea 16(a7), d7
0620: 664E           bne 78
0622: 7600           moveq #0, d3
0624: 602C           bra 44
0626: 7016           moveq #22, d0
0628: C1C3           muls d3, d0
062A: 41ED F0B0      lea -3920(a5), d0
062E: 4230 0800      clrb.ex 0(a0,d0.l)
0632: 7016           moveq #22, d0
0634: C1C3           muls d3, d0
0636: 5088           addql #0, a0
0638: 5088           addql #0, a0
063A: 42B0 0800      clrl.ex 0(a0,d0.l)
063E: 7016           moveq #22, d0
0640: C1C3           muls d3, d0
0642: 41ED F0B0      lea -3920(a5), d0
0646: 41E8 0014      lea 20(a0), d0
064A: 31BC 0001 0800 movew.emx #1, 0(a0,d0.l)
0650: 5243           addqw #1, d3
0652: 0C43 0009      cmpiw #9, d3
0656: 6DCE           blt -50
0658: 1B7C 0009 F176 moveb.emx #9, -3722(a5)
065E: 42AD F178      clrl.ex -3720(a5)
0662: 4EBA 5D8E      jsr.ex -10246(pc)
0666: 3F3C 03E8      movew.ex #1000, -(a7)
066A: 4E4F           trap #15
066C: A195           sysTrapFrmHelp
066E: 544F           addqw #2, a7
0670: 3D7C 08BA FFF6 movew.emx #2234, -10(a6)
0676: 4227           clrb -(a7)
0678: 486E FFF6      pea.ex -10(a6)
067C: 486D F414      pea.ex -3052(a5)
0680: 4267           clrw -(a7)
0682: 2F3C 5250 5444 movel.exl #1380996164, -(a7)
0688: 4E4F           trap #15
068A: A2D3           sysTrapPrefGetAppPreferences
068C: 5240           addqw #1, d0
068E: 4FEF 0010      lea 16(a7), d7
0692: 6608           bne 8
0694: 1B7C 0001 F414 moveb.emx #1, -3052(a5)
069A: 6012           bra 18
069C: 0C2D 0007 F414 cmpib.ex #7, -3052(a5)
06A2: 670A           beq 10
06A4: 1B6D F414 F24B moveb.emx -3052(a5), -3509(a5)
06AA: 422D F414      clrb.ex -3052(a5)
06AE: 486E FFF4      pea.ex -12(a6)
06B2: 4227           clrb -(a7)
06B4: 2F3C 00A0 0091 movel.exl #10485905, -(a7)
06BA: 4E4F           trap #15
06BC: A1F7           sysTrapWinCreateOffscreenWindow
06BE: 2B48 F428      movel.mx a0, -3032(a5)
06C2: 2F2D F428      movel.ex -3032(a5), -(a7)
06C6: 4E4F           trap #15
06C8: A1FD           sysTrapWinSetDrawWindow
06CA: 426E FFF8      clrw.ex -8(a6)
06CE: 426E FFFA      clrw.ex -6(a6)
06D2: 3D7C 00A0 FFFC movew.emx #160, -4(a6)
06D8: 3D7C 0091 FFFE movew.emx #145, -2(a6)
06DE: 4267           clrw -(a7)
06E0: 486E FFF8      pea.ex -8(a6)
06E4: 4E4F           trap #15
06E6: A219           sysTrapWinEraseRectangle
06E8: 2F0A           movel a2, -(a7)
06EA: 4E4F           trap #15
06EC: A1FD           sysTrapWinSetDrawWindow
06EE: 1B7C 0001 F43E moveb.emx #1, -3010(a5)
06F4: 422D F4C9      clrb.ex -2871(a5)
06F8: 4E4F           trap #15
06FA: A0F7           sysTrapTimGetTicks
06FC: 2B40 F424      movel.mx d0, -3036(a5)
0700: 2F2D F424      movel.ex -3036(a5), -(a7)
0704: 4E4F           trap #15
0706: A0C2           sysTrapSysRandom
0708: 7000           moveq #0, d0
070A: 4FEF 001C      lea 28(a7), d7
070E: 4CDF 0408      movem (a7)+, <0408>
0712: 4E5E           unlk a6
0714: 4E75           rts
0716: 9053           subrw (a3), d0
0718: 7461           moveq #97, d2
071A: 7274           moveq #116, d1
071C: 4170           dc.w #16752
071E: 706C           moveq #108, d0
0720: 6963           bvs 99
0722: 6174           bsr 116
0724: 696F           bvs 111
0726: 6E00 0000      bgt 0
072A: 4E56 0000      link a6, #0
072E: 2F03           movel d3, -(a7)
0730: 4227           clrb -(a7)
0732: 2F2D F428      movel.ex -3032(a5), -(a7)
0736: 4E4F           trap #15
0738: A1F8           sysTrapWinDeleteWindow
073A: 7600           moveq #0, d3
073C: 5C4F           addqw #6, a7
073E: 604E           bra 78
0740: 3043           movew d3, a0
0742: 2008           movel a0, d0
0744: E588           lslil #2, d0
0746: 41ED F2E4      lea -3356(a5), d0
074A: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
074E: 4E4F           trap #15
0750: A035           sysTrapMemPtrUnlock
0752: 3043           movew d3, a0
0754: 2008           movel a0, d0
0756: E588           lslil #2, d0
0758: 41ED F37C      lea -3204(a5), d0
075C: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
0760: 4E4F           trap #15
0762: A061           sysTrapDmReleaseResource
0764: 3043           movew d3, a0
0766: 2008           movel a0, d0
0768: E588           lslil #2, d0
076A: 41ED F24C      lea -3508(a5), d0
076E: 4AB0 0800      tstl.ex 0(a0,d0.l)
0772: 504F           addqw #0, a7
0774: 6716           beq 22
0776: 4227           clrb -(a7)
0778: 3043           movew d3, a0
077A: 2008           movel a0, d0
077C: E588           lslil #2, d0
077E: 41ED F24C      lea -3508(a5), d0
0782: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
0786: 4E4F           trap #15
0788: A1F8           sysTrapWinDeleteWindow
078A: 5C4F           addqw #6, a7
078C: 5243           addqw #1, d3
078E: 0C43 0026      cmpiw #38, d3
0792: 6DAC           blt -84
0794: 4E4F           trap #15
0796: A0F7           sysTrapTimGetTicks
0798: 90AD F424      subrl.ex -3036(a5), d0
079C: D1AD F178      addml.ex d0, -3720(a5)
07A0: 4A2D F414      tstb.ex -3052(a5)
07A4: 6606           bne 6
07A6: 1B6D F24B F414 moveb.emx -3509(a5), -3052(a5)
07AC: 1F3C 0001      moveb.ex #1, -(a7)
07B0: 3F3C 00CC      movew.ex #204, -(a7)
07B4: 486D F0B0      pea.ex -3920(a5)
07B8: 4878 0004      pea.ex (0004).w
07BC: 2F3C 5250 5444 movel.exl #1380996164, -(a7)
07C2: 4E4F           trap #15
07C4: A2D4           sysTrapPrefSetAppPreferences
07C6: 4227           clrb -(a7)
07C8: 3F3C 08BA      movew.ex #2234, -(a7)
07CC: 486D F414      pea.ex -3052(a5)
07D0: 4878 0004      pea.ex (0004).w
07D4: 2F3C 5250 5444 movel.exl #1380996164, -(a7)
07DA: 4E4F           trap #15
07DC: A2D4           sysTrapPrefSetAppPreferences
07DE: 4EBA 012C      jsr.ex 32020(pc)
07E2: 4FEF 0020      lea 32(a7), d7
07E6: 261F           movel (a7)+, d3
07E8: 4E5E           unlk a6
07EA: 4E75           rts
07EC: 8F53           ormw d7, (a3)
07EE: 746F           moveq #111, d2
07F0: 7041           moveq #65, d0
07F2: 7070           moveq #112, d0
07F4: 6C69           bge 105
07F6: 6361           bls 97
07F8: 7469           moveq #105, d2
07FA: 6F6E           ble 110
07FC: 0000           dc.w #0
07FE: 4E56 FFD6      link a6, #-42
0802: 48E7 1C00      movem <1c00>, -(a7)
0806: 2A2E 0008      movel.ex 8(a6), d5
080A: 382E 000C      movew.ex 12(a6), d4
080E: 486E FFF8      pea.ex -8(a6)
0812: 3F3C 0001      movew.ex #1, -(a7)
0816: 2F3C 7073 7973 movel.exl #1886615923, -(a7)
081C: 4E4F           trap #15
081E: A27B           sysTrapFtrGet
0820: BAAE FFF8      cmpl.ex -8(a6), d5
0824: 4FEF 000A      lea 10(a7), d7
0828: 6300 00A0      bls 160
082C: 3004           movew d4, d0
082E: 0240 000C      andiw #12, d0
0832: 0C40 000C      cmpiw #12, d0
0836: 6600 008C      bne 140
083A: 3F3C 03E9      movew.ex #1001, -(a7)
083E: 4E4F           trap #15
0840: A192           sysTrapFrmAlert
0842: 0CAE 0200 0000 FFF8 cmpil.ex #33554432, -8(a6)
084A: 544F           addqw #2, a7
084C: 6476           bcc 118
084E: 486E FFFC      pea.ex -4(a6)
0852: 486E FFF6      pea.ex -10(a6)
0856: 1F3C 0001      moveb.ex #1, -(a7)
085A: 2F3C 7072 6566 movel.exl #1886545254, -(a7)
0860: 2F3C 6170 706C movel.exl #1634758764, -(a7)
0866: 486E FFD6      pea.ex -42(a6)
086A: 1F3C 0001      moveb.ex #1, -(a7)
086E: 4E4F           trap #15
0870: A078           sysTrapDmGetNextDatabaseByTypeCreator
0872: 4AAE FFFC      tstl.ex -4(a6)
0876: 4FEF 0018      lea 24(a7), d7
087A: 6614           bne 20
087C: 486D FE08      pea.ex -504(a5)
0880: 3F3C 0582      movew.ex #1410, -(a7)
0884: 486D FDEA      pea.ex -534(a5)
0888: 4E4F           trap #15
088A: A084           sysTrapErrDisplayFileLineMsg
088C: 4FEF 000A      lea 10(a7), d7
0890: 4AAE FFFC      tstl.ex -4(a6)
0894: 672E           beq 46
0896: 42A7           clrl -(a7)
0898: 4267           clrw -(a7)
089A: 2F2E FFFC      movel.ex -4(a6), -(a7)
089E: 3F2E FFF6      movew.ex -10(a6), -(a7)
08A2: 4E4F           trap #15
08A4: A0A7           sysTrapSysUIAppSwitch
08A6: 3600           movew d0, d3
08A8: 4A43           tstw d3
08AA: 4FEF 000C      lea 12(a7), d7
08AE: 6714           beq 20
08B0: 486D FE1C      pea.ex -484(a5)
08B4: 3F3C 0582      movew.ex #1410, -(a7)
08B8: 486D FDEA      pea.ex -534(a5)
08BC: 4E4F           trap #15
08BE: A084           sysTrapErrDisplayFileLineMsg
08C0: 4FEF 000A      lea 10(a7), d7
08C4: 303C 050C      movew.ex #1292, d0
08C8: 6002           bra 2
08CA: 7000           moveq #0, d0
08CC: 4CDF 0038      movem (a7)+, <0038>
08D0: 4E5E           unlk a6
08D2: 4E75           rts
08D4: 9452           subrw (a2), d2
08D6: 6F6D           ble 109
08D8: 5665           addqw #3, -(a5)
08DA: 7273           moveq #115, d1
08DC: 696F           bvs 111
08DE: 6E43           bgt 67
08E0: 6F6D           ble 109
08E2: 7061           moveq #97, d0
08E4: 7469           moveq #105, d2
08E6: 626C           bhi 108
08E8: 6500 0000      bcs 0
08EC: 4E56 FFFA      link a6, #-6
08F0: 4878 FF85      pea.ex (ff85).w
08F4: 4E4F           trap #15
08F6: A2CF           sysTrapKeySetMask
08F8: 4E5E           unlk a6
08FA: 4E75           rts
08FC: 8C47           orrw d7, d6
08FE: 616D           bsr 109
0900: 654D           bcs 77
0902: 6173           bsr 115
0904: 6B4B           bmi 75
0906: 6579           bcs 121
0908: 7300           dc.w #29440
090A: 0000           dc.w #0
090C: 4E56 0000      link a6, #0
0910: 4878 FFFF      pea.ex (ffff).w
0914: 4E4F           trap #15
0916: A2CF           sysTrapKeySetMask
0918: 486D F212      pea.ex -3566(a5)
091C: 486D F210      pea.ex -3568(a5)
0920: 486D F20E      pea.ex -3570(a5)
0924: 486D F20C      pea.ex -3572(a5)
0928: 1F3C 0001      moveb.ex #1, -(a7)
092C: 4E4F           trap #15
092E: A2A4           sysTrapKeyRates
0930: 4E5E           unlk a6
0932: 4E75           rts
0934: 8E47           orrw d7, d7
0936: 616D           bsr 109
0938: 6555           bcs 85
093A: 6E6D           bgt 109
093C: 6173           bsr 115
093E: 6B4B           bmi 75
0940: 6579           bcs 121
0942: 7300           dc.w #29440
0944: 0000           dc.w #0
0946: 4E56 0000      link a6, #0
094A: 2F06           movel d6, -(a7)
094C: 3C2E 0008      movew.ex 8(a6), d6
0950: 342E 000A      movew.ex 10(a6), d2
0954: 322E 000C      movew.ex 12(a6), d1
0958: 302E 000E      movew.ex 14(a6), d0
095C: BC6D F434      cmpw.ex -3020(a5), d6
0960: 6C04           bge 4
0962: 3B46 F434      movew.mx d6, -3020(a5)
0966: B46D F436      cmpw.ex -3018(a5), d2
096A: 6C04           bge 4
096C: 3B42 F436      movew.mx d2, -3018(a5)
0970: B26D F438      cmpw.ex -3016(a5), d1
0974: 6F04           ble 4
0976: 3B41 F438      movew.mx d1, -3016(a5)
097A: B06D F43A      cmpw.ex -3014(a5), d0
097E: 6F04           ble 4
0980: 3B40 F43A      movew.mx d0, -3014(a5)
0984: 2C1F           movel (a7)+, d6
0986: 4E5E           unlk a6
0988: 4E75           rts
098A: 9753           submw d3, (a3)
098C: 6372           bls 114
098E: 6565           bcs 101
0990: 6E42           bgt 66
0992: 6F75           ble 117
0994: 6E64           bgt 100
0996: 7349           dc.w #29513
0998: 6E63           bgt 99
099A: 6C75           bge 117
099C: 6465           bcc 101
099E: 4172           dc.w #16754
09A0: 6561           bcs 97
09A2: 0000           dc.w #0
09A4: 4E56 FFF8      link a6, #-8
09A8: 48E7 1F00      movem <1f00>, -(a7)
09AC: 362E 0008      movew.ex 8(a6), d3
09B0: 3E2E 000C      movew.ex 12(a6), d7
09B4: 1C2E 000E      moveb.ex 14(a6), d6
09B8: 3043           movew d3, a0
09BA: 2008           movel a0, d0
09BC: E588           lslil #2, d0
09BE: 41ED F24C      lea -3508(a5), d0
09C2: 4AB0 0800      tstl.ex 0(a0,d0.l)
09C6: 6614           bne 20
09C8: 486D FE32      pea.ex -462(a5)
09CC: 3F3C 0647      movew.ex #1607, -(a7)
09D0: 486D FDEA      pea.ex -534(a5)
09D4: 4E4F           trap #15
09D6: A084           sysTrapErrDisplayFileLineMsg
09D8: 4FEF 000A      lea 10(a7), d7
09DC: 382E 000A      movew.ex 10(a6), d4
09E0: E844           asriw #4, d4
09E2: 0644 FFF0      addiw #-16, d4
09E6: 3A07           movew d7, d5
09E8: E845           asriw #4, d5
09EA: 0645 FFF0      addiw #-16, d5
09EE: 4878 0008      pea.ex (0008).w
09F2: 3043           movew d3, a0
09F4: 2008           movel a0, d0
09F6: E588           lslil #2, d0
09F8: 41ED F24C      lea -3508(a5), d0
09FC: 2070 0800      movel.ex 0(a0,d0.l), a0
0A00: 4868 000A      pea.ex 10(a0)
0A04: 486E FFF8      pea.ex -8(a6)
0A08: 4E4F           trap #15
0A0A: A026           sysTrapMemMove
0A0C: 1F06           moveb d6, -(a7)
0A0E: 3F05           movew d5, -(a7)
0A10: 3F04           movew d4, -(a7)
0A12: 486E FFF8      pea.ex -8(a6)
0A16: 42A7           clrl -(a7)
0A18: 3043           movew d3, a0
0A1A: 2008           movel a0, d0
0A1C: E588           lslil #2, d0
0A1E: 41ED F24C      lea -3508(a5), d0
0A22: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
0A26: 4E4F           trap #15
0A28: A209           sysTrapWinCopyRectangle
0A2A: 3043           movew d3, a0
0A2C: 2008           movel a0, d0
0A2E: E588           lslil #2, d0
0A30: 41ED F24C      lea -3508(a5), d0
0A34: 2070 0800      movel.ex 0(a0,d0.l), a0
0A38: 3005           movew d5, d0
0A3A: D068 0010      addrw.ex 16(a0), d0
0A3E: 5340           subqw #1, d0
0A40: 3F00           movew d0, -(a7)
0A42: 3043           movew d3, a0
0A44: 2008           movel a0, d0
0A46: E588           lslil #2, d0
0A48: 41ED F24C      lea -3508(a5), d0
0A4C: 2070 0800      movel.ex 0(a0,d0.l), a0
0A50: 3004           movew d4, d0
0A52: D068 000E      addrw.ex 14(a0), d0
0A56: 5340           subqw #1, d0
0A58: 3F00           movew d0, -(a7)
0A5A: 3F05           movew d5, -(a7)
0A5C: 3F04           movew d4, -(a7)
0A5E: 4EBA FEE6      jsr.ex 32078(pc)
0A62: 4FEF 0026      lea 38(a7), d7
0A66: 4CDF 00F8      movem (a7)+, <00f8>
0A6A: 4E5E           unlk a6
0A6C: 4E75           rts
0A6E: 8A44           orrw d4, d5
0A70: 7261           moveq #97, d1
0A72: 774F           dc.w #30543
0A74: 626A           bhi 106
0A76: 6563           bcs 99
0A78: 7400           moveq #0, d2
0A7A: 0000           dc.w #0
0A7C: 4E56 0000      link a6, #0
0A80: 48E7 1E00      movem <1e00>, -(a7)
0A84: 3C2E 0008      movew.ex 8(a6), d6
0A88: 3A2E 000A      movew.ex 10(a6), d5
0A8C: 3606           movew d6, d3
0A8E: E843           asriw #4, d3
0A90: 0643 FFF0      addiw #-16, d3
0A94: 3805           movew d5, d4
0A96: E844           asriw #4, d4
0A98: 0644 FFF0      addiw #-16, d4
0A9C: 3F04           movew d4, -(a7)
0A9E: 3F03           movew d3, -(a7)
0AA0: 3F04           movew d4, -(a7)
0AA2: 3F03           movew d3, -(a7)
0AA4: 4E4F           trap #15
0AA6: A213           sysTrapWinDrawLine
0AA8: 3F04           movew d4, -(a7)
0AAA: 3F03           movew d3, -(a7)
0AAC: 3F04           movew d4, -(a7)
0AAE: 3F03           movew d3, -(a7)
0AB0: 4EBA FE94      jsr.ex 32078(pc)
0AB4: 4FEF 0010      lea 16(a7), d7
0AB8: 4CDF 0078      movem (a7)+, <0078>
0ABC: 4E5E           unlk a6
0ABE: 4E75           rts
0AC0: 8944           dc.w #35140
0AC2: 7261           moveq #97, d1
0AC4: 7750           dc.w #30544
0AC6: 6F69           ble 105
0AC8: 6E74           bgt 116
0ACA: 0000           dc.w #0
0ACC: 4E56 0000      link a6, #0
0AD0: 142E 0008      moveb.ex 8(a6), d2
0AD4: 7000           moveq #0, d0
0AD6: 102D FCCA      moveb.ex -822(a5), d0
0ADA: E788           lslil #3, d0
0ADC: 41ED FD16      lea -746(a5), d0
0AE0: 7200           moveq #0, d1
0AE2: 1202           moveb d2, d1
0AE4: E789           lslil #3, d1
0AE6: 2248           movel a0, a1
0AE8: 1231 1800      moveb.ex 0(a1,d1.l), d1
0AEC: B230 0800      cmpb.ex 0(a0,d0.l), d1
0AF0: 6512           bcs 18
0AF2: 1B42 FCCA      moveb.mx d2, -822(a5)
0AF6: 7000           moveq #0, d0
0AF8: 1002           moveb d2, d0
0AFA: E788           lslil #3, d0
0AFC: 5288           addql #1, a0
0AFE: 1B70 0800 FCCB moveb.emx 0(a0,d0.l), -821(a5)
0B04: 4E5E           unlk a6
0B06: 4E75           rts
0B08: 9047           subrw d7, d0
0B0A: 616D           bsr 109
0B0C: 6552           bcs 82
0B0E: 6571           bcs 113
0B10: 7565           dc.w #30053
0B12: 7374           dc.w #29556
0B14: 536F 756E      subqw.ex #1, 30062(a7)
0B18: 6400 0000      bcc 0
0B1C: 4E56 0000      link a6, #0
0B20: 2F03           movel d3, -(a7)
0B22: 162E 000C      moveb.ex 12(a6), d3
0B26: 0C03 0004      cmpib #4, d3
0B2A: 6400 0150      bcc 336
0B2E: 7000           moveq #0, d0
0B30: 1003           moveb d3, d0
0B32: C1FC 001E      muls.ex #30, d0
0B36: 41ED F414      lea -3052(a5), d0
0B3A: 41E8 0040      lea 64(a0), d0
0B3E: 41F0 0800      lea 0(a0,d0.l), d0
0B42: 202E 0008      movel.ex 8(a6), d0
0B46: D190           addml d0, (a0)
0B48: 7000           moveq #0, d0
0B4A: 1003           moveb d3, d0
0B4C: C1FC 001E      muls.ex #30, d0
0B50: 41ED F414      lea -3052(a5), d0
0B54: 41E8 0044      lea 68(a0), d0
0B58: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
0B5E: B62D F4C8      cmpb.ex -2872(a5), d3
0B62: 6600 009A      bne 154
0B66: 4A2D FCCC      tstb.ex -820(a5)
0B6A: 6646           bne 70
0B6C: 7000           moveq #0, d0
0B6E: 1003           moveb d3, d0
0B70: C1FC 001E      muls.ex #30, d0
0B74: 41ED F414      lea -3052(a5), d0
0B78: 41E8 0040      lea 64(a0), d0
0B7C: 2030 0800      movel.ex 0(a0,d0.l), d0
0B80: B0AD F170      cmpl.ex -3728(a5), d0
0B84: 6F2C           ble 44
0B86: 4AAD F170      tstl.ex -3728(a5)
0B8A: 6F26           ble 38
0B8C: 1B7C 0001 FCCC moveb.emx #1, -820(a5)
0B92: 7000           moveq #0, d0
0B94: 1003           moveb d3, d0
0B96: C1FC 001E      muls.ex #30, d0
0B9A: 41ED F414      lea -3052(a5), d0
0B9E: 41E8 0040      lea 64(a0), d0
0BA2: 4AB0 0800      tstl.ex 0(a0,d0.l)
0BA6: 6F0A           ble 10
0BA8: 1F3C 0009      moveb.ex #9, -(a7)
0BAC: 4EBA FF1E      jsr.ex 32468(pc)
0BB0: 544F           addqw #2, a7
0BB2: 4A2D FCCD      tstb.ex -819(a5)
0BB6: 6646           bne 70
0BB8: 7000           moveq #0, d0
0BBA: 1003           moveb d3, d0
0BBC: C1FC 001E      muls.ex #30, d0
0BC0: 41ED F414      lea -3052(a5), d0
0BC4: 41E8 0040      lea 64(a0), d0
0BC8: 2030 0800      movel.ex 0(a0,d0.l), d0
0BCC: B0AD F0C0      cmpl.ex -3904(a5), d0
0BD0: 6F2C           ble 44
0BD2: 4AAD F0C0      tstl.ex -3904(a5)
0BD6: 6F26           ble 38
0BD8: 1B7C 0001 FCCD moveb.emx #1, -819(a5)
0BDE: 7000           moveq #0, d0
0BE0: 1003           moveb d3, d0
0BE2: C1FC 001E      muls.ex #30, d0
0BE6: 41ED F414      lea -3052(a5), d0
0BEA: 41E8 0040      lea 64(a0), d0
0BEE: 4AB0 0800      tstl.ex 0(a0,d0.l)
0BF2: 6F0A           ble 10
0BF4: 1F3C 0009      moveb.ex #9, -(a7)
0BF8: 4EBA FED2      jsr.ex 32468(pc)
0BFC: 544F           addqw #2, a7
0BFE: 7000           moveq #0, d0
0C00: 1003           moveb d3, d0
0C02: C1FC 001E      muls.ex #30, d0
0C06: 41ED F414      lea -3052(a5), d0
0C0A: 41E8 0046      lea 70(a0), d0
0C0E: 7200           moveq #0, d1
0C10: 1203           moveb d3, d1
0C12: C3FC 001E      muls.ex #30, d1
0C16: 43ED F414      lea -3052(a5), d1
0C1A: 43E9 0040      lea 64(a1), d1
0C1E: 2231 1800      movel.ex 0(a1,d1.l), d1
0C22: B2B0 0800      cmpl.ex 0(a0,d0.l), d1
0C26: 6D54           blt 84
0C28: 7000           moveq #0, d0
0C2A: 1003           moveb d3, d0
0C2C: C1FC 001E      muls.ex #30, d0
0C30: 41ED F414      lea -3052(a5), d0
0C34: 41E8 0046      lea 70(a0), d0
0C38: 41F0 0800      lea 0(a0,d0.l), d0
0C3C: 0690 0000 2710 addil #10000, (a0)
0C42: 7000           moveq #0, d0
0C44: 1003           moveb d3, d0
0C46: C1FC 001E      muls.ex #30, d0
0C4A: 41ED F414      lea -3052(a5), d0
0C4E: 41E8 004A      lea 74(a0), d0
0C52: 5230 0800      addqb.ex #1, 0(a0,d0.l)
0C56: 7000           moveq #0, d0
0C58: 1003           moveb d3, d0
0C5A: C1FC 001E      muls.ex #30, d0
0C5E: 41ED F414      lea -3052(a5), d0
0C62: 41E8 004B      lea 75(a0), d0
0C66: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
0C6C: B62D F4C8      cmpb.ex -2872(a5), d3
0C70: 660A           bne 10
0C72: 1F3C 000A      moveb.ex #10, -(a7)
0C76: 4EBA FE54      jsr.ex 32468(pc)
0C7A: 544F           addqw #2, a7
0C7C: 261F           movel (a7)+, d3
0C7E: 4E5E           unlk a6
0C80: 4E75           rts
0C82: 8D49           dc.w #36169
0C84: 6E63           bgt 99
0C86: 7265           moveq #101, d1
0C88: 6173           bsr 115
0C8A: 6553           bcs 83
0C8C: 636F           bls 111
0C8E: 7265           moveq #101, d1
0C90: 0000           dc.w #0
0C92: 4E56 0000      link a6, #0
0C96: 206E 0008      movel.ex 8(a6), a0
0C9A: 30BC 0100      movew.ex #256, (a0)
0C9E: 317C 0B00 0004 movew.emx #2816, 4(a0)
0CA4: 317C 0100 0002 movew.emx #256, 2(a0)
0CAA: 317C 0A10 0006 movew.emx #2576, 6(a0)
0CB0: 4E5E           unlk a6
0CB2: 4E75           rts
0CB4: 9347           subxrw d7, d1
0CB6: 6574           bcs 116
0CB8: 5669 7369      addqw.ex #3, 29545(a1)
0CBC: 626C           bhi 108
0CBE: 6547           bcs 71
0CC0: 616D           bsr 109
0CC2: 6553           bcs 83
0CC4: 7061           moveq #97, d0
0CC6: 6365           bls 101
0CC8: 0000           dc.w #0
0CCA: 4E56 0000      link a6, #0
0CCE: 206E 0008      movel.ex 8(a6), a0
0CD2: 4A50           tstw (a0)
0CD4: 6C0C           bge 12
0CD6: 0650 0B00      addiw #2816, (a0)
0CDA: 0668 0B00 0004 addiw.ex #2816, 4(a0)
0CE0: 6010           bra 16
0CE2: 0C50 0B00      cmpiw #2816, (a0)
0CE6: 6D0A           blt 10
0CE8: 0450 0B00      subiw #2816, (a0)
0CEC: 0468 0B00 0004 subiw.ex #2816, 4(a0)
0CF2: 4A68 0002      tstw.ex 2(a0)
0CF6: 6C0E           bge 14
0CF8: 0668 0A10 0002 addiw.ex #2576, 2(a0)
0CFE: 0668 0A10 0006 addiw.ex #2576, 6(a0)
0D04: 6014           bra 20
0D06: 0C68 0A10 0002 cmpiw.ex #2576, 2(a0)
0D0C: 6D0C           blt 12
0D0E: 0468 0A10 0002 subiw.ex #2576, 2(a0)
0D14: 0468 0A10 0006 subiw.ex #2576, 6(a0)
0D1A: 4E5E           unlk a6
0D1C: 4E75           rts
0D1E: 8C4C           dc.w #35916
0D20: 6F63           ble 99
0D22: 6174           bsr 116
0D24: 696F           bvs 111
0D26: 6E57           bgt 87
0D28: 7261           moveq #97, d1
0D2A: 7000           moveq #0, d0
0D2C: 0000           dc.w #0
0D2E: 4E56 FFF8      link a6, #-8
0D32: 2F0A           movel a2, -(a7)
0D34: 246E 0008      movel.ex 8(a6), a2
0D38: 486E FFF8      pea.ex -8(a6)
0D3C: 4EBA FF54      jsr.ex -32614(pc)
0D40: 302A 0002      movew.ex 2(a2), d0
0D44: B06E FFFA      cmpw.ex -6(a6), d0
0D48: 584F           addqw #4, a7
0D4A: 6C04           bge 4
0D4C: 7000           moveq #0, d0
0D4E: 602A           bra 42
0D50: 302A 0006      movew.ex 6(a2), d0
0D54: B06E FFFE      cmpw.ex -2(a6), d0
0D58: 6F04           ble 4
0D5A: 7000           moveq #0, d0
0D5C: 601C           bra 28
0D5E: 3012           movew (a2), d0
0D60: B06E FFF8      cmpw.ex -8(a6), d0
0D64: 6C04           bge 4
0D66: 7000           moveq #0, d0
0D68: 6010           bra 16
0D6A: 302A 0004      movew.ex 4(a2), d0
0D6E: B06E FFFC      cmpw.ex -4(a6), d0
0D72: 6F04           ble 4
0D74: 7000           moveq #0, d0
0D76: 6002           bra 2
0D78: 7001           moveq #1, d0
0D7A: 245F           movel (a7)+, a2
0D7C: 4E5E           unlk a6
0D7E: 4E75           rts
0D80: 974F           subxmw -(a7), -(a3)
0D82: 626A           bhi 106
0D84: 6563           bcs 99
0D86: 7443           moveq #67, d2
0D88: 6F6D           ble 109
0D8A: 706C           moveq #108, d0
0D8C: 6574           bcs 116
0D8E: 656C           bcs 108
0D90: 7956           dc.w #31062
0D92: 6973           bvs 115
0D94: 6962           bvs 98
0D96: 6C65           bge 101
0D98: 0000           dc.w #0
0D9A: 4E56 FFF8      link a6, #-8
0D9E: 48E7 1C20      movem <1c20>, -(a7)
0DA2: 246E 0008      movel.ex 8(a6), a2
0DA6: 1A2E 000C      moveb.ex 12(a6), d5
0DAA: 486E FFF8      pea.ex -8(a6)
0DAE: 4EBA FEE2      jsr.ex -32614(pc)
0DB2: 362A 0004      movew.ex 4(a2), d3
0DB6: 9652           subrw (a2), d3
0DB8: 382A 0006      movew.ex 6(a2), d4
0DBC: 986A 0002      subrw.ex 2(a2), d4
0DC0: 4A05           tstb d5
0DC2: 584F           addqw #4, a7
0DC4: 6718           beq 24
0DC6: 066E 0080 FFF8 addiw.ex #128, -8(a6)
0DCC: 066E 0080 FFFA addiw.ex #128, -6(a6)
0DD2: 046E 0080 FFFC subiw.ex #128, -4(a6)
0DD8: 046E 0080 FFFE subiw.ex #128, -2(a6)
0DDE: 42A7           clrl -(a7)
0DE0: 4E4F           trap #15
0DE2: A0C2           sysTrapSysRandom
0DE4: 322E FFFC      movew.ex -4(a6), d1
0DE8: 9243           subrw d3, d1
0DEA: 926E FFF8      subrw.ex -8(a6), d1
0DEE: C3C0           muls d0, d1
0DF0: 2001           movel d1, d0
0DF2: E080           asril #0, d0
0DF4: EC80           asril #6, d0
0DF6: E088           lsril #0, d0
0DF8: E088           lsril #0, d0
0DFA: E288           lsril #1, d0
0DFC: D081           addrl d1, d0
0DFE: E080           asril #0, d0
0E00: EE80           asril #7, d0
0E02: 306E FFF8      movew.ex -8(a6), a0
0E06: D088           addrl a0, d0
0E08: 3480           movew d0, (a2)
0E0A: 42A7           clrl -(a7)
0E0C: 4E4F           trap #15
0E0E: A0C2           sysTrapSysRandom
0E10: 322E FFFE      movew.ex -2(a6), d1
0E14: 9244           subrw d4, d1
0E16: 926E FFFA      subrw.ex -6(a6), d1
0E1A: C3C0           muls d0, d1
0E1C: 2001           movel d1, d0
0E1E: E080           asril #0, d0
0E20: EC80           asril #6, d0
0E22: E088           lsril #0, d0
0E24: E088           lsril #0, d0
0E26: E288           lsril #1, d0
0E28: D081           addrl d1, d0
0E2A: E080           asril #0, d0
0E2C: EE80           asril #7, d0
0E2E: 306E FFFA      movew.ex -6(a6), a0
0E32: D088           addrl a0, d0
0E34: 3540 0002      movew.mx d0, 2(a2)
0E38: 3012           movew (a2), d0
0E3A: D043           addrw d3, d0
0E3C: 3540 0004      movew.mx d0, 4(a2)
0E40: 302A 0002      movew.ex 2(a2), d0
0E44: D044           addrw d4, d0
0E46: 3540 0006      movew.mx d0, 6(a2)
0E4A: 504F           addqw #0, a7
0E4C: 4CDF 0438      movem (a7)+, <0438>
0E50: 4E5E           unlk a6
0E52: 4E75           rts
0E54: 9C4F           subrw a7, d6
0E56: 626A           bhi 106
0E58: 6563           bcs 99
0E5A: 744D           moveq #77, d2
0E5C: 6F76           ble 118
0E5E: 6554           bcs 84
0E60: 6F56           ble 86
0E62: 6973           bvs 115
0E64: 6962           bvs 98
0E66: 6C65           bge 101
0E68: 4761           dc.w #18273
0E6A: 6D65           blt 101
0E6C: 5370 6163      subqw.ex #1, 99(a0,d6.w)
0E70: 6500 0000      bcs 0
0E74: 4E56 0000      link a6, #0
0E78: 4227           clrb -(a7)
0E7A: 4878 0090      pea.ex (0090).w
0E7E: 486D F17C      pea.ex -3716(a5)
0E82: 4E4F           trap #15
0E84: A027           sysTrapMemSet
0E86: 4E5E           unlk a6
0E88: 4E75           rts
0E8A: 8B53           ormw d5, (a3)
0E8C: 6563           bcs 99
0E8E: 746F           moveq #111, d2
0E90: 7252           moveq #82, d1
0E92: 6573           bcs 115
0E94: 6574           bcs 116
0E96: 0000           dc.w #0
0E98: 4E56 0000      link a6, #0
0E9C: 48E7 1C00      movem <1c00>, -(a7)
0EA0: 3A2E 0008      movew.ex 8(a6), d5
0EA4: 382E 000A      movew.ex 10(a6), d4
0EA8: 0C45 0C00      cmpiw #3072, d5
0EAC: 6E04           bgt 4
0EAE: 4A45           tstw d5
0EB0: 6C14           bge 20
0EB2: 486D FE4A      pea.ex -438(a5)
0EB6: 3F3C 07C3      movew.ex #1987, -(a7)
0EBA: 486D FDEA      pea.ex -534(a5)
0EBE: 4E4F           trap #15
0EC0: A084           sysTrapErrDisplayFileLineMsg
0EC2: 4FEF 000A      lea 10(a7), d7
0EC6: 0C44 0B10      cmpiw #2832, d4
0ECA: 6E04           bgt 4
0ECC: 4A44           tstw d4
0ECE: 6C14           bge 20
0ED0: 486D FE4A      pea.ex -438(a5)
0ED4: 3F3C 07C5      movew.ex #1989, -(a7)
0ED8: 486D FDEA      pea.ex -534(a5)
0EDC: 4E4F           trap #15
0EDE: A084           sysTrapErrDisplayFileLineMsg
0EE0: 4FEF 000A      lea 10(a7), d7
0EE4: 0C45 0B00      cmpiw #2816, d5
0EE8: 6D02           blt 2
0EEA: 7A00           moveq #0, d5
0EEC: 0C44 0A10      cmpiw #2576, d4
0EF0: 6D02           blt 2
0EF2: 7800           moveq #0, d4
0EF4: 3604           movew d4, d3
0EF6: E043           asriw #0, d3
0EF8: EE4B           lsriw #7, d3
0EFA: D644           addrw d4, d3
0EFC: E043           asriw #0, d3
0EFE: E243           asriw #1, d3
0F00: 0283 0000 00FF andil #255, d3
0F06: C6FC 0006      mulu.ex #6, d3
0F0A: 3005           movew d5, d0
0F0C: E040           asriw #0, d0
0F0E: EE48           lsriw #7, d0
0F10: D045           addrw d5, d0
0F12: E040           asriw #0, d0
0F14: E240           asriw #1, d0
0F16: D600           addrb d0, d3
0F18: 1003           moveb d3, d0
0F1A: 4CDF 0038      movem (a7)+, <0038>
0F1E: 4E5E           unlk a6
0F20: 4E75           rts
0F22: 8947           dc.w #35143
0F24: 6574           bcs 116
0F26: 5365           subqw #1, -(a5)
0F28: 6374           bls 116
0F2A: 6F72           ble 114
0F2C: 0000           dc.w #0
0F2E: 4E56 0000      link a6, #0
0F32: 48E7 1030      movem <1030>, -(a7)
0F36: 246E 0008      movel.ex 8(a6), a2
0F3A: 302A 0002      movew.ex 2(a2), d0
0F3E: D06A 0006      addrw.ex 6(a2), d0
0F42: 3200           movew d0, d1
0F44: E049           lsriw #0, d1
0F46: EE49           lsriw #7, d1
0F48: D240           addrw d0, d1
0F4A: E241           asriw #1, d1
0F4C: 3F01           movew d1, -(a7)
0F4E: 3012           movew (a2), d0
0F50: D06A 0004      addrw.ex 4(a2), d0
0F54: 3200           movew d0, d1
0F56: E049           lsriw #0, d1
0F58: EE49           lsriw #7, d1
0F5A: D240           addrw d0, d1
0F5C: E241           asriw #1, d1
0F5E: 3F01           movew d1, -(a7)
0F60: 4EBA FF36      jsr.ex -32096(pc)
0F64: 1600           moveb d0, d3
0F66: 7000           moveq #0, d0
0F68: 1003           moveb d3, d0
0F6A: E588           lslil #2, d0
0F6C: 47ED F17C      lea -3716(a5), d3
0F70: D7C0           addal d0, a3
0F72: 584F           addqw #4, a7
0F74: 6012           bra 18
0F76: B5D3           cmpal (a3), a2
0F78: 6606           bne 6
0F7A: 26AA 0010      movel.ex 16(a2), (a3)
0F7E: 600C           bra 12
0F80: 2053           movel (a3), a0
0F82: 5088           addql #0, a0
0F84: 5088           addql #0, a0
0F86: 2648           movel a0, a3
0F88: 4A93           tstl (a3)
0F8A: 66EA           bne -22
0F8C: 4CDF 0C08      movem (a7)+, <0c08>
0F90: 4E5E           unlk a6
0F92: 4E75           rts
0F94: 964F           subrw a7, d3
0F96: 626A           bhi 106
0F98: 6563           bcs 99
0F9A: 7452           moveq #82, d2
0F9C: 656D           bcs 109
0F9E: 6F76           ble 118
0FA0: 6546           bcs 70
0FA2: 726F           moveq #111, d1
0FA4: 6D53           blt 83
0FA6: 6563           bcs 99
0FA8: 746F           moveq #111, d2
0FAA: 7200           moveq #0, d1
0FAC: 0000           dc.w #0
0FAE: 4E56 0000      link a6, #0
0FB2: 48E7 0700      movem <0700>, -(a7)
0FB6: 3E2E 0008      movew.ex 8(a6), d7
0FBA: 3C2E 000A      movew.ex 10(a6), d6
0FBE: 3A2E 000C      movew.ex 12(a6), d5
0FC2: 342E 000E      movew.ex 14(a6), d2
0FC6: 122E 0010      moveb.ex 16(a6), d1
0FCA: 0C2D 0028 F4DB cmpib.ex #40, -2853(a5)
0FD0: 6504           bcs 4
0FD2: 7000           moveq #0, d0
0FD4: 602C           bra 44
0FD6: 522D F4DB      addqb.ex #1, -2853(a5)
0FDA: 7000           moveq #0, d0
0FDC: 102D F4DB      moveb.ex -2853(a5), d0
0FE0: C1FC 000A      muls.ex #10, d0
0FE4: 41ED F414      lea -3052(a5), d0
0FE8: D1C0           addal d0, a0
0FEA: 41E8 06CC      lea 1740(a0), d0
0FEE: 3087           movew d7, (a0)
0FF0: 3146 0002      movew.mx d6, 2(a0)
0FF4: 3145 0004      movew.mx d5, 4(a0)
0FF8: 3142 0006      movew.mx d2, 6(a0)
0FFC: 1141 0008      moveb.mx d1, 8(a0)
1000: 7001           moveq #1, d0
1002: 4CDF 00E0      movem (a7)+, <00e0>
1006: 4E5E           unlk a6
1008: 4E75           rts
100A: 8853           orrw (a3), d4
100C: 7061           moveq #97, d0
100E: 726B           moveq #107, d1
1010: 4164           dc.w #16740
1012: 6400 0000      bcc 0
1016: 4E56 0000      link a6, #0
101A: 48E7 0030      movem <0030>, -(a7)
101E: 246E 0008      movel.ex 8(a6), a2
1022: 7000           moveq #0, d0
1024: 102D F4DB      moveb.ex -2853(a5), d0
1028: C1FC 000A      muls.ex #10, d0
102C: 47ED F414      lea -3052(a5), d3
1030: D7C0           addal d0, a3
1032: 47EB 06CC      lea 1740(a3), d3
1036: B5CB           cmpal a3, a2
1038: 6314           bls 20
103A: 486D FE60      pea.ex -416(a5)
103E: 3F3C 084F      movew.ex #2127, -(a7)
1042: 486D FDEA      pea.ex -534(a5)
1046: 4E4F           trap #15
1048: A084           sysTrapErrDisplayFileLineMsg
104A: 4FEF 000A      lea 10(a7), d7
104E: B5CB           cmpal a3, a2
1050: 6722           beq 34
1052: 204B           movel a3, a0
1054: 91CA           subal a2, a0
1056: 2008           movel a0, d0
1058: 720A           moveq #10, d1
105A: 4EBA F3D6      jsr.ex 30778(pc)
105E: 720A           moveq #10, d1
1060: 4EBA F328      jsr.ex 30610(pc)
1064: 2F00           movel d0, -(a7)
1066: 486A 000A      pea.ex 10(a2)
106A: 2F0A           movel a2, -(a7)
106C: 4E4F           trap #15
106E: A026           sysTrapMemMove
1070: 4FEF 000C      lea 12(a7), d7
1074: 532D F4DB      subqb.ex #1, -2853(a5)
1078: 4CDF 0C00      movem (a7)+, <0c00>
107C: 4E5E           unlk a6
107E: 4E75           rts
1080: 8B53           ormw d5, (a3)
1082: 7061           moveq #97, d0
1084: 726B           moveq #107, d1
1086: 5265           addqw #1, -(a5)
1088: 6D6F           blt 111
108A: 7665           moveq #101, d3
108C: 0000           dc.w #0
108E: 4E56 0000      link a6, #0
1092: 48E7 0030      movem <0030>, -(a7)
1096: 7000           moveq #0, d0
1098: 102D F4DB      moveb.ex -2853(a5), d0
109C: C1FC 000A      muls.ex #10, d0
10A0: 45ED F414      lea -3052(a5), d2
10A4: D5C0           addal d0, a2
10A6: 45EA 06CC      lea 1740(a2), d2
10AA: 47ED F414      lea -3052(a5), d3
10AE: 47EB 06D6      lea 1750(a3), d3
10B2: 6012           bra 18
10B4: 4A2A 0008      tstb.ex 8(a2)
10B8: 6608           bne 8
10BA: 2F0A           movel a2, -(a7)
10BC: 4EBA FF58      jsr.ex -31714(pc)
10C0: 584F           addqw #4, a7
10C2: 700A           moveq #10, d0
10C4: 95C0           subal d0, a2
10C6: B5CB           cmpal a3, a2
10C8: 64EA           bcc -22
10CA: 4CDF 0C00      movem (a7)+, <0c00>
10CE: 4E5E           unlk a6
10D0: 4E75           rts
10D2: 9253           subrw (a3), d1
10D4: 7061           moveq #97, d0
10D6: 726B           moveq #107, d1
10D8: 5265           addqw #1, -(a5)
10DA: 6D6F           blt 111
10DC: 7665           moveq #101, d3
10DE: 4578           dc.w #17784
10E0: 7069           moveq #105, d0
10E2: 7265           moveq #101, d1
10E4: 6400 0000      bcc 0
10E8: 4E56 FFFC      link a6, #-4
10EC: 48E7 1F30      movem <1f30>, -(a7)
10F0: 266E 0008      movel.ex 8(a6), a3
10F4: 246E 000C      movel.ex 12(a6), a2
10F8: B7CA           cmpal a2, a3
10FA: 6718           beq 24
10FC: 302B 0008      movew.ex 8(a3), d0
1100: D06A 0008      addrw.ex 8(a2), d0
1104: 3D40 FFFE      movew.mx d0, -2(a6)
1108: 302B 000A      movew.ex 10(a3), d0
110C: D06A 000A      addrw.ex 10(a2), d0
1110: 3D40 FFFC      movew.mx d0, -4(a6)
1114: 42A7           clrl -(a7)
1116: 4E4F           trap #15
1118: A0C2           sysTrapSysRandom
111A: 48C0           extl d0
111C: E588           lslil #2, d0
111E: 2200           movel d0, d1
1120: E081           asril #0, d1
1122: EC81           asril #6, d1
1124: E089           lsril #0, d1
1126: E089           lsril #0, d1
1128: E289           lsril #1, d1
112A: D280           addrl d0, d1
112C: E081           asril #0, d1
112E: EE81           asril #7, d1
1130: 5481           addql #2, d1
1132: 3A01           movew d1, d5
1134: 363C 0180      movew.ex #384, d3
1138: 3013           movew (a3), d0
113A: D06B 0004      addrw.ex 4(a3), d0
113E: D052           addrw (a2), d0
1140: D06A 0004      addrw.ex 4(a2), d0
1144: 3C00           movew d0, d6
1146: E246           asriw #1, d6
1148: E04E           lsriw #0, d6
114A: EC4E           lsriw #6, d6
114C: DC40           addrw d0, d6
114E: E446           asriw #2, d6
1150: 302B 0002      movew.ex 2(a3), d0
1154: D06B 0006      addrw.ex 6(a3), d0
1158: D06A 0002      addrw.ex 2(a2), d0
115C: D06A 0006      addrw.ex 6(a2), d0
1160: 3E00           movew d0, d7
1162: E247           asriw #1, d7
1164: E04F           lsriw #0, d7
1166: EC4F           lsriw #6, d7
1168: DE40           addrw d0, d7
116A: E447           asriw #2, d7
116C: 584F           addqw #4, a7
116E: 6000 00AC      bra 172
1172: 42A7           clrl -(a7)
1174: 4E4F           trap #15
1176: A0C2           sysTrapSysRandom
1178: C1FC 000A      muls.ex #10, d0
117C: 2200           movel d0, d1
117E: E081           asril #0, d1
1180: EC81           asril #6, d1
1182: E089           lsril #0, d1
1184: E089           lsril #0, d1
1186: E289           lsril #1, d1
1188: D280           addrl d0, d1
118A: E081           asril #0, d1
118C: EE81           asril #7, d1
118E: 7014           moveq #20, d0
1190: D280           addrl d0, d1
1192: 3801           movew d1, d4
1194: 1F04           moveb d4, -(a7)
1196: 42A7           clrl -(a7)
1198: 4E4F           trap #15
119A: A0C2           sysTrapSysRandom
119C: C1C3           muls d3, d0
119E: 2200           movel d0, d1
11A0: E081           asril #0, d1
11A2: EC81           asril #6, d1
11A4: E089           lsril #0, d1
11A6: E089           lsril #0, d1
11A8: E289           lsril #1, d1
11AA: D280           addrl d0, d1
11AC: E081           asril #0, d1
11AE: EE81           asril #7, d1
11B0: 3003           movew d3, d0
11B2: E048           lsriw #0, d0
11B4: EE48           lsriw #7, d0
11B6: D043           addrw d3, d0
11B8: E240           asriw #1, d0
11BA: 48C0           extl d0
11BC: 9280           subrl d0, d1
11BE: 306E FFFC      movew.ex -4(a6), a0
11C2: D288           addrl a0, d1
11C4: 3044           movew d4, a0
11C6: 2001           movel d1, d0
11C8: 2208           movel a0, d1
11CA: 4EBA F266      jsr.ex 30778(pc)
11CE: 584F           addqw #4, a7
11D0: 3F00           movew d0, -(a7)
11D2: 42A7           clrl -(a7)
11D4: 4E4F           trap #15
11D6: A0C2           sysTrapSysRandom
11D8: C1C3           muls d3, d0
11DA: 2200           movel d0, d1
11DC: E081           asril #0, d1
11DE: EC81           asril #6, d1
11E0: E089           lsril #0, d1
11E2: E089           lsril #0, d1
11E4: E289           lsril #1, d1
11E6: D280           addrl d0, d1
11E8: E081           asril #0, d1
11EA: EE81           asril #7, d1
11EC: 3003           movew d3, d0
11EE: E048           lsriw #0, d0
11F0: EE48           lsriw #7, d0
11F2: D043           addrw d3, d0
11F4: E240           asriw #1, d0
11F6: 48C0           extl d0
11F8: 9280           subrl d0, d1
11FA: 306E FFFE      movew.ex -2(a6), a0
11FE: D288           addrl a0, d1
1200: 3044           movew d4, a0
1202: 2001           movel d1, d0
1204: 2208           movel a0, d1
1206: 4EBA F22A      jsr.ex 30778(pc)
120A: 584F           addqw #4, a7
120C: 3F00           movew d0, -(a7)
120E: 3F07           movew d7, -(a7)
1210: 3F06           movew d6, -(a7)
1212: 4EBA FD9A      jsr.ex -31818(pc)
1216: 5345           subqw #1, d5
1218: 4FEF 000E      lea 14(a7), d7
121C: 4A45           tstw d5
121E: 6E00 FF52      bgt 65362
1222: 4CDF 0CF8      movem (a7)+, <0cf8>
1226: 4E5E           unlk a6
1228: 4E75           rts
122A: 9653           subrw (a3), d3
122C: 7061           moveq #97, d0
122E: 726B           moveq #107, d1
1230: 4164           dc.w #16740
1232: 6442           bcc 66
1234: 6574           bcs 116
1236: 7765           dc.w #30565
1238: 656E           bcs 110
123A: 4F62           dc.w #20322
123C: 6A65           bpl 101
123E: 6374           bls 116
1240: 7300           dc.w #29440
1242: 0000           dc.w #0
1244: 4E56 0000      link a6, #0
1248: 48E7 1820      movem <1820>, -(a7)
124C: 246E 0008      movel.ex 8(a6), a2
1250: 363C 0180      movew.ex #384, d3
1254: 42A7           clrl -(a7)
1256: 4E4F           trap #15
1258: A0C2           sysTrapSysRandom
125A: C1FC 000A      muls.ex #10, d0
125E: 2200           movel d0, d1
1260: E081           asril #0, d1
1262: EC81           asril #6, d1
1264: E089           lsril #0, d1
1266: E089           lsril #0, d1
1268: E289           lsril #1, d1
126A: D280           addrl d0, d1
126C: E081           asril #0, d1
126E: EE81           asril #7, d1
1270: 7014           moveq #20, d0
1272: D280           addrl d0, d1
1274: 3801           movew d1, d4
1276: 1F04           moveb d4, -(a7)
1278: 42A7           clrl -(a7)
127A: 4E4F           trap #15
127C: A0C2           sysTrapSysRandom
127E: C1C3           muls d3, d0
1280: 2200           movel d0, d1
1282: E081           asril #0, d1
1284: EC81           asril #6, d1
1286: E089           lsril #0, d1
1288: E089           lsril #0, d1
128A: E289           lsril #1, d1
128C: D280           addrl d0, d1
128E: E081           asril #0, d1
1290: EE81           asril #7, d1
1292: 3003           movew d3, d0
1294: E048           lsriw #0, d0
1296: EE48           lsriw #7, d0
1298: D043           addrw d3, d0
129A: E240           asriw #1, d0
129C: 48C0           extl d0
129E: 9280           subrl d0, d1
12A0: 3044           movew d4, a0
12A2: 2001           movel d1, d0
12A4: 2208           movel a0, d1
12A6: 4EBA F18A      jsr.ex 30778(pc)
12AA: 584F           addqw #4, a7
12AC: 3F00           movew d0, -(a7)
12AE: 42A7           clrl -(a7)
12B0: 4E4F           trap #15
12B2: A0C2           sysTrapSysRandom
12B4: C1C3           muls d3, d0
12B6: 2200           movel d0, d1
12B8: E081           asril #0, d1
12BA: EC81           asril #6, d1
12BC: E089           lsril #0, d1
12BE: E089           lsril #0, d1
12C0: E289           lsril #1, d1
12C2: D280           addrl d0, d1
12C4: E081           asril #0, d1
12C6: EE81           asril #7, d1
12C8: 3003           movew d3, d0
12CA: E048           lsriw #0, d0
12CC: EE48           lsriw #7, d0
12CE: D043           addrw d3, d0
12D0: E240           asriw #1, d0
12D2: 48C0           extl d0
12D4: 9280           subrl d0, d1
12D6: 3044           movew d4, a0
12D8: 2001           movel d1, d0
12DA: 2208           movel a0, d1
12DC: 4EBA F154      jsr.ex 30778(pc)
12E0: 584F           addqw #4, a7
12E2: 3F00           movew d0, -(a7)
12E4: 42A7           clrl -(a7)
12E6: 4E4F           trap #15
12E8: A0C2           sysTrapSysRandom
12EA: 322A 0006      movew.ex 6(a2), d1
12EE: 926A 0002      subrw.ex 2(a2), d1
12F2: C3C0           muls d0, d1
12F4: 2001           movel d1, d0
12F6: E080           asril #0, d0
12F8: EC80           asril #6, d0
12FA: E088           lsril #0, d0
12FC: E088           lsril #0, d0
12FE: E288           lsril #1, d0
1300: D081           addrl d1, d0
1302: E080           asril #0, d0
1304: EE80           asril #7, d0
1306: 306A 0002      movew.ex 2(a2), a0
130A: D088           addrl a0, d0
130C: 584F           addqw #4, a7
130E: 3F00           movew d0, -(a7)
1310: 42A7           clrl -(a7)
1312: 4E4F           trap #15
1314: A0C2           sysTrapSysRandom
1316: 322A 0004      movew.ex 4(a2), d1
131A: 9252           subrw (a2), d1
131C: C3C0           muls d0, d1
131E: 2001           movel d1, d0
1320: E080           asril #0, d0
1322: EC80           asril #6, d0
1324: E088           lsril #0, d0
1326: E088           lsril #0, d0
1328: E288           lsril #1, d0
132A: D081           addrl d1, d0
132C: E080           asril #0, d0
132E: EE80           asril #7, d0
1330: 3052           movew (a2), a0
1332: D088           addrl a0, d0
1334: 584F           addqw #4, a7
1336: 3F00           movew d0, -(a7)
1338: 4EBA FC74      jsr.ex -31818(pc)
133C: 4FEF 000E      lea 14(a7), d7
1340: 4CDF 0418      movem (a7)+, <0418>
1344: 4E5E           unlk a6
1346: 4E75           rts
1348: 9253           subrw (a3), d1
134A: 7061           moveq #97, d0
134C: 726B           moveq #107, d1
134E: 4164           dc.w #16740
1350: 6446           bcc 70
1352: 726F           moveq #111, d1
1354: 6D4F           blt 79
1356: 626A           bhi 106
1358: 6563           bcs 99
135A: 7400           moveq #0, d2
135C: 0000           dc.w #0
135E: 4E56 0000      link a6, #0
1362: 48E7 1F20      movem <1f20>, -(a7)
1366: 3E2E 0008      movew.ex 8(a6), d7
136A: 3C2E 000A      movew.ex 10(a6), d6
136E: 3A2E 000C      movew.ex 12(a6), d5
1372: 0C2D 0005 F4DC cmpib.ex #5, -2852(a5)
1378: 6506           bcs 6
137A: 7000           moveq #0, d0
137C: 6000 0098      bra 152
1380: 522D F4DC      addqb.ex #1, -2852(a5)
1384: 7000           moveq #0, d0
1386: 102D F4DC      moveb.ex -2852(a5), d0
138A: E988           lslil #4, d0
138C: 45ED F414      lea -3052(a5), d2
1390: D5C0           addal d0, a2
1392: 45EA 0856      lea 2134(a2), d2
1396: 3045           movew d5, a0
1398: 2F08           movel a0, -(a7)
139A: 486A 0009      pea.ex 9(a2)
139E: 4E4F           trap #15
13A0: A0C9           sysTrapStrIToA
13A2: 486A 0009      pea.ex 9(a2)
13A6: 4E4F           trap #15
13A8: A0C7           sysTrapStrLen
13AA: 1540 000F      moveb.mx d0, 15(a2)
13AE: 4227           clrb -(a7)
13B0: 4E4F           trap #15
13B2: A164           sysTrapFntSetFont
13B4: 7000           moveq #0, d0
13B6: 102A 000F      moveb.ex 15(a2), d0
13BA: 3F00           movew d0, -(a7)
13BC: 486A 0009      pea.ex 9(a2)
13C0: 4E4F           trap #15
13C2: A16B           sysTrapFntCharsWidth
13C4: 5340           subqw #1, d0
13C6: E948           lsliw #4, d0
13C8: 5340           subqw #1, d0
13CA: 3600           movew d0, d3
13CC: 4E4F           trap #15
13CE: A166           sysTrapFntBaseLine
13D0: E948           lsliw #4, d0
13D2: 5340           subqw #1, d0
13D4: 3800           movew d0, d4
13D6: 3003           movew d3, d0
13D8: E048           lsriw #0, d0
13DA: EE48           lsriw #7, d0
13DC: D043           addrw d3, d0
13DE: E240           asriw #1, d0
13E0: 3207           movew d7, d1
13E2: 9240           subrw d0, d1
13E4: 3481           movew d1, (a2)
13E6: 3004           movew d4, d0
13E8: E048           lsriw #0, d0
13EA: EE48           lsriw #7, d0
13EC: D044           addrw d4, d0
13EE: E240           asriw #1, d0
13F0: 3206           movew d6, d1
13F2: 9240           subrw d0, d1
13F4: 3541 0002      movew.mx d1, 2(a2)
13F8: 3012           movew (a2), d0
13FA: D043           addrw d3, d0
13FC: 3540 0004      movew.mx d0, 4(a2)
1400: 302A 0002      movew.ex 2(a2), d0
1404: D044           addrw d4, d0
1406: 3540 0006      movew.mx d0, 6(a2)
140A: 157C 002D 0008 moveb.emx #45, 8(a2)
1410: 7001           moveq #1, d0
1412: 4FEF 0014      lea 20(a7), d7
1416: 4CDF 04F8      movem (a7)+, <04f8>
141A: 4E5E           unlk a6
141C: 4E75           rts
141E: 8853           orrw (a3), d4
1420: 636F           bls 111
1422: 7265           moveq #101, d1
1424: 4164           dc.w #16740
1426: 6400 0000      bcc 0
142A: 4E56 0000      link a6, #0
142E: 48E7 0030      movem <0030>, -(a7)
1432: 246E 0008      movel.ex 8(a6), a2
1436: 7000           moveq #0, d0
1438: 102D F4DC      moveb.ex -2852(a5), d0
143C: E988           lslil #4, d0
143E: 47ED F414      lea -3052(a5), d3
1442: D7C0           addal d0, a3
1444: 47EB 0856      lea 2134(a3), d3
1448: B5CB           cmpal a3, a2
144A: 6314           bls 20
144C: 486D FE84      pea.ex -380(a5)
1450: 3F3C 0939      movew.ex #2361, -(a7)
1454: 486D FDEA      pea.ex -534(a5)
1458: 4E4F           trap #15
145A: A084           sysTrapErrDisplayFileLineMsg
145C: 4FEF 000A      lea 10(a7), d7
1460: B5CB           cmpal a3, a2
1462: 6726           beq 38
1464: 204B           movel a3, a0
1466: 91CA           subal a2, a0
1468: 2008           movel a0, d0
146A: E680           asril #3, d0
146C: E088           lsril #0, d0
146E: E088           lsril #0, d0
1470: E088           lsril #0, d0
1472: E888           lsril #4, d0
1474: D088           addrl a0, d0
1476: E880           asril #4, d0
1478: E988           lslil #4, d0
147A: 2F00           movel d0, -(a7)
147C: 486A 0010      pea.ex 16(a2)
1480: 2F0A           movel a2, -(a7)
1482: 4E4F           trap #15
1484: A026           sysTrapMemMove
1486: 4FEF 000C      lea 12(a7), d7
148A: 532D F4DC      subqb.ex #1, -2852(a5)
148E: 4CDF 0C00      movem (a7)+, <0c00>
1492: 4E5E           unlk a6
1494: 4E75           rts
1496: 8B53           ormw d5, (a3)
1498: 636F           bls 111
149A: 7265           moveq #101, d1
149C: 5265           addqw #1, -(a5)
149E: 6D6F           blt 111
14A0: 7665           moveq #101, d3
14A2: 0000           dc.w #0
14A4: 4E56 0000      link a6, #0
14A8: 48E7 0030      movem <0030>, -(a7)
14AC: 7000           moveq #0, d0
14AE: 102D F4DC      moveb.ex -2852(a5), d0
14B2: E988           lslil #4, d0
14B4: 45ED F414      lea -3052(a5), d2
14B8: D5C0           addal d0, a2
14BA: 45EA 0856      lea 2134(a2), d2
14BE: 47ED F414      lea -3052(a5), d3
14C2: 47EB 0866      lea 2150(a3), d3
14C6: 6012           bra 18
14C8: 4A2A 0008      tstb.ex 8(a2)
14CC: 6608           bne 8
14CE: 2F0A           movel a2, -(a7)
14D0: 4EBA FF58      jsr.ex -30670(pc)
14D4: 584F           addqw #4, a7
14D6: 7010           moveq #16, d0
14D8: 95C0           subal d0, a2
14DA: B5CB           cmpal a3, a2
14DC: 64EA           bcc -22
14DE: 4CDF 0C00      movem (a7)+, <0c00>
14E2: 4E5E           unlk a6
14E4: 4E75           rts
14E6: 9253           subrw (a3), d1
14E8: 636F           bls 111
14EA: 7265           moveq #101, d1
14EC: 5265           addqw #1, -(a5)
14EE: 6D6F           blt 111
14F0: 7665           moveq #101, d3
14F2: 4578           dc.w #17784
14F4: 7069           moveq #105, d0
14F6: 7265           moveq #101, d1
14F8: 6400 0000      bcc 0
14FC: 4E56 0000      link a6, #0
1500: 48E7 1038      movem <1038>, -(a7)
1504: 0C2D 0003 F4D9 cmpib.ex #3, -2855(a5)
150A: 6506           bcs 6
150C: 7000           moveq #0, d0
150E: 6000 0104      bra 260
1512: 522D F4D9      addqb.ex #1, -2855(a5)
1516: 7000           moveq #0, d0
1518: 102D F4D9      moveb.ex -2855(a5), d0
151C: C1FC 0016      muls.ex #22, d0
1520: 45ED F414      lea -3052(a5), d2
1524: D5C0           addal d0, a2
1526: 45EA 067E      lea 1662(a2), d2
152A: 42A7           clrl -(a7)
152C: 4E4F           trap #15
152E: A0C2           sysTrapSysRandom
1530: C1FC 0006      muls.ex #6, d0
1534: 2200           movel d0, d1
1536: E081           asril #0, d1
1538: EC81           asril #6, d1
153A: E089           lsril #0, d1
153C: E089           lsril #0, d1
153E: E289           lsril #1, d1
1540: D280           addrl d0, d1
1542: E081           asril #0, d1
1544: EE81           asril #7, d1
1546: 700C           moveq #12, d0
1548: D280           addrl d0, d1
154A: 1541 000E      moveb.mx d1, 14(a2)
154E: 0C2A 0011 000E cmpib.ex #17, 14(a2)
1554: 584F           addqw #4, a7
1556: 6628           bne 40
1558: 42A7           clrl -(a7)
155A: 4E4F           trap #15
155C: A0C2           sysTrapSysRandom
155E: 48C0           extl d0
1560: D080           addrl d0, d0
1562: 2200           movel d0, d1
1564: E081           asril #0, d1
1566: EC81           asril #6, d1
1568: E089           lsril #0, d1
156A: E089           lsril #0, d1
156C: E289           lsril #1, d1
156E: D280           addrl d0, d1
1570: E081           asril #0, d1
1572: EE81           asril #7, d1
1574: 5381           subql #1, d1
1576: 584F           addqw #4, a7
1578: 6606           bne 6
157A: 157C 0010 000E moveb.emx #16, 14(a2)
1580: 157C 0001 000D moveb.emx #1, 13(a2)
1586: 157C 0001 000C moveb.emx #1, 12(a2)
158C: 4252           clrw (a2)
158E: 426A 0002      clrw.ex 2(a2)
1592: 357C 00C0 0004 movew.emx #192, 4(a2)
1598: 357C 00C0 0006 movew.emx #192, 6(a2)
159E: 4227           clrb -(a7)
15A0: 2F0A           movel a2, -(a7)
15A2: 4EBA F7F6      jsr.ex -32350(pc)
15A6: 7600           moveq #0, d3
15A8: 7000           moveq #0, d0
15AA: 102D F4D0      moveb.ex -2864(a5), d0
15AE: C1FC 0026      muls.ex #38, d0
15B2: 47ED F414      lea -3052(a5), d3
15B6: D7C0           addal d0, a3
15B8: 47EB 00A4      lea 164(a3), d3
15BC: 49ED F414      lea -3052(a5), d4
15C0: 49EC 00CA      lea 202(a4), d4
15C4: 5C4F           addqw #6, a7
15C6: 601C           bra 28
15C8: 1F3C 0001      moveb.ex #1, -(a7)
15CC: 2F0B           movel a3, -(a7)
15CE: 2F0A           movel a2, -(a7)
15D0: 4EBA 284A      jsr.ex -19932(pc)
15D4: 4A00           tstb d0
15D6: 4FEF 000A      lea 10(a7), d7
15DA: 6704           beq 4
15DC: 7601           moveq #1, d3
15DE: 6008           bra 8
15E0: 7026           moveq #38, d0
15E2: 97C0           subal d0, a3
15E4: B7CC           cmpal a4, a3
15E6: 64E0           bcc -32
15E8: 4A03           tstb d3
15EA: 66A0           bne -96
15EC: 42A7           clrl -(a7)
15EE: 4E4F           trap #15
15F0: A0C2           sysTrapSysRandom
15F2: C1FC 007F      muls.ex #127, d0
15F6: 2200           movel d0, d1
15F8: E081           asril #0, d1
15FA: EC81           asril #6, d1
15FC: E089           lsril #0, d1
15FE: E089           lsril #0, d1
1600: E289           lsril #1, d1
1602: D280           addrl d0, d1
1604: E081           asril #0, d1
1606: EE81           asril #7, d1
1608: 707F           moveq #127, d0
160A: D280           addrl d0, d1
160C: 1541 0014      moveb.mx d1, 20(a2)
1610: 7001           moveq #1, d0
1612: 584F           addqw #4, a7
1614: 4CDF 1C08      movem (a7)+, <1c08>
1618: 4E5E           unlk a6
161A: 4E75           rts
161C: 8842           orrw d2, d4
161E: 6F6E           ble 110
1620: 7573           dc.w #30067
1622: 4164           dc.w #16740
1624: 6400 0000      bcc 0
1628: 4E56 0000      link a6, #0
162C: 48E7 0030      movem <0030>, -(a7)
1630: 246E 0008      movel.ex 8(a6), a2
1634: 7000           moveq #0, d0
1636: 102D F4D9      moveb.ex -2855(a5), d0
163A: C1FC 0016      muls.ex #22, d0
163E: 47ED F414      lea -3052(a5), d3
1642: D7C0           addal d0, a3
1644: 47EB 067E      lea 1662(a3), d3
1648: B5CB           cmpal a3, a2
164A: 6314           bls 20
164C: 486D FEA8      pea.ex -344(a5)
1650: 3F3C 09DD      movew.ex #2525, -(a7)
1654: 486D FDEA      pea.ex -534(a5)
1658: 4E4F           trap #15
165A: A084           sysTrapErrDisplayFileLineMsg
165C: 4FEF 000A      lea 10(a7), d7
1660: B5CB           cmpal a3, a2
1662: 6722           beq 34
1664: 204B           movel a3, a0
1666: 91CA           subal a2, a0
1668: 2008           movel a0, d0
166A: 7216           moveq #22, d1
166C: 4EBA EDC4      jsr.ex 30778(pc)
1670: 7216           moveq #22, d1
1672: 4EBA ED16      jsr.ex 30610(pc)
1676: 2F00           movel d0, -(a7)
1678: 486A 0016      pea.ex 22(a2)
167C: 2F0A           movel a2, -(a7)
167E: 4E4F           trap #15
1680: A026           sysTrapMemMove
1682: 4FEF 000C      lea 12(a7), d7
1686: 532D F4D9      subqb.ex #1, -2855(a5)
168A: 4CDF 0C00      movem (a7)+, <0c00>
168E: 4E5E           unlk a6
1690: 4E75           rts
1692: 8B42           dc.w #35650
1694: 6F6E           ble 110
1696: 7573           dc.w #30067
1698: 5265           addqw #1, -(a5)
169A: 6D6F           blt 111
169C: 7665           moveq #101, d3
169E: 0000           dc.w #0
16A0: 4E56 0000      link a6, #0
16A4: 48E7 0030      movem <0030>, -(a7)
16A8: 7000           moveq #0, d0
16AA: 102D F4D9      moveb.ex -2855(a5), d0
16AE: C1FC 0016      muls.ex #22, d0
16B2: 45ED F414      lea -3052(a5), d2
16B6: D5C0           addal d0, a2
16B8: 45EA 067E      lea 1662(a2), d2
16BC: 47ED F414      lea -3052(a5), d3
16C0: 47EB 0694      lea 1684(a3), d3
16C4: 6018           bra 24
16C6: 4A2A 0014      tstb.ex 20(a2)
16CA: 6706           beq 6
16CC: 4A2A 000C      tstb.ex 12(a2)
16D0: 6608           bne 8
16D2: 2F0A           movel a2, -(a7)
16D4: 4EBA FF52      jsr.ex -30160(pc)
16D8: 584F           addqw #4, a7
16DA: 7016           moveq #22, d0
16DC: 95C0           subal d0, a2
16DE: B5CB           cmpal a3, a2
16E0: 64E4           bcc -28
16E2: 4CDF 0C00      movem (a7)+, <0c00>
16E6: 4E5E           unlk a6
16E8: 4E75           rts
16EA: 9C42           subrw d2, d6
16EC: 6F6E           ble 110
16EE: 7573           dc.w #30067
16F0: 5265           addqw #1, -(a5)
16F2: 6D6F           blt 111
16F4: 7665           moveq #101, d3
16F6: 4578           dc.w #17784
16F8: 7069           moveq #105, d0
16FA: 7265           moveq #101, d1
16FC: 644F           bcc 79
16FE: 7255           moveq #85, d1
1700: 6E75           bgt 117
1702: 7361           dc.w #29537
1704: 626C           bhi 108
1706: 6500 0000      bcs 0
170A: 4E56 0000      link a6, #0
170E: 2F0A           movel a2, -(a7)
1710: 246E 0008      movel.ex 8(a6), a2
1714: 0C2A 000C 000E cmpib.ex #12, 14(a2)
171A: 6508           bcs 8
171C: 0C2A 0011 000E cmpib.ex #17, 14(a2)
1722: 6314           bls 20
1724: 486D FECC      pea.ex -308(a5)
1728: 3F3C 0A25      movew.ex #2597, -(a7)
172C: 486D FDEA      pea.ex -534(a5)
1730: 4E4F           trap #15
1732: A084           sysTrapErrDisplayFileLineMsg
1734: 4FEF 000A      lea 10(a7), d7
1738: 422A 000C      clrb.ex 12(a2)
173C: 422A 000D      clrb.ex 13(a2)
1740: 245F           movel (a7)+, a2
1742: 4E5E           unlk a6
1744: 4E75           rts
1746: 9142           subxrw d2, d0
1748: 6F6E           ble 110
174A: 7573           dc.w #30067
174C: 5365           subqw #1, -(a5)
174E: 744E           moveq #78, d2
1750: 6F74           ble 116
1752: 5573 6162      subqw.ex #2, 98(a3,d6.w)
1756: 6C65           bge 101
1758: 0000           dc.w #0
175A: 4E56 0000      link a6, #0
175E: 48E7 1F00      movem <1f00>, -(a7)
1762: 1C2E 0008      moveb.ex 8(a6), d6
1766: 3E2E 0010      movew.ex 16(a6), d7
176A: 0C2D 001E F4D7 cmpib.ex #30, -2857(a5)
1770: 6506           bcs 6
1772: 7000           moveq #0, d0
1774: 6000 00FC      bra 252
1778: 7000           moveq #0, d0
177A: 1006           moveb d6, d0
177C: 0440 0009      subiw #9, d0
1780: 670A           beq 10
1782: 5340           subqw #1, d0
1784: 6710           beq 16
1786: 5340           subqw #1, d0
1788: 6716           beq 22
178A: 601C           bra 28
178C: 383C 0080      movew.ex #128, d4
1790: 3A3C 0080      movew.ex #128, d5
1794: 6012           bra 18
1796: 383C 00C0      movew.ex #192, d4
179A: 3A3C 00C0      movew.ex #192, d5
179E: 6008           bra 8
17A0: 383C 0100      movew.ex #256, d4
17A4: 3A3C 0100      movew.ex #256, d5
17A8: 7600           moveq #0, d3
17AA: 162D F4D7      moveb.ex -2857(a5), d3
17AE: 7014           moveq #20, d0
17B0: C1C3           muls d3, d0
17B2: 41ED F414      lea -3052(a5), d0
17B6: 41E8 0448      lea 1096(a0), d0
17BA: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
17C0: 7014           moveq #20, d0
17C2: C1C3           muls d3, d0
17C4: 41ED F414      lea -3052(a5), d0
17C8: 41E8 0449      lea 1097(a0), d0
17CC: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
17D2: 7014           moveq #20, d0
17D4: C1C3           muls d3, d0
17D6: 41ED F414      lea -3052(a5), d0
17DA: 41E8 044A      lea 1098(a0), d0
17DE: 1186 0800      moveb.mx d6, 0(a0,d0.l)
17E2: 7014           moveq #20, d0
17E4: C1C3           muls d3, d0
17E6: 41ED F414      lea -3052(a5), d0
17EA: 41E8 043C      lea 1084(a0), d0
17EE: 31AE 000A 0800 movew.emx 10(a6), 0(a0,d0.l)
17F4: 7014           moveq #20, d0
17F6: C1C3           muls d3, d0
17F8: 41ED F414      lea -3052(a5), d0
17FC: 41E8 043E      lea 1086(a0), d0
1800: 31AE 000C 0800 movew.emx 12(a6), 0(a0,d0.l)
1806: 7014           moveq #20, d0
1808: C1C3           muls d3, d0
180A: 41ED F414      lea -3052(a5), d0
180E: 41E8 043C      lea 1084(a0), d0
1812: 3030 0800      movew.ex 0(a0,d0.l), d0
1816: D044           addrw d4, d0
1818: 7214           moveq #20, d1
181A: C3C3           muls d3, d1
181C: 41ED F414      lea -3052(a5), d0
1820: 41E8 0440      lea 1088(a0), d0
1824: 3180 1800      movew.mx d0, 0(a0,d1.l)
1828: 7014           moveq #20, d0
182A: C1C3           muls d3, d0
182C: 41ED F414      lea -3052(a5), d0
1830: 41E8 043E      lea 1086(a0), d0
1834: 3030 0800      movew.ex 0(a0,d0.l), d0
1838: D045           addrw d5, d0
183A: 7214           moveq #20, d1
183C: C3C3           muls d3, d1
183E: 41ED F414      lea -3052(a5), d0
1842: 41E8 0442      lea 1090(a0), d0
1846: 3180 1800      movew.mx d0, 0(a0,d1.l)
184A: 7014           moveq #20, d0
184C: C1C3           muls d3, d0
184E: 41ED F414      lea -3052(a5), d0
1852: 41E8 0444      lea 1092(a0), d0
1856: 31AE 000E 0800 movew.emx 14(a6), 0(a0,d0.l)
185C: 7014           moveq #20, d0
185E: C1C3           muls d3, d0
1860: 41ED F414      lea -3052(a5), d0
1864: 41E8 0446      lea 1094(a0), d0
1868: 3187 0800      movew.mx d7, 0(a0,d0.l)
186C: 522D F4D7      addqb.ex #1, -2857(a5)
1870: 7001           moveq #1, d0
1872: 4CDF 00F8      movem (a7)+, <00f8>
1876: 4E5E           unlk a6
1878: 4E75           rts
187A: 8752           ormw d3, (a2)
187C: 6F63           ble 99
187E: 6B41           bmi 65
1880: 6464           bcc 100
1882: 0000           dc.w #0
1884: 4E56 FFFC      link a6, #-4
1888: 48E7 1F20      movem <1f20>, -(a7)
188C: 42A7           clrl -(a7)
188E: 4E4F           trap #15
1890: A0C2           sysTrapSysRandom
1892: C1FC 0B00      muls.ex #2816, d0
1896: 2200           movel d0, d1
1898: E081           asril #0, d1
189A: EC81           asril #6, d1
189C: E089           lsril #0, d1
189E: E089           lsril #0, d1
18A0: E289           lsril #1, d1
18A2: D280           addrl d0, d1
18A4: E081           asril #0, d1
18A6: EE81           asril #7, d1
18A8: 3A01           movew d1, d5
18AA: 42A7           clrl -(a7)
18AC: 4E4F           trap #15
18AE: A0C2           sysTrapSysRandom
18B0: C1FC 0A10      muls.ex #2576, d0
18B4: 2200           movel d0, d1
18B6: E081           asril #0, d1
18B8: EC81           asril #6, d1
18BA: E089           lsril #0, d1
18BC: E089           lsril #0, d1
18BE: E289           lsril #1, d1
18C0: D280           addrl d0, d1
18C2: E081           asril #0, d1
18C4: EE81           asril #7, d1
18C6: 3C01           movew d1, d6
18C8: 780E           moveq #14, d4
18CA: 504F           addqw #0, a7
18CC: 6038           bra 56
18CE: 7026           moveq #38, d0
18D0: C1C4           muls d4, d0
18D2: 45ED F414      lea -3052(a5), d2
18D6: D5C0           addal d0, a2
18D8: 45EA 00CA      lea 202(a2), d2
18DC: 4A2A 000C      tstb.ex 12(a2)
18E0: 6722           beq 34
18E2: 3612           movew (a2), d3
18E4: 9645           subrw d5, d3
18E6: 4A43           tstw d3
18E8: 6C02           bge 2
18EA: 4443           negw d3
18EC: 0C43 01E0      cmpiw #480, d3
18F0: 6D7A           blt 122
18F2: 362A 0002      movew.ex 2(a2), d3
18F6: 9646           subrw d6, d3
18F8: 4A43           tstw d3
18FA: 6C02           bge 2
18FC: 4443           negw d3
18FE: 0C43 01E0      cmpiw #480, d3
1902: 6D68           blt 104
1904: 5344           subqw #1, d4
1906: 4A44           tstw d4
1908: 6CC4           bge -60
190A: 42A7           clrl -(a7)
190C: 4E4F           trap #15
190E: A0C2           sysTrapSysRandom
1910: 48C0           extl d0
1912: E988           lslil #4, d0
1914: 2200           movel d0, d1
1916: E081           asril #0, d1
1918: EC81           asril #6, d1
191A: E089           lsril #0, d1
191C: E089           lsril #0, d1
191E: E289           lsril #1, d1
1920: D280           addrl d0, d1
1922: E081           asril #0, d1
1924: EE81           asril #7, d1
1926: 3E01           movew d1, d7
1928: 3047           movew d7, a0
192A: 2008           movel a0, d0
192C: D080           addrl d0, d0
192E: 41ED FCCE      lea -818(a5), d0
1932: 3030 0800      movew.ex 0(a0,d0.l), d0
1936: E548           lsliw #2, d0
1938: 3D40 FFFE      movew.mx d0, -2(a6)
193C: 3047           movew d7, a0
193E: 2008           movel a0, d0
1940: D080           addrl d0, d0
1942: 41ED FCEE      lea -786(a5), d0
1946: 3030 0800      movew.ex 0(a0,d0.l), d0
194A: E548           lsliw #2, d0
194C: 3D40 FFFC      movew.mx d0, -4(a6)
1950: 3F2E FFFC      movew.ex -4(a6), -(a7)
1954: 3F2E FFFE      movew.ex -2(a6), -(a7)
1958: 3F06           movew d6, -(a7)
195A: 3F05           movew d5, -(a7)
195C: 1F3C 000B      moveb.ex #11, -(a7)
1960: 4EBA FDF8      jsr.ex -29854(pc)
1964: 532D F43C      subqb.ex #1, -3012(a5)
1968: 4FEF 000E      lea 14(a7), d7
196C: 4CDF 04F8      movem (a7)+, <04f8>
1970: 4E5E           unlk a6
1972: 4E75           rts
1974: 9252           subrw (a2), d1
1976: 6F63           ble 99
1978: 6B41           bmi 65
197A: 6464           bcc 100
197C: 496E           dc.w #18798
197E: 6974           bvs 116
1980: 6961           bvs 97
1982: 6C52           bge 82
1984: 6F63           ble 99
1986: 6B00 0000      bmi 0
198A: 4E56 0000      link a6, #0
198E: 48E7 0030      movem <0030>, -(a7)
1992: 246E 0008      movel.ex 8(a6), a2
1996: 0C2A 0009 000E cmpib.ex #9, 14(a2)
199C: 6508           bcs 8
199E: 0C2A 000B 000E cmpib.ex #11, 14(a2)
19A4: 6314           bls 20
19A6: 486D FECC      pea.ex -308(a5)
19AA: 3F3C 0ADC      movew.ex #2780, -(a7)
19AE: 486D FDEA      pea.ex -534(a5)
19B2: 4E4F           trap #15
19B4: A084           sysTrapErrDisplayFileLineMsg
19B6: 4FEF 000A      lea 10(a7), d7
19BA: 7000           moveq #0, d0
19BC: 102D F4D7      moveb.ex -2857(a5), d0
19C0: 5340           subqw #1, d0
19C2: C1FC 0014      muls.ex #20, d0
19C6: 47ED F414      lea -3052(a5), d3
19CA: D7C0           addal d0, a3
19CC: 47EB 043C      lea 1084(a3), d3
19D0: B5CB           cmpal a3, a2
19D2: 6314           bls 20
19D4: 486D FEE0      pea.ex -288(a5)
19D8: 3F3C 0AE2      movew.ex #2786, -(a7)
19DC: 486D FDEA      pea.ex -534(a5)
19E0: 4E4F           trap #15
19E2: A084           sysTrapErrDisplayFileLineMsg
19E4: 4FEF 000A      lea 10(a7), d7
19E8: B5CB           cmpal a3, a2
19EA: 6722           beq 34
19EC: 204B           movel a3, a0
19EE: 91CA           subal a2, a0
19F0: 2008           movel a0, d0
19F2: 7214           moveq #20, d1
19F4: 4EBA EA3C      jsr.ex 30778(pc)
19F8: 7214           moveq #20, d1
19FA: 4EBA E98E      jsr.ex 30610(pc)
19FE: 2F00           movel d0, -(a7)
1A00: 486A 0014      pea.ex 20(a2)
1A04: 2F0A           movel a2, -(a7)
1A06: 4E4F           trap #15
1A08: A026           sysTrapMemMove
1A0A: 4FEF 000C      lea 12(a7), d7
1A0E: 422B 000C      clrb.ex 12(a3)
1A12: 422B 000D      clrb.ex 13(a3)
1A16: 532D F4D7      subqb.ex #1, -2857(a5)
1A1A: 4CDF 0C00      movem (a7)+, <0c00>
1A1E: 4E5E           unlk a6
1A20: 4E75           rts
1A22: 8A52           orrw (a2), d5
1A24: 6F63           ble 99
1A26: 6B52           bmi 82
1A28: 656D           bcs 109
1A2A: 6F76           ble 118
1A2C: 6500 0000      bcs 0
1A30: 4E56 0000      link a6, #0
1A34: 48E7 1020      movem <1020>, -(a7)
1A38: 7600           moveq #0, d3
1A3A: 162D F4D8      moveb.ex -2856(a5), d3
1A3E: 7000           moveq #0, d0
1A40: 102D F4D7      moveb.ex -2857(a5), d0
1A44: C1FC 0014      muls.ex #20, d0
1A48: 45ED F414      lea -3052(a5), d2
1A4C: D5C0           addal d0, a2
1A4E: 45EA 0428      lea 1064(a2), d2
1A52: 6034           bra 52
1A54: 41ED F414      lea -3052(a5), d0
1A58: 41E8 043C      lea 1084(a0), d0
1A5C: B5C8           cmpal a0, a2
1A5E: 6414           bcc 20
1A60: 486D FF04      pea.ex -252(a5)
1A64: 3F3C 0B13      movew.ex #2835, -(a7)
1A68: 486D FDEA      pea.ex -534(a5)
1A6C: 4E4F           trap #15
1A6E: A084           sysTrapErrDisplayFileLineMsg
1A70: 4FEF 000A      lea 10(a7), d7
1A74: 4A2A 000C      tstb.ex 12(a2)
1A78: 660A           bne 10
1A7A: 2F0A           movel a2, -(a7)
1A7C: 4EBA FF0C      jsr.ex -29294(pc)
1A80: 5343           subqw #1, d3
1A82: 584F           addqw #4, a7
1A84: 7014           moveq #20, d0
1A86: 95C0           subal d0, a2
1A88: 4A43           tstw d3
1A8A: 6EC8           bgt -56
1A8C: 422D F4D8      clrb.ex -2856(a5)
1A90: 4CDF 0408      movem (a7)+, <0408>
1A94: 4E5E           unlk a6
1A96: 4E75           rts
1A98: 9252           subrw (a2), d1
1A9A: 6F63           ble 99
1A9C: 6B52           bmi 82
1A9E: 656D           bcs 109
1AA0: 6F76           ble 118
1AA2: 6555           bcs 85
1AA4: 6E75           bgt 117
1AA6: 7361           dc.w #29537
1AA8: 626C           bhi 108
1AAA: 6500 0000      bcs 0
1AAE: 4E56 0000      link a6, #0
1AB2: 2F0A           movel a2, -(a7)
1AB4: 246E 0008      movel.ex 8(a6), a2
1AB8: 0C2A 0009 000E cmpib.ex #9, 14(a2)
1ABE: 6508           bcs 8
1AC0: 0C2A 000B 000E cmpib.ex #11, 14(a2)
1AC6: 6314           bls 20
1AC8: 486D FECC      pea.ex -308(a5)
1ACC: 3F3C 0B37      movew.ex #2871, -(a7)
1AD0: 486D FDEA      pea.ex -534(a5)
1AD4: 4E4F           trap #15
1AD6: A084           sysTrapErrDisplayFileLineMsg
1AD8: 4FEF 000A      lea 10(a7), d7
1ADC: 422A 000C      clrb.ex 12(a2)
1AE0: 422A 000D      clrb.ex 13(a2)
1AE4: 522D F4D8      addqb.ex #1, -2856(a5)
1AE8: 245F           movel (a7)+, a2
1AEA: 4E5E           unlk a6
1AEC: 4E75           rts
1AEE: 9052           subrw (a2), d0
1AF0: 6F63           ble 99
1AF2: 6B53           bmi 83
1AF4: 6574           bcs 116
1AF6: 4E6F           move usp, a7
1AF8: 7455           moveq #85, d2
1AFA: 7361           dc.w #29537
1AFC: 626C           bhi 108
1AFE: 6500 0000      bcs 0
1B02: 4E56 FFFE      link a6, #-2
1B06: 48E7 1F30      movem <1f30>, -(a7)
1B0A: 246E 0008      movel.ex 8(a6), a2
1B0E: 266E 000C      movel.ex 12(a6), a3
1B12: 7602           moveq #2, d3
1B14: 0C2A 0009 000E cmpib.ex #9, 14(a2)
1B1A: 6508           bcs 8
1B1C: 0C2A 000B 000E cmpib.ex #11, 14(a2)
1B22: 6314           bls 20
1B24: 486D FECC      pea.ex -308(a5)
1B28: 3F3C 0B61      movew.ex #2913, -(a7)
1B2C: 486D FDEA      pea.ex -534(a5)
1B30: 4E4F           trap #15
1B32: A084           sysTrapErrDisplayFileLineMsg
1B34: 4FEF 000A      lea 10(a7), d7
1B38: B7CA           cmpal a2, a3
1B3A: 673C           beq 60
1B3C: 0C2B 0007 000E cmpib.ex #7, 14(a3)
1B42: 6524           bcs 36
1B44: 0C2B 0008 000E cmpib.ex #8, 14(a3)
1B4A: 621C           bhi 28
1B4C: 302B 0008      movew.ex 8(a3), d0
1B50: 48C0           extl d0
1B52: 81FC 0010      divs.ex #16, d0
1B56: 3740 0008      movew.mx d0, 8(a3)
1B5A: 302B 000A      movew.ex 10(a3), d0
1B5E: 48C0           extl d0
1B60: 81FC 0010      divs.ex #16, d0
1B64: 3740 000A      movew.mx d0, 10(a3)
1B68: 302B 0008      movew.ex 8(a3), d0
1B6C: D16A 0008      addmw.ex d0, 8(a2)
1B70: 302B 000A      movew.ex 10(a3), d0
1B74: D16A 000A      addmw.ex d0, 10(a2)
1B78: 0C2A 0009 000E cmpib.ex #9, 14(a2)
1B7E: 6700 0140      beq 320
1B82: 3012           movew (a2), d0
1B84: D06A 0004      addrw.ex 4(a2), d0
1B88: 3800           movew d0, d4
1B8A: E04C           lsriw #0, d4
1B8C: EE4C           lsriw #7, d4
1B8E: D840           addrw d0, d4
1B90: E244           asriw #1, d4
1B92: 302A 0002      movew.ex 2(a2), d0
1B96: D06A 0006      addrw.ex 6(a2), d0
1B9A: 3A00           movew d0, d5
1B9C: E04D           lsriw #0, d5
1B9E: EE4D           lsriw #7, d5
1BA0: DA40           addrw d0, d5
1BA2: E245           asriw #1, d5
1BA4: 0C2A 000B 000E cmpib.ex #11, 14(a2)
1BAA: 665E           bne 94
1BAC: 7605           moveq #5, d3
1BAE: 302A 000A      movew.ex 10(a2), d0
1BB2: D06A 0008      addrw.ex 8(a2), d0
1BB6: 3F00           movew d0, -(a7)
1BB8: 302A 0008      movew.ex 8(a2), d0
1BBC: 906A 000A      subrw.ex 10(a2), d0
1BC0: 3F00           movew d0, -(a7)
1BC2: 70A0           moveq #160, d0
1BC4: D045           addrw d5, d0
1BC6: 3F00           movew d0, -(a7)
1BC8: 70A0           moveq #160, d0
1BCA: D044           addrw d4, d0
1BCC: 3F00           movew d0, -(a7)
1BCE: 1F3C 000A      moveb.ex #10, -(a7)
1BD2: 4EBA FB86      jsr.ex -29854(pc)
1BD6: 302A 000A      movew.ex 10(a2), d0
1BDA: 906A 0008      subrw.ex 8(a2), d0
1BDE: 3F00           movew d0, -(a7)
1BE0: 302A 0008      movew.ex 8(a2), d0
1BE4: D06A 000A      addrw.ex 10(a2), d0
1BE8: 3F00           movew d0, -(a7)
1BEA: 70A0           moveq #160, d0
1BEC: D045           addrw d5, d0
1BEE: 3F00           movew d0, -(a7)
1BF0: 70A0           moveq #160, d0
1BF2: D044           addrw d4, d0
1BF4: 3F00           movew d0, -(a7)
1BF6: 1F3C 000A      moveb.ex #10, -(a7)
1BFA: 4EBA FB5E      jsr.ex -29854(pc)
1BFE: 1D7C 0003 FFFF moveb.emx #3, -1(a6)
1C04: 4FEF 0014      lea 20(a7), d7
1C08: 6064           bra 100
1C0A: 0C2A 000A 000E cmpib.ex #10, 14(a2)
1C10: 665C           bne 92
1C12: 7603           moveq #3, d3
1C14: 302A 000A      movew.ex 10(a2), d0
1C18: D06A 0008      addrw.ex 8(a2), d0
1C1C: 3F00           movew d0, -(a7)
1C1E: 302A 0008      movew.ex 8(a2), d0
1C22: 906A 000A      subrw.ex 10(a2), d0
1C26: 3F00           movew d0, -(a7)
1C28: 70C0           moveq #192, d0
1C2A: D045           addrw d5, d0
1C2C: 3F00           movew d0, -(a7)
1C2E: 70C0           moveq #192, d0
1C30: D044           addrw d4, d0
1C32: 3F00           movew d0, -(a7)
1C34: 1F3C 0009      moveb.ex #9, -(a7)
1C38: 4EBA FB20      jsr.ex -29854(pc)
1C3C: 302A 000A      movew.ex 10(a2), d0
1C40: 906A 0008      subrw.ex 8(a2), d0
1C44: 3F00           movew d0, -(a7)
1C46: 302A 0008      movew.ex 8(a2), d0
1C4A: D06A 000A      addrw.ex 10(a2), d0
1C4E: 3F00           movew d0, -(a7)
1C50: 70C0           moveq #192, d0
1C52: D045           addrw d5, d0
1C54: 3F00           movew d0, -(a7)
1C56: 70C0           moveq #192, d0
1C58: D044           addrw d4, d0
1C5A: 3F00           movew d0, -(a7)
1C5C: 1F3C 0009      moveb.ex #9, -(a7)
1C60: 4EBA FAF8      jsr.ex -29854(pc)
1C64: 1D7C 0004 FFFF moveb.emx #4, -1(a6)
1C6A: 4FEF 0014      lea 20(a7), d7
1C6E: 42A7           clrl -(a7)
1C70: 4E4F           trap #15
1C72: A0C2           sysTrapSysRandom
1C74: C1FC 0006      muls.ex #6, d0
1C78: 2200           movel d0, d1
1C7A: E081           asril #0, d1
1C7C: EC81           asril #6, d1
1C7E: E089           lsril #0, d1
1C80: E089           lsril #0, d1
1C82: E289           lsril #1, d1
1C84: D280           addrl d0, d1
1C86: E081           asril #0, d1
1C88: EE81           asril #7, d1
1C8A: 5381           subql #1, d1
1C8C: 584F           addqw #4, a7
1C8E: 6636           bne 54
1C90: D643           addrw d3, d3
1C92: 302A 000A      movew.ex 10(a2), d0
1C96: D06A 0008      addrw.ex 8(a2), d0
1C9A: 3F00           movew d0, -(a7)
1C9C: 302A 0008      movew.ex 8(a2), d0
1CA0: D06A 0008      addrw.ex 8(a2), d0
1CA4: 3F00           movew d0, -(a7)
1CA6: 70C0           moveq #192, d0
1CA8: D045           addrw d5, d0
1CAA: 3F00           movew d0, -(a7)
1CAC: 70C0           moveq #192, d0
1CAE: D044           addrw d4, d0
1CB0: 3F00           movew d0, -(a7)
1CB2: 1F3C 0009      moveb.ex #9, -(a7)
1CB6: 4EBA FAA2      jsr.ex -29854(pc)
1CBA: 4FEF 000A      lea 10(a7), d7
1CBE: 6006           bra 6
1CC0: 1D7C 0005 FFFF moveb.emx #5, -1(a6)
1CC6: 42A7           clrl -(a7)
1CC8: 4E4F           trap #15
1CCA: A0C2           sysTrapSysRandom
1CCC: C1FC 0003      muls.ex #3, d0
1CD0: 2200           movel d0, d1
1CD2: E081           asril #0, d1
1CD4: EC81           asril #6, d1
1CD6: E089           lsril #0, d1
1CD8: E089           lsril #0, d1
1CDA: E289           lsril #1, d1
1CDC: D280           addrl d0, d1
1CDE: E081           asril #0, d1
1CE0: EE81           asril #7, d1
1CE2: D641           addrw d1, d3
1CE4: 3C2A 0004      movew.ex 4(a2), d6
1CE8: 9C52           subrw (a2), d6
1CEA: E54E           lsliw #2, d6
1CEC: 3012           movew (a2), d0
1CEE: D06A 0004      addrw.ex 4(a2), d0
1CF2: D053           addrw (a3), d0
1CF4: D06B 0004      addrw.ex 4(a3), d0
1CF8: 3800           movew d0, d4
1CFA: E244           asriw #1, d4
1CFC: E04C           lsriw #0, d4
1CFE: EC4C           lsriw #6, d4
1D00: D840           addrw d0, d4
1D02: E444           asriw #2, d4
1D04: 302A 0002      movew.ex 2(a2), d0
1D08: D06A 0006      addrw.ex 6(a2), d0
1D0C: D06B 0002      addrw.ex 2(a3), d0
1D10: D06B 0006      addrw.ex 6(a3), d0
1D14: 3A00           movew d0, d5
1D16: E245           asriw #1, d5
1D18: E04D           lsriw #0, d5
1D1A: EC4D           lsriw #6, d5
1D1C: DA40           addrw d0, d5
1D1E: E445           asriw #2, d5
1D20: 584F           addqw #4, a7
1D22: 6000 00A0      bra 160
1D26: 42A7           clrl -(a7)
1D28: 4E4F           trap #15
1D2A: A0C2           sysTrapSysRandom
1D2C: C1FC 000A      muls.ex #10, d0
1D30: 2200           movel d0, d1
1D32: E081           asril #0, d1
1D34: EC81           asril #6, d1
1D36: E089           lsril #0, d1
1D38: E089           lsril #0, d1
1D3A: E289           lsril #1, d1
1D3C: D280           addrl d0, d1
1D3E: E081           asril #0, d1
1D40: EE81           asril #7, d1
1D42: 7014           moveq #20, d0
1D44: D280           addrl d0, d1
1D46: 3E01           movew d1, d7
1D48: 1F07           moveb d7, -(a7)
1D4A: 42A7           clrl -(a7)
1D4C: 4E4F           trap #15
1D4E: A0C2           sysTrapSysRandom
1D50: C1C6           muls d6, d0
1D52: 2200           movel d0, d1
1D54: E081           asril #0, d1
1D56: EC81           asril #6, d1
1D58: E089           lsril #0, d1
1D5A: E089           lsril #0, d1
1D5C: E289           lsril #1, d1
1D5E: D280           addrl d0, d1
1D60: E081           asril #0, d1
1D62: EE81           asril #7, d1
1D64: 3006           movew d6, d0
1D66: E048           lsriw #0, d0
1D68: EE48           lsriw #7, d0
1D6A: D046           addrw d6, d0
1D6C: E240           asriw #1, d0
1D6E: 48C0           extl d0
1D70: 9280           subrl d0, d1
1D72: 3047           movew d7, a0
1D74: 2001           movel d1, d0
1D76: 2208           movel a0, d1
1D78: 4EBA E6B8      jsr.ex 30778(pc)
1D7C: 584F           addqw #4, a7
1D7E: 3F00           movew d0, -(a7)
1D80: 42A7           clrl -(a7)
1D82: 4E4F           trap #15
1D84: A0C2           sysTrapSysRandom
1D86: C1C6           muls d6, d0
1D88: 2200           movel d0, d1
1D8A: E081           asril #0, d1
1D8C: EC81           asril #6, d1
1D8E: E089           lsril #0, d1
1D90: E089           lsril #0, d1
1D92: E289           lsril #1, d1
1D94: D280           addrl d0, d1
1D96: E081           asril #0, d1
1D98: EE81           asril #7, d1
1D9A: 3006           movew d6, d0
1D9C: E048           lsriw #0, d0
1D9E: EE48           lsriw #7, d0
1DA0: D046           addrw d6, d0
1DA2: E240           asriw #1, d0
1DA4: 48C0           extl d0
1DA6: 9280           subrl d0, d1
1DA8: 3047           movew d7, a0
1DAA: 2001           movel d1, d0
1DAC: 2208           movel a0, d1
1DAE: 4EBA E682      jsr.ex 30778(pc)
1DB2: 584F           addqw #4, a7
1DB4: 3F00           movew d0, -(a7)
1DB6: 3F05           movew d5, -(a7)
1DB8: 3F04           movew d4, -(a7)
1DBA: 4EBA F1F2      jsr.ex -31818(pc)
1DBE: 5343           subqw #1, d3
1DC0: 4FEF 000E      lea 14(a7), d7
1DC4: 4A43           tstw d3
1DC6: 6E00 FF5E      bgt 65374
1DCA: 2F0A           movel a2, -(a7)
1DCC: 4EBA FCE0      jsr.ex -29002(pc)
1DD0: 1F2E FFFF      moveb.ex -1(a6), -(a7)
1DD4: 4EBA ECF6      jsr.ex 32468(pc)
1DD8: 5C4F           addqw #6, a7
1DDA: 4CDF 0CF8      movem (a7)+, <0cf8>
1DDE: 4E5E           unlk a6
1DE0: 4E75           rts
1DE2: 8B52           ormw d5, (a2)
1DE4: 6F63           ble 99
1DE6: 6B45           bmi 69
1DE8: 7870           moveq #112, d4
1DEA: 6C6F           bge 111
1DEC: 6465           bcc 101
1DEE: 0000           dc.w #0
1DF0: 4E56 FFFC      link a6, #-4
1DF4: 48E7 1F38      movem <1f38>, -(a7)
1DF8: 286E 0008      movel.ex 8(a6), a4
1DFC: 2C3C 7FFF FFFF movel.exl #2147483647, d6
1E02: 97CB           subal a3, a3
1E04: 7000           moveq #0, d0
1E06: 102D F4D0      moveb.ex -2864(a5), d0
1E0A: C1FC 0026      muls.ex #38, d0
1E0E: 45ED F414      lea -3052(a5), d2
1E12: D5C0           addal d0, a2
1E14: 45EA 00A4      lea 164(a2), d2
1E18: 41ED F414      lea -3052(a5), d0
1E1C: 41E8 00CA      lea 202(a0), d0
1E20: 2E08           movel a0, d7
1E22: 6056           bra 86
1E24: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
1E2A: 644A           bcc 74
1E2C: 082A 0005 001E btst #5, 30(a2)
1E32: 6642           bne 66
1E34: 082A 0004 001E btst #4, 30(a2)
1E3A: 663A           bne 58
1E3C: 082A 0004 0020 btst #4, 32(a2)
1E42: 6632           bne 50
1E44: 3614           movew (a4), d3
1E46: 9652           subrw (a2), d3
1E48: 48C3           extl d3
1E4A: 382C 0002      movew.ex 2(a4), d4
1E4E: 986A 0002      subrw.ex 2(a2), d4
1E52: 48C4           extl d4
1E54: 2004           movel d4, d0
1E56: 2204           movel d4, d1
1E58: 4EBA E530      jsr.ex 30610(pc)
1E5C: 2D40 FFFC      movel.mx d0, -4(a6)
1E60: 2003           movel d3, d0
1E62: 2203           movel d3, d1
1E64: 4EBA E524      jsr.ex 30610(pc)
1E68: D0AE FFFC      addrl.ex -4(a6), d0
1E6C: 2A00           movel d0, d5
1E6E: BA86           cmpl d6, d5
1E70: 6C04           bge 4
1E72: 2C05           movel d5, d6
1E74: 264A           movel a2, a3
1E76: 7026           moveq #38, d0
1E78: 95C0           subal d0, a2
1E7A: B5C7           cmpal d7, a2
1E7C: 64A6           bcc -90
1E7E: 204B           movel a3, a0
1E80: 4CDF 1CF8      movem (a7)+, <1cf8>
1E84: 4E5E           unlk a6
1E86: 4E75           rts
1E88: 9453           subrw (a3), d2
1E8A: 6869           bvc 105
1E8C: 7047           moveq #71, d0
1E8E: 6574           bcs 116
1E90: 4E65           move a5, usp
1E92: 6172           bsr 114
1E94: 6573           bcs 115
1E96: 7450           moveq #80, d2
1E98: 6C61           bge 97
1E9A: 7965           dc.w #31077
1E9C: 7200           moveq #0, d1
1E9E: 0000           dc.w #0
1EA0: 4E56 0000      link a6, #0
1EA4: 322E 0008      movew.ex 8(a6), d1
1EA8: 7000           moveq #0, d0
1EAA: 102D F4D0      moveb.ex -2864(a5), d0
1EAE: C1FC 0026      muls.ex #38, d0
1EB2: 41ED F414      lea -3052(a5), d0
1EB6: D1C0           addal d0, a0
1EB8: 41E8 00A4      lea 164(a0), d0
1EBC: 43ED F414      lea -3052(a5), d1
1EC0: 43E9 00CA      lea 202(a1), d1
1EC4: 600C           bra 12
1EC6: B268 0016      cmpw.ex 22(a0), d1
1ECA: 6602           bne 2
1ECC: 600A           bra 10
1ECE: 7026           moveq #38, d0
1ED0: 91C0           subal d0, a0
1ED2: B1C9           cmpal a1, a0
1ED4: 64F0           bcc -16
1ED6: 91C8           subal a0, a0
1ED8: 4E5E           unlk a6
1EDA: 4E75           rts
1EDC: 9253           subrw (a3), d1
1EDE: 6869           bvc 105
1EE0: 7046           moveq #70, d0
1EE2: 696E           bvs 110
1EE4: 6442           bcc 66
1EE6: 7955           dc.w #31061
1EE8: 6E69           bgt 105
1EEA: 7175           dc.w #29045
1EEC: 6549           bcs 73
1EEE: 4400           negb d0
1EF0: 0000           dc.w #0
1EF2: 4E56 0000      link a6, #0
1EF6: 426D F4D4      clrw.ex -2860(a5)
1EFA: 4E5E           unlk a6
1EFC: 4E75           rts
1EFE: 9053           subrw (a3), d0
1F00: 6869           bvc 105
1F02: 7049           moveq #73, d0
1F04: 6E69           bgt 105
1F06: 7455           moveq #85, d2
1F08: 6E69           bgt 105
1F0A: 7175           dc.w #29045
1F0C: 6549           bcs 73
1F0E: 4400           negb d0
1F10: 0000           dc.w #0
1F12: 4E56 0000      link a6, #0
1F16: 302D F4D4      movew.ex -2860(a5), d0
1F1A: 526D F4D4      addqw.ex #1, -2860(a5)
1F1E: 4E5E           unlk a6
1F20: 4E75           rts
1F22: 9253           subrw (a3), d1
1F24: 6869           bvc 105
1F26: 7047           moveq #71, d0
1F28: 6574           bcs 116
1F2A: 4E65           move a5, usp
1F2C: 7755           dc.w #30549
1F2E: 6E69           bgt 105
1F30: 7175           dc.w #29045
1F32: 6549           bcs 73
1F34: 4400           negb d0
1F36: 0000           dc.w #0
1F38: 4E56 0000      link a6, #0
1F3C: 48E7 0038      movem <0038>, -(a7)
1F40: 286E 0008      movel.ex 8(a6), a4
1F44: 7000           moveq #0, d0
1F46: 102C 0014      moveb.ex 20(a4), d0
1F4A: C1FC 001E      muls.ex #30, d0
1F4E: 45ED F414      lea -3052(a5), d2
1F52: D5C0           addal d0, a2
1F54: 45EA 003C      lea 60(a2), d2
1F58: 264A           movel a2, a3
1F5A: 47EB 0014      lea 20(a3), d3
1F5E: 204A           movel a2, a0
1F60: 41E8 001A      lea 26(a0), d0
1F64: 2208           movel a0, d1
1F66: 603A           bra 58
1F68: 3013           movew (a3), d0
1F6A: B06C 0016      cmpw.ex 22(a4), d0
1F6E: 6630           bne 48
1F70: 204A           movel a2, a0
1F72: 41E8 0014      lea 20(a0), d0
1F76: 224B           movel a3, a1
1F78: 93C8           subal a0, a1
1F7A: 2409           movel a1, d2
1F7C: E08A           lsril #0, d2
1F7E: E08A           lsril #0, d2
1F80: E08A           lsril #0, d2
1F82: EE8A           lsril #7, d2
1F84: D489           addrl a1, d2
1F86: E282           asril #1, d2
1F88: 7000           moveq #0, d0
1F8A: 1002           moveb d2, d0
1F8C: D080           addrl d0, d0
1F8E: 204A           movel a2, a0
1F90: 41E8 0014      lea 20(a0), d0
1F94: 31BC FFFF 0800 movew.emx #65535, 0(a0,d0.l)
1F9A: 532A 0013      subqb.ex #1, 19(a2)
1F9E: 6006           bra 6
1FA0: 548B           addql #2, a3
1FA2: B7C1           cmpal d1, a3
1FA4: 63C2           bls -62
1FA6: 4CDF 1C00      movem (a7)+, <1c00>
1FAA: 4E5E           unlk a6
1FAC: 4E75           rts
1FAE: 9753           submw d3, (a3)
1FB0: 6869           bvc 105
1FB2: 7052           moveq #82, d0
1FB4: 656D           bcs 109
1FB6: 6F76           ble 118
1FB8: 6546           bcs 70
1FBA: 726F           moveq #111, d1
1FBC: 6D46           blt 70
1FBE: 6F72           ble 114
1FC0: 6D61           blt 97
1FC2: 7469           moveq #105, d2
1FC4: 6F6E           ble 110
1FC6: 0000           dc.w #0
1FC8: 4E56 FFF6      link a6, #-10
1FCC: 48E7 1F38      movem <1f38>, -(a7)
1FD0: 246E 0008      movel.ex 8(a6), a2
1FD4: 302A 0004      movew.ex 4(a2), d0
1FD8: 9052           subrw (a2), d0
1FDA: 3D40 FFFC      movew.mx d0, -4(a6)
1FDE: 302A 0006      movew.ex 6(a2), d0
1FE2: 906A 0002      subrw.ex 2(a2), d0
1FE6: 3D40 FFFA      movew.mx d0, -6(a6)
1FEA: 7000           moveq #0, d0
1FEC: 102A 0014      moveb.ex 20(a2), d0
1FF0: C1FC 001E      muls.ex #30, d0
1FF4: 41ED F414      lea -3052(a5), d0
1FF8: D1C0           addal d0, a0
1FFA: 41E8 003C      lea 60(a0), d0
1FFE: 2808           movel a0, d4
2000: 2044           movel d4, a0
2002: 41E8 001A      lea 26(a0), d0
2006: 2D48 FFF6      movel.mx a0, -10(a6)
200A: 2044           movel d4, a0
200C: 41E8 0014      lea 20(a0), d0
2010: 2C08           movel a0, d6
2012: 600A           bra 10
2014: 2046           movel d6, a0
2016: 0C50 FFFF      cmpiw #-1, (a0)
201A: 6708           beq 8
201C: 5486           addql #2, d6
201E: BCAE FFF6      cmpl.ex -10(a6), d6
2022: 63F0           bls -16
2024: 2644           movel d4, a3
2026: 47EB 0014      lea 20(a3), d3
202A: 6008           bra 8
202C: 0C53 FFFF      cmpiw #-1, (a3)
2030: 6608           bne 8
2032: 548B           addql #2, a3
2034: B7EE FFF6      cmpal.ex -10(a6), a3
2038: 63F2           bls -14
203A: 3F13           movew (a3), -(a7)
203C: 4EBA FE62      jsr.ex -27992(pc)
2040: 2848           movel a0, a4
2042: 3E14           movew (a4), d7
2044: 3D6C 0002 FFFE movew.emx 2(a4), -2(a6)
204A: 2044           movel d4, a0
204C: 41E8 0014      lea 20(a0), d0
2050: B7C8           cmpal a0, a3
2052: 544F           addqw #2, a7
2054: 6700 0080      beq 128
2058: 2044           movel d4, a0
205A: 41E8 0014      lea 20(a0), d0
205E: 224B           movel a3, a1
2060: 93C8           subal a0, a1
2062: 2A09           movel a1, d5
2064: E08D           lsril #0, d5
2066: E08D           lsril #0, d5
2068: E08D           lsril #0, d5
206A: EE8D           lsril #7, d5
206C: DA89           addrl a1, d5
206E: E285           asril #1, d5
2070: 7000           moveq #0, d0
2072: 1005           moveb d5, d0
2074: 5340           subqw #1, d0
2076: D040           addrw d0, d0
2078: 48C0           extl d0
207A: D080           addrl d0, d0
207C: 41ED FCCE      lea -818(a5), d0
2080: 3630 0800      movew.ex 0(a0,d0.l), d3
2084: 3003           movew d3, d0
2086: E048           lsriw #0, d0
2088: EE48           lsriw #7, d0
208A: D043           addrw d3, d0
208C: E240           asriw #1, d0
208E: D043           addrw d3, d0
2090: C1EE FFFC      muls.ex -4(a6), d0
2094: 3200           movew d0, d1
2096: E049           lsriw #0, d1
2098: EE49           lsriw #7, d1
209A: D240           addrw d0, d1
209C: E241           asriw #1, d1
209E: 3601           movew d1, d3
20A0: 9E43           subrw d3, d7
20A2: 7000           moveq #0, d0
20A4: 1005           moveb d5, d0
20A6: 5340           subqw #1, d0
20A8: D040           addrw d0, d0
20AA: 48C0           extl d0
20AC: D080           addrl d0, d0
20AE: 41ED FCEE      lea -786(a5), d0
20B2: 3630 0800      movew.ex 0(a0,d0.l), d3
20B6: 3003           movew d3, d0
20B8: E048           lsriw #0, d0
20BA: EE48           lsriw #7, d0
20BC: D043           addrw d3, d0
20BE: E240           asriw #1, d0
20C0: D043           addrw d3, d0
20C2: C1EE FFFA      muls.ex -6(a6), d0
20C6: 3200           movew d0, d1
20C8: E049           lsriw #0, d1
20CA: EE49           lsriw #7, d1
20CC: D240           addrw d0, d1
20CE: E241           asriw #1, d1
20D0: 3601           movew d1, d3
20D2: 976E FFFE      submw.ex d3, -2(a6)
20D6: 2044           movel d4, a0
20D8: 41E8 0014      lea 20(a0), d0
20DC: BC88           cmpl a0, d6
20DE: 6606           bne 6
20E0: 7A00           moveq #0, d5
20E2: 6000 0080      bra 128
20E6: 2044           movel d4, a0
20E8: 41E8 0014      lea 20(a0), d0
20EC: 2246           movel d6, a1
20EE: 93C8           subal a0, a1
20F0: 2A09           movel a1, d5
20F2: E08D           lsril #0, d5
20F4: E08D           lsril #0, d5
20F6: E08D           lsril #0, d5
20F8: EE8D           lsril #7, d5
20FA: DA89           addrl a1, d5
20FC: E285           asril #1, d5
20FE: 7000           moveq #0, d0
2100: 1005           moveb d5, d0
2102: 5340           subqw #1, d0
2104: D040           addrw d0, d0
2106: 48C0           extl d0
2108: D080           addrl d0, d0
210A: 41ED FCCE      lea -818(a5), d0
210E: 3630 0800      movew.ex 0(a0,d0.l), d3
2112: 3003           movew d3, d0
2114: E048           lsriw #0, d0
2116: EE48           lsriw #7, d0
2118: D043           addrw d3, d0
211A: E240           asriw #1, d0
211C: D043           addrw d3, d0
211E: C1EE FFFC      muls.ex -4(a6), d0
2122: 3200           movew d0, d1
2124: E049           lsriw #0, d1
2126: EE49           lsriw #7, d1
2128: D240           addrw d0, d1
212A: E241           asriw #1, d1
212C: 3601           movew d1, d3
212E: 9E43           subrw d3, d7
2130: 7000           moveq #0, d0
2132: 1005           moveb d5, d0
2134: 5340           subqw #1, d0
2136: D040           addrw d0, d0
2138: 48C0           extl d0
213A: D080           addrl d0, d0
213C: 41ED FCEE      lea -786(a5), d0
2140: 3630 0800      movew.ex 0(a0,d0.l), d3
2144: 3003           movew d3, d0
2146: E048           lsriw #0, d0
2148: EE48           lsriw #7, d0
214A: D043           addrw d3, d0
214C: E240           asriw #1, d0
214E: D043           addrw d3, d0
2150: C1EE FFFA      muls.ex -6(a6), d0
2154: 3200           movew d0, d1
2156: E049           lsriw #0, d1
2158: EE49           lsriw #7, d1
215A: D240           addrw d0, d1
215C: E241           asriw #1, d1
215E: 3601           movew d1, d3
2160: 976E FFFE      submw.ex d3, -2(a6)
2164: 3487           movew d7, (a2)
2166: 356E FFFE 0002 movew.emx -2(a6), 2(a2)
216C: 3007           movew d7, d0
216E: D06E FFFC      addrw.ex -4(a6), d0
2172: 3540 0004      movew.mx d0, 4(a2)
2176: 302E FFFE      movew.ex -2(a6), d0
217A: D06E FFFA      addrw.ex -6(a6), d0
217E: 3540 0006      movew.mx d0, 6(a2)
2182: 156C 0018 0018 moveb.emx 24(a4), 24(a2)
2188: 356C 0008 0008 movew.emx 8(a4), 8(a2)
218E: 356C 000A 000A movew.emx 10(a4), 10(a2)
2194: 1005           moveb d5, d0
2196: 4CDF 1CF8      movem (a7)+, <1cf8>
219A: 4E5E           unlk a6
219C: 4E75           rts
219E: 9753           submw d3, (a3)
21A0: 6869           bvc 105
21A2: 7050           moveq #80, d0
21A4: 6F73           ble 115
21A6: 6974           bvs 116
21A8: 696F           bvs 111
21AA: 6E49           bgt 73
21AC: 6E46           bgt 70
21AE: 6F72           ble 114
21B0: 6D61           blt 97
21B2: 7469           moveq #105, d2
21B4: 6F6E           ble 110
21B6: 0000           dc.w #0
21B8: 4E56 0000      link a6, #0
21BC: 48E7 1F30      movem <1f30>, -(a7)
21C0: 246E 0008      movel.ex 8(a6), a2
21C4: 7000           moveq #0, d0
21C6: 102A 0014      moveb.ex 20(a2), d0
21CA: C1FC 001E      muls.ex #30, d0
21CE: 47ED F414      lea -3052(a5), d3
21D2: D7C0           addal d0, a3
21D4: 47EB 003C      lea 60(a3), d3
21D8: 3A2A 0004      movew.ex 4(a2), d5
21DC: 9A52           subrw (a2), d5
21DE: 3C2A 0006      movew.ex 6(a2), d6
21E2: 9C6A 0002      subrw.ex 2(a2), d6
21E6: 4A2B 0013      tstb.ex 19(a3)
21EA: 6600 00AC      bne 172
21EE: 0C2D 0019 F4CA cmpib.ex #25, -2870(a5)
21F4: 660A           bne 10
21F6: 4A2D F4D2      tstb.ex -2862(a5)
21FA: 6704           beq 4
21FC: 7603           moveq #3, d3
21FE: 6008           bra 8
2200: 7600           moveq #0, d3
2202: 162D F43E      moveb.ex -3010(a5), d3
2206: 5343           subqw #1, d3
2208: 42A7           clrl -(a7)
220A: 4E4F           trap #15
220C: A0C2           sysTrapSysRandom
220E: 3203           movew d3, d1
2210: 5241           addqw #1, d1
2212: C3C0           muls d0, d1
2214: 2001           movel d1, d0
2216: E080           asril #0, d0
2218: EC80           asril #6, d0
221A: E088           lsril #0, d0
221C: E088           lsril #0, d0
221E: E288           lsril #1, d0
2220: D081           addrl d1, d0
2222: E080           asril #0, d0
2224: EE80           asril #7, d0
2226: 3800           movew d0, d4
2228: 3044           movew d4, a0
222A: 2008           movel a0, d0
222C: E588           lslil #2, d0
222E: 3043           movew d3, a0
2230: 2208           movel a0, d1
2232: E989           lslil #4, d1
2234: 41ED FD92      lea -622(a5), d0
2238: D1C1           addal d1, a0
223A: 34B0 0800      movew.ex 0(a0,d0.l), (a2)
223E: 3044           movew d4, a0
2240: 2008           movel a0, d0
2242: E588           lslil #2, d0
2244: 3043           movew d3, a0
2246: 2208           movel a0, d1
2248: E989           lslil #4, d1
224A: 41ED FD92      lea -622(a5), d0
224E: 5488           addql #2, a0
2250: D1C1           addal d1, a0
2252: 3570 0800 0002 movew.emx 0(a0,d0.l), 2(a2)
2258: 3044           movew d4, a0
225A: 2008           movel a0, d0
225C: E588           lslil #2, d0
225E: 3043           movew d3, a0
2260: 2208           movel a0, d1
2262: E989           lslil #4, d1
2264: 41ED FD92      lea -622(a5), d0
2268: D1C1           addal d1, a0
226A: 3205           movew d5, d1
226C: D270 0800      addrw.ex 0(a0,d0.l), d1
2270: 3541 0004      movew.mx d1, 4(a2)
2274: 3044           movew d4, a0
2276: 2008           movel a0, d0
2278: E588           lslil #2, d0
227A: 3043           movew d3, a0
227C: 2208           movel a0, d1
227E: E989           lslil #4, d1
2280: 41ED FD92      lea -622(a5), d0
2284: 5488           addql #2, a0
2286: D1C1           addal d1, a0
2288: 3206           movew d6, d1
228A: D270 0800      addrw.ex 0(a0,d0.l), d1
228E: 3541 0006      movew.mx d1, 6(a2)
2292: 7E00           moveq #0, d7
2294: 584F           addqw #4, a7
2296: 600A           bra 10
2298: 2F0A           movel a2, -(a7)
229A: 4EBA FD2C      jsr.ex -27696(pc)
229E: 1E00           moveb d0, d7
22A0: 584F           addqw #4, a7
22A2: 0452 0100      subiw #256, (a2)
22A6: 046A 0100 0002 subiw.ex #256, 2(a2)
22AC: 066A 0100 0004 addiw.ex #256, 4(a2)
22B2: 066A 0100 0006 addiw.ex #256, 6(a2)
22B8: 1F3C 0001      moveb.ex #1, -(a7)
22BC: 2F0A           movel a2, -(a7)
22BE: 4EBA 1D42      jsr.ex -19446(pc)
22C2: 4A00           tstb d0
22C4: 5C4F           addqw #6, a7
22C6: 673E           beq 62
22C8: 0652 0100      addiw #256, (a2)
22CC: 066A 0100 0002 addiw.ex #256, 2(a2)
22D2: 046A 0100 0004 subiw.ex #256, 4(a2)
22D8: 046A 0100 0006 subiw.ex #256, 6(a2)
22DE: 4227           clrb -(a7)
22E0: 2F0A           movel a2, -(a7)
22E2: 4EBA 1D1E      jsr.ex -19446(pc)
22E6: 7000           moveq #0, d0
22E8: 1007           moveb d7, d0
22EA: D080           addrl d0, d0
22EC: 204B           movel a3, a0
22EE: 41E8 0014      lea 20(a0), d0
22F2: 31AA 0016 0800 movew.emx 22(a2), 0(a0,d0.l)
22F8: 522B 0013      addqb.ex #1, 19(a3)
22FC: 022A 00EF 0020 andib.ex #239, 32(a2)
2302: 5C4F           addqw #6, a7
2304: 600E           bra 14
2306: 4252           clrw (a2)
2308: 426A 0002      clrw.ex 2(a2)
230C: 3545 0004      movew.mx d5, 4(a2)
2310: 3546 0006      movew.mx d6, 6(a2)
2314: 4CDF 0CF8      movem (a7)+, <0cf8>
2318: 4E5E           unlk a6
231A: 4E75           rts
231C: 9053           subrw (a3), d0
231E: 6869           bvc 105
2320: 7041           moveq #65, d0
2322: 7070           moveq #112, d0
2324: 6561           bcs 97
2326: 7253           moveq #83, d1
2328: 6166           bsr 102
232A: 656C           bcs 108
232C: 7900           dc.w #30976
232E: 0000           dc.w #0
2330: 4E56 0000      link a6, #0
2334: 48E7 1020      movem <1020>, -(a7)
2338: 162E 0008      moveb.ex 8(a6), d3
233C: 0C2D 000F F4D0 cmpib.ex #15, -2864(a5)
2342: 6506           bcs 6
2344: 7000           moveq #0, d0
2346: 6000 00F0      bra 240
234A: 102D F4D0      moveb.ex -2864(a5), d0
234E: 522D F4D0      addqb.ex #1, -2864(a5)
2352: 7200           moveq #0, d1
2354: 1200           moveb d0, d1
2356: C3FC 0026      muls.ex #38, d1
235A: 45ED F414      lea -3052(a5), d2
235E: D5C1           addal d1, a2
2360: 45EA 00CA      lea 202(a2), d2
2364: 422A 000E      clrb.ex 14(a2)
2368: 157C 0001 000D moveb.emx #1, 13(a2)
236E: 157C 0001 000C moveb.emx #1, 12(a2)
2374: 002A 0010 0020 orib.ex #16, 32(a2)
237A: 4252           clrw (a2)
237C: 426A 0002      clrw.ex 2(a2)
2380: 357C 00BF 0004 movew.emx #191, 4(a2)
2386: 357C 00BF 0006 movew.emx #191, 6(a2)
238C: 42A7           clrl -(a7)
238E: 4E4F           trap #15
2390: A0C2           sysTrapSysRandom
2392: 48C0           extl d0
2394: E988           lslil #4, d0
2396: 2200           movel d0, d1
2398: E081           asril #0, d1
239A: EC81           asril #6, d1
239C: E089           lsril #0, d1
239E: E089           lsril #0, d1
23A0: E289           lsril #1, d1
23A2: D280           addrl d0, d1
23A4: E081           asril #0, d1
23A6: EE81           asril #7, d1
23A8: 1541 0018      moveb.mx d1, 24(a2)
23AC: 426A 0008      clrw.ex 8(a2)
23B0: 426A 000A      clrw.ex 10(a2)
23B4: 157C 0004 001A moveb.emx #4, 26(a2)
23BA: 422A 001C      clrb.ex 28(a2)
23BE: 022A 00F7 0020 andib.ex #247, 32(a2)
23C4: 022A 007F 0020 andib.ex #127, 32(a2)
23CA: 022A 00BF 0020 andib.ex #191, 32(a2)
23D0: 022A 00DF 0020 andib.ex #223, 32(a2)
23D6: 022A 007F 001E andib.ex #127, 30(a2)
23DC: 022A 00BF 001E andib.ex #191, 30(a2)
23E2: 022A 00DF 001E andib.ex #223, 30(a2)
23E8: 022A 00EF 001E andib.ex #239, 30(a2)
23EE: 022A 00F7 001E andib.ex #247, 30(a2)
23F4: 022A 00FB 001E andib.ex #251, 30(a2)
23FA: 422A 0019      clrb.ex 25(a2)
23FE: 1543 0014      moveb.mx d3, 20(a2)
2402: 4EBA FB0E      jsr.ex -27878(pc)
2406: 3540 0016      movew.mx d0, 22(a2)
240A: 7000           moveq #0, d0
240C: 1003           moveb d3, d0
240E: C1FC 001E      muls.ex #30, d0
2412: 41ED F414      lea -3052(a5), d0
2416: 41E8 004E      lea 78(a0), d0
241A: 5230 0800      addqb.ex #1, 0(a0,d0.l)
241E: 7000           moveq #0, d0
2420: 1003           moveb d3, d0
2422: C1FC 001E      muls.ex #30, d0
2426: 41ED F414      lea -3052(a5), d0
242A: 41E8 0058      lea 88(a0), d0
242E: 11BC 0032 0800 moveb.emx #50, 0(a0,d0.l)
2434: 7001           moveq #1, d0
2436: 584F           addqw #4, a7
2438: 4CDF 0408      movem (a7)+, <0408>
243C: 4E5E           unlk a6
243E: 4E75           rts
2440: 8D53           ormw d6, (a3)
2442: 6869           bvc 105
2444: 7041           moveq #65, d0
2446: 6464           bcc 100
2448: 506C 6179      addqw.ex #0, 24953(a4)
244C: 6572           bcs 114
244E: 0000           dc.w #0
2450: 4E56 0000      link a6, #0
2454: 48E7 0030      movem <0030>, -(a7)
2458: 246E 0008      movel.ex 8(a6), a2
245C: 0C2A 0006 000E cmpib.ex #6, 14(a2)
2462: 6314           bls 20
2464: 486D FECC      pea.ex -308(a5)
2468: 3F3C 0DC9      movew.ex #3529, -(a7)
246C: 486D FDEA      pea.ex -534(a5)
2470: 4E4F           trap #15
2472: A084           sysTrapErrDisplayFileLineMsg
2474: 4FEF 000A      lea 10(a7), d7
2478: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
247E: 6438           bcc 56
2480: 7000           moveq #0, d0
2482: 102A 0014      moveb.ex 20(a2), d0
2486: C1FC 001E      muls.ex #30, d0
248A: 41ED F414      lea -3052(a5), d0
248E: 41E8 004E      lea 78(a0), d0
2492: 5330 0800      subqb.ex #1, 0(a0,d0.l)
2496: 082A 0005 001E btst #5, 30(a2)
249C: 6632           bne 50
249E: 082A 0004 001E btst #4, 30(a2)
24A4: 662A           bne 42
24A6: 082A 0004 0020 btst #4, 32(a2)
24AC: 6622           bne 34
24AE: 2F0A           movel a2, -(a7)
24B0: 4EBA FA86      jsr.ex -27840(pc)
24B4: 584F           addqw #4, a7
24B6: 6018           bra 24
24B8: 532D F4D1      subqb.ex #1, -2863(a5)
24BC: 0C2A 0002 000E cmpib.ex #2, 14(a2)
24C2: 650C           bcs 12
24C4: 0C2A 0003 000E cmpib.ex #3, 14(a2)
24CA: 6204           bhi 4
24CC: 532D F4D2      subqb.ex #1, -2862(a5)
24D0: 7000           moveq #0, d0
24D2: 102D F4D0      moveb.ex -2864(a5), d0
24D6: 5340           subqw #1, d0
24D8: C1FC 0026      muls.ex #38, d0
24DC: 47ED F414      lea -3052(a5), d3
24E0: D7C0           addal d0, a3
24E2: 47EB 00CA      lea 202(a3), d3
24E6: B5CB           cmpal a3, a2
24E8: 6314           bls 20
24EA: 486D FF1E      pea.ex -226(a5)
24EE: 3F3C 0DEA      movew.ex #3562, -(a7)
24F2: 486D FDEA      pea.ex -534(a5)
24F6: 4E4F           trap #15
24F8: A084           sysTrapErrDisplayFileLineMsg
24FA: 4FEF 000A      lea 10(a7), d7
24FE: B5CB           cmpal a3, a2
2500: 6722           beq 34
2502: 204B           movel a3, a0
2504: 91CA           subal a2, a0
2506: 2008           movel a0, d0
2508: 7226           moveq #38, d1
250A: 4EBA DF26      jsr.ex 30778(pc)
250E: 7226           moveq #38, d1
2510: 4EBA DE78      jsr.ex 30610(pc)
2514: 2F00           movel d0, -(a7)
2516: 486A 0026      pea.ex 38(a2)
251A: 2F0A           movel a2, -(a7)
251C: 4E4F           trap #15
251E: A026           sysTrapMemMove
2520: 4FEF 000C      lea 12(a7), d7
2524: 422B 000C      clrb.ex 12(a3)
2528: 422B 000D      clrb.ex 13(a3)
252C: 532D F4D0      subqb.ex #1, -2864(a5)
2530: 4CDF 0C00      movem (a7)+, <0c00>
2534: 4E5E           unlk a6
2536: 4E75           rts
2538: 8A53           orrw (a3), d5
253A: 6869           bvc 105
253C: 7052           moveq #82, d0
253E: 656D           bcs 109
2540: 6F76           ble 118
2542: 6500 0000      bcs 0
2546: 4E56 0000      link a6, #0
254A: 48E7 1020      movem <1020>, -(a7)
254E: 7000           moveq #0, d0
2550: 102D F4D0      moveb.ex -2864(a5), d0
2554: C1FC 0026      muls.ex #38, d0
2558: 45ED F414      lea -3052(a5), d2
255C: D5C0           addal d0, a2
255E: 45EA 00A4      lea 164(a2), d2
2562: 7600           moveq #0, d3
2564: 162D F4D6      moveb.ex -2858(a5), d3
2568: 6034           bra 52
256A: 41ED F414      lea -3052(a5), d0
256E: 41E8 00CA      lea 202(a0), d0
2572: B5C8           cmpal a0, a2
2574: 6414           bcc 20
2576: 486D FF42      pea.ex -190(a5)
257A: 3F3C 0E1B      movew.ex #3611, -(a7)
257E: 486D FDEA      pea.ex -534(a5)
2582: 4E4F           trap #15
2584: A084           sysTrapErrDisplayFileLineMsg
2586: 4FEF 000A      lea 10(a7), d7
258A: 4A2A 000C      tstb.ex 12(a2)
258E: 660A           bne 10
2590: 2F0A           movel a2, -(a7)
2592: 4EBA FEBC      jsr.ex -26536(pc)
2596: 5343           subqw #1, d3
2598: 584F           addqw #4, a7
259A: 7026           moveq #38, d0
259C: 95C0           subal d0, a2
259E: 4A43           tstw d3
25A0: 6EC8           bgt -56
25A2: 422D F4D6      clrb.ex -2858(a5)
25A6: 4CDF 0408      movem (a7)+, <0408>
25AA: 4E5E           unlk a6
25AC: 4E75           rts
25AE: 9253           subrw (a3), d1
25B0: 6869           bvc 105
25B2: 7052           moveq #82, d0
25B4: 656D           bcs 109
25B6: 6F76           ble 118
25B8: 6555           bcs 85
25BA: 6E75           bgt 117
25BC: 7361           dc.w #29537
25BE: 626C           bhi 108
25C0: 6500 0000      bcs 0
25C4: 4E56 0000      link a6, #0
25C8: 2F0A           movel a2, -(a7)
25CA: 246E 0008      movel.ex 8(a6), a2
25CE: 0C2A 0006 000E cmpib.ex #6, 14(a2)
25D4: 6314           bls 20
25D6: 486D FECC      pea.ex -308(a5)
25DA: 3F3C 0E3F      movew.ex #3647, -(a7)
25DE: 486D FDEA      pea.ex -534(a5)
25E2: 4E4F           trap #15
25E4: A084           sysTrapErrDisplayFileLineMsg
25E6: 4FEF 000A      lea 10(a7), d7
25EA: 422A 000C      clrb.ex 12(a2)
25EE: 422A 000D      clrb.ex 13(a2)
25F2: 522D F4D6      addqb.ex #1, -2858(a5)
25F6: 245F           movel (a7)+, a2
25F8: 4E5E           unlk a6
25FA: 4E75           rts
25FC: 9053           subrw (a3), d0
25FE: 6869           bvc 105
2600: 7053           moveq #83, d0
2602: 6574           bcs 116
2604: 4E6F           move usp, a7
2606: 7455           moveq #85, d2
2608: 7361           dc.w #29537
260A: 626C           bhi 108
260C: 6500 0000      bcs 0
2610: 4E56 FFFC      link a6, #-4
2614: 48E7 1F20      movem <1f20>, -(a7)
2618: 246E 0008      movel.ex 8(a6), a2
261C: 0C2A 0002 000E cmpib.ex #2, 14(a2)
2622: 6208           bhi 8
2624: 0C2A 0001 000E cmpib.ex #1, 14(a2)
262A: 6414           bcc 20
262C: 486D FECC      pea.ex -308(a5)
2630: 3F3C 0E66      movew.ex #3686, -(a7)
2634: 486D FDEA      pea.ex -534(a5)
2638: 4E4F           trap #15
263A: A084           sysTrapErrDisplayFileLineMsg
263C: 4FEF 000A      lea 10(a7), d7
2640: 3012           movew (a2), d0
2642: D06A 0004      addrw.ex 4(a2), d0
2646: 3C00           movew d0, d6
2648: E04E           lsriw #0, d6
264A: EE4E           lsriw #7, d6
264C: DC40           addrw d0, d6
264E: E246           asriw #1, d6
2650: 302A 0002      movew.ex 2(a2), d0
2654: D06A 0006      addrw.ex 6(a2), d0
2658: 3E00           movew d0, d7
265A: E04F           lsriw #0, d7
265C: EE4F           lsriw #7, d7
265E: DE40           addrw d0, d7
2660: E247           asriw #1, d7
2662: 780C           moveq #12, d4
2664: 363C 0230      movew.ex #560, d3
2668: 6000 00BE      bra 190
266C: 42A7           clrl -(a7)
266E: 4E4F           trap #15
2670: A0C2           sysTrapSysRandom
2672: 48C0           extl d0
2674: E988           lslil #4, d0
2676: 2200           movel d0, d1
2678: E081           asril #0, d1
267A: EC81           asril #6, d1
267C: E089           lsril #0, d1
267E: E089           lsril #0, d1
2680: E289           lsril #1, d1
2682: D280           addrl d0, d1
2684: E081           asril #0, d1
2686: EE81           asril #7, d1
2688: 3A01           movew d1, d5
268A: 3045           movew d5, a0
268C: 2008           movel a0, d0
268E: D080           addrl d0, d0
2690: 41ED FCCE      lea -818(a5), d0
2694: 3030 0800      movew.ex 0(a0,d0.l), d0
2698: E548           lsliw #2, d0
269A: 3D40 FFFE      movew.mx d0, -2(a6)
269E: 3045           movew d5, a0
26A0: 2008           movel a0, d0
26A2: D080           addrl d0, d0
26A4: 41ED FCEE      lea -786(a5), d0
26A8: 3030 0800      movew.ex 0(a0,d0.l), d0
26AC: E548           lsliw #2, d0
26AE: 3D40 FFFC      movew.mx d0, -4(a6)
26B2: 3F2E FFFC      movew.ex -4(a6), -(a7)
26B6: 3F2E FFFE      movew.ex -2(a6), -(a7)
26BA: 42A7           clrl -(a7)
26BC: 4E4F           trap #15
26BE: A0C2           sysTrapSysRandom
26C0: C1C3           muls d3, d0
26C2: 2200           movel d0, d1
26C4: E081           asril #0, d1
26C6: EC81           asril #6, d1
26C8: E089           lsril #0, d1
26CA: E089           lsril #0, d1
26CC: E289           lsril #1, d1
26CE: D280           addrl d0, d1
26D0: E081           asril #0, d1
26D2: EE81           asril #7, d1
26D4: 3003           movew d3, d0
26D6: E048           lsriw #0, d0
26D8: EE48           lsriw #7, d0
26DA: D043           addrw d3, d0
26DC: E240           asriw #1, d0
26DE: 48C0           extl d0
26E0: 9280           subrl d0, d1
26E2: 3047           movew d7, a0
26E4: D288           addrl a0, d1
26E6: 584F           addqw #4, a7
26E8: 3F01           movew d1, -(a7)
26EA: 42A7           clrl -(a7)
26EC: 4E4F           trap #15
26EE: A0C2           sysTrapSysRandom
26F0: C1C3           muls d3, d0
26F2: 2200           movel d0, d1
26F4: E081           asril #0, d1
26F6: EC81           asril #6, d1
26F8: E089           lsril #0, d1
26FA: E089           lsril #0, d1
26FC: E289           lsril #1, d1
26FE: D280           addrl d0, d1
2700: E081           asril #0, d1
2702: EE81           asril #7, d1
2704: 3003           movew d3, d0
2706: E048           lsriw #0, d0
2708: EE48           lsriw #7, d0
270A: D043           addrw d3, d0
270C: E240           asriw #1, d0
270E: 48C0           extl d0
2710: 9280           subrl d0, d1
2712: 3046           movew d6, a0
2714: D288           addrl a0, d1
2716: 584F           addqw #4, a7
2718: 3F01           movew d1, -(a7)
271A: 1F3C 000B      moveb.ex #11, -(a7)
271E: 4EBA F03A      jsr.ex -29854(pc)
2722: 5344           subqw #1, d4
2724: 4FEF 000E      lea 14(a7), d7
2728: 4A44           tstw d4
272A: 6E00 FF40      bgt 65344
272E: 422D F43D      clrb.ex -3011(a5)
2732: 4CDF 04F8      movem (a7)+, <04f8>
2736: 4E5E           unlk a6
2738: 4E75           rts
273A: 8D50           ormw d6, (a0)
273C: 6C61           bge 97
273E: 6E65           bgt 101
2740: 7445           moveq #69, d2
2742: 7870           moveq #112, d4
2744: 6C6F           bge 111
2746: 6465           bcc 101
2748: 0000           dc.w #0
274A: 4E56 0000      link a6, #0
274E: 48E7 1F30      movem <1f30>, -(a7)
2752: 246E 0008      movel.ex 8(a6), a2
2756: 266E 000C      movel.ex 12(a6), a3
275A: 0C2A 0006 000E cmpib.ex #6, 14(a2)
2760: 6314           bls 20
2762: 486D FECC      pea.ex -308(a5)
2766: 3F3C 0EA4      movew.ex #3748, -(a7)
276A: 486D FDEA      pea.ex -534(a5)
276E: 4E4F           trap #15
2770: A084           sysTrapErrDisplayFileLineMsg
2772: 4FEF 000A      lea 10(a7), d7
2776: 0C2A 0002 000E cmpib.ex #2, 14(a2)
277C: 621C           bhi 28
277E: 0C2A 0001 000E cmpib.ex #1, 14(a2)
2784: 6514           bcs 20
2786: 2F0A           movel a2, -(a7)
2788: 4EBA FE86      jsr.ex -26088(pc)
278C: 7000           moveq #0, d0
278E: 102D F4DB      moveb.ex -2853(a5), d0
2792: 7828           moveq #40, d4
2794: 9840           subrw d0, d4
2796: 584F           addqw #4, a7
2798: 6024           bra 36
279A: 42A7           clrl -(a7)
279C: 4E4F           trap #15
279E: A0C2           sysTrapSysRandom
27A0: C1FC 000A      muls.ex #10, d0
27A4: 2200           movel d0, d1
27A6: E081           asril #0, d1
27A8: EC81           asril #6, d1
27AA: E089           lsril #0, d1
27AC: E089           lsril #0, d1
27AE: E289           lsril #1, d1
27B0: D280           addrl d0, d1
27B2: E081           asril #0, d1
27B4: EE81           asril #7, d1
27B6: 700A           moveq #10, d0
27B8: D280           addrl d0, d1
27BA: 3801           movew d1, d4
27BC: 584F           addqw #4, a7
27BE: 362A 0004      movew.ex 4(a2), d3
27C2: 9652           subrw (a2), d3
27C4: E54B           lsliw #2, d3
27C6: 3012           movew (a2), d0
27C8: D06A 0004      addrw.ex 4(a2), d0
27CC: 3C00           movew d0, d6
27CE: E04E           lsriw #0, d6
27D0: EE4E           lsriw #7, d6
27D2: DC40           addrw d0, d6
27D4: E246           asriw #1, d6
27D6: 302A 0002      movew.ex 2(a2), d0
27DA: D06A 0006      addrw.ex 6(a2), d0
27DE: 3E00           movew d0, d7
27E0: E04F           lsriw #0, d7
27E2: EE4F           lsriw #7, d7
27E4: DE40           addrw d0, d7
27E6: E247           asriw #1, d7
27E8: B5CB           cmpal a3, a2
27EA: 6700 00DC      beq 220
27EE: 302A 0008      movew.ex 8(a2), d0
27F2: D06B 0008      addrw.ex 8(a3), d0
27F6: 3200           movew d0, d1
27F8: E049           lsriw #0, d1
27FA: EE49           lsriw #7, d1
27FC: D240           addrw d0, d1
27FE: E241           asriw #1, d1
2800: 3541 0008      movew.mx d1, 8(a2)
2804: 302A 000A      movew.ex 10(a2), d0
2808: D06B 000A      addrw.ex 10(a3), d0
280C: 3200           movew d0, d1
280E: E049           lsriw #0, d1
2810: EE49           lsriw #7, d1
2812: D240           addrw d0, d1
2814: E241           asriw #1, d1
2816: 3541 000A      movew.mx d1, 10(a2)
281A: 6000 00AC      bra 172
281E: 42A7           clrl -(a7)
2820: 4E4F           trap #15
2822: A0C2           sysTrapSysRandom
2824: 48C0           extl d0
2826: E788           lslil #3, d0
2828: 2200           movel d0, d1
282A: E081           asril #0, d1
282C: EC81           asril #6, d1
282E: E089           lsril #0, d1
2830: E089           lsril #0, d1
2832: E289           lsril #1, d1
2834: D280           addrl d0, d1
2836: E081           asril #0, d1
2838: EE81           asril #7, d1
283A: 7010           moveq #16, d0
283C: D280           addrl d0, d1
283E: 3A01           movew d1, d5
2840: 1F05           moveb d5, -(a7)
2842: 42A7           clrl -(a7)
2844: 4E4F           trap #15
2846: A0C2           sysTrapSysRandom
2848: C1C3           muls d3, d0
284A: 2200           movel d0, d1
284C: E081           asril #0, d1
284E: EC81           asril #6, d1
2850: E089           lsril #0, d1
2852: E089           lsril #0, d1
2854: E289           lsril #1, d1
2856: D280           addrl d0, d1
2858: E081           asril #0, d1
285A: EE81           asril #7, d1
285C: 3003           movew d3, d0
285E: E048           lsriw #0, d0
2860: EE48           lsriw #7, d0
2862: D043           addrw d3, d0
2864: E240           asriw #1, d0
2866: 48C0           extl d0
2868: 9280           subrl d0, d1
286A: 3045           movew d5, a0
286C: 2001           movel d1, d0
286E: 2208           movel a0, d1
2870: 4EBA DBC0      jsr.ex 30778(pc)
2874: 306A 000A      movew.ex 10(a2), a0
2878: D088           addrl a0, d0
287A: 584F           addqw #4, a7
287C: 3F00           movew d0, -(a7)
287E: 42A7           clrl -(a7)
2880: 4E4F           trap #15
2882: A0C2           sysTrapSysRandom
2884: C1C3           muls d3, d0
2886: 2200           movel d0, d1
2888: E081           asril #0, d1
288A: EC81           asril #6, d1
288C: E089           lsril #0, d1
288E: E089           lsril #0, d1
2890: E289           lsril #1, d1
2892: D280           addrl d0, d1
2894: E081           asril #0, d1
2896: EE81           asril #7, d1
2898: 3003           movew d3, d0
289A: E048           lsriw #0, d0
289C: EE48           lsriw #7, d0
289E: D043           addrw d3, d0
28A0: E240           asriw #1, d0
28A2: 48C0           extl d0
28A4: 9280           subrl d0, d1
28A6: 3045           movew d5, a0
28A8: 2001           movel d1, d0
28AA: 2208           movel a0, d1
28AC: 4EBA DB84      jsr.ex 30778(pc)
28B0: 306A 0008      movew.ex 8(a2), a0
28B4: D088           addrl a0, d0
28B6: 584F           addqw #4, a7
28B8: 3F00           movew d0, -(a7)
28BA: 3F07           movew d7, -(a7)
28BC: 3F06           movew d6, -(a7)
28BE: 4EBA E6EE      jsr.ex -31818(pc)
28C2: 5344           subqw #1, d4
28C4: 4FEF 000E      lea 14(a7), d7
28C8: 4A44           tstw d4
28CA: 6E00 FF52      bgt 65362
28CE: 2F0A           movel a2, -(a7)
28D0: 4EBA FCF2      jsr.ex -26164(pc)
28D4: 1F3C 0007      moveb.ex #7, -(a7)
28D8: 4EBA E1F2      jsr.ex 32468(pc)
28DC: 5C4F           addqw #6, a7
28DE: 4CDF 0CF8      movem (a7)+, <0cf8>
28E2: 4E5E           unlk a6
28E4: 4E75           rts
28E6: 8B53           ormw d5, (a3)
28E8: 6869           bvc 105
28EA: 7045           moveq #69, d0
28EC: 7870           moveq #112, d4
28EE: 6C6F           bge 111
28F0: 6465           bcc 101
28F2: 0000           dc.w #0
28F4: 4E56 0000      link a6, #0
28F8: 48E7 1830      movem <1830>, -(a7)
28FC: 246E 0008      movel.ex 8(a6), a2
2900: 266E 000C      movel.ex 12(a6), a3
2904: 7600           moveq #0, d3
2906: 7000           moveq #0, d0
2908: 102B 000E      moveb.ex 14(a3), d0
290C: 0440 000C      subiw #12, d0
2910: 0C40 0005      cmpiw #5, d0
2914: 627A           bhi 122
2916: D040           addrw d0, d0
2918: 303B 0006      movew.ex 6(pc,d0.w), d0
291C: 4EFB 0002      jmp.ex 2(pc,d0.w)
2920: 000C           dc.w #12
2922: 0012 0026      orib #38, (a2)
2926: 003A           dc.w #58
2928: 003E           dc.w #62
292A: 0064 542A      oriw #21546, -(a4)
292E: 001A           dc.w #26
2930: 605E           bra 94
2932: 082A 0003 001E btst #3, 30(a2)
2938: 6704           beq 4
293A: 7602           moveq #2, d3
293C: 6052           bra 82
293E: 002A 0008 001E orib.ex #8, 30(a2)
2944: 604A           bra 74
2946: 082A 0002 001E btst #2, 30(a2)
294C: 6704           beq 4
294E: 7602           moveq #2, d3
2950: 603E           bra 62
2952: 002A 0004 001E orib.ex #4, 30(a2)
2958: 6036           bra 54
295A: 7601           moveq #1, d3
295C: 6032           bra 50
295E: 522A 001C      addqb.ex #1, 28(a2)
2962: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
2968: 6426           bcc 38
296A: 7000           moveq #0, d0
296C: 102A 0014      moveb.ex 20(a2), d0
2970: C1FC 001E      muls.ex #30, d0
2974: 41ED F414      lea -3052(a5), d0
2978: 41E8 004C      lea 76(a0), d0
297C: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
2982: 600C           bra 12
2984: 1B7C 0001 F43F moveb.emx #1, -3009(a5)
298A: 1B6A 0014 F440 moveb.emx 20(a2), -3008(a5)
2990: 4A03           tstb d3
2992: 676C           beq 108
2994: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
299A: 6464           bcc 100
299C: 42A7           clrl -(a7)
299E: 4E4F           trap #15
29A0: A0C2           sysTrapSysRandom
29A2: C1FC 0006      muls.ex #6, d0
29A6: 2200           movel d0, d1
29A8: E081           asril #0, d1
29AA: EC81           asril #6, d1
29AC: E089           lsril #0, d1
29AE: E089           lsril #0, d1
29B0: E289           lsril #1, d1
29B2: D280           addrl d0, d1
29B4: E081           asril #0, d1
29B6: EE81           asril #7, d1
29B8: 7019           moveq #25, d0
29BA: 4EBA D9CE      jsr.ex 30610(pc)
29BE: 7264           moveq #100, d1
29C0: D081           addrl d1, d0
29C2: 3800           movew d0, d4
29C4: 3F04           movew d4, -(a7)
29C6: 302B 0002      movew.ex 2(a3), d0
29CA: D06B 0006      addrw.ex 6(a3), d0
29CE: 3200           movew d0, d1
29D0: E049           lsriw #0, d1
29D2: EE49           lsriw #7, d1
29D4: D240           addrw d0, d1
29D6: E241           asriw #1, d1
29D8: 3F01           movew d1, -(a7)
29DA: 3013           movew (a3), d0
29DC: D06B 0004      addrw.ex 4(a3), d0
29E0: 3200           movew d0, d1
29E2: E049           lsriw #0, d1
29E4: EE49           lsriw #7, d1
29E6: D240           addrw d0, d1
29E8: E241           asriw #1, d1
29EA: 3F01           movew d1, -(a7)
29EC: 4EBA E970      jsr.ex -30874(pc)
29F0: 1F2A 0014      moveb.ex 20(a2), -(a7)
29F4: 3044           movew d4, a0
29F6: 2F08           movel a0, -(a7)
29F8: 4EBA E122      jsr.ex 32548(pc)
29FC: 4FEF 0010      lea 16(a7), d7
2A00: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
2A06: 640A           bcc 10
2A08: 1F3C 0008      moveb.ex #8, -(a7)
2A0C: 4EBA E0BE      jsr.ex 32468(pc)
2A10: 544F           addqw #2, a7
2A12: 4CDF 0C18      movem (a7)+, <0c18>
2A16: 4E5E           unlk a6
2A18: 4E75           rts
2A1A: 8E53           orrw (a3), d7
2A1C: 6869           bvc 105
2A1E: 7041           moveq #65, d0
2A20: 7761           dc.w #30561
2A22: 7264           moveq #100, d1
2A24: 426F 6E75      clrw.ex 28277(a7)
2A28: 7300           dc.w #29440
2A2A: 0000           dc.w #0
2A2C: 4E56 0000      link a6, #0
2A30: 48E7 1E30      movem <1e30>, -(a7)
2A34: 246E 0008      movel.ex 8(a6), a2
2A38: 266E 000C      movel.ex 12(a6), a3
2A3C: 0C2A 0007 000E cmpib.ex #7, 14(a2)
2A42: 6508           bcs 8
2A44: 0C2A 0008 000E cmpib.ex #8, 14(a2)
2A4A: 6314           bls 20
2A4C: 486D FECC      pea.ex -308(a5)
2A50: 3F3C 0F42      movew.ex #3906, -(a7)
2A54: 486D FDEA      pea.ex -534(a5)
2A58: 4E4F           trap #15
2A5A: A084           sysTrapErrDisplayFileLineMsg
2A5C: 4FEF 000A      lea 10(a7), d7
2A60: 0C2B 0006 000E cmpib.ex #6, 14(a3)
2A66: 6314           bls 20
2A68: 486D FECC      pea.ex -308(a5)
2A6C: 3F3C 0F43      movew.ex #3907, -(a7)
2A70: 486D FDEA      pea.ex -534(a5)
2A74: 4E4F           trap #15
2A76: A084           sysTrapErrDisplayFileLineMsg
2A78: 4FEF 000A      lea 10(a7), d7
2A7C: 0C2A 0008 000E cmpib.ex #8, 14(a2)
2A82: 6606           bne 6
2A84: 7A50           moveq #80, d5
2A86: 7C50           moveq #80, d6
2A88: 6004           bra 4
2A8A: 7A20           moveq #32, d5
2A8C: 7C20           moveq #32, d6
2A8E: 4A6A 0008      tstw.ex 8(a2)
2A92: 6726           beq 38
2A94: 362B 0004      movew.ex 4(a3), d3
2A98: 9653           subrw (a3), d3
2A9A: 3005           movew d5, d0
2A9C: D040           addrw d0, d0
2A9E: 3203           movew d3, d1
2AA0: E049           lsriw #0, d1
2AA2: EE49           lsriw #7, d1
2AA4: D243           addrw d3, d1
2AA6: E241           asriw #1, d1
2AA8: D240           addrw d0, d1
2AAA: 48C1           extl d1
2AAC: 83EA 0008      divs.ex 8(a2), d1
2AB0: 3601           movew d1, d3
2AB2: 4A43           tstw d3
2AB4: 6C06           bge 6
2AB6: 4443           negw d3
2AB8: 6002           bra 2
2ABA: 7600           moveq #0, d3
2ABC: 4A6A 000A      tstw.ex 10(a2)
2AC0: 6728           beq 40
2AC2: 382B 0006      movew.ex 6(a3), d4
2AC6: 986B 0002      subrw.ex 2(a3), d4
2ACA: 3006           movew d6, d0
2ACC: D040           addrw d0, d0
2ACE: 3204           movew d4, d1
2AD0: E049           lsriw #0, d1
2AD2: EE49           lsriw #7, d1
2AD4: D244           addrw d4, d1
2AD6: E241           asriw #1, d1
2AD8: D240           addrw d0, d1
2ADA: 48C1           extl d1
2ADC: 83EA 000A      divs.ex 10(a2), d1
2AE0: 3801           movew d1, d4
2AE2: 4A44           tstw d4
2AE4: 6C06           bge 6
2AE6: 4444           negw d4
2AE8: 6002           bra 2
2AEA: 7800           moveq #0, d4
2AEC: B644           cmpw d4, d3
2AEE: 6C04           bge 4
2AF0: 4A43           tstw d3
2AF2: 6604           bne 4
2AF4: 4A44           tstw d4
2AF6: 6614           bne 20
2AF8: 302A 0008      movew.ex 8(a2), d0
2AFC: C1C3           muls d3, d0
2AFE: D152           addmw d0, (a2)
2B00: 302A 000A      movew.ex 10(a2), d0
2B04: C1C3           muls d3, d0
2B06: D16A 0002      addmw.ex d0, 2(a2)
2B0A: 6012           bra 18
2B0C: 302A 0008      movew.ex 8(a2), d0
2B10: C1C4           muls d4, d0
2B12: D152           addmw d0, (a2)
2B14: 302A 000A      movew.ex 10(a2), d0
2B18: C1C4           muls d4, d0
2B1A: D16A 0002      addmw.ex d0, 2(a2)
2B1E: 3012           movew (a2), d0
2B20: D045           addrw d5, d0
2B22: 3540 0004      movew.mx d0, 4(a2)
2B26: 302A 0002      movew.ex 2(a2), d0
2B2A: D046           addrw d6, d0
2B2C: 3540 0006      movew.mx d0, 6(a2)
2B30: 4CDF 0C78      movem (a7)+, <0c78>
2B34: 4E5E           unlk a6
2B36: 4E75           rts
2B38: 8953           ormw d4, (a3)
2B3A: 686F           bvc 111
2B3C: 7450           moveq #80, d2
2B3E: 6C61           bge 97
2B40: 6365           bls 101
2B42: 0000           dc.w #0
2B44: 4E56 FFF0      link a6, #-16
2B48: 48E7 1F38      movem <1f38>, -(a7)
2B4C: 266E 0008      movel.ex 8(a6), a3
2B50: 246E 000C      movel.ex 12(a6), a2
2B54: 286E 0014      movel.ex 20(a6), a4
2B58: 200A           movel a2, d0
2B5A: 6614           bne 20
2B5C: 486D FECC      pea.ex -308(a5)
2B60: 3F3C 0F9C      movew.ex #3996, -(a7)
2B64: 486D FDEA      pea.ex -534(a5)
2B68: 4E4F           trap #15
2B6A: A084           sysTrapErrDisplayFileLineMsg
2B6C: 4FEF 000A      lea 10(a7), d7
2B70: 3013           movew (a3), d0
2B72: D06B 0004      addrw.ex 4(a3), d0
2B76: 3200           movew d0, d1
2B78: E049           lsriw #0, d1
2B7A: EE49           lsriw #7, d1
2B7C: D240           addrw d0, d1
2B7E: E241           asriw #1, d1
2B80: 3012           movew (a2), d0
2B82: D06A 0004      addrw.ex 4(a2), d0
2B86: 3600           movew d0, d3
2B88: E04B           lsriw #0, d3
2B8A: EE4B           lsriw #7, d3
2B8C: D640           addrw d0, d3
2B8E: E243           asriw #1, d3
2B90: 9641           subrw d1, d3
2B92: 4A43           tstw d3
2B94: 5CC0           sge d0
2B96: 4400           negb d0
2B98: 4880           extw d0
2B9A: 6706           beq 6
2B9C: 3D43 FFFC      movew.mx d3, -4(a6)
2BA0: 6008           bra 8
2BA2: 3003           movew d3, d0
2BA4: 4440           negw d0
2BA6: 3D40 FFFC      movew.mx d0, -4(a6)
2BAA: 3A2E FFFC      movew.ex -4(a6), d5
2BAE: 0C45 0580      cmpiw #1408, d5
2BB2: 6F14           ble 20
2BB4: 303C 0B00      movew.ex #2816, d0
2BB8: 9045           subrw d5, d0
2BBA: 3A00           movew d0, d5
2BBC: 4A43           tstw d3
2BBE: 6D06           blt 6
2BC0: 3605           movew d5, d3
2BC2: 4443           negw d3
2BC4: 6002           bra 2
2BC6: 3605           movew d5, d3
2BC8: 302B 0002      movew.ex 2(a3), d0
2BCC: D06B 0006      addrw.ex 6(a3), d0
2BD0: 3200           movew d0, d1
2BD2: E049           lsriw #0, d1
2BD4: EE49           lsriw #7, d1
2BD6: D240           addrw d0, d1
2BD8: E241           asriw #1, d1
2BDA: 302A 0002      movew.ex 2(a2), d0
2BDE: D06A 0006      addrw.ex 6(a2), d0
2BE2: 3800           movew d0, d4
2BE4: E04C           lsriw #0, d4
2BE6: EE4C           lsriw #7, d4
2BE8: D840           addrw d0, d4
2BEA: E244           asriw #1, d4
2BEC: 9841           subrw d1, d4
2BEE: 4A44           tstw d4
2BF0: 5CC0           sge d0
2BF2: 4400           negb d0
2BF4: 4880           extw d0
2BF6: 6706           beq 6
2BF8: 3D44 FFFA      movew.mx d4, -6(a6)
2BFC: 6008           bra 8
2BFE: 3004           movew d4, d0
2C00: 4440           negw d0
2C02: 3D40 FFFA      movew.mx d0, -6(a6)
2C06: 3C2E FFFA      movew.ex -6(a6), d6
2C0A: 0C46 0580      cmpiw #1408, d6
2C0E: 6F14           ble 20
2C10: 303C 0B00      movew.ex #2816, d0
2C14: 9046           subrw d6, d0
2C16: 3C00           movew d0, d6
2C18: 4A44           tstw d4
2C1A: 6D06           blt 6
2C1C: 3806           movew d6, d4
2C1E: 4444           negw d4
2C20: 6002           bra 2
2C22: 3806           movew d6, d4
2C24: BA46           cmpw d6, d5
2C26: 5EC0           sgt d0
2C28: 4400           negb d0
2C2A: 4880           extw d0
2C2C: 6706           beq 6
2C2E: 3D45 FFF8      movew.mx d5, -8(a6)
2C32: 6004           bra 4
2C34: 3D46 FFF8      movew.mx d6, -8(a6)
2C38: 3E2E FFF8      movew.ex -8(a6), d7
2C3C: 48C7           extl d7
2C3E: 8FFC 0004      divs.ex #4, d7
2C42: 42A7           clrl -(a7)
2C44: 4E4F           trap #15
2C46: A0C2           sysTrapSysRandom
2C48: C1C7           muls d7, d0
2C4A: 2200           movel d0, d1
2C4C: E081           asril #0, d1
2C4E: EC81           asril #6, d1
2C50: E089           lsril #0, d1
2C52: E089           lsril #0, d1
2C54: E289           lsril #1, d1
2C56: D280           addrl d0, d1
2C58: E081           asril #0, d1
2C5A: EE81           asril #7, d1
2C5C: 3007           movew d7, d0
2C5E: E048           lsriw #0, d0
2C60: EE48           lsriw #7, d0
2C62: D047           addrw d7, d0
2C64: E240           asriw #1, d0
2C66: 48C0           extl d0
2C68: 9280           subrl d0, d1
2C6A: D641           addrw d1, d3
2C6C: 42A7           clrl -(a7)
2C6E: 4E4F           trap #15
2C70: A0C2           sysTrapSysRandom
2C72: C1C7           muls d7, d0
2C74: 2200           movel d0, d1
2C76: E081           asril #0, d1
2C78: EC81           asril #6, d1
2C7A: E089           lsril #0, d1
2C7C: E089           lsril #0, d1
2C7E: E289           lsril #1, d1
2C80: D280           addrl d0, d1
2C82: E081           asril #0, d1
2C84: EE81           asril #7, d1
2C86: 3007           movew d7, d0
2C88: E048           lsriw #0, d0
2C8A: EE48           lsriw #7, d0
2C8C: D047           addrw d7, d0
2C8E: E240           asriw #1, d0
2C90: 48C0           extl d0
2C92: 9280           subrl d0, d1
2C94: D841           addrw d1, d4
2C96: 42A7           clrl -(a7)
2C98: 4E4F           trap #15
2C9A: A0C2           sysTrapSysRandom
2C9C: C1FC 0064      muls.ex #100, d0
2CA0: 2200           movel d0, d1
2CA2: E081           asril #0, d1
2CA4: EC81           asril #6, d1
2CA6: E089           lsril #0, d1
2CA8: E089           lsril #0, d1
2CAA: E289           lsril #1, d1
2CAC: D280           addrl d0, d1
2CAE: E081           asril #0, d1
2CB0: EE81           asril #7, d1
2CB2: 7014           moveq #20, d0
2CB4: B280           cmpl d0, d1
2CB6: 4FEF 000C      lea 12(a7), d7
2CBA: 6C4A           bge 74
2CBC: 42A7           clrl -(a7)
2CBE: 4E4F           trap #15
2CC0: A0C2           sysTrapSysRandom
2CC2: C1FC 0240      muls.ex #576, d0
2CC6: 2200           movel d0, d1
2CC8: E081           asril #0, d1
2CCA: EC81           asril #6, d1
2CCC: E089           lsril #0, d1
2CCE: E089           lsril #0, d1
2CD0: E289           lsril #1, d1
2CD2: D280           addrl d0, d1
2CD4: E081           asril #0, d1
2CD6: EE81           asril #7, d1
2CD8: 0681 FFFF FEE0 addil #-288, d1
2CDE: D641           addrw d1, d3
2CE0: 42A7           clrl -(a7)
2CE2: 4E4F           trap #15
2CE4: A0C2           sysTrapSysRandom
2CE6: C1FC 0240      muls.ex #576, d0
2CEA: 2200           movel d0, d1
2CEC: E081           asril #0, d1
2CEE: EC81           asril #6, d1
2CF0: E089           lsril #0, d1
2CF2: E089           lsril #0, d1
2CF4: E289           lsril #1, d1
2CF6: D280           addrl d0, d1
2CF8: E081           asril #0, d1
2CFA: EE81           asril #7, d1
2CFC: 0681 FFFF FEE0 addil #-288, d1
2D02: D841           addrw d1, d4
2D04: 504F           addqw #0, a7
2D06: 4A44           tstw d4
2D08: 5CC0           sge d0
2D0A: 4400           negb d0
2D0C: 4880           extw d0
2D0E: 6706           beq 6
2D10: 3D44 FFF6      movew.mx d4, -10(a6)
2D14: 6008           bra 8
2D16: 3004           movew d4, d0
2D18: 4440           negw d0
2D1A: 3D40 FFF6      movew.mx d0, -10(a6)
2D1E: 4A43           tstw d3
2D20: 5CC0           sge d0
2D22: 4400           negb d0
2D24: 4880           extw d0
2D26: 6706           beq 6
2D28: 3D43 FFF4      movew.mx d3, -12(a6)
2D2C: 6008           bra 8
2D2E: 3003           movew d3, d0
2D30: 4440           negw d0
2D32: 3D40 FFF4      movew.mx d0, -12(a6)
2D36: 302E FFF4      movew.ex -12(a6), d0
2D3A: B06E FFF6      cmpw.ex -10(a6), d0
2D3E: 6D28           blt 40
2D40: 4A43           tstw d3
2D42: 5CC0           sge d0
2D44: 4400           negb d0
2D46: 4880           extw d0
2D48: 6706           beq 6
2D4A: 3D43 FFF2      movew.mx d3, -14(a6)
2D4E: 6008           bra 8
2D50: 3003           movew d3, d0
2D52: 4440           negw d0
2D54: 3D40 FFF2      movew.mx d0, -14(a6)
2D58: 306E FFF2      movew.ex -14(a6), a0
2D5C: 2008           movel a0, d0
2D5E: 81FC 0018      divs.ex #24, d0
2D62: 3D40 FFFE      movew.mx d0, -2(a6)
2D66: 6026           bra 38
2D68: 4A44           tstw d4
2D6A: 5CC0           sge d0
2D6C: 4400           negb d0
2D6E: 4880           extw d0
2D70: 6706           beq 6
2D72: 3D44 FFF0      movew.mx d4, -16(a6)
2D76: 6008           bra 8
2D78: 3004           movew d4, d0
2D7A: 4440           negw d0
2D7C: 3D40 FFF0      movew.mx d0, -16(a6)
2D80: 306E FFF0      movew.ex -16(a6), a0
2D84: 2008           movel a0, d0
2D86: 81FC 0018      divs.ex #24, d0
2D8A: 3D40 FFFE      movew.mx d0, -2(a6)
2D8E: 3043           movew d3, a0
2D90: 2008           movel a0, d0
2D92: 81EE FFFE      divs.ex -2(a6), d0
2D96: 3600           movew d0, d3
2D98: 3044           movew d4, a0
2D9A: 2008           movel a0, d0
2D9C: 81EE FFFE      divs.ex -2(a6), d0
2DA0: 3800           movew d0, d4
2DA2: 3003           movew d3, d0
2DA4: E240           asriw #1, d0
2DA6: E048           lsriw #0, d0
2DA8: EC48           lsriw #6, d0
2DAA: D043           addrw d3, d0
2DAC: E440           asriw #2, d0
2DAE: 3203           movew d3, d1
2DB0: 9240           subrw d0, d1
2DB2: 206E 0010      movel.ex 16(a6), a0
2DB6: 3081           movew d1, (a0)
2DB8: 3004           movew d4, d0
2DBA: E240           asriw #1, d0
2DBC: E048           lsriw #0, d0
2DBE: EC48           lsriw #6, d0
2DC0: D044           addrw d4, d0
2DC2: E440           asriw #2, d0
2DC4: 3204           movew d4, d1
2DC6: 9240           subrw d0, d1
2DC8: 3881           movew d1, (a4)
2DCA: 4CDF 1CF8      movem (a7)+, <1cf8>
2DCE: 4E5E           unlk a6
2DD0: 4E75           rts
2DD2: 9553           submw d2, (a3)
2DD4: 686F           bvc 111
2DD6: 7448           moveq #72, d2
2DD8: 6561           bcs 97
2DDA: 6454           bcc 84
2DDC: 6F77           ble 119
2DDE: 6172           bsr 114
2DE0: 6473           bcc 115
2DE2: 4F62           dc.w #20322
2DE4: 6A65           bpl 101
2DE6: 6374           bls 116
2DE8: 0000           dc.w #0
2DEA: 4E56 0000      link a6, #0
2DEE: 48E7 1F30      movem <1f30>, -(a7)
2DF2: 1E2E 0008      moveb.ex 8(a6), d7
2DF6: 266E 000A      movel.ex 10(a6), a3
2DFA: 0C2B 0006 000E cmpib.ex #6, 14(a3)
2E00: 6314           bls 20
2E02: 486D FECC      pea.ex -308(a5)
2E06: 3F3C 1000      movew.ex #4096, -(a7)
2E0A: 486D FDEA      pea.ex -534(a5)
2E0E: 4E4F           trap #15
2E10: A084           sysTrapErrDisplayFileLineMsg
2E12: 4FEF 000A      lea 10(a7), d7
2E16: 0C2D 000C F4CE cmpib.ex #12, -2866(a5)
2E1C: 6506           bcs 6
2E1E: 7000           moveq #0, d0
2E20: 6000 00C4      bra 196
2E24: 4A2B 001A      tstb.ex 26(a3)
2E28: 6606           bne 6
2E2A: 7000           moveq #0, d0
2E2C: 6000 00B8      bra 184
2E30: 532B 001A      subqb.ex #1, 26(a3)
2E34: 102D F4CE      moveb.ex -2866(a5), d0
2E38: 522D F4CE      addqb.ex #1, -2866(a5)
2E3C: 7800           moveq #0, d4
2E3E: 1800           moveb d0, d4
2E40: 701A           moveq #26, d0
2E42: C1C4           muls d4, d0
2E44: 45ED F414      lea -3052(a5), d2
2E48: D5C0           addal d0, a2
2E4A: 45EA 0304      lea 772(a2), d2
2E4E: 157C 0001 000C moveb.emx #1, 12(a2)
2E54: 157C 0001 000D moveb.emx #1, 13(a2)
2E5A: 1547 000E      moveb.mx d7, 14(a2)
2E5E: 0C07 0008      cmpib #8, d7
2E62: 6608           bne 8
2E64: 157C 0064 0018 moveb.emx #100, 24(a2)
2E6A: 6012           bra 18
2E6C: 082B 0003 001E btst #3, 30(a3)
2E72: 6704           beq 4
2E74: 7628           moveq #40, d3
2E76: 6002           bra 2
2E78: 7614           moveq #20, d3
2E7A: 1543 0018      moveb.mx d3, 24(a2)
2E7E: 156B 0014 0014 moveb.emx 20(a3), 20(a2)
2E84: 356B 0016 0016 movew.emx 22(a3), 22(a2)
2E8A: 356E 000E 0008 movew.emx 14(a6), 8(a2)
2E90: 356E 0010 000A movew.emx 16(a6), 10(a2)
2E96: 302B 0004      movew.ex 4(a3), d0
2E9A: 9053           subrw (a3), d0
2E9C: 3A00           movew d0, d5
2E9E: E04D           lsriw #0, d5
2EA0: EE4D           lsriw #7, d5
2EA2: DA40           addrw d0, d5
2EA4: E245           asriw #1, d5
2EA6: 302B 0006      movew.ex 6(a3), d0
2EAA: 906B 0002      subrw.ex 2(a3), d0
2EAE: 3C00           movew d0, d6
2EB0: E04E           lsriw #0, d6
2EB2: EE4E           lsriw #7, d6
2EB4: DC40           addrw d0, d6
2EB6: E246           asriw #1, d6
2EB8: 3013           movew (a3), d0
2EBA: D045           addrw d5, d0
2EBC: 0640 FFF0      addiw #-16, d0
2EC0: 3480           movew d0, (a2)
2EC2: 302B 0002      movew.ex 2(a3), d0
2EC6: D046           addrw d6, d0
2EC8: 0640 FFF0      addiw #-16, d0
2ECC: 3540 0002      movew.mx d0, 2(a2)
2ED0: 2F0B           movel a3, -(a7)
2ED2: 2F0A           movel a2, -(a7)
2ED4: 4EBA FB56      jsr.ex -25036(pc)
2ED8: 1F3C 0001      moveb.ex #1, -(a7)
2EDC: 4EBA DBEE      jsr.ex 32468(pc)
2EE0: 7001           moveq #1, d0
2EE2: 4FEF 000A      lea 10(a7), d7
2EE6: 4CDF 0CF8      movem (a7)+, <0cf8>
2EEA: 4E5E           unlk a6
2EEC: 4E75           rts
2EEE: 8753           ormw d3, (a3)
2EF0: 686F           bvc 111
2EF2: 7441           moveq #65, d2
2EF4: 6464           bcc 100
2EF6: 0000           dc.w #0
2EF8: 4E56 0000      link a6, #0
2EFC: 48E7 1038      movem <1038>, -(a7)
2F00: 246E 0008      movel.ex 8(a6), a2
2F04: 0C2A 0007 000E cmpib.ex #7, 14(a2)
2F0A: 6508           bcs 8
2F0C: 0C2A 0008 000E cmpib.ex #8, 14(a2)
2F12: 6314           bls 20
2F14: 486D FECC      pea.ex -308(a5)
2F18: 3F3C 1049      movew.ex #4169, -(a7)
2F1C: 486D FDEA      pea.ex -534(a5)
2F20: 4E4F           trap #15
2F22: A084           sysTrapErrDisplayFileLineMsg
2F24: 4FEF 000A      lea 10(a7), d7
2F28: 7000           moveq #0, d0
2F2A: 102D F4D0      moveb.ex -2864(a5), d0
2F2E: C1FC 0026      muls.ex #38, d0
2F32: 49ED F414      lea -3052(a5), d4
2F36: D9C0           addal d0, a4
2F38: 49EC 00A4      lea 164(a4), d4
2F3C: 41ED F414      lea -3052(a5), d0
2F40: 41E8 00CA      lea 202(a0), d0
2F44: 2608           movel a0, d3
2F46: 6014           bra 20
2F48: 302C 0016      movew.ex 22(a4), d0
2F4C: B06A 0016      cmpw.ex 22(a2), d0
2F50: 6606           bne 6
2F52: 522C 001A      addqb.ex #1, 26(a4)
2F56: 6008           bra 8
2F58: 7026           moveq #38, d0
2F5A: 99C0           subal d0, a4
2F5C: B9C3           cmpal d3, a4
2F5E: 64E8           bcc -24
2F60: 7000           moveq #0, d0
2F62: 102D F4CE      moveb.ex -2866(a5), d0
2F66: C1FC 001A      muls.ex #26, d0
2F6A: 47ED F414      lea -3052(a5), d3
2F6E: D7C0           addal d0, a3
2F70: 47EB 02EA      lea 746(a3), d3
2F74: B5CB           cmpal a3, a2
2F76: 6314           bls 20
2F78: 486D FF5C      pea.ex -164(a5)
2F7C: 3F3C 105E      movew.ex #4190, -(a7)
2F80: 486D FDEA      pea.ex -534(a5)
2F84: 4E4F           trap #15
2F86: A084           sysTrapErrDisplayFileLineMsg
2F88: 4FEF 000A      lea 10(a7), d7
2F8C: B5CB           cmpal a3, a2
2F8E: 6722           beq 34
2F90: 204B           movel a3, a0
2F92: 91CA           subal a2, a0
2F94: 2008           movel a0, d0
2F96: 721A           moveq #26, d1
2F98: 4EBA D498      jsr.ex 30778(pc)
2F9C: 721A           moveq #26, d1
2F9E: 4EBA D3EA      jsr.ex 30610(pc)
2FA2: 2F00           movel d0, -(a7)
2FA4: 486A 001A      pea.ex 26(a2)
2FA8: 2F0A           movel a2, -(a7)
2FAA: 4E4F           trap #15
2FAC: A026           sysTrapMemMove
2FAE: 4FEF 000C      lea 12(a7), d7
2FB2: 422B 000C      clrb.ex 12(a3)
2FB6: 422B 000D      clrb.ex 13(a3)
2FBA: 532D F4CE      subqb.ex #1, -2866(a5)
2FBE: 4CDF 1C08      movem (a7)+, <1c08>
2FC2: 4E5E           unlk a6
2FC4: 4E75           rts
2FC6: 8A53           orrw (a3), d5
2FC8: 686F           bvc 111
2FCA: 7452           moveq #82, d2
2FCC: 656D           bcs 109
2FCE: 6F76           ble 118
2FD0: 6500 0000      bcs 0
2FD4: 4E56 0000      link a6, #0
2FD8: 48E7 1020      movem <1020>, -(a7)
2FDC: 7600           moveq #0, d3
2FDE: 162D F4CF      moveb.ex -2865(a5), d3
2FE2: 7000           moveq #0, d0
2FE4: 102D F4CE      moveb.ex -2866(a5), d0
2FE8: C1FC 001A      muls.ex #26, d0
2FEC: 45ED F414      lea -3052(a5), d2
2FF0: D5C0           addal d0, a2
2FF2: 45EA 02EA      lea 746(a2), d2
2FF6: 6034           bra 52
2FF8: 41ED F414      lea -3052(a5), d0
2FFC: 41E8 0304      lea 772(a0), d0
3000: B5C8           cmpal a0, a2
3002: 6414           bcc 20
3004: 486D FF80      pea.ex -128(a5)
3008: 3F3C 108F      movew.ex #4239, -(a7)
300C: 486D FDEA      pea.ex -534(a5)
3010: 4E4F           trap #15
3012: A084           sysTrapErrDisplayFileLineMsg
3014: 4FEF 000A      lea 10(a7), d7
3018: 4A2A 000C      tstb.ex 12(a2)
301C: 660A           bne 10
301E: 2F0A           movel a2, -(a7)
3020: 4EBA FED6      jsr.ex -23808(pc)
3024: 5343           subqw #1, d3
3026: 584F           addqw #4, a7
3028: 701A           moveq #26, d0
302A: 95C0           subal d0, a2
302C: 4A43           tstw d3
302E: 6EC8           bgt -56
3030: 422D F4CF      clrb.ex -2865(a5)
3034: 4CDF 0408      movem (a7)+, <0408>
3038: 4E5E           unlk a6
303A: 4E75           rts
303C: 9253           subrw (a3), d1
303E: 686F           bvc 111
3040: 7452           moveq #82, d2
3042: 656D           bcs 109
3044: 6F76           ble 118
3046: 6555           bcs 85
3048: 6E75           bgt 117
304A: 7361           dc.w #29537
304C: 626C           bhi 108
304E: 6500 0000      bcs 0
3052: 4E56 0000      link a6, #0
3056: 2F0A           movel a2, -(a7)
3058: 246E 0008      movel.ex 8(a6), a2
305C: 0C2A 0007 000E cmpib.ex #7, 14(a2)
3062: 6508           bcs 8
3064: 0C2A 0008 000E cmpib.ex #8, 14(a2)
306A: 6314           bls 20
306C: 486D FECC      pea.ex -308(a5)
3070: 3F3C 10B3      movew.ex #4275, -(a7)
3074: 486D FDEA      pea.ex -534(a5)
3078: 4E4F           trap #15
307A: A084           sysTrapErrDisplayFileLineMsg
307C: 4FEF 000A      lea 10(a7), d7
3080: 422A 000C      clrb.ex 12(a2)
3084: 422A 000D      clrb.ex 13(a2)
3088: 522D F4CF      addqb.ex #1, -2865(a5)
308C: 245F           movel (a7)+, a2
308E: 4E5E           unlk a6
3090: 4E75           rts
3092: 9053           subrw (a3), d0
3094: 686F           bvc 111
3096: 7453           moveq #83, d2
3098: 6574           bcs 116
309A: 4E6F           move usp, a7
309C: 7455           moveq #85, d2
309E: 7361           dc.w #29537
30A0: 626C           bhi 108
30A2: 6500 0000      bcs 0
30A6: 4E56 0000      link a6, #0
30AA: 206E 0008      movel.ex 8(a6), a0
30AE: 117C 0002 000E moveb.emx #2, 14(a0)
30B4: 30BC 0460      movew.ex #1120, (a0)
30B8: 317C 0460 0002 movew.emx #1120, 2(a0)
30BE: 3010           movew (a0), d0
30C0: 0640 032F      addiw #815, d0
30C4: 3140 0004      movew.mx d0, 4(a0)
30C8: 3028 0002      movew.ex 2(a0), d0
30CC: 0640 032F      addiw #815, d0
30D0: 3140 0006      movew.mx d0, 6(a0)
30D4: 117C 0004 0018 moveb.emx #4, 24(a0)
30DA: 0228 0007 0022 andib.ex #7, 34(a0)
30E0: 4268 0008      clrw.ex 8(a0)
30E4: 4268 000A      clrw.ex 10(a0)
30E8: 4228 001A      clrb.ex 26(a0)
30EC: 117C 0040 001C moveb.emx #64, 28(a0)
30F2: 0228 003F 0020 andib.ex #63, 32(a0)
30F8: 0028 0080 0020 orib.ex #128, 32(a0)
30FE: 0228 00CF 0020 andib.ex #207, 32(a0)
3104: 4E5E           unlk a6
3106: 4E75           rts
3108: 9653           subrw (a3), d3
310A: 6869           bvc 105
310C: 7049           moveq #73, d0
310E: 6E69           bgt 105
3110: 7441           moveq #65, d2
3112: 6C69           bge 105
3114: 656E           bcs 110
3116: 486F 6D65      pea.ex 28005(a7)
311A: 576F 726C      subqw.ex #3, 29292(a7)
311E: 6400 0000      bcc 0
3122: 4E56 0000      link a6, #0
3126: 48E7 0018      movem <0018>, -(a7)
312A: 226E 0008      movel.ex 8(a6), a1
312E: 7200           moveq #0, d1
3130: 7000           moveq #0, d0
3132: 102D F4D0      moveb.ex -2864(a5), d0
3136: C1FC 0026      muls.ex #38, d0
313A: 47ED F414      lea -3052(a5), d3
313E: D7C0           addal d0, a3
3140: 47EB 00A4      lea 164(a3), d3
3144: 49ED F414      lea -3052(a5), d4
3148: 49EC 00CA      lea 202(a4), d4
314C: 600E           bra 14
314E: 0C2B 0003 000E cmpib.ex #3, 14(a3)
3154: 6602           bne 2
3156: 5241           addqw #1, d1
3158: 7026           moveq #38, d0
315A: 97C0           subal d0, a3
315C: B7CC           cmpal a4, a3
315E: 64EE           bcc -18
3160: 137C 0003 000E moveb.emx #3, 14(a1)
3166: 3041           movew d1, a0
3168: 2008           movel a0, d0
316A: E588           lslil #2, d0
316C: 41ED FDD2      lea -558(a5), d0
3170: 32B0 0800      movew.ex 0(a0,d0.l), (a1)
3174: 3041           movew d1, a0
3176: 2008           movel a0, d0
3178: E588           lslil #2, d0
317A: 41ED FDD2      lea -558(a5), d0
317E: 5488           addql #2, a0
3180: 3370 0800 0002 movew.emx 0(a0,d0.l), 2(a1)
3186: 3011           movew (a1), d0
3188: 0640 01AF      addiw #431, d0
318C: 3340 0004      movew.mx d0, 4(a1)
3190: 3029 0002      movew.ex 2(a1), d0
3194: 0640 015F      addiw #351, d0
3198: 3340 0006      movew.mx d0, 6(a1)
319C: 137C 0004 0018 moveb.emx #4, 24(a1)
31A2: 0C2D 0019 F4CA cmpib.ex #25, -2870(a5)
31A8: 6608           bne 8
31AA: 0229 0007 0022 andib.ex #7, 34(a1)
31B0: 6006           bra 6
31B2: 0229 0007 0022 andib.ex #7, 34(a1)
31B8: 4269 0008      clrw.ex 8(a1)
31BC: 4269 000A      clrw.ex 10(a1)
31C0: 137C 0001 001A moveb.emx #1, 26(a1)
31C6: 137C 001B 001C moveb.emx #27, 28(a1)
31CC: 0229 003F 0020 andib.ex #63, 32(a1)
31D2: 0029 0080 0020 orib.ex #128, 32(a1)
31D8: 0029 0030 0020 orib.ex #48, 32(a1)
31DE: 4CDF 1800      movem (a7)+, <1800>
31E2: 4E5E           unlk a6
31E4: 4E75           rts
31E6: 9153           submw d0, (a3)
31E8: 6869           bvc 105
31EA: 7049           moveq #73, d0
31EC: 6E69           bgt 105
31EE: 7441           moveq #65, d2
31F0: 6C69           bge 105
31F2: 656E           bcs 110
31F4: 4261           clrw -(a1)
31F6: 7365           dc.w #29541
31F8: 0000           dc.w #0
31FA: 4E56 FFFC      link a6, #-4
31FE: 48E7 1C20      movem <1c20>, -(a7)
3202: 246E 0008      movel.ex 8(a6), a2
3206: 0C2D 0006 F4CA cmpib.ex #6, -2870(a5)
320C: 653A           bcs 58
320E: 42A7           clrl -(a7)
3210: 4E4F           trap #15
3212: A0C2           sysTrapSysRandom
3214: 48C0           extl d0
3216: D080           addrl d0, d0
3218: 2200           movel d0, d1
321A: E081           asril #0, d1
321C: EC81           asril #6, d1
321E: E089           lsril #0, d1
3220: E089           lsril #0, d1
3222: E289           lsril #1, d1
3224: D280           addrl d0, d1
3226: E081           asril #0, d1
3228: EE81           asril #7, d1
322A: 584F           addqw #4, a7
322C: 671A           beq 26
322E: 157C 0005 000E moveb.emx #5, 14(a2)
3234: 383C 014F      movew.ex #335, d4
3238: 7A6F           moveq #111, d5
323A: 022A 0007 0022 andib.ex #7, 34(a2)
3240: 002A 0038 0022 orib.ex #56, 34(a2)
3246: 6018           bra 24
3248: 157C 0004 000E moveb.emx #4, 14(a2)
324E: 383C 014F      movew.ex #335, d4
3252: 7A6F           moveq #111, d5
3254: 022A 0007 0022 andib.ex #7, 34(a2)
325A: 002A 0020 0022 orib.ex #32, 34(a2)
3260: 42A7           clrl -(a7)
3262: 4E4F           trap #15
3264: A0C2           sysTrapSysRandom
3266: 48C0           extl d0
3268: D080           addrl d0, d0
326A: 2200           movel d0, d1
326C: E081           asril #0, d1
326E: EC81           asril #6, d1
3270: E089           lsril #0, d1
3272: E089           lsril #0, d1
3274: E289           lsril #1, d1
3276: D280           addrl d0, d1
3278: E081           asril #0, d1
327A: EE81           asril #7, d1
327C: 584F           addqw #4, a7
327E: 6726           beq 38
3280: 42A7           clrl -(a7)
3282: 4E4F           trap #15
3284: A0C2           sysTrapSysRandom
3286: C1FC 0B00      muls.ex #2816, d0
328A: 2200           movel d0, d1
328C: E081           asril #0, d1
328E: EC81           asril #6, d1
3290: E089           lsril #0, d1
3292: E089           lsril #0, d1
3294: E289           lsril #1, d1
3296: D280           addrl d0, d1
3298: E081           asril #0, d1
329A: EE81           asril #7, d1
329C: 3481           movew d1, (a2)
329E: 426A 0002      clrw.ex 2(a2)
32A2: 584F           addqw #4, a7
32A4: 6024           bra 36
32A6: 4252           clrw (a2)
32A8: 42A7           clrl -(a7)
32AA: 4E4F           trap #15
32AC: A0C2           sysTrapSysRandom
32AE: C1FC 0A10      muls.ex #2576, d0
32B2: 2200           movel d0, d1
32B4: E081           asril #0, d1
32B6: EC81           asril #6, d1
32B8: E089           lsril #0, d1
32BA: E089           lsril #0, d1
32BC: E289           lsril #1, d1
32BE: D280           addrl d0, d1
32C0: E081           asril #0, d1
32C2: EE81           asril #7, d1
32C4: 3541 0002      movew.mx d1, 2(a2)
32C8: 584F           addqw #4, a7
32CA: 3012           movew (a2), d0
32CC: D044           addrw d4, d0
32CE: 3540 0004      movew.mx d0, 4(a2)
32D2: 302A 0002      movew.ex 2(a2), d0
32D6: D045           addrw d5, d0
32D8: 3540 0006      movew.mx d0, 6(a2)
32DC: 42A7           clrl -(a7)
32DE: 4E4F           trap #15
32E0: A0C2           sysTrapSysRandom
32E2: 48C0           extl d0
32E4: E988           lslil #4, d0
32E6: 2200           movel d0, d1
32E8: E081           asril #0, d1
32EA: EC81           asril #6, d1
32EC: E089           lsril #0, d1
32EE: E089           lsril #0, d1
32F0: E289           lsril #1, d1
32F2: D280           addrl d0, d1
32F4: E081           asril #0, d1
32F6: EE81           asril #7, d1
32F8: 1541 0018      moveb.mx d1, 24(a2)
32FC: 102A 0018      moveb.ex 24(a2), d0
3300: 4880           extw d0
3302: 48C0           extl d0
3304: 81FC 0004      divs.ex #4, d0
3308: 4840           swap d0
330A: 4A40           tstw d0
330C: 584F           addqw #4, a7
330E: 67CC           beq -52
3310: 102A 0018      moveb.ex 24(a2), d0
3314: 4880           extw d0
3316: 48C0           extl d0
3318: D080           addrl d0, d0
331A: 41ED FCCE      lea -818(a5), d0
331E: 7200           moveq #0, d1
3320: 122A 0022      moveb.ex 34(a2), d1
3324: E609           lsrib #3, d1
3326: C3F0 0800      muls.ex 0(a0,d0.l), d1
332A: 3541 0008      movew.mx d1, 8(a2)
332E: 102A 0018      moveb.ex 24(a2), d0
3332: 4880           extw d0
3334: 48C0           extl d0
3336: D080           addrl d0, d0
3338: 41ED FCEE      lea -786(a5), d0
333C: 7200           moveq #0, d1
333E: 122A 0022      moveb.ex 34(a2), d1
3342: E609           lsrib #3, d1
3344: C3F0 0800      muls.ex 0(a0,d0.l), d1
3348: 3541 000A      movew.mx d1, 10(a2)
334C: 157C 0001 001A moveb.emx #1, 26(a2)
3352: 6004           bra 4
3354: 522A 001A      addqb.ex #1, 26(a2)
3358: 7000           moveq #0, d0
335A: 102D F4CA      moveb.ex -2870(a5), d0
335E: C1FC 000A      muls.ex #10, d0
3362: 0640 00FA      addiw #250, d0
3366: 48C0           extl d0
3368: 2D40 FFFC      movel.mx d0, -4(a6)
336C: 42A7           clrl -(a7)
336E: 4E4F           trap #15
3370: A0C2           sysTrapSysRandom
3372: C1FC 03E8      muls.ex #1000, d0
3376: 2200           movel d0, d1
3378: E081           asril #0, d1
337A: EC81           asril #6, d1
337C: E089           lsril #0, d1
337E: E089           lsril #0, d1
3380: E289           lsril #1, d1
3382: D280           addrl d0, d1
3384: E081           asril #0, d1
3386: EE81           asril #7, d1
3388: B2AE FFFC      cmpl.ex -4(a6), d1
338C: 584F           addqw #4, a7
338E: 6DC4           blt -60
3390: 422A 001C      clrb.ex 28(a2)
3394: 0C2D 000D F4CA cmpib.ex #13, -2870(a5)
339A: 654E           bcs 78
339C: 6004           bra 4
339E: 522A 001C      addqb.ex #1, 28(a2)
33A2: 7000           moveq #0, d0
33A4: 102A 001C      moveb.ex 28(a2), d0
33A8: 5240           addqw #1, d0
33AA: 7200           moveq #0, d1
33AC: 122D F4CA      moveb.ex -2870(a5), d1
33B0: C3FC 001E      muls.ex #30, d1
33B4: 48C1           extl d1
33B6: 83C0           divs d0, d1
33B8: 48C1           extl d1
33BA: 2D41 FFFC      movel.mx d1, -4(a6)
33BE: 42A7           clrl -(a7)
33C0: 4E4F           trap #15
33C2: A0C2           sysTrapSysRandom
33C4: C1FC 03E8      muls.ex #1000, d0
33C8: 2200           movel d0, d1
33CA: E081           asril #0, d1
33CC: EC81           asril #6, d1
33CE: E089           lsril #0, d1
33D0: E089           lsril #0, d1
33D2: E289           lsril #1, d1
33D4: D280           addrl d0, d1
33D6: E081           asril #0, d1
33D8: EE81           asril #7, d1
33DA: B2AE FFFC      cmpl.ex -4(a6), d1
33DE: 584F           addqw #4, a7
33E0: 6C08           bge 8
33E2: 0C2A 0006 001C cmpib.ex #6, 28(a2)
33E8: 65B4           bcs -76
33EA: 0C2A 0005 000E cmpib.ex #5, 14(a2)
33F0: 665C           bne 92
33F2: 0C2D 0010 F4CA cmpib.ex #16, -2870(a5)
33F8: 6554           bcs 84
33FA: 42A7           clrl -(a7)
33FC: 4E4F           trap #15
33FE: A0C2           sysTrapSysRandom
3400: C1FC 03E8      muls.ex #1000, d0
3404: 2200           movel d0, d1
3406: E081           asril #0, d1
3408: EC81           asril #6, d1
340A: E089           lsril #0, d1
340C: E089           lsril #0, d1
340E: E289           lsril #1, d1
3410: D280           addrl d0, d1
3412: E081           asril #0, d1
3414: EE81           asril #7, d1
3416: 0C81 0000 00C8 cmpil #200, d1
341C: 584F           addqw #4, a7
341E: 6C2E           bge 46
3420: 157C 0006 000E moveb.emx #6, 14(a2)
3426: 522A 001C      addqb.ex #1, 28(a2)
342A: 542A 001A      addqb.ex #2, 26(a2)
342E: 002A 00C0 0020 orib.ex #192, 32(a2)
3434: 022A 00CF 0020 andib.ex #207, 32(a2)
343A: 002A 0020 0020 orib.ex #32, 32(a2)
3440: 022A 0007 0022 andib.ex #7, 34(a2)
3446: 002A 0048 0022 orib.ex #72, 34(a2)
344C: 6072           bra 114
344E: 0C2D 000B F4CA cmpib.ex #11, -2870(a5)
3454: 650E           bcs 14
3456: 022A 003F 0020 andib.ex #63, 32(a2)
345C: 002A 0040 0020 orib.ex #64, 32(a2)
3462: 6006           bra 6
3464: 022A 003F 0020 andib.ex #63, 32(a2)
346A: 0C2D 000F F4CA cmpib.ex #15, -2870(a5)
3470: 6504           bcs 4
3472: 7603           moveq #3, d3
3474: 601A           bra 26
3476: 0C2D 000A F4CA cmpib.ex #10, -2870(a5)
347C: 6504           bcs 4
347E: 7602           moveq #2, d3
3480: 600E           bra 14
3482: 0C2D 0005 F4CA cmpib.ex #5, -2870(a5)
3488: 6504           bcs 4
348A: 7601           moveq #1, d3
348C: 6002           bra 2
348E: 7600           moveq #0, d3
3490: 42A7           clrl -(a7)
3492: 4E4F           trap #15
3494: A0C2           sysTrapSysRandom
3496: 3203           movew d3, d1
3498: 5241           addqw #1, d1
349A: C3C0           muls d0, d1
349C: 2001           movel d1, d0
349E: E080           asril #0, d0
34A0: EC80           asril #6, d0
34A2: E088           lsril #0, d0
34A4: E088           lsril #0, d0
34A6: E288           lsril #1, d0
34A8: D081           addrl d1, d0
34AA: E080           asril #0, d0
34AC: EE80           asril #7, d0
34AE: E908           lslib #4, d0
34B0: 0200 0030      andib #48, d0
34B4: 022A 00CF 0020 andib.ex #207, 32(a2)
34BA: 812A 0020      ormb.ex d0, 32(a2)
34BE: 584F           addqw #4, a7
34C0: 4CDF 0438      movem (a7)+, <0438>
34C4: 4E5E           unlk a6
34C6: 4E75           rts
34C8: 9153           submw d0, (a3)
34CA: 6869           bvc 105
34CC: 7049           moveq #73, d0
34CE: 6E69           bgt 105
34D0: 7441           moveq #65, d2
34D2: 6C69           bge 105
34D4: 656E           bcs 110
34D6: 5368 6970      subqw.ex #1, 26992(a0)
34DA: 0000           dc.w #0
34DC: 4E56 0000      link a6, #0
34E0: 2F0A           movel a2, -(a7)
34E2: 0C2D 000F F4D0 cmpib.ex #15, -2864(a5)
34E8: 6400 00BC      bcc 188
34EC: 102D F4D0      moveb.ex -2864(a5), d0
34F0: 522D F4D0      addqb.ex #1, -2864(a5)
34F4: 7200           moveq #0, d1
34F6: 1200           moveb d0, d1
34F8: C3FC 0026      muls.ex #38, d1
34FC: 45ED F414      lea -3052(a5), d2
3500: D5C1           addal d1, a2
3502: 45EA 00CA      lea 202(a2), d2
3506: 157C 0001 000D moveb.emx #1, 13(a2)
350C: 157C 0001 000C moveb.emx #1, 12(a2)
3512: 022A 007F 001E andib.ex #127, 30(a2)
3518: 022A 00BF 001E andib.ex #191, 30(a2)
351E: 022A 00DF 001E andib.ex #223, 30(a2)
3524: 022A 00EF 001E andib.ex #239, 30(a2)
352A: 022A 00F7 001E andib.ex #247, 30(a2)
3530: 022A 00FB 001E andib.ex #251, 30(a2)
3536: 022A 00F7 001E andib.ex #247, 30(a2)
353C: 022A 00F7 0020 andib.ex #247, 32(a2)
3542: 422A 0019      clrb.ex 25(a2)
3546: 157C 00FE 0014 moveb.emx #254, 20(a2)
354C: 4EBA E9C4      jsr.ex -27878(pc)
3550: 3540 0016      movew.mx d0, 22(a2)
3554: 7000           moveq #0, d0
3556: 102E 0008      moveb.ex 8(a6), d0
355A: 5540           subqw #2, d0
355C: 670A           beq 10
355E: 5340           subqw #1, d0
3560: 6710           beq 16
3562: 5340           subqw #1, d0
3564: 6716           beq 22
3566: 601C           bra 28
3568: 2F0A           movel a2, -(a7)
356A: 4EBA FB3A      jsr.ex -23378(pc)
356E: 584F           addqw #4, a7
3570: 6012           bra 18
3572: 2F0A           movel a2, -(a7)
3574: 4EBA FBAC      jsr.ex -23254(pc)
3578: 584F           addqw #4, a7
357A: 6008           bra 8
357C: 2F0A           movel a2, -(a7)
357E: 4EBA FC7A      jsr.ex -23038(pc)
3582: 584F           addqw #4, a7
3584: 522D F4D1      addqb.ex #1, -2863(a5)
3588: 4A2D F43D      tstb.ex -3011(a5)
358C: 6704           beq 4
358E: 532D F43D      subqb.ex #1, -3011(a5)
3592: 0C2A 0002 000E cmpib.ex #2, 14(a2)
3598: 650C           bcs 12
359A: 0C2A 0003 000E cmpib.ex #3, 14(a2)
35A0: 6204           bhi 4
35A2: 522D F4D2      addqb.ex #1, -2862(a5)
35A6: 245F           movel (a7)+, a2
35A8: 4E5E           unlk a6
35AA: 4E75           rts
35AC: 8C53           orrw (a3), d6
35AE: 6869           bvc 105
35B0: 7041           moveq #65, d0
35B2: 6464           bcc 100
35B4: 416C           dc.w #16748
35B6: 6965           bvs 101
35B8: 6E00 0000      bgt 0
35BC: 4E56 0000      link a6, #0
35C0: 2F0A           movel a2, -(a7)
35C2: 246E 0008      movel.ex 8(a6), a2
35C6: 0C2A 0002 000E cmpib.ex #2, 14(a2)
35CC: 6508           bcs 8
35CE: 0C2A 0006 000E cmpib.ex #6, 14(a2)
35D4: 6314           bls 20
35D6: 486D FECC      pea.ex -308(a5)
35DA: 3F3C 1208      movew.ex #4616, -(a7)
35DE: 486D FDEA      pea.ex -534(a5)
35E2: 4E4F           trap #15
35E4: A084           sysTrapErrDisplayFileLineMsg
35E6: 4FEF 000A      lea 10(a7), d7
35EA: 7030           moveq #48, d0
35EC: C02A 0020      andrb.ex 32(a2), d0
35F0: E808           lsrib #4, d0
35F2: 6700 008A      beq 138
35F6: 5340           subqw #1, d0
35F8: 670A           beq 10
35FA: 5340           subqw #1, d0
35FC: 6732           beq 50
35FE: 5340           subqw #1, d0
3600: 675A           beq 90
3602: 607A           bra 122
3604: 42A7           clrl -(a7)
3606: 4E4F           trap #15
3608: A0C2           sysTrapSysRandom
360A: 48C0           extl d0
360C: D080           addrl d0, d0
360E: 2200           movel d0, d1
3610: E081           asril #0, d1
3612: EC81           asril #6, d1
3614: E089           lsril #0, d1
3616: E089           lsril #0, d1
3618: E289           lsril #1, d1
361A: D280           addrl d0, d1
361C: E081           asril #0, d1
361E: EE81           asril #7, d1
3620: 584F           addqw #4, a7
3622: 6706           beq 6
3624: 522A 0018      addqb.ex #1, 24(a2)
3628: 6054           bra 84
362A: 532A 0018      subqb.ex #1, 24(a2)
362E: 604E           bra 78
3630: 42A7           clrl -(a7)
3632: 4E4F           trap #15
3634: A0C2           sysTrapSysRandom
3636: 48C0           extl d0
3638: D080           addrl d0, d0
363A: 2200           movel d0, d1
363C: E081           asril #0, d1
363E: EC81           asril #6, d1
3640: E089           lsril #0, d1
3642: E089           lsril #0, d1
3644: E289           lsril #1, d1
3646: D280           addrl d0, d1
3648: E081           asril #0, d1
364A: EE81           asril #7, d1
364C: 584F           addqw #4, a7
364E: 6706           beq 6
3650: 582A 0018      addqb.ex #4, 24(a2)
3654: 6028           bra 40
3656: 592A 0018      subqb.ex #4, 24(a2)
365A: 6022           bra 34
365C: 42A7           clrl -(a7)
365E: 4E4F           trap #15
3660: A0C2           sysTrapSysRandom
3662: 48C0           extl d0
3664: E988           lslil #4, d0
3666: 2200           movel d0, d1
3668: E081           asril #0, d1
366A: EC81           asril #6, d1
366C: E089           lsril #0, d1
366E: E089           lsril #0, d1
3670: E289           lsril #1, d1
3672: D280           addrl d0, d1
3674: E081           asril #0, d1
3676: EE81           asril #7, d1
3678: 1541 0018      moveb.mx d1, 24(a2)
367C: 584F           addqw #4, a7
367E: 4A2A 0018      tstb.ex 24(a2)
3682: 6C08           bge 8
3684: 062A 0010 0018 addib.ex #16, 24(a2)
368A: 600E           bra 14
368C: 0C2A 0010 0018 cmpib.ex #16, 24(a2)
3692: 6D06           blt 6
3694: 042A 0010 0018 subib.ex #16, 24(a2)
369A: 102A 0018      moveb.ex 24(a2), d0
369E: 4880           extw d0
36A0: 48C0           extl d0
36A2: D080           addrl d0, d0
36A4: 41ED FCCE      lea -818(a5), d0
36A8: 7200           moveq #0, d1
36AA: 122A 0022      moveb.ex 34(a2), d1
36AE: E609           lsrib #3, d1
36B0: C3F0 0800      muls.ex 0(a0,d0.l), d1
36B4: 3541 0008      movew.mx d1, 8(a2)
36B8: 102A 0018      moveb.ex 24(a2), d0
36BC: 4880           extw d0
36BE: 48C0           extl d0
36C0: D080           addrl d0, d0
36C2: 41ED FCEE      lea -786(a5), d0
36C6: 7200           moveq #0, d1
36C8: 122A 0022      moveb.ex 34(a2), d1
36CC: E609           lsrib #3, d1
36CE: C3F0 0800      muls.ex 0(a0,d0.l), d1
36D2: 3541 000A      movew.mx d1, 10(a2)
36D6: 245F           movel (a7)+, a2
36D8: 4E5E           unlk a6
36DA: 4E75           rts
36DC: 9441           subrw d1, d2
36DE: 6C69           bge 105
36E0: 656E           bcs 110
36E2: 4368           dc.w #17256
36E4: 616E           bsr 110
36E6: 6765           beq 101
36E8: 4469 7265      negw.ex 29285(a1)
36EC: 6374           bls 116
36EE: 696F           bvs 111
36F0: 6E00 0000      bgt 0
36F4: 4E56 FFFC      link a6, #-4
36F8: 48E7 1838      movem <1838>, -(a7)
36FC: 246E 0008      movel.ex 8(a6), a2
3700: 266E 000C      movel.ex 12(a6), a3
3704: 0C2A 0002 000E cmpib.ex #2, 14(a2)
370A: 6508           bcs 8
370C: 0C2A 0006 000E cmpib.ex #6, 14(a2)
3712: 6314           bls 20
3714: 486D FECC      pea.ex -308(a5)
3718: 3F3C 1247      movew.ex #4679, -(a7)
371C: 486D FDEA      pea.ex -534(a5)
3720: 4E4F           trap #15
3722: A084           sysTrapErrDisplayFileLineMsg
3724: 4FEF 000A      lea 10(a7), d7
3728: 2F0A           movel a2, -(a7)
372A: 4EBA D602      jsr.ex -32458(pc)
372E: 4A00           tstb d0
3730: 584F           addqw #4, a7
3732: 6700 0162      beq 354
3736: 0C2D 000B F4CE cmpib.ex #11, -2866(a5)
373C: 6400 0158      bcc 344
3740: 7607           moveq #7, d3
3742: 7000           moveq #0, d0
3744: 102A 0020      moveb.ex 32(a2), d0
3748: EC08           lsrib #6, d0
374A: 6714           beq 20
374C: 5340           subqw #1, d0
374E: 6700 00B0      beq 176
3752: 5340           subqw #1, d0
3754: 6756           beq 86
3756: 5340           subqw #1, d0
3758: 6700 00D6      beq 214
375C: 6000 0124      bra 292
3760: 42A7           clrl -(a7)
3762: 4E4F           trap #15
3764: A0C2           sysTrapSysRandom
3766: 48C0           extl d0
3768: E988           lslil #4, d0
376A: 2200           movel d0, d1
376C: E081           asril #0, d1
376E: EC81           asril #6, d1
3770: E089           lsril #0, d1
3772: E089           lsril #0, d1
3774: E289           lsril #1, d1
3776: D280           addrl d0, d1
3778: E081           asril #0, d1
377A: EE81           asril #7, d1
377C: 1801           moveb d1, d4
377E: 7000           moveq #0, d0
3780: 1004           moveb d4, d0
3782: D080           addrl d0, d0
3784: 41ED FCCE      lea -818(a5), d0
3788: 7218           moveq #24, d1
378A: C3F0 0800      muls.ex 0(a0,d0.l), d1
378E: 3D41 FFFE      movew.mx d1, -2(a6)
3792: 7000           moveq #0, d0
3794: 1004           moveb d4, d0
3796: D080           addrl d0, d0
3798: 41ED FCEE      lea -786(a5), d0
379C: 7218           moveq #24, d1
379E: C3F0 0800      muls.ex 0(a0,d0.l), d1
37A2: 3D41 FFFC      movew.mx d1, -4(a6)
37A6: 584F           addqw #4, a7
37A8: 6000 00D8      bra 216
37AC: 42A7           clrl -(a7)
37AE: 4E4F           trap #15
37B0: A0C2           sysTrapSysRandom
37B2: C1FC 0064      muls.ex #100, d0
37B6: 2200           movel d0, d1
37B8: E081           asril #0, d1
37BA: EC81           asril #6, d1
37BC: E089           lsril #0, d1
37BE: E089           lsril #0, d1
37C0: E289           lsril #1, d1
37C2: D280           addrl d0, d1
37C4: E081           asril #0, d1
37C6: EE81           asril #7, d1
37C8: 700A           moveq #10, d0
37CA: B280           cmpl d0, d1
37CC: 584F           addqw #4, a7
37CE: 6E90           bgt -112
37D0: 200B           movel a3, d0
37D2: 6612           bne 18
37D4: 2F0A           movel a2, -(a7)
37D6: 4EBA E618      jsr.ex -28168(pc)
37DA: 2848           movel a0, a4
37DC: 200C           movel a4, d0
37DE: 584F           addqw #4, a7
37E0: 6700 FF7E      beq 65406
37E4: 264C           movel a4, a3
37E6: 7608           moveq #8, d3
37E8: 486E FFFC      pea.ex -4(a6)
37EC: 486E FFFE      pea.ex -2(a6)
37F0: 2F0B           movel a3, -(a7)
37F2: 2F0A           movel a2, -(a7)
37F4: 4EBA F34E      jsr.ex -24756(pc)
37F8: 4FEF 0010      lea 16(a7), d7
37FC: 6000 0084      bra 132
3800: 200B           movel a3, d0
3802: 6700 FF5C      beq 65372
3806: 0C2B 0009 000E cmpib.ex #9, 14(a3)
380C: 6500 FF52      bcs 65362
3810: 0C2B 000B 000E cmpib.ex #11, 14(a3)
3816: 6200 FF48      bhi 65352
381A: 486E FFFC      pea.ex -4(a6)
381E: 486E FFFE      pea.ex -2(a6)
3822: 2F0B           movel a3, -(a7)
3824: 2F0A           movel a2, -(a7)
3826: 4EBA F31C      jsr.ex -24756(pc)
382A: 4FEF 0010      lea 16(a7), d7
382E: 6052           bra 82
3830: 2F0A           movel a2, -(a7)
3832: 4EBA E5BC      jsr.ex -28168(pc)
3836: 2848           movel a0, a4
3838: 200C           movel a4, d0
383A: 584F           addqw #4, a7
383C: 6700 FF22      beq 65314
3840: 0C2A 0006 000E cmpib.ex #6, 14(a2)
3846: 6626           bne 38
3848: 42A7           clrl -(a7)
384A: 4E4F           trap #15
384C: A0C2           sysTrapSysRandom
384E: C1FC 0064      muls.ex #100, d0
3852: 2200           movel d0, d1
3854: E081           asril #0, d1
3856: EC81           asril #6, d1
3858: E089           lsril #0, d1
385A: E089           lsril #0, d1
385C: E289           lsril #1, d1
385E: D280           addrl d0, d1
3860: E081           asril #0, d1
3862: EE81           asril #7, d1
3864: 700A           moveq #10, d0
3866: B280           cmpl d0, d1
3868: 584F           addqw #4, a7
386A: 6E02           bgt 2
386C: 7608           moveq #8, d3
386E: 486E FFFC      pea.ex -4(a6)
3872: 486E FFFE      pea.ex -2(a6)
3876: 2F0C           movel a4, -(a7)
3878: 2F0A           movel a2, -(a7)
387A: 4EBA F2C8      jsr.ex -24756(pc)
387E: 4FEF 0010      lea 16(a7), d7
3882: 3F2E FFFC      movew.ex -4(a6), -(a7)
3886: 3F2E FFFE      movew.ex -2(a6), -(a7)
388A: 2F0A           movel a2, -(a7)
388C: 1F03           moveb d3, -(a7)
388E: 4EBA F55A      jsr.ex -24078(pc)
3892: 4FEF 000A      lea 10(a7), d7
3896: 4CDF 1C18      movem (a7)+, <1c18>
389A: 4E5E           unlk a6
389C: 4E75           rts
389E: 8A41           orrw d1, d5
38A0: 6C69           bge 105
38A2: 656E           bcs 110
38A4: 5368 6F6F      subqw.ex #1, 28527(a0)
38A8: 7400           moveq #0, d2
38AA: 0000           dc.w #0
38AC: 4E56 0000      link a6, #0
38B0: 48E7 0030      movem <0030>, -(a7)
38B4: 246E 0008      movel.ex 8(a6), a2
38B8: 266E 000C      movel.ex 12(a6), a3
38BC: 0C2A 0002 000E cmpib.ex #2, 14(a2)
38C2: 6508           bcs 8
38C4: 0C2A 0006 000E cmpib.ex #6, 14(a2)
38CA: 6314           bls 20
38CC: 486D FECC      pea.ex -308(a5)
38D0: 3F3C 12A7      movew.ex #4775, -(a7)
38D4: 486D FDEA      pea.ex -534(a5)
38D8: 4E4F           trap #15
38DA: A084           sysTrapErrDisplayFileLineMsg
38DC: 4FEF 000A      lea 10(a7), d7
38E0: 4A2A 001B      tstb.ex 27(a2)
38E4: 6710           beq 16
38E6: 002A 0040 001E orib.ex #64, 30(a2)
38EC: 422A 0019      clrb.ex 25(a2)
38F0: 532A 001B      subqb.ex #1, 27(a2)
38F4: 606E           bra 110
38F6: 4A2A 001A      tstb.ex 26(a2)
38FA: 673A           beq 58
38FC: 0C2B 0006 000E cmpib.ex #6, 14(a3)
3902: 6224           bhi 36
3904: 42A7           clrl -(a7)
3906: 4E4F           trap #15
3908: A0C2           sysTrapSysRandom
390A: 48C0           extl d0
390C: E588           lslil #2, d0
390E: 2200           movel d0, d1
3910: E081           asril #0, d1
3912: EC81           asril #6, d1
3914: E089           lsril #0, d1
3916: E089           lsril #0, d1
3918: E289           lsril #1, d1
391A: D280           addrl d0, d1
391C: E081           asril #0, d1
391E: EE81           asril #7, d1
3920: 7001           moveq #1, d0
3922: B280           cmpl d0, d1
3924: 584F           addqw #4, a7
3926: 6E0E           bgt 14
3928: 2F2A 0010      movel.ex 16(a2), -(a7)
392C: 2F0A           movel a2, -(a7)
392E: 4EBA FDC4      jsr.ex -21764(pc)
3932: 504F           addqw #0, a7
3934: 602E           bra 46
3936: 102A 0022      moveb.ex 34(a2), d0
393A: E608           lsrib #3, d0
393C: 6726           beq 38
393E: 102A 0022      moveb.ex 34(a2), d0
3942: E608           lsrib #3, d0
3944: 5C00           addqb #6, d0
3946: E708           lslib #3, d0
3948: 0200 00F8      andib #248, d0
394C: 022A 0007 0022 andib.ex #7, 34(a2)
3952: 812A 0022      ormb.ex d0, 34(a2)
3956: 2F0A           movel a2, -(a7)
3958: 4EBA FC62      jsr.ex -22076(pc)
395C: 002A 0008 0020 orib.ex #8, 32(a2)
3962: 584F           addqw #4, a7
3964: 4CDF 0C00      movem (a7)+, <0c00>
3968: 4E5E           unlk a6
396A: 4E75           rts
396C: 9041           subrw d1, d0
396E: 6C69           bge 105
3970: 656E           bcs 110
3972: 4176           dc.w #16758
3974: 6F69           ble 105
3976: 644F           bcc 79
3978: 626A           bhi 106
397A: 6563           bcs 99
397C: 7400           moveq #0, d2
397E: 0000           dc.w #0
3980: 4E56 0000      link a6, #0
3984: 48E7 1020      movem <1020>, -(a7)
3988: 246E 0008      movel.ex 8(a6), a2
398C: 0C2A 0002 000E cmpib.ex #2, 14(a2)
3992: 6508           bcs 8
3994: 0C2A 0006 000E cmpib.ex #6, 14(a2)
399A: 6314           bls 20
399C: 486D FECC      pea.ex -308(a5)
39A0: 3F3C 12DF      movew.ex #4831, -(a7)
39A4: 486D FDEA      pea.ex -534(a5)
39A8: 4E4F           trap #15
39AA: A084           sysTrapErrDisplayFileLineMsg
39AC: 4FEF 000A      lea 10(a7), d7
39B0: 764B           moveq #75, d3
39B2: 7030           moveq #48, d0
39B4: C02A 0020      andrb.ex 32(a2), d0
39B8: E808           lsrib #4, d0
39BA: C1FC 0019      muls.ex #25, d0
39BE: D640           addrw d0, d3
39C0: 7000           moveq #0, d0
39C2: 102A 0020      moveb.ex 32(a2), d0
39C6: EC08           lsrib #6, d0
39C8: C1FC 004B      muls.ex #75, d0
39CC: D640           addrw d0, d3
39CE: 7000           moveq #0, d0
39D0: 102A 001C      moveb.ex 28(a2), d0
39D4: C1FC 0032      muls.ex #50, d0
39D8: D640           addrw d0, d3
39DA: 4A2A 001B      tstb.ex 27(a2)
39DE: 6704           beq 4
39E0: 0643 0032      addiw #50, d3
39E4: 0C2A 0005 000E cmpib.ex #5, 14(a2)
39EA: 6604           bne 4
39EC: D643           addrw d3, d3
39EE: 6020           bra 32
39F0: 0C2A 0003 000E cmpib.ex #3, 14(a2)
39F6: 6708           beq 8
39F8: 0C2A 0002 000E cmpib.ex #2, 14(a2)
39FE: 6610           bne 16
3A00: 7000           moveq #0, d0
3A02: 102A 000E      moveb.ex 14(a2), d0
3A06: D080           addrl d0, d0
3A08: 41ED FD6E      lea -658(a5), d0
3A0C: 3630 0800      movew.ex 0(a0,d0.l), d3
3A10: 3003           movew d3, d0
3A12: 4CDF 0408      movem (a7)+, <0408>
3A16: 4E5E           unlk a6
3A18: 4E75           rts
3A1A: 9241           subrw d1, d1
3A1C: 6C69           bge 105
3A1E: 656E           bcs 110
3A20: 4576           dc.w #17782
3A22: 616C           bsr 108
3A24: 7561           dc.w #30049
3A26: 7465           moveq #101, d2
3A28: 576F 7274      subqw.ex #3, 29300(a7)
3A2C: 6800 0000      bvc 0
3A30: 4E56 0000      link a6, #0
3A34: 48E7 1820      movem <1820>, -(a7)
3A38: 246E 0008      movel.ex 8(a6), a2
3A3C: 182E 000C      moveb.ex 12(a6), d4
3A40: 0C04 0004      cmpib #4, d4
3A44: 6460           bcc 96
3A46: 7000           moveq #0, d0
3A48: 102A 000E      moveb.ex 14(a2), d0
3A4C: D080           addrl d0, d0
3A4E: 41ED FD6E      lea -658(a5), d0
3A52: 3630 0800      movew.ex 0(a0,d0.l), d3
3A56: 0C2A 0006 000E cmpib.ex #6, 14(a2)
3A5C: 6248           bhi 72
3A5E: 0C2A 0002 000E cmpib.ex #2, 14(a2)
3A64: 6512           bcs 18
3A66: 0C2A 0006 000E cmpib.ex #6, 14(a2)
3A6C: 620A           bhi 10
3A6E: 2F0A           movel a2, -(a7)
3A70: 4EBA FF0E      jsr.ex -21112(pc)
3A74: 3600           movew d0, d3
3A76: 584F           addqw #4, a7
3A78: 3F03           movew d3, -(a7)
3A7A: 302A 0002      movew.ex 2(a2), d0
3A7E: D06A 0006      addrw.ex 6(a2), d0
3A82: 3200           movew d0, d1
3A84: E049           lsriw #0, d1
3A86: EE49           lsriw #7, d1
3A88: D240           addrw d0, d1
3A8A: E241           asriw #1, d1
3A8C: 3F01           movew d1, -(a7)
3A8E: 3012           movew (a2), d0
3A90: D06A 0004      addrw.ex 4(a2), d0
3A94: 3200           movew d0, d1
3A96: E049           lsriw #0, d1
3A98: EE49           lsriw #7, d1
3A9A: D240           addrw d0, d1
3A9C: E241           asriw #1, d1
3A9E: 3F01           movew d1, -(a7)
3AA0: 4EBA D8BC      jsr.ex -30874(pc)
3AA4: 5C4F           addqw #6, a7
3AA6: 1F04           moveb d4, -(a7)
3AA8: 3043           movew d3, a0
3AAA: 2F08           movel a0, -(a7)
3AAC: 4EBA D06E      jsr.ex 32548(pc)
3AB0: 5C4F           addqw #6, a7
3AB2: 4CDF 0418      movem (a7)+, <0418>
3AB6: 4E5E           unlk a6
3AB8: 4E75           rts
3ABA: 9441           subrw d1, d2
3ABC: 7761           dc.w #30561
3ABE: 7264           moveq #100, d1
3AC0: 506F 696E      addqw.ex #0, 26990(a7)
3AC4: 7473           moveq #115, d2
3AC6: 466F 724F      notw.ex 29263(a7)
3ACA: 626A           bhi 106
3ACC: 6563           bcs 99
3ACE: 7400           moveq #0, d2
3AD0: 0000           dc.w #0
3AD2: 4E56 0000      link a6, #0
3AD6: 48E7 0030      movem <0030>, -(a7)
3ADA: 246E 0008      movel.ex 8(a6), a2
3ADE: 266E 000C      movel.ex 12(a6), a3
3AE2: 7000           moveq #0, d0
3AE4: 102A 000E      moveb.ex 14(a2), d0
3AE8: 671A           beq 26
3AEA: 0C40 0011      cmpiw #17, d0
3AEE: 6264           bhi 100
3AF0: 5540           subqw #2, d0
3AF2: 6D60           blt 96
3AF4: 5B40           subqw #5, d0
3AF6: 6D0C           blt 12
3AF8: 673C           beq 60
3AFA: 5340           subqw #1, d0
3AFC: 672E           beq 46
3AFE: 5940           subqw #4, d0
3B00: 6D16           blt 22
3B02: 6042           bra 66
3B04: 2F0A           movel a2, -(a7)
3B06: 4EBA D426      jsr.ex -31946(pc)
3B0A: 2F0B           movel a3, -(a7)
3B0C: 2F0A           movel a2, -(a7)
3B0E: 4EBA EC3A      jsr.ex -25774(pc)
3B12: 4FEF 000C      lea 12(a7), d7
3B16: 603C           bra 60
3B18: 2F0A           movel a2, -(a7)
3B1A: 4EBA D412      jsr.ex -31946(pc)
3B1E: 2F0B           movel a3, -(a7)
3B20: 2F0A           movel a2, -(a7)
3B22: 4EBA DFDE      jsr.ex -28918(pc)
3B26: 4FEF 000C      lea 12(a7), d7
3B2A: 6028           bra 40
3B2C: 2F0B           movel a3, -(a7)
3B2E: 2F0A           movel a2, -(a7)
3B30: 4EBA D5B6      jsr.ex -31504(pc)
3B34: 504F           addqw #0, a7
3B36: 2F0A           movel a2, -(a7)
3B38: 4EBA D3F4      jsr.ex -31946(pc)
3B3C: 2F0A           movel a2, -(a7)
3B3E: 4EBA F512      jsr.ex -23462(pc)
3B42: 504F           addqw #0, a7
3B44: 600E           bra 14
3B46: 2F0A           movel a2, -(a7)
3B48: 4EBA D3E4      jsr.ex -31946(pc)
3B4C: 2F0A           movel a2, -(a7)
3B4E: 4EBA DBBA      jsr.ex -29934(pc)
3B52: 504F           addqw #0, a7
3B54: 7001           moveq #1, d0
3B56: 4CDF 0C00      movem (a7)+, <0c00>
3B5A: 4E5E           unlk a6
3B5C: 4E75           rts
3B5E: 8D4F           dc.w #36175
3B60: 626A           bhi 106
3B62: 6563           bcs 99
3B64: 7444           moveq #68, d2
3B66: 6573           bcs 115
3B68: 7472           moveq #114, d2
3B6A: 6F79           ble 121
3B6C: 0000           dc.w #0
3B6E: 4E56 FFFA      link a6, #-6
3B72: 48E7 1F38      movem <1f38>, -(a7)
3B76: 246E 0008      movel.ex 8(a6), a2
3B7A: 266E 000C      movel.ex 12(a6), a3
3B7E: B7CA           cmpal a2, a3
3B80: 6306           bls 6
3B82: 284A           movel a2, a4
3B84: 244B           movel a3, a2
3B86: 244C           movel a4, a2
3B88: 0C2B 0007 000E cmpib.ex #7, 14(a3)
3B8E: 650E           bcs 14
3B90: 0C2B 0008 000E cmpib.ex #8, 14(a3)
3B96: 6206           bhi 6
3B98: 182B 0014      moveb.ex 20(a3), d4
3B9C: 6012           bra 18
3B9E: 0C2B 0006 000E cmpib.ex #6, 14(a3)
3BA4: 6206           bhi 6
3BA6: 182B 0014      moveb.ex 20(a3), d4
3BAA: 6004           bra 4
3BAC: 183C 00FF      moveb.ex #255, d4
3BB0: 0C2A 0007 000E cmpib.ex #7, 14(a2)
3BB6: 650E           bcs 14
3BB8: 0C2A 0008 000E cmpib.ex #8, 14(a2)
3BBE: 6206           bhi 6
3BC0: 162A 0014      moveb.ex 20(a2), d3
3BC4: 6012           bra 18
3BC6: 0C2A 0006 000E cmpib.ex #6, 14(a2)
3BCC: 6206           bhi 6
3BCE: 162A 0014      moveb.ex 20(a2), d3
3BD2: 6004           bra 4
3BD4: 163C 00FF      moveb.ex #255, d3
3BD8: B604           cmpb d4, d3
3BDA: 660C           bne 12
3BDC: 0C03 00FF      cmpib #255, d3
3BE0: 6706           beq 6
3BE2: 7000           moveq #0, d0
3BE4: 6000 021A      bra 538
3BE8: 4A2D F4C9      tstb.ex -2871(a5)
3BEC: 6612           bne 18
3BEE: 0C03 0004      cmpib #4, d3
3BF2: 640C           bcc 12
3BF4: 0C04 0004      cmpib #4, d4
3BF8: 6406           bcc 6
3BFA: 7000           moveq #0, d0
3BFC: 6000 0202      bra 514
3C00: 0C2A 0002 000E cmpib.ex #2, 14(a2)
3C06: 6218           bhi 24
3C08: 0C2A 0001 000E cmpib.ex #1, 14(a2)
3C0E: 6510           bcs 16
3C10: 0C2B 0007 000E cmpib.ex #7, 14(a3)
3C16: 6528           bcs 40
3C18: 0C2B 0008 000E cmpib.ex #8, 14(a3)
3C1E: 6220           bhi 32
3C20: 0C2B 0002 000E cmpib.ex #2, 14(a3)
3C26: 621E           bhi 30
3C28: 0C2B 0001 000E cmpib.ex #1, 14(a3)
3C2E: 6516           bcs 22
3C30: 0C2A 0007 000E cmpib.ex #7, 14(a2)
3C36: 6508           bcs 8
3C38: 0C2A 0008 000E cmpib.ex #8, 14(a2)
3C3E: 6306           bls 6
3C40: 7000           moveq #0, d0
3C42: 6000 01BC      bra 444
3C46: 0C2B 000C 000E cmpib.ex #12, 14(a3)
3C4C: 6400 0090      bcc 144
3C50: 7E00           moveq #0, d7
3C52: 0C2A 0006 000E cmpib.ex #6, 14(a2)
3C58: 53C0           sls d0
3C5A: 4400           negb d0
3C5C: 4880           extw d0
3C5E: 6720           beq 32
3C60: 3D7C 0001 FFFE movew.emx #1, -2(a6)
3C66: 082A 0007 001E btst #7, 30(a2)
3C6C: 660A           bne 10
3C6E: 4A2A 001C      tstb.ex 28(a2)
3C72: 6604           bne 4
3C74: 426E FFFE      clrw.ex -2(a6)
3C78: 4A6E FFFE      tstw.ex -2(a6)
3C7C: 6702           beq 2
3C7E: 7E01           moveq #1, d7
3C80: 3007           movew d7, d0
3C82: 57C0           seq d0
3C84: 4400           negb d0
3C86: 4880           extw d0
3C88: 1A00           moveb d0, d5
3C8A: 4A05           tstb d5
3C8C: 6626           bne 38
3C8E: 0C2A 0006 000E cmpib.ex #6, 14(a2)
3C94: 621E           bhi 30
3C96: 4A2A 001C      tstb.ex 28(a2)
3C9A: 6704           beq 4
3C9C: 532A 001C      subqb.ex #1, 28(a2)
3CA0: 2F0B           movel a3, -(a7)
3CA2: 2F0A           movel a2, -(a7)
3CA4: 4EBA D442      jsr.ex -31504(pc)
3CA8: 1F3C 0006      moveb.ex #6, -(a7)
3CAC: 4EBA CE1E      jsr.ex 32468(pc)
3CB0: 4FEF 000A      lea 10(a7), d7
3CB4: 0C2A 0006 000E cmpib.ex #6, 14(a2)
3CBA: 6224           bhi 36
3CBC: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
3CC2: 641C           bcc 28
3CC4: 7000           moveq #0, d0
3CC6: 102A 0014      moveb.ex 20(a2), d0
3CCA: C1FC 001E      muls.ex #30, d0
3CCE: 41ED F414      lea -3052(a5), d0
3CD2: 41E8 004C      lea 76(a0), d0
3CD6: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
3CDC: 6002           bra 2
3CDE: 7A00           moveq #0, d5
3CE0: 0C2A 000C 000E cmpib.ex #12, 14(a2)
3CE6: 6400 0098      bcc 152
3CEA: 426E FFFC      clrw.ex -4(a6)
3CEE: 0C2B 0006 000E cmpib.ex #6, 14(a3)
3CF4: 53C0           sls d0
3CF6: 4400           negb d0
3CF8: 4880           extw d0
3CFA: 6724           beq 36
3CFC: 3D7C 0001 FFFA movew.emx #1, -6(a6)
3D02: 082B 0007 001E btst #7, 30(a3)
3D08: 660A           bne 10
3D0A: 4A2B 001C      tstb.ex 28(a3)
3D0E: 6604           bne 4
3D10: 426E FFFA      clrw.ex -6(a6)
3D14: 4A6E FFFA      tstw.ex -6(a6)
3D18: 6706           beq 6
3D1A: 3D7C 0001 FFFC movew.emx #1, -4(a6)
3D20: 302E FFFC      movew.ex -4(a6), d0
3D24: 57C0           seq d0
3D26: 4400           negb d0
3D28: 4880           extw d0
3D2A: 1C00           moveb d0, d6
3D2C: 4A06           tstb d6
3D2E: 6626           bne 38
3D30: 0C2B 0006 000E cmpib.ex #6, 14(a3)
3D36: 621E           bhi 30
3D38: 4A2B 001C      tstb.ex 28(a3)
3D3C: 6704           beq 4
3D3E: 532B 001C      subqb.ex #1, 28(a3)
3D42: 2F0A           movel a2, -(a7)
3D44: 2F0B           movel a3, -(a7)
3D46: 4EBA D3A0      jsr.ex -31504(pc)
3D4A: 1F3C 0006      moveb.ex #6, -(a7)
3D4E: 4EBA CD7C      jsr.ex 32468(pc)
3D52: 4FEF 000A      lea 10(a7), d7
3D56: 0C2B 0006 000E cmpib.ex #6, 14(a3)
3D5C: 6224           bhi 36
3D5E: 0C2B 0004 0014 cmpib.ex #4, 20(a3)
3D64: 641C           bcc 28
3D66: 7000           moveq #0, d0
3D68: 102B 0014      moveb.ex 20(a3), d0
3D6C: C1FC 001E      muls.ex #30, d0
3D70: 41ED F414      lea -3052(a5), d0
3D74: 41E8 004C      lea 76(a0), d0
3D78: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
3D7E: 6002           bra 2
3D80: 7C00           moveq #0, d6
3D82: 0C03 0004      cmpib #4, d3
3D86: 640E           bcc 14
3D88: 4A06           tstb d6
3D8A: 670A           beq 10
3D8C: 1F03           moveb d3, -(a7)
3D8E: 2F0B           movel a3, -(a7)
3D90: 4EBA FC9E      jsr.ex -20936(pc)
3D94: 5C4F           addqw #6, a7
3D96: 4A05           tstb d5
3D98: 672C           beq 44
3D9A: 0C2A 000C 000E cmpib.ex #12, 14(a2)
3DA0: 6514           bcs 20
3DA2: 0C2A 0011 000E cmpib.ex #17, 14(a2)
3DA8: 620C           bhi 12
3DAA: 2F0A           movel a2, -(a7)
3DAC: 2F0B           movel a3, -(a7)
3DAE: 4EBA EB44      jsr.ex -25348(pc)
3DB2: 504F           addqw #0, a7
3DB4: 6010           bra 16
3DB6: 0C04 0004      cmpib #4, d4
3DBA: 640A           bcc 10
3DBC: 1F04           moveb d4, -(a7)
3DBE: 2F0A           movel a2, -(a7)
3DC0: 4EBA FC6E      jsr.ex -20936(pc)
3DC4: 5C4F           addqw #6, a7
3DC6: 4A05           tstb d5
3DC8: 670A           beq 10
3DCA: 2F0B           movel a3, -(a7)
3DCC: 2F0A           movel a2, -(a7)
3DCE: 4EBA FD02      jsr.ex -20774(pc)
3DD2: 504F           addqw #0, a7
3DD4: 4A06           tstb d6
3DD6: 671A           beq 26
3DD8: 0C2A 000C 000E cmpib.ex #12, 14(a2)
3DDE: 6508           bcs 8
3DE0: 0C2A 0011 000E cmpib.ex #17, 14(a2)
3DE6: 630A           bls 10
3DE8: 2F0A           movel a2, -(a7)
3DEA: 2F0B           movel a3, -(a7)
3DEC: 4EBA FCE4      jsr.ex -20774(pc)
3DF0: 504F           addqw #0, a7
3DF2: 4A05           tstb d5
3DF4: 6608           bne 8
3DF6: 4A06           tstb d6
3DF8: 6604           bne 4
3DFA: 7000           moveq #0, d0
3DFC: 6002           bra 2
3DFE: 7001           moveq #1, d0
3E00: 4CDF 1CF8      movem (a7)+, <1cf8>
3E04: 4E5E           unlk a6
3E06: 4E75           rts
3E08: 9143           subxrw d3, d0
3E0A: 6F6C           ble 108
3E0C: 6C69           bge 105
3E0E: 6465           bcc 101
3E10: 5477 6F4F      addqw.ex #2, 79(a7,d6.l)
3E14: 626A           bhi 106
3E16: 6563           bcs 99
3E18: 7473           moveq #115, d2
3E1A: 0000           dc.w #0
3E1C: 4E56 0000      link a6, #0
3E20: 48E7 1F30      movem <1f30>, -(a7)
3E24: 266E 0008      movel.ex 8(a6), a3
3E28: 246E 000C      movel.ex 12(a6), a2
3E2C: 1E2E 0010      moveb.ex 16(a6), d7
3E30: B7CA           cmpal a2, a3
3E32: 6614           bne 20
3E34: 486D FF9A      pea.ex -102(a5)
3E38: 3F3C 143B      movew.ex #5179, -(a7)
3E3C: 486D FDEA      pea.ex -534(a5)
3E40: 4E4F           trap #15
3E42: A084           sysTrapErrDisplayFileLineMsg
3E44: 4FEF 000A      lea 10(a7), d7
3E48: 4A2B 000C      tstb.ex 12(a3)
3E4C: 6614           bne 20
3E4E: 486D FFC6      pea.ex -58(a5)
3E52: 3F3C 143C      movew.ex #5180, -(a7)
3E56: 486D FDEA      pea.ex -534(a5)
3E5A: 4E4F           trap #15
3E5C: A084           sysTrapErrDisplayFileLineMsg
3E5E: 4FEF 000A      lea 10(a7), d7
3E62: 4A2A 000C      tstb.ex 12(a2)
3E66: 6606           bne 6
3E68: 7000           moveq #0, d0
3E6A: 6000 00A0      bra 160
3E6E: 302B 0004      movew.ex 4(a3), d0
3E72: 9053           subrw (a3), d0
3E74: D06A 0004      addrw.ex 4(a2), d0
3E78: 9052           subrw (a2), d0
3E7A: 3A00           movew d0, d5
3E7C: E04D           lsriw #0, d5
3E7E: EE4D           lsriw #7, d5
3E80: DA40           addrw d0, d5
3E82: E245           asriw #1, d5
3E84: 362A 0004      movew.ex 4(a2), d3
3E88: D652           addrw (a2), d3
3E8A: 322B 0004      movew.ex 4(a3), d1
3E8E: D253           addrw (a3), d1
3E90: 9243           subrw d3, d1
3E92: 3601           movew d1, d3
3E94: E04B           lsriw #0, d3
3E96: EE4B           lsriw #7, d3
3E98: D641           addrw d1, d3
3E9A: E243           asriw #1, d3
3E9C: 4A43           tstw d3
3E9E: 6C02           bge 2
3EA0: 4443           negw d3
3EA2: B645           cmpw d5, d3
3EA4: 6C64           bge 100
3EA6: 302B 0006      movew.ex 6(a3), d0
3EAA: 906B 0002      subrw.ex 2(a3), d0
3EAE: D06A 0006      addrw.ex 6(a2), d0
3EB2: 906A 0002      subrw.ex 2(a2), d0
3EB6: 3C00           movew d0, d6
3EB8: E04E           lsriw #0, d6
3EBA: EE4E           lsriw #7, d6
3EBC: DC40           addrw d0, d6
3EBE: E246           asriw #1, d6
3EC0: 382A 0006      movew.ex 6(a2), d4
3EC4: D86A 0002      addrw.ex 2(a2), d4
3EC8: 322B 0006      movew.ex 6(a3), d1
3ECC: D26B 0002      addrw.ex 2(a3), d1
3ED0: 9244           subrw d4, d1
3ED2: 3801           movew d1, d4
3ED4: E04C           lsriw #0, d4
3ED6: EE4C           lsriw #7, d4
3ED8: D841           addrw d1, d4
3EDA: E244           asriw #1, d4
3EDC: 4A44           tstw d4
3EDE: 6C02           bge 2
3EE0: 4444           negw d4
3EE2: B846           cmpw d6, d4
3EE4: 6C24           bge 36
3EE6: 3005           movew d5, d0
3EE8: D046           addrw d6, d0
3EEA: 0640 FFC0      addiw #-64, d0
3EEE: 3203           movew d3, d1
3EF0: D244           addrw d4, d1
3EF2: B240           cmpw d0, d1
3EF4: 6C14           bge 20
3EF6: 4A07           tstb d7
3EF8: 6704           beq 4
3EFA: 7001           moveq #1, d0
3EFC: 600E           bra 14
3EFE: 2F0A           movel a2, -(a7)
3F00: 2F0B           movel a3, -(a7)
3F02: 4EBA FC6A      jsr.ex -20618(pc)
3F06: 504F           addqw #0, a7
3F08: 6002           bra 2
3F0A: 7000           moveq #0, d0
3F0C: 4CDF 0CF8      movem (a7)+, <0cf8>
3F10: 4E5E           unlk a6
3F12: 4E75           rts
3F14: 9B43           subxrw d3, d5
3F16: 6865           bvc 101
3F18: 636B           bls 107
3F1A: 466F 7243      notw.ex 29251(a7)
3F1E: 6F6C           ble 108
3F20: 6C69           bge 105
3F22: 7369           dc.w #29545
3F24: 6F6E           ble 110
3F26: 5769 7468      subqw.ex #3, 29800(a1)
3F2A: 4F62           dc.w #20322
3F2C: 6A65           bpl 101
3F2E: 6374           bls 116
3F30: 0000           dc.w #0
3F32: 4E56 0000      link a6, #0
3F36: 48E7 1830      movem <1830>, -(a7)
3F3A: 266E 0008      movel.ex 8(a6), a3
3F3E: 182E 000C      moveb.ex 12(a6), d4
3F42: 162E 000E      moveb.ex 14(a6), d3
3F46: 0C2B 0009 000E cmpib.ex #9, 14(a3)
3F4C: 650C           bcs 12
3F4E: 0C2B 000B 000E cmpib.ex #11, 14(a3)
3F54: 6204           bhi 4
3F56: 7000           moveq #0, d0
3F58: 607A           bra 122
3F5A: 0C2B 000C 000E cmpib.ex #12, 14(a3)
3F60: 6542           bcs 66
3F62: 0C2B 0011 000E cmpib.ex #17, 14(a3)
3F68: 623A           bhi 58
3F6A: 7000           moveq #0, d0
3F6C: 1004           moveb d4, d0
3F6E: E588           lslil #2, d0
3F70: 41ED F17C      lea -3716(a5), d0
3F74: 2470 0800      movel.ex 0(a0,d0.l), a2
3F78: 601A           bra 26
3F7A: 1F03           moveb d3, -(a7)
3F7C: 2F0A           movel a2, -(a7)
3F7E: 2F0B           movel a3, -(a7)
3F80: 4EBA FE9A      jsr.ex -19932(pc)
3F84: 4A00           tstb d0
3F86: 4FEF 000A      lea 10(a7), d7
3F8A: 6704           beq 4
3F8C: 7001           moveq #1, d0
3F8E: 6044           bra 68
3F90: 246A 0010      movel.ex 16(a2), a2
3F94: 200A           movel a2, d0
3F96: 6708           beq 8
3F98: 0C2A 0006 000E cmpib.ex #6, 14(a2)
3F9E: 63DA           bls -38
3FA0: 7000           moveq #0, d0
3FA2: 6030           bra 48
3FA4: 7000           moveq #0, d0
3FA6: 1004           moveb d4, d0
3FA8: E588           lslil #2, d0
3FAA: 41ED F17C      lea -3716(a5), d0
3FAE: 2470 0800      movel.ex 0(a0,d0.l), a2
3FB2: 601A           bra 26
3FB4: 1F03           moveb d3, -(a7)
3FB6: 2F0A           movel a2, -(a7)
3FB8: 2F0B           movel a3, -(a7)
3FBA: 4EBA FE60      jsr.ex -19932(pc)
3FBE: 4A00           tstb d0
3FC0: 4FEF 000A      lea 10(a7), d7
3FC4: 6704           beq 4
3FC6: 7001           moveq #1, d0
3FC8: 600A           bra 10
3FCA: 246A 0010      movel.ex 16(a2), a2
3FCE: 200A           movel a2, d0
3FD0: 66E2           bne -30
3FD2: 7000           moveq #0, d0
3FD4: 4CDF 0C18      movem (a7)+, <0c18>
3FD8: 4E5E           unlk a6
3FDA: 4E75           rts
3FDC: 8022           orrb -(a2), d0
3FDE: 4368           dc.w #17256
3FE0: 6563           bcs 99
3FE2: 6B46           bmi 70
3FE4: 6F72           ble 114
3FE6: 436F           dc.w #17263
3FE8: 6C6C           bge 108
3FEA: 6973           bvs 115
3FEC: 696F           bvs 111
3FEE: 6E57           bgt 87
3FF0: 6974           bvs 116
3FF2: 6853           bvc 83
3FF4: 6563           bcs 99
3FF6: 746F           moveq #111, d2
3FF8: 724F           moveq #79, d1
3FFA: 626A           bhi 106
3FFC: 6563           bcs 99
3FFE: 7473           moveq #115, d2
4000: 0000           dc.w #0
4002: 4E56 FFF4      link a6, #-12
4006: 48E7 1F20      movem <1f20>, -(a7)
400A: 246E 0008      movel.ex 8(a6), a2
400E: 1A2E 000C      moveb.ex 12(a6), d5
4012: 7600           moveq #0, d3
4014: 4A2A 000C      tstb.ex 12(a2)
4018: 6614           bne 20
401A: 486D FFDA      pea.ex -38(a5)
401E: 3F3C 14CF      movew.ex #5327, -(a7)
4022: 486D FDEA      pea.ex -534(a5)
4026: 4E4F           trap #15
4028: A084           sysTrapErrDisplayFileLineMsg
402A: 4FEF 000A      lea 10(a7), d7
402E: 3012           movew (a2), d0
4030: D06A 0004      addrw.ex 4(a2), d0
4034: 0640 FE00      addiw #-512, d0
4038: 3200           movew d0, d1
403A: E049           lsriw #0, d1
403C: EE49           lsriw #7, d1
403E: D240           addrw d0, d1
4040: E241           asriw #1, d1
4042: 3D41 FFF8      movew.mx d1, -8(a6)
4046: 302A 0002      movew.ex 2(a2), d0
404A: D06A 0006      addrw.ex 6(a2), d0
404E: 0640 FE00      addiw #-512, d0
4052: 3200           movew d0, d1
4054: E049           lsriw #0, d1
4056: EE49           lsriw #7, d1
4058: D240           addrw d0, d1
405A: E241           asriw #1, d1
405C: 3D41 FFFA      movew.mx d1, -6(a6)
4060: 302E FFF8      movew.ex -8(a6), d0
4064: 0640 0100      addiw #256, d0
4068: 3D40 FFFC      movew.mx d0, -4(a6)
406C: 302E FFFA      movew.ex -6(a6), d0
4070: 0640 0100      addiw #256, d0
4074: 3D40 FFFE      movew.mx d0, -2(a6)
4078: 486E FFF8      pea.ex -8(a6)
407C: 4EBA CC4C      jsr.ex -32558(pc)
4080: 3F2E FFFA      movew.ex -6(a6), -(a7)
4084: 3F2E FFF8      movew.ex -8(a6), -(a7)
4088: 4EBA CE0E      jsr.ex -32096(pc)
408C: 1C00           moveb d0, d6
408E: 3F2E FFFE      movew.ex -2(a6), -(a7)
4092: 3F2E FFFC      movew.ex -4(a6), -(a7)
4096: 4EBA CE00      jsr.ex -32096(pc)
409A: 1E00           moveb d0, d7
409C: BC07           cmpb d7, d6
409E: 4FEF 000C      lea 12(a7), d7
40A2: 6614           bne 20
40A4: 1806           moveb d6, d4
40A6: 1F05           moveb d5, -(a7)
40A8: 1F04           moveb d4, -(a7)
40AA: 2F0A           movel a2, -(a7)
40AC: 4EBA FE84      jsr.ex -19654(pc)
40B0: 1600           moveb d0, d3
40B2: 504F           addqw #0, a7
40B4: 6000 0184      bra 388
40B8: 3F2E FFFE      movew.ex -2(a6), -(a7)
40BC: 3F2E FFF8      movew.ex -8(a6), -(a7)
40C0: 4EBA CDD6      jsr.ex -32096(pc)
40C4: 1D40 FFF7      moveb.mx d0, -9(a6)
40C8: BC00           cmpb d0, d6
40CA: 584F           addqw #4, a7
40CC: 665C           bne 92
40CE: 3F2E FFFE      movew.ex -2(a6), -(a7)
40D2: 302E FFF8      movew.ex -8(a6), d0
40D6: D06E FFFC      addrw.ex -4(a6), d0
40DA: 3200           movew d0, d1
40DC: E049           lsriw #0, d1
40DE: EE49           lsriw #7, d1
40E0: D240           addrw d0, d1
40E2: E241           asriw #1, d1
40E4: 3F01           movew d1, -(a7)
40E6: 4EBA CDB0      jsr.ex -32096(pc)
40EA: 1800           moveb d0, d4
40EC: 1F05           moveb d5, -(a7)
40EE: 1F04           moveb d4, -(a7)
40F0: 2F0A           movel a2, -(a7)
40F2: 4EBA FE3E      jsr.ex -19654(pc)
40F6: 1600           moveb d0, d3
40F8: 4A03           tstb d3
40FA: 4FEF 000C      lea 12(a7), d7
40FE: 6600 013A      bne 314
4102: B806           cmpb d6, d4
4104: 56C0           sne d0
4106: 4400           negb d0
4108: 4880           extw d0
410A: 6706           beq 6
410C: 1D46 FFF5      moveb.mx d6, -11(a6)
4110: 6004           bra 4
4112: 1D47 FFF5      moveb.mx d7, -11(a6)
4116: 1F05           moveb d5, -(a7)
4118: 1F2E FFF5      moveb.ex -11(a6), -(a7)
411C: 2F0A           movel a2, -(a7)
411E: 4EBA FE12      jsr.ex -19654(pc)
4122: 1600           moveb d0, d3
4124: 504F           addqw #0, a7
4126: 6000 0112      bra 274
412A: BE2E FFF7      cmpb.ex -9(a6), d7
412E: 665C           bne 92
4130: 302E FFFA      movew.ex -6(a6), d0
4134: D06E FFFE      addrw.ex -2(a6), d0
4138: 3200           movew d0, d1
413A: E049           lsriw #0, d1
413C: EE49           lsriw #7, d1
413E: D240           addrw d0, d1
4140: E241           asriw #1, d1
4142: 3F01           movew d1, -(a7)
4144: 3F2E FFF8      movew.ex -8(a6), -(a7)
4148: 4EBA CD4E      jsr.ex -32096(pc)
414C: 1800           moveb d0, d4
414E: 1F05           moveb d5, -(a7)
4150: 1F04           moveb d4, -(a7)
4152: 2F0A           movel a2, -(a7)
4154: 4EBA FDDC      jsr.ex -19654(pc)
4158: 1600           moveb d0, d3
415A: 4A03           tstb d3
415C: 4FEF 000C      lea 12(a7), d7
4160: 6600 00D8      bne 216
4164: B806           cmpb d6, d4
4166: 56C0           sne d0
4168: 4400           negb d0
416A: 4880           extw d0
416C: 6706           beq 6
416E: 1D46 FFF4      moveb.mx d6, -12(a6)
4172: 6004           bra 4
4174: 1D47 FFF4      moveb.mx d7, -12(a6)
4178: 1F05           moveb d5, -(a7)
417A: 1F2E FFF4      moveb.ex -12(a6), -(a7)
417E: 2F0A           movel a2, -(a7)
4180: 4EBA FDB0      jsr.ex -19654(pc)
4184: 1600           moveb d0, d3
4186: 504F           addqw #0, a7
4188: 6000 00B0      bra 176
418C: 3F2E FFFA      movew.ex -6(a6), -(a7)
4190: 3F2E FFFC      movew.ex -4(a6), -(a7)
4194: 4EBA CD02      jsr.ex -32096(pc)
4198: 1D40 FFF6      moveb.mx d0, -10(a6)
419C: 302E FFFA      movew.ex -6(a6), d0
41A0: D06E FFFE      addrw.ex -2(a6), d0
41A4: 3200           movew d0, d1
41A6: E049           lsriw #0, d1
41A8: EE49           lsriw #7, d1
41AA: D240           addrw d0, d1
41AC: E241           asriw #1, d1
41AE: 3F01           movew d1, -(a7)
41B0: 302E FFF8      movew.ex -8(a6), d0
41B4: D06E FFFC      addrw.ex -4(a6), d0
41B8: 3200           movew d0, d1
41BA: E049           lsriw #0, d1
41BC: EE49           lsriw #7, d1
41BE: D240           addrw d0, d1
41C0: E241           asriw #1, d1
41C2: 3F01           movew d1, -(a7)
41C4: 4EBA CCD2      jsr.ex -32096(pc)
41C8: 1800           moveb d0, d4
41CA: 1F05           moveb d5, -(a7)
41CC: 1F04           moveb d4, -(a7)
41CE: 2F0A           movel a2, -(a7)
41D0: 4EBA FD60      jsr.ex -19654(pc)
41D4: 1600           moveb d0, d3
41D6: 4A03           tstb d3
41D8: 4FEF 0010      lea 16(a7), d7
41DC: 6612           bne 18
41DE: B806           cmpb d6, d4
41E0: 670E           beq 14
41E2: 1F05           moveb d5, -(a7)
41E4: 1F06           moveb d6, -(a7)
41E6: 2F0A           movel a2, -(a7)
41E8: 4EBA FD48      jsr.ex -19654(pc)
41EC: 1600           moveb d0, d3
41EE: 504F           addqw #0, a7
41F0: 4A03           tstb d3
41F2: 6616           bne 22
41F4: B82E FFF6      cmpb.ex -10(a6), d4
41F8: 6710           beq 16
41FA: 1F05           moveb d5, -(a7)
41FC: 1F2E FFF6      moveb.ex -10(a6), -(a7)
4200: 2F0A           movel a2, -(a7)
4202: 4EBA FD2E      jsr.ex -19654(pc)
4206: 1600           moveb d0, d3
4208: 504F           addqw #0, a7
420A: 4A03           tstb d3
420C: 6616           bne 22
420E: B82E FFF7      cmpb.ex -9(a6), d4
4212: 6710           beq 16
4214: 1F05           moveb d5, -(a7)
4216: 1F2E FFF7      moveb.ex -9(a6), -(a7)
421A: 2F0A           movel a2, -(a7)
421C: 4EBA FD14      jsr.ex -19654(pc)
4220: 1600           moveb d0, d3
4222: 504F           addqw #0, a7
4224: 4A03           tstb d3
4226: 6612           bne 18
4228: B807           cmpb d7, d4
422A: 670E           beq 14
422C: 1F05           moveb d5, -(a7)
422E: 1F07           moveb d7, -(a7)
4230: 2F0A           movel a2, -(a7)
4232: 4EBA FCFE      jsr.ex -19654(pc)
4236: 1600           moveb d0, d3
4238: 504F           addqw #0, a7
423A: 4A03           tstb d3
423C: 6622           bne 34
423E: 4A05           tstb d5
4240: 661A           bne 26
4242: 7000           moveq #0, d0
4244: 1004           moveb d4, d0
4246: E588           lslil #2, d0
4248: 41ED F17C      lea -3716(a5), d0
424C: 2570 0800 0010 movel.emx 0(a0,d0.l), 16(a2)
4252: 7000           moveq #0, d0
4254: 1004           moveb d4, d0
4256: E588           lslil #2, d0
4258: 218A 0800      movel.mx a2, 0(a0,d0.l)
425C: 7001           moveq #1, d0
425E: 6002           bra 2
4260: 7000           moveq #0, d0
4262: 4CDF 04F8      movem (a7)+, <04f8>
4266: 4E5E           unlk a6
4268: 4E75           rts
426A: 8F53           ormw d7, (a3)
426C: 6563           bcs 99
426E: 746F           moveq #111, d2
4270: 7241           moveq #65, d1
4272: 6464           bcc 100
4274: 4F62           dc.w #20322
4276: 6A65           bpl 101
4278: 6374           bls 116
427A: 0000           dc.w #0
427C: 4E56 0000      link a6, #0
4280: 48E7 1830      movem <1830>, -(a7)
4284: 182E 0008      moveb.ex 8(a6), d4
4288: 0C04 0004      cmpib #4, d4
428C: 55C0           scs d0
428E: 4400           negb d0
4290: 4880           extw d0
4292: 1600           moveb d0, d3
4294: 47ED F414      lea -3052(a5), d3
4298: 47EB 043C      lea 1084(a3), d3
429C: 7000           moveq #0, d0
429E: 102D F4D7      moveb.ex -2857(a5), d0
42A2: C1FC 0014      muls.ex #20, d0
42A6: 45ED F414      lea -3052(a5), d2
42AA: D5C0           addal d0, a2
42AC: 45EA 0428      lea 1064(a2), d2
42B0: 6022           bra 34
42B2: 4A2A 000C      tstb.ex 12(a2)
42B6: 6718           beq 24
42B8: 4A03           tstb d3
42BA: 670A           beq 10
42BC: 1F04           moveb d4, -(a7)
42BE: 2F0A           movel a2, -(a7)
42C0: 4EBA F76E      jsr.ex -20936(pc)
42C4: 5C4F           addqw #6, a7
42C6: 2F0A           movel a2, -(a7)
42C8: 2F0A           movel a2, -(a7)
42CA: 4EBA D836      jsr.ex -28918(pc)
42CE: 504F           addqw #0, a7
42D0: 7014           moveq #20, d0
42D2: 95C0           subal d0, a2
42D4: B5CB           cmpal a3, a2
42D6: 64DA           bcc -38
42D8: 4CDF 0C18      movem (a7)+, <0c18>
42DC: 4E5E           unlk a6
42DE: 4E75           rts
42E0: 8E52           orrw (a2), d7
42E2: 6F63           ble 99
42E4: 6B45           bmi 69
42E6: 7870           moveq #112, d4
42E8: 6C6F           bge 111
42EA: 6465           bcc 101
42EC: 416C           dc.w #16748
42EE: 6C00 0000      bge 0
42F2: 4E56 0000      link a6, #0
42F6: 48E7 1830      movem <1830>, -(a7)
42FA: 162E 0008      moveb.ex 8(a6), d3
42FE: 0C03 0004      cmpib #4, d3
4302: 55C0           scs d0
4304: 4400           negb d0
4306: 4880           extw d0
4308: 1800           moveb d0, d4
430A: 7000           moveq #0, d0
430C: 102D F4D0      moveb.ex -2864(a5), d0
4310: C1FC 0026      muls.ex #38, d0
4314: 45ED F414      lea -3052(a5), d2
4318: D5C0           addal d0, a2
431A: 45EA 00A4      lea 164(a2), d2
431E: 47ED F414      lea -3052(a5), d3
4322: 47EB 00CA      lea 202(a3), d3
4326: 6000 0094      bra 148
432A: B62A 0014      cmpb.ex 20(a2), d3
432E: 6700 0088      beq 136
4332: 0C2A 0006 000E cmpib.ex #6, 14(a2)
4338: 620E           bhi 14
433A: 082A 0007 001E btst #7, 30(a2)
4340: 6676           bne 118
4342: 4A2A 001C      tstb.ex 28(a2)
4346: 6670           bne 112
4348: 4A2A 000C      tstb.ex 12(a2)
434C: 676A           beq 106
434E: 4A2A 001C      tstb.ex 28(a2)
4352: 6730           beq 48
4354: 532A 001C      subqb.ex #1, 28(a2)
4358: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
435E: 6418           bcc 24
4360: 7000           moveq #0, d0
4362: 102A 0014      moveb.ex 20(a2), d0
4366: C1FC 001E      muls.ex #30, d0
436A: 41ED F414      lea -3052(a5), d0
436E: 41E8 004C      lea 76(a0), d0
4372: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
4378: 2F0A           movel a2, -(a7)
437A: 2F0A           movel a2, -(a7)
437C: 4EBA CD6A      jsr.ex -31504(pc)
4380: 504F           addqw #0, a7
4382: 6014           bra 20
4384: 1F03           moveb d3, -(a7)
4386: 2F0A           movel a2, -(a7)
4388: 4EBA F6A6      jsr.ex -20936(pc)
438C: 2F0A           movel a2, -(a7)
438E: 2F0A           movel a2, -(a7)
4390: 4EBA E3B8      jsr.ex -25774(pc)
4394: 4FEF 000E      lea 14(a7), d7
4398: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
439E: 6418           bcc 24
43A0: 7000           moveq #0, d0
43A2: 102A 0014      moveb.ex 20(a2), d0
43A6: C1FC 001E      muls.ex #30, d0
43AA: 41ED F414      lea -3052(a5), d0
43AE: 41E8 004C      lea 76(a0), d0
43B2: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
43B8: 7026           moveq #38, d0
43BA: 95C0           subal d0, a2
43BC: B5CB           cmpal a3, a2
43BE: 6400 FF6A      bcc 65386
43C2: 4CDF 0C18      movem (a7)+, <0c18>
43C6: 4E5E           unlk a6
43C8: 4E75           rts
43CA: 8E53           orrw (a3), d7
43CC: 6869           bvc 105
43CE: 7045           moveq #69, d0
43D0: 7870           moveq #112, d4
43D2: 6C6F           bge 111
43D4: 6465           bcc 101
43D6: 416C           dc.w #16748
43D8: 6C00 0000      bge 0
43DC: 4E56 0000      link a6, #0
43E0: 48E7 1830      movem <1830>, -(a7)
43E4: 182E 0008      moveb.ex 8(a6), d4
43E8: 0C04 0004      cmpib #4, d4
43EC: 55C0           scs d0
43EE: 4400           negb d0
43F0: 4880           extw d0
43F2: 1600           moveb d0, d3
43F4: 47ED F414      lea -3052(a5), d3
43F8: 47EB 0304      lea 772(a3), d3
43FC: 7000           moveq #0, d0
43FE: 102D F4CE      moveb.ex -2866(a5), d0
4402: C1FC 001A      muls.ex #26, d0
4406: 45ED F414      lea -3052(a5), d2
440A: D5C0           addal d0, a2
440C: 45EA 02EA      lea 746(a2), d2
4410: 6020           bra 32
4412: 4A2A 000C      tstb.ex 12(a2)
4416: 6716           beq 22
4418: 4A03           tstb d3
441A: 670A           beq 10
441C: 1F04           moveb d4, -(a7)
441E: 2F0A           movel a2, -(a7)
4420: 4EBA F60E      jsr.ex -20936(pc)
4424: 5C4F           addqw #6, a7
4426: 2F0A           movel a2, -(a7)
4428: 4EBA EC28      jsr.ex -23462(pc)
442C: 584F           addqw #4, a7
442E: 701A           moveq #26, d0
4430: 95C0           subal d0, a2
4432: B5CB           cmpal a3, a2
4434: 64DC           bcc -36
4436: 4CDF 0C18      movem (a7)+, <0c18>
443A: 4E5E           unlk a6
443C: 4E75           rts
443E: 8E53           orrw (a3), d7
4440: 686F           bvc 111
4442: 7445           moveq #69, d2
4444: 7870           moveq #112, d4
4446: 6C6F           bge 111
4448: 6465           bcc 101
444A: 416C           dc.w #16748
444C: 6C00 0000      bge 0
4450: 4E56 0000      link a6, #0
4454: 1F2D F440      moveb.ex -3008(a5), -(a7)
4458: 4EBA FE22      jsr.ex -18812(pc)
445C: 1F2D F440      moveb.ex -3008(a5), -(a7)
4460: 4EBA FE90      jsr.ex -18694(pc)
4464: 1F2D F440      moveb.ex -3008(a5), -(a7)
4468: 4EBA FF72      jsr.ex -18460(pc)
446C: 4E5E           unlk a6
446E: 4E75           rts
4470: 8B42           dc.w #35650
4472: 6F6D           ble 109
4474: 6245           bhi 69
4476: 7870           moveq #112, d4
4478: 6C6F           bge 111
447A: 6465           bcc 101
447C: 0000           dc.w #0
447E: 4E56 FFFE      link a6, #-2
4482: 48E7 1E30      movem <1e30>, -(a7)
4486: 7000           moveq #0, d0
4488: 102D F4C8      moveb.ex -2872(a5), d0
448C: C1FC 001E      muls.ex #30, d0
4490: 41ED F414      lea -3052(a5), d0
4494: 41E8 004E      lea 78(a0), d0
4498: 1830 0800      moveb.ex 0(a0,d0.l), d4
449C: 1A2D F4C8      moveb.ex -2872(a5), d5
44A0: 4A04           tstb d4
44A2: 6742           beq 66
44A4: 163C 00FF      moveb.ex #255, d3
44A8: 45ED F414      lea -3052(a5), d2
44AC: 45EA 00CA      lea 202(a2), d2
44B0: 7000           moveq #0, d0
44B2: 102D F4D0      moveb.ex -2864(a5), d0
44B6: C1FC 0026      muls.ex #38, d0
44BA: 47ED F414      lea -3052(a5), d3
44BE: D7C0           addal d0, a3
44C0: 47EB 00A4      lea 164(a3), d3
44C4: 601A           bra 26
44C6: BA2A 0014      cmpb.ex 20(a2), d5
44CA: 6610           bne 16
44CC: B62A 001C      cmpb.ex 28(a2), d3
44D0: 6304           bls 4
44D2: 162A 001C      moveb.ex 28(a2), d3
44D6: 5304           subqb #1, d4
44D8: 4A04           tstb d4
44DA: 670C           beq 12
44DC: 7026           moveq #38, d0
44DE: D5C0           addal d0, a2
44E0: B5CB           cmpal a3, a2
44E2: 63E2           bls -30
44E4: 6002           bra 2
44E6: 7600           moveq #0, d3
44E8: 4A03           tstb d3
44EA: 675A           beq 90
44EC: 2F3C 0058 0001 movel.exl #5767169, -(a7)
44F2: 2F2D F370      movel.ex -3216(a5), -(a7)
44F6: 4E4F           trap #15
44F8: A226           sysTrapWinDrawBitmap
44FA: 0C03 000A      cmpib #10, d3
44FE: 504F           addqw #0, a7
4500: 6432           bcc 50
4502: 7000           moveq #0, d0
4504: 1003           moveb d3, d0
4506: 0640 0030      addiw #48, d0
450A: 1D40 FFFF      moveb.mx d0, -1(a6)
450E: 4227           clrb -(a7)
4510: 4E4F           trap #15
4512: A164           sysTrapFntSetFont
4514: 1C00           moveb d0, d6
4516: 3F3C 0001      movew.ex #1, -(a7)
451A: 2F3C 0001 005B movel.exl #65627, -(a7)
4520: 486E FFFF      pea.ex -1(a6)
4524: 4E4F           trap #15
4526: A22A           sysTrapWinDrawInvertedChars
4528: 1F06           moveb d6, -(a7)
452A: 4E4F           trap #15
452C: A164           sysTrapFntSetFont
452E: 4FEF 000E      lea 14(a7), d7
4532: 603C           bra 60
4534: 2F3C 0058 0001 movel.exl #5767169, -(a7)
453A: 2F2D F378      movel.ex -3208(a5), -(a7)
453E: 4E4F           trap #15
4540: A226           sysTrapWinDrawBitmap
4542: 504F           addqw #0, a7
4544: 602A           bra 42
4546: 2F3C 0058 0001 movel.exl #5767169, -(a7)
454C: 2F2D F374      movel.ex -3212(a5), -(a7)
4550: 4E4F           trap #15
4552: A226           sysTrapWinDrawBitmap
4554: 7000           moveq #0, d0
4556: 1005           moveb d5, d0
4558: C1FC 001E      muls.ex #30, d0
455C: 41ED F414      lea -3052(a5), d0
4560: 7200           moveq #0, d1
4562: 1230 084A      moveb.ex 74(a0,d0.l), d1
4566: 3F01           movew d1, -(a7)
4568: 4EBA 0024      jsr.ex -18026(pc)
456C: 4FEF 000A      lea 10(a7), d7
4570: 4CDF 0C78      movem (a7)+, <0c78>
4574: 4E5E           unlk a6
4576: 4E75           rts
4578: 9247           subrw d7, d1
457A: 616D           bsr 109
457C: 6544           bcs 68
457E: 7261           moveq #97, d1
4580: 7741           dc.w #30529
4582: 726D           moveq #109, d1
4584: 6F72           ble 114
4586: 4761           dc.w #18273
4588: 7567           dc.w #30055
458A: 6500 0000      bcs 0
458E: 4E56 FFF8      link a6, #-8
4592: 48E7 1C00      movem <1c00>, -(a7)
4596: 3A2E 0008      movew.ex 8(a6), d5
459A: 3D7C 0004 FFFA movew.emx #4, -6(a6)
45A0: 3D7C 0007 FFFC movew.emx #7, -4(a6)
45A6: 3D7C 0007 FFFE movew.emx #7, -2(a6)
45AC: 7600           moveq #0, d3
45AE: 6030           bra 48
45B0: 7809           moveq #9, d4
45B2: C9C3           muls d3, d4
45B4: 0644 0035      addiw #53, d4
45B8: BA43           cmpw d3, d5
45BA: 6F12           ble 18
45BC: 3F3C 0004      movew.ex #4, -(a7)
45C0: 3F04           movew d4, -(a7)
45C2: 2F2D F36C      movel.ex -3220(a5), -(a7)
45C6: 4E4F           trap #15
45C8: A226           sysTrapWinDrawBitmap
45CA: 504F           addqw #0, a7
45CC: 6010           bra 16
45CE: 3D44 FFF8      movew.mx d4, -8(a6)
45D2: 4267           clrw -(a7)
45D4: 486E FFF8      pea.ex -8(a6)
45D8: 4E4F           trap #15
45DA: A219           sysTrapWinEraseRectangle
45DC: 5C4F           addqw #6, a7
45DE: 5243           addqw #1, d3
45E0: 0C43 0005      cmpiw #5, d3
45E4: 6DCA           blt -54
45E6: 4CDF 0038      movem (a7)+, <0038>
45EA: 4E5E           unlk a6
45EC: 4E75           rts
45EE: 9247           subrw d7, d1
45F0: 616D           bsr 109
45F2: 6544           bcs 68
45F4: 7261           moveq #97, d1
45F6: 774C           dc.w #30540
45F8: 6976           bvs 118
45FA: 6573           bcs 115
45FC: 4761           dc.w #18273
45FE: 7567           dc.w #30055
4600: 6500 0000      bcs 0
4604: 4E56 FFFA      link a6, #-6
4608: 48E7 1C00      movem <1c00>, -(a7)
460C: 2A2E 0008      movel.ex 8(a6), d5
4610: 1F3C 0001      moveb.ex #1, -(a7)
4614: 4E4F           trap #15
4616: A164           sysTrapFntSetFont
4618: 1600           moveb d0, d3
461A: 4A85           tstl d5
461C: 544F           addqw #2, a7
461E: 6F0E           ble 14
4620: 2F05           movel d5, -(a7)
4622: 486E FFFA      pea.ex -6(a6)
4626: 4E4F           trap #15
4628: A0C9           sysTrapStrIToA
462A: 504F           addqw #0, a7
462C: 6030           bra 48
462E: 1F3C 0080      moveb.ex #128, -(a7)
4632: 4878 0005      pea.ex (0005).w
4636: 486E FFFA      pea.ex -6(a6)
463A: 4E4F           trap #15
463C: A027           sysTrapMemSet
463E: 3F3C 0002      movew.ex #2, -(a7)
4642: 2F3C 0005 0083 movel.exl #327811, -(a7)
4648: 486E FFFA      pea.ex -6(a6)
464C: 4E4F           trap #15
464E: A220           sysTrapWinDrawChars
4650: 1D7C 0030 FFFA moveb.emx #48, -6(a6)
4656: 422E FFFB      clrb.ex -5(a6)
465A: 4FEF 0014      lea 20(a7), d7
465E: 486E FFFA      pea.ex -6(a6)
4662: 4E4F           trap #15
4664: A0C7           sysTrapStrLen
4666: 3800           movew d0, d4
4668: 2F3C 0083 0002 movel.exl #8585218, -(a7)
466E: 3F04           movew d4, -(a7)
4670: 486E FFFA      pea.ex -6(a6)
4674: 4E4F           trap #15
4676: A220           sysTrapWinDrawChars
4678: 1F03           moveb d3, -(a7)
467A: 4E4F           trap #15
467C: A164           sysTrapFntSetFont
467E: 4FEF 0010      lea 16(a7), d7
4682: 4CDF 0038      movem (a7)+, <0038>
4686: 4E5E           unlk a6
4688: 4E75           rts
468A: 9247           subrw d7, d1
468C: 616D           bsr 109
468E: 6544           bcs 68
4690: 7261           moveq #97, d1
4692: 7753           dc.w #30547
4694: 636F           bls 111
4696: 7265           moveq #101, d1
4698: 4761           dc.w #18273
469A: 7567           dc.w #30055
469C: 6500 0000      bcs 0
46A0: 4E56 0000      link a6, #0
46A4: 48E7 1800      movem <1800>, -(a7)
46A8: 362E 0008      movew.ex 8(a6), d3
46AC: 0C43 0019      cmpiw #25, d3
46B0: 6610           bne 16
46B2: 3F3C 03EB      movew.ex #1003, -(a7)
46B6: 486D F226      pea.ex -3546(a5)
46BA: 4E4F           trap #15
46BC: A0BC           sysTrapSysCopyStringResource
46BE: 5C4F           addqw #6, a7
46C0: 6060           bra 96
46C2: 0C43 0005      cmpiw #5, d3
46C6: 6718           beq 24
46C8: 0C43 000A      cmpiw #10, d3
46CC: 6712           beq 18
46CE: 0C43 000F      cmpiw #15, d3
46D2: 670C           beq 12
46D4: 0C43 0014      cmpiw #20, d3
46D8: 6706           beq 6
46DA: 0C43 0019      cmpiw #25, d3
46DE: 6610           bne 16
46E0: 3F3C 03EA      movew.ex #1002, -(a7)
46E4: 486D F226      pea.ex -3546(a5)
46E8: 4E4F           trap #15
46EA: A0BC           sysTrapSysCopyStringResource
46EC: 5C4F           addqw #6, a7
46EE: 6032           bra 50
46F0: 3F3C 03E9      movew.ex #1001, -(a7)
46F4: 486D F226      pea.ex -3546(a5)
46F8: 4E4F           trap #15
46FA: A0BC           sysTrapSysCopyStringResource
46FC: 3003           movew d3, d0
46FE: 5240           addqw #1, d0
4700: 48C0           extl d0
4702: 2F00           movel d0, -(a7)
4704: 486D F226      pea.ex -3546(a5)
4708: 4E4F           trap #15
470A: A0C7           sysTrapStrLen
470C: 7200           moveq #0, d1
470E: 3200           movew d0, d1
4710: 41ED F226      lea -3546(a5), d0
4714: 584F           addqw #4, a7
4716: 4870 1800      pea.ex 0(a0,d1.l)
471A: 4E4F           trap #15
471C: A0C9           sysTrapStrIToA
471E: 4FEF 000E      lea 14(a7), d7
4722: 486D F226      pea.ex -3546(a5)
4726: 4E4F           trap #15
4728: A0C7           sysTrapStrLen
472A: 1B40 F225      moveb.mx d0, -3547(a5)
472E: 1F3C 0002      moveb.ex #2, -(a7)
4732: 4E4F           trap #15
4734: A164           sysTrapFntSetFont
4736: 1800           moveb d0, d4
4738: 7000           moveq #0, d0
473A: 102D F225      moveb.ex -3547(a5), d0
473E: 3F00           movew d0, -(a7)
4740: 486D F226      pea.ex -3546(a5)
4744: 4E4F           trap #15
4746: A16B           sysTrapFntCharsWidth
4748: 5340           subqw #1, d0
474A: 3B40 F21A      movew.mx d0, -3558(a5)
474E: 4E4F           trap #15
4750: A168           sysTrapFntLineHeight
4752: 5340           subqw #1, d0
4754: 3B40 F218      movew.mx d0, -3560(a5)
4758: 303C 00A0      movew.ex #160, d0
475C: 906D F21A      subrw.ex -3558(a5), d0
4760: 3200           movew d0, d1
4762: E049           lsriw #0, d1
4764: EE49           lsriw #7, d1
4766: D240           addrw d0, d1
4768: E241           asriw #1, d1
476A: 3B41 F21C      movew.mx d1, -3556(a5)
476E: 3B7C 0024 F21E movew.emx #36, -3554(a5)
4774: 302D F21C      movew.ex -3556(a5), d0
4778: D06D F21A      addrw.ex -3558(a5), d0
477C: 3B40 F220      movew.mx d0, -3552(a5)
4780: 302D F21E      movew.ex -3554(a5), d0
4784: D06D F218      addrw.ex -3560(a5), d0
4788: 3B40 F222      movew.mx d0, -3550(a5)
478C: 1B7C 0028 F217 moveb.emx #40, -3561(a5)
4792: 1F04           moveb d4, -(a7)
4794: 4E4F           trap #15
4796: A164           sysTrapFntSetFont
4798: 4FEF 000E      lea 14(a7), d7
479C: 4CDF 0018      movem (a7)+, <0018>
47A0: 4E5E           unlk a6
47A2: 4E75           rts
47A4: 9347           subxrw d7, d1
47A6: 616D           bsr 109
47A8: 6553           bcs 83
47AA: 6574           bcs 116
47AC: 4C65           dc.w #19557
47AE: 7665           moveq #101, d3
47B0: 6C4D           bge 77
47B2: 6573           bcs 115
47B4: 7361           dc.w #29537
47B6: 6765           beq 101
47B8: 0000           dc.w #0
47BA: 4E56 FFF4      link a6, #-12
47BE: 48E7 1F00      movem <1f00>, -(a7)
47C2: 302D F42C      movew.ex -3028(a5), d0
47C6: B06D F434      cmpw.ex -3020(a5), d0
47CA: 5DC0           slt d0
47CC: 4400           negb d0
47CE: 4880           extw d0
47D0: 6706           beq 6
47D2: 382D F42C      movew.ex -3028(a5), d4
47D6: 6004           bra 4
47D8: 382D F434      movew.ex -3020(a5), d4
47DC: 3D44 FFF8      movew.mx d4, -8(a6)
47E0: 4A44           tstw d4
47E2: 5EC0           sgt d0
47E4: 4400           negb d0
47E6: 4880           extw d0
47E8: 6704           beq 4
47EA: 3A04           movew d4, d5
47EC: 6002           bra 2
47EE: 7A00           moveq #0, d5
47F0: 3D45 FFF8      movew.mx d5, -8(a6)
47F4: 302D F42E      movew.ex -3026(a5), d0
47F8: B06D F436      cmpw.ex -3018(a5), d0
47FC: 5DC0           slt d0
47FE: 4400           negb d0
4800: 4880           extw d0
4802: 6706           beq 6
4804: 3C2D F42E      movew.ex -3026(a5), d6
4808: 6004           bra 4
480A: 3C2D F436      movew.ex -3018(a5), d6
480E: 3D46 FFFA      movew.mx d6, -6(a6)
4812: 4A46           tstw d6
4814: 5EC0           sgt d0
4816: 4400           negb d0
4818: 4880           extw d0
481A: 6704           beq 4
481C: 3E06           movew d6, d7
481E: 6002           bra 2
4820: 7E00           moveq #0, d7
4822: 3D47 FFFA      movew.mx d7, -6(a6)
4826: 302D F430      movew.ex -3024(a5), d0
482A: B06D F438      cmpw.ex -3016(a5), d0
482E: 5EC0           sgt d0
4830: 4400           negb d0
4832: 4880           extw d0
4834: 6708           beq 8
4836: 3D6D F430 FFF6 movew.emx -3024(a5), -10(a6)
483C: 6006           bra 6
483E: 3D6D F438 FFF6 movew.emx -3016(a5), -10(a6)
4844: 302E FFF6      movew.ex -10(a6), d0
4848: 906E FFF8      subrw.ex -8(a6), d0
484C: 5240           addqw #1, d0
484E: 3D40 FFFC      movew.mx d0, -4(a6)
4852: 302D F432      movew.ex -3022(a5), d0
4856: B06D F43A      cmpw.ex -3014(a5), d0
485A: 5EC0           sgt d0
485C: 4400           negb d0
485E: 4880           extw d0
4860: 6708           beq 8
4862: 3D6D F432 FFF4 movew.emx -3022(a5), -12(a6)
4868: 6006           bra 6
486A: 3D6D F43A FFF4 movew.emx -3014(a5), -12(a6)
4870: 302E FFF4      movew.ex -12(a6), d0
4874: 906E FFFA      subrw.ex -6(a6), d0
4878: 5240           addqw #1, d0
487A: 3D40 FFFE      movew.mx d0, -2(a6)
487E: 362E FFF8      movew.ex -8(a6), d3
4882: 0243 000F      andiw #15, d3
4886: 4A43           tstw d3
4888: 6708           beq 8
488A: 976E FFF8      submw.ex d3, -8(a6)
488E: D76E FFFC      addmw.ex d3, -4(a6)
4892: 700F           moveq #15, d0
4894: D06E FFFC      addrw.ex -4(a6), d0
4898: 0240 FFF0      andiw #-16, d0
489C: 3D40 FFFC      movew.mx d0, -4(a6)
48A0: 4227           clrb -(a7)
48A2: 700F           moveq #15, d0
48A4: D06E FFFA      addrw.ex -6(a6), d0
48A8: 3F00           movew d0, -(a7)
48AA: 3F2E FFF8      movew.ex -8(a6), -(a7)
48AE: 486E FFF8      pea.ex -8(a6)
48B2: 42A7           clrl -(a7)
48B4: 2F2D F428      movel.ex -3032(a5), -(a7)
48B8: 4E4F           trap #15
48BA: A209           sysTrapWinCopyRectangle
48BC: 7000           moveq #0, d0
48BE: 102D F4C8      moveb.ex -2872(a5), d0
48C2: C1FC 001E      muls.ex #30, d0
48C6: 41ED F414      lea -3052(a5), d0
48CA: 41E8 0044      lea 68(a0), d0
48CE: 4A30 0800      tstb.ex 0(a0,d0.l)
48D2: 4FEF 0012      lea 18(a7), d7
48D6: 672E           beq 46
48D8: 7000           moveq #0, d0
48DA: 102D F4C8      moveb.ex -2872(a5), d0
48DE: C1FC 001E      muls.ex #30, d0
48E2: 41ED F414      lea -3052(a5), d0
48E6: 2F30 0840      movel.ex 64(a0,d0.l), -(a7)
48EA: 4EBA FD18      jsr.ex -17908(pc)
48EE: 7000           moveq #0, d0
48F0: 102D F4C8      moveb.ex -2872(a5), d0
48F4: C1FC 001E      muls.ex #30, d0
48F8: 41ED F414      lea -3052(a5), d0
48FC: 41E8 0044      lea 68(a0), d0
4900: 4230 0800      clrb.ex 0(a0,d0.l)
4904: 584F           addqw #4, a7
4906: 7000           moveq #0, d0
4908: 102D F4C8      moveb.ex -2872(a5), d0
490C: C1FC 001E      muls.ex #30, d0
4910: 41ED F414      lea -3052(a5), d0
4914: 41E8 004B      lea 75(a0), d0
4918: 4A30 0800      tstb.ex 0(a0,d0.l)
491C: 6618           bne 24
491E: 7000           moveq #0, d0
4920: 102D F4C8      moveb.ex -2872(a5), d0
4924: C1FC 001E      muls.ex #30, d0
4928: 41ED F414      lea -3052(a5), d0
492C: 41E8 004C      lea 76(a0), d0
4930: 4A30 0800      tstb.ex 0(a0,d0.l)
4934: 674C           beq 76
4936: 7000           moveq #0, d0
4938: 102D F4C8      moveb.ex -2872(a5), d0
493C: C1FC 001E      muls.ex #30, d0
4940: 41ED F414      lea -3052(a5), d0
4944: 7200           moveq #0, d1
4946: 1230 084A      moveb.ex 74(a0,d0.l), d1
494A: 3F01           movew d1, -(a7)
494C: 4EBA FC40      jsr.ex -18026(pc)
4950: 7000           moveq #0, d0
4952: 102D F4C8      moveb.ex -2872(a5), d0
4956: C1FC 001E      muls.ex #30, d0
495A: 41ED F414      lea -3052(a5), d0
495E: 41E8 004B      lea 75(a0), d0
4962: 4230 0800      clrb.ex 0(a0,d0.l)
4966: 4EBA FB16      jsr.ex -18298(pc)
496A: 7000           moveq #0, d0
496C: 102D F4C8      moveb.ex -2872(a5), d0
4970: C1FC 001E      muls.ex #30, d0
4974: 41ED F414      lea -3052(a5), d0
4978: 41E8 004C      lea 76(a0), d0
497C: 4230 0800      clrb.ex 0(a0,d0.l)
4980: 544F           addqw #2, a7
4982: 4CDF 00F8      movem (a7)+, <00f8>
4986: 4E5E           unlk a6
4988: 4E75           rts
498A: 8D47           dc.w #36167
498C: 616D           bsr 109
498E: 6553           bcs 83
4990: 7461           moveq #97, d2
4992: 7465           moveq #101, d2
4994: 4472 6177      negw.ex 119(a2,d6.w)
4998: 0000           dc.w #0
499A: 4E56 0000      link a6, #0
499E: 7000           moveq #0, d0
49A0: 102D F4C8      moveb.ex -2872(a5), d0
49A4: C1FC 001E      muls.ex #30, d0
49A8: 41ED F414      lea -3052(a5), d0
49AC: 41E8 0044      lea 68(a0), d0
49B0: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
49B6: 7000           moveq #0, d0
49B8: 102D F4C8      moveb.ex -2872(a5), d0
49BC: C1FC 001E      muls.ex #30, d0
49C0: 41ED F414      lea -3052(a5), d0
49C4: 41E8 004B      lea 75(a0), d0
49C8: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
49CE: 7000           moveq #0, d0
49D0: 102D F4C8      moveb.ex -2872(a5), d0
49D4: C1FC 001E      muls.ex #30, d0
49D8: 41ED F414      lea -3052(a5), d0
49DC: 41E8 004C      lea 76(a0), d0
49E0: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
49E6: 4EBA FDD2      jsr.ex -17470(pc)
49EA: 4E5E           unlk a6
49EC: 4E75           rts
49EE: 9747           subxrw d7, d3
49F0: 616D           bsr 109
49F2: 6553           bcs 83
49F4: 7461           moveq #97, d2
49F6: 7465           moveq #101, d2
49F8: 4472 6177      negw.ex 119(a2,d6.w)
49FC: 4576           dc.w #17782
49FE: 6572           bcs 114
4A00: 7974           dc.w #31092
4A02: 6869           bvc 105
4A04: 6E67           bgt 103
4A06: 0000           dc.w #0
4A08: 4E56 FFD8      link a6, #-40
4A0C: 48E7 1F38      movem <1f38>, -(a7)
4A10: 4878 0008      pea.ex (0008).w
4A14: 486D F434      pea.ex -3020(a5)
4A18: 486D F42C      pea.ex -3028(a5)
4A1C: 4E4F           trap #15
4A1E: A026           sysTrapMemMove
4A20: 3B7C 00A0 F434 movew.emx #160, -3020(a5)
4A26: 3B7C 0091 F436 movew.emx #145, -3018(a5)
4A2C: 426D F438      clrw.ex -3016(a5)
4A30: 426D F43A      clrw.ex -3014(a5)
4A34: 2F2D F428      movel.ex -3032(a5), -(a7)
4A38: 4E4F           trap #15
4A3A: A1FD           sysTrapWinSetDrawWindow
4A3C: 2D48 FFDA      movel.mx a0, -38(a6)
4A40: 4A2D F43F      tstb.ex -3009(a5)
4A44: 4FEF 0010      lea 16(a7), d7
4A48: 6740           beq 64
4A4A: 426E FFF8      clrw.ex -8(a6)
4A4E: 426E FFFA      clrw.ex -6(a6)
4A52: 3D7C 00A0 FFFC movew.emx #160, -4(a6)
4A58: 3D7C 0091 FFFE movew.emx #145, -2(a6)
4A5E: 486D FDE2      pea.ex -542(a5)
4A62: 4E4F           trap #15
4A64: A224           sysTrapWinSetPattern
4A66: 4267           clrw -(a7)
4A68: 486E FFF8      pea.ex -8(a6)
4A6C: 4E4F           trap #15
4A6E: A229           sysTrapWinFillRectangle
4A70: 4878 0008      pea.ex (0008).w
4A74: 486E FFF8      pea.ex -8(a6)
4A78: 486D F434      pea.ex -3020(a5)
4A7C: 4E4F           trap #15
4A7E: A026           sysTrapMemMove
4A80: 422D F43F      clrb.ex -3009(a5)
4A84: 4FEF 0016      lea 22(a7), d7
4A88: 6034           bra 52
4A8A: 3D6D F42C FFF8 movew.emx -3028(a5), -8(a6)
4A90: 3D6D F42E FFFA movew.emx -3026(a5), -6(a6)
4A96: 302D F430      movew.ex -3024(a5), d0
4A9A: 906D F42C      subrw.ex -3028(a5), d0
4A9E: 5240           addqw #1, d0
4AA0: 3D40 FFFC      movew.mx d0, -4(a6)
4AA4: 302D F432      movew.ex -3022(a5), d0
4AA8: 906D F42E      subrw.ex -3026(a5), d0
4AAC: 5240           addqw #1, d0
4AAE: 3D40 FFFE      movew.mx d0, -2(a6)
4AB2: 4267           clrw -(a7)
4AB4: 486E FFF8      pea.ex -8(a6)
4AB8: 4E4F           trap #15
4ABA: A219           sysTrapWinEraseRectangle
4ABC: 5C4F           addqw #6, a7
4ABE: 4A2D F225      tstb.ex -3547(a5)
4AC2: 6744           beq 68
4AC4: 1F3C 0002      moveb.ex #2, -(a7)
4AC8: 4E4F           trap #15
4ACA: A164           sysTrapFntSetFont
4ACC: 1D40 FFD9      moveb.mx d0, -39(a6)
4AD0: 3F2D F21E      movew.ex -3554(a5), -(a7)
4AD4: 3F2D F21C      movew.ex -3556(a5), -(a7)
4AD8: 7000           moveq #0, d0
4ADA: 102D F225      moveb.ex -3547(a5), d0
4ADE: 3F00           movew d0, -(a7)
4AE0: 486D F226      pea.ex -3546(a5)
4AE4: 4E4F           trap #15
4AE6: A222           sysTrapWinInvertChars
4AE8: 3F2D F222      movew.ex -3550(a5), -(a7)
4AEC: 3F2D F220      movew.ex -3552(a5), -(a7)
4AF0: 3F2D F21E      movew.ex -3554(a5), -(a7)
4AF4: 3F2D F21C      movew.ex -3556(a5), -(a7)
4AF8: 4EBA BE4C      jsr.ex 32078(pc)
4AFC: 1F2E FFD9      moveb.ex -39(a6), -(a7)
4B00: 4E4F           trap #15
4B02: A164           sysTrapFntSetFont
4B04: 4FEF 0016      lea 22(a7), d7
4B08: 7000           moveq #0, d0
4B0A: 102D F4DC      moveb.ex -2852(a5), d0
4B0E: E988           lslil #4, d0
4B10: 47ED F414      lea -3052(a5), d3
4B14: D7C0           addal d0, a3
4B16: 47EB 0856      lea 2134(a3), d3
4B1A: 41ED F414      lea -3052(a5), d0
4B1E: 41E8 0866      lea 2150(a0), d0
4B22: 2D48 FFE2      movel.mx a0, -30(a6)
4B26: 6074           bra 116
4B28: 3C13           movew (a3), d6
4B2A: E646           asriw #3, d6
4B2C: E04E           lsriw #0, d6
4B2E: E84E           lsriw #4, d6
4B30: DC53           addrw (a3), d6
4B32: E846           asriw #4, d6
4B34: 0646 FFF0      addiw #-16, d6
4B38: 3E2B 0002      movew.ex 2(a3), d7
4B3C: E647           asriw #3, d7
4B3E: E04F           lsriw #0, d7
4B40: E84F           lsriw #4, d7
4B42: DE6B 0002      addrw.ex 2(a3), d7
4B46: E847           asriw #4, d7
4B48: 0647 FFF0      addiw #-16, d7
4B4C: 3F07           movew d7, -(a7)
4B4E: 3F06           movew d6, -(a7)
4B50: 7000           moveq #0, d0
4B52: 102B 000F      moveb.ex 15(a3), d0
4B56: 3F00           movew d0, -(a7)
4B58: 486B 0009      pea.ex 9(a3)
4B5C: 4E4F           trap #15
4B5E: A222           sysTrapWinInvertChars
4B60: 302B 0006      movew.ex 6(a3), d0
4B64: E640           asriw #3, d0
4B66: E048           lsriw #0, d0
4B68: E848           lsriw #4, d0
4B6A: D06B 0006      addrw.ex 6(a3), d0
4B6E: E840           asriw #4, d0
4B70: 0640 FFF0      addiw #-16, d0
4B74: 3F00           movew d0, -(a7)
4B76: 302B 0004      movew.ex 4(a3), d0
4B7A: E640           asriw #3, d0
4B7C: E048           lsriw #0, d0
4B7E: E848           lsriw #4, d0
4B80: D06B 0004      addrw.ex 4(a3), d0
4B84: E840           asriw #4, d0
4B86: 0640 FFF0      addiw #-16, d0
4B8A: 3F00           movew d0, -(a7)
4B8C: 3F07           movew d7, -(a7)
4B8E: 3F06           movew d6, -(a7)
4B90: 4EBA BDB4      jsr.ex 32078(pc)
4B94: 7010           moveq #16, d0
4B96: 97C0           subal d0, a3
4B98: 4FEF 0012      lea 18(a7), d7
4B9C: B7EE FFE2      cmpal.ex -30(a6), a3
4BA0: 6486           bcc -122
4BA2: 7000           moveq #0, d0
4BA4: 102D F4D0      moveb.ex -2864(a5), d0
4BA8: C1FC 0026      muls.ex #38, d0
4BAC: 45ED F414      lea -3052(a5), d2
4BB0: D5C0           addal d0, a2
4BB2: 45EA 00A4      lea 164(a2), d2
4BB6: 41ED F414      lea -3052(a5), d0
4BBA: 41E8 00CA      lea 202(a0), d0
4BBE: 2D48 FFF2      movel.mx a0, -14(a6)
4BC2: 6000 00D0      bra 208
4BC6: 4A2A 000C      tstb.ex 12(a2)
4BCA: 6700 00C4      beq 196
4BCE: 082A 0005 001E btst #5, 30(a2)
4BD4: 6600 00BA      bne 186
4BD8: 082A 0004 001E btst #4, 30(a2)
4BDE: 6600 00B0      bne 176
4BE2: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
4BE8: 6414           bcc 20
4BEA: 082A 0004 0020 btst #4, 32(a2)
4BF0: 6600 009E      bne 158
4BF4: 082A 0003 0020 btst #3, 32(a2)
4BFA: 6600 0094      bne 148
4BFE: 0C2A 0002 000E cmpib.ex #2, 14(a2)
4C04: 55C0           scs d0
4C06: 4400           negb d0
4C08: 4880           extw d0
4C0A: 670C           beq 12
4C0C: 102A 0018      moveb.ex 24(a2), d0
4C10: 4880           extw d0
4C12: 3D40 FFF6      movew.mx d0, -10(a6)
4C16: 600E           bra 14
4C18: 7000           moveq #0, d0
4C1A: 102A 000E      moveb.ex 14(a2), d0
4C1E: 0640 000F      addiw #15, d0
4C22: 3D40 FFF6      movew.mx d0, -10(a6)
4C26: 1F3C 0004      moveb.ex #4, -(a7)
4C2A: 3F2A 0002      movew.ex 2(a2), -(a7)
4C2E: 3F12           movew (a2), -(a7)
4C30: 3F2E FFF6      movew.ex -10(a6), -(a7)
4C34: 4EBA BD6E      jsr.ex 32172(pc)
4C38: 0C2D 0001 F43E cmpib.ex #1, -3010(a5)
4C3E: 504F           addqw #0, a7
4C40: 634E           bls 78
4C42: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
4C48: 6446           bcc 70
4C4A: 7000           moveq #0, d0
4C4C: 102A 0014      moveb.ex 20(a2), d0
4C50: 0640 0031      addiw #49, d0
4C54: 1D40 FFD8      moveb.mx d0, -40(a6)
4C58: 302A 0002      movew.ex 2(a2), d0
4C5C: E640           asriw #3, d0
4C5E: E048           lsriw #0, d0
4C60: E848           lsriw #4, d0
4C62: D06A 0002      addrw.ex 2(a2), d0
4C66: E840           asriw #4, d0
4C68: 0640 FFF1      addiw #-15, d0
4C6C: 3F00           movew d0, -(a7)
4C6E: 3012           movew (a2), d0
4C70: E640           asriw #3, d0
4C72: E048           lsriw #0, d0
4C74: E848           lsriw #4, d0
4C76: D052           addrw (a2), d0
4C78: E840           asriw #4, d0
4C7A: 0640 FFF3      addiw #-13, d0
4C7E: 3F00           movew d0, -(a7)
4C80: 3F3C 0001      movew.ex #1, -(a7)
4C84: 486E FFD8      pea.ex -40(a6)
4C88: 4E4F           trap #15
4C8A: A222           sysTrapWinInvertChars
4C8C: 4FEF 000A      lea 10(a7), d7
4C90: 7026           moveq #38, d0
4C92: 95C0           subal d0, a2
4C94: B5EE FFF2      cmpal.ex -14(a6), a2
4C98: 6400 FF2C      bcc 65324
4C9C: 7000           moveq #0, d0
4C9E: 102D F4CE      moveb.ex -2866(a5), d0
4CA2: C1FC 001A      muls.ex #26, d0
4CA6: 49ED F414      lea -3052(a5), d4
4CAA: D9C0           addal d0, a4
4CAC: 49EC 02EA      lea 746(a4), d4
4CB0: 41ED F414      lea -3052(a5), d0
4CB4: 41E8 0304      lea 772(a0), d0
4CB8: 2D48 FFEE      movel.mx a0, -18(a6)
4CBC: 6024           bra 36
4CBE: 4A2C 000C      tstb.ex 12(a4)
4CC2: 671A           beq 26
4CC4: 4227           clrb -(a7)
4CC6: 3F2C 0002      movew.ex 2(a4), -(a7)
4CCA: 3F14           movew (a4), -(a7)
4CCC: 7000           moveq #0, d0
4CCE: 102C 000E      moveb.ex 14(a4), d0
4CD2: 0640 0012      addiw #18, d0
4CD6: 3F00           movew d0, -(a7)
4CD8: 4EBA BCCA      jsr.ex 32172(pc)
4CDC: 504F           addqw #0, a7
4CDE: 701A           moveq #26, d0
4CE0: 99C0           subal d0, a4
4CE2: B9EE FFEE      cmpal.ex -18(a6), a4
4CE6: 64D6           bcc -42
4CE8: 7000           moveq #0, d0
4CEA: 102D F4D7      moveb.ex -2857(a5), d0
4CEE: C1FC 0014      muls.ex #20, d0
4CF2: 41ED F414      lea -3052(a5), d0
4CF6: D1C0           addal d0, a0
4CF8: 41E8 0428      lea 1064(a0), d0
4CFC: 2608           movel a0, d3
4CFE: 41ED F414      lea -3052(a5), d0
4D02: 41E8 043C      lea 1084(a0), d0
4D06: 2D48 FFEA      movel.mx a0, -22(a6)
4D0A: 6028           bra 40
4D0C: 2043           movel d3, a0
4D0E: 4A28 000C      tstb.ex 12(a0)
4D12: 671C           beq 28
4D14: 1F3C 0004      moveb.ex #4, -(a7)
4D18: 3F28 0002      movew.ex 2(a0), -(a7)
4D1C: 3F10           movew (a0), -(a7)
4D1E: 7000           moveq #0, d0
4D20: 1028 000E      moveb.ex 14(a0), d0
4D24: 0640 000D      addiw #13, d0
4D28: 3F00           movew d0, -(a7)
4D2A: 4EBA BC78      jsr.ex 32172(pc)
4D2E: 504F           addqw #0, a7
4D30: 7014           moveq #20, d0
4D32: 9680           subrl d0, d3
4D34: B6AE FFEA      cmpl.ex -22(a6), d3
4D38: 64D2           bcc -46
4D3A: 7000           moveq #0, d0
4D3C: 102D F4D9      moveb.ex -2855(a5), d0
4D40: C1FC 0016      muls.ex #22, d0
4D44: 41ED F414      lea -3052(a5), d0
4D48: D1C0           addal d0, a0
4D4A: 41E8 067E      lea 1662(a0), d0
4D4E: 2808           movel a0, d4
4D50: 41ED F414      lea -3052(a5), d0
4D54: 41E8 0694      lea 1684(a0), d0
4D58: 2D48 FFDE      movel.mx a0, -34(a6)
4D5C: 6028           bra 40
4D5E: 2044           movel d4, a0
4D60: 4A28 000C      tstb.ex 12(a0)
4D64: 671C           beq 28
4D66: 1F3C 0004      moveb.ex #4, -(a7)
4D6A: 3F28 0002      movew.ex 2(a0), -(a7)
4D6E: 3F10           movew (a0), -(a7)
4D70: 7000           moveq #0, d0
4D72: 1028 000E      moveb.ex 14(a0), d0
4D76: 0640 0010      addiw #16, d0
4D7A: 3F00           movew d0, -(a7)
4D7C: 4EBA BC26      jsr.ex 32172(pc)
4D80: 504F           addqw #0, a7
4D82: 7016           moveq #22, d0
4D84: 9880           subrl d0, d4
4D86: B8AE FFDE      cmpl.ex -34(a6), d4
4D8A: 64D2           bcc -46
4D8C: 7000           moveq #0, d0
4D8E: 102D F4DB      moveb.ex -2853(a5), d0
4D92: C1FC 000A      muls.ex #10, d0
4D96: 41ED F414      lea -3052(a5), d0
4D9A: D1C0           addal d0, a0
4D9C: 41E8 06CC      lea 1740(a0), d0
4DA0: 2A08           movel a0, d5
4DA2: 41ED F414      lea -3052(a5), d0
4DA6: 41E8 06D6      lea 1750(a0), d0
4DAA: 2D48 FFE6      movel.mx a0, -26(a6)
4DAE: 6012           bra 18
4DB0: 2045           movel d5, a0
4DB2: 3F28 0002      movew.ex 2(a0), -(a7)
4DB6: 3F10           movew (a0), -(a7)
4DB8: 4EBA BCC2      jsr.ex 32388(pc)
4DBC: 700A           moveq #10, d0
4DBE: 9A80           subrl d0, d5
4DC0: 584F           addqw #4, a7
4DC2: BAAE FFE6      cmpl.ex -26(a6), d5
4DC6: 64E8           bcc -24
4DC8: 2F2E FFDA      movel.ex -38(a6), -(a7)
4DCC: 4E4F           trap #15
4DCE: A1FD           sysTrapWinSetDrawWindow
4DD0: 584F           addqw #4, a7
4DD2: 4CDF 1CF8      movem (a7)+, <1cf8>
4DD6: 4E5E           unlk a6
4DD8: 4E75           rts
4DDA: 9447           subrw d7, d2
4DDC: 616D           bsr 109
4DDE: 6553           bcs 83
4DE0: 7461           moveq #97, d2
4DE2: 7465           moveq #101, d2
4DE4: 5072 6570      addqw.ex #0, 112(a2,d6.w)
4DE8: 6172           bsr 114
4DEA: 6544           bcs 68
4DEC: 7261           moveq #97, d1
4DEE: 7700           dc.w #30464
4DF0: 0000           dc.w #0
4DF2: 4E56 0000      link a6, #0
4DF6: 48E7 1020      movem <1020>, -(a7)
4DFA: 0C2D 001A F4CA cmpib.ex #26, -2870(a5)
4E00: 6636           bne 54
4E02: 1B7C 0005 F414 moveb.emx #5, -3052(a5)
4E08: 7000           moveq #0, d0
4E0A: 102D F4D0      moveb.ex -2864(a5), d0
4E0E: C1FC 0026      muls.ex #38, d0
4E12: 45ED F414      lea -3052(a5), d2
4E16: D5C0           addal d0, a2
4E18: 45EA 00A4      lea 164(a2), d2
4E1C: 600A           bra 10
4E1E: 002A 0004 001E orib.ex #4, 30(a2)
4E24: 7026           moveq #38, d0
4E26: 95C0           subal d0, a2
4E28: 41ED F414      lea -3052(a5), d0
4E2C: 41E8 00CA      lea 202(a0), d0
4E30: B5C8           cmpal a0, a2
4E32: 64EA           bcc -22
4E34: 6000 009E      bra 158
4E38: 7000           moveq #0, d0
4E3A: 102D F4CA      moveb.ex -2870(a5), d0
4E3E: 3F00           movew d0, -(a7)
4E40: 4EBA F85E      jsr.ex -17752(pc)
4E44: 4EBA F974      jsr.ex -17470(pc)
4E48: 1B7C 0003 F414 moveb.emx #3, -3052(a5)
4E4E: 3B7C 0050 F4CC movew.emx #80, -2868(a5)
4E54: 0C2D 0005 F4CA cmpib.ex #5, -2870(a5)
4E5A: 544F           addqw #2, a7
4E5C: 6720           beq 32
4E5E: 0C2D 000A F4CA cmpib.ex #10, -2870(a5)
4E64: 6718           beq 24
4E66: 0C2D 000F F4CA cmpib.ex #15, -2870(a5)
4E6C: 6710           beq 16
4E6E: 0C2D 0014 F4CA cmpib.ex #20, -2870(a5)
4E74: 6708           beq 8
4E76: 0C2D 0019 F4CA cmpib.ex #25, -2870(a5)
4E7C: 6640           bne 64
4E7E: 7000           moveq #0, d0
4E80: 102D F4CA      moveb.ex -2870(a5), d0
4E84: 5A40           addqw #5, d0
4E86: 1B40 F43D      moveb.mx d0, -3011(a5)
4E8A: 422D F43C      clrb.ex -3012(a5)
4E8E: 0C2D 0019 F4CA cmpib.ex #25, -2870(a5)
4E94: 660C           bne 12
4E96: 1F3C 0002      moveb.ex #2, -(a7)
4E9A: 4EBA E640      jsr.ex -22300(pc)
4E9E: 544F           addqw #2, a7
4EA0: 6032           bra 50
4EA2: 7600           moveq #0, d3
4EA4: 162D F4CA      moveb.ex -2870(a5), d3
4EA8: 5B43           subqw #5, d3
4EAA: 600C           bra 12
4EAC: 1F3C 0003      moveb.ex #3, -(a7)
4EB0: 4EBA E62A      jsr.ex -22300(pc)
4EB4: 5B43           subqw #5, d3
4EB6: 544F           addqw #2, a7
4EB8: 4A43           tstw d3
4EBA: 6EF0           bgt -16
4EBC: 6016           bra 22
4EBE: 7000           moveq #0, d0
4EC0: 102D F4CA      moveb.ex -2870(a5), d0
4EC4: 48C0           extl d0
4EC6: 81FC 0005      divs.ex #5, d0
4ECA: 5640           addqw #3, d0
4ECC: 1B40 F43C      moveb.mx d0, -3012(a5)
4ED0: 422D F43D      clrb.ex -3011(a5)
4ED4: 4CDF 0408      movem (a7)+, <0408>
4ED8: 4E5E           unlk a6
4EDA: 4E75           rts
4EDC: 8D47           dc.w #36167
4EDE: 616D           bsr 109
4EE0: 6549           bcs 73
4EE2: 6E69           bgt 105
4EE4: 744C           moveq #76, d2
4EE6: 6576           bcs 118
4EE8: 656C           bcs 108
4EEA: 0000           dc.w #0
4EEC: 4E56 0000      link a6, #0
4EF0: 1B7C 0004 F414 moveb.emx #4, -3052(a5)
4EF6: 3B7C 0023 F4CC movew.emx #35, -2868(a5)
4EFC: 4E5E           unlk a6
4EFE: 4E75           rts
4F00: 8C47           orrw d7, d6
4F02: 616D           bsr 109
4F04: 6545           bcs 69
4F06: 6E64           bgt 100
4F08: 4C65           dc.w #19557
4F0A: 7665           moveq #101, d3
4F0C: 6C00 0000      bge 0
4F10: 4E56 0000      link a6, #0
4F14: 0C2D 0004 F414 cmpib.ex #4, -3052(a5)
4F1A: 660E           bne 14
4F1C: 4A6D F4CC      tstw.ex -2868(a5)
4F20: 6608           bne 8
4F22: 522D F4CA      addqb.ex #1, -2870(a5)
4F26: 4EBA FECA      jsr.ex -15878(pc)
4F2A: 4E5E           unlk a6
4F2C: 4E75           rts
4F2E: 9047           subrw d7, d0
4F30: 616D           bsr 109
4F32: 6553           bcs 83
4F34: 7461           moveq #97, d2
4F36: 7465           moveq #101, d2
4F38: 4164           dc.w #16740
4F3A: 7661           moveq #97, d3
4F3C: 6E63           bgt 99
4F3E: 6500 0000      bcs 0
4F42: 4E56 0000      link a6, #0
4F46: 48E7 1820      movem <1820>, -(a7)
4F4A: 4EBA B9A0      jsr.ex 31988(pc)
4F4E: 422D F41E      clrb.ex -3042(a5)
4F52: 42AD F420      clrl.ex -3040(a5)
4F56: 422D F4C8      clrb.ex -2872(a5)
4F5A: 4A2D F414      tstb.ex -3052(a5)
4F5E: 6700 0186      beq 390
4F62: 42AD F416      clrl.ex -3050(a5)
4F66: 422D FCCA      clrb.ex -822(a5)
4F6A: 422D F4CA      clrb.ex -2870(a5)
4F6E: 422D FCCC      clrb.ex -820(a5)
4F72: 422D FCCD      clrb.ex -819(a5)
4F76: 422D F43F      clrb.ex -3009(a5)
4F7A: 7600           moveq #0, d3
4F7C: 606A           bra 106
4F7E: 701E           moveq #30, d0
4F80: C1C3           muls d3, d0
4F82: 45ED F414      lea -3052(a5), d2
4F86: D5C0           addal d0, a2
4F88: 45EA 003C      lea 60(a2), d2
4F8C: 42AA 0004      clrl.ex 4(a2)
4F90: 157C 0001 0008 moveb.emx #1, 8(a2)
4F96: 257C 0000 2710 000A movel.emxl #10000, 10(a2)
4F9E: 157C 0001 0011 moveb.emx #1, 17(a2)
4FA4: 7000           moveq #0, d0
4FA6: 102A 0011      moveb.ex 17(a2), d0
4FAA: C1FC 0005      muls.ex #5, d0
4FAE: 1540 000E      moveb.mx d0, 14(a2)
4FB2: 157C 0001 000F moveb.emx #1, 15(a2)
4FB8: 157C 0001 0010 moveb.emx #1, 16(a2)
4FBE: 422A 0012      clrb.ex 18(a2)
4FC2: 422A 001C      clrb.ex 28(a2)
4FC6: 7803           moveq #3, d4
4FC8: 6014           bra 20
4FCA: 3044           movew d4, a0
4FCC: 2008           movel a0, d0
4FCE: D080           addrl d0, d0
4FD0: 204A           movel a2, a0
4FD2: 41E8 0014      lea 20(a0), d0
4FD6: 31BC FFFF 0800 movew.emx #65535, 0(a0,d0.l)
4FDC: 5344           subqw #1, d4
4FDE: 4A44           tstw d4
4FE0: 6CE8           bge -24
4FE2: 422A 0013      clrb.ex 19(a2)
4FE6: 5243           addqw #1, d3
4FE8: 0C43 0004      cmpiw #4, d3
4FEC: 6D90           blt -112
4FEE: 7600           moveq #0, d3
4FF0: 6022           bra 34
4FF2: 7026           moveq #38, d0
4FF4: C1C3           muls d3, d0
4FF6: 41ED F414      lea -3052(a5), d0
4FFA: 41E8 00D6      lea 214(a0), d0
4FFE: 4230 0800      clrb.ex 0(a0,d0.l)
5002: 7026           moveq #38, d0
5004: C1C3           muls d3, d0
5006: 41ED F414      lea -3052(a5), d0
500A: 41E8 00D7      lea 215(a0), d0
500E: 4230 0800      clrb.ex 0(a0,d0.l)
5012: 5243           addqw #1, d3
5014: 0C43 000F      cmpiw #15, d3
5018: 6DD8           blt -40
501A: 422D F4D6      clrb.ex -2858(a5)
501E: 422D F4D0      clrb.ex -2864(a5)
5022: 422D F4D1      clrb.ex -2863(a5)
5026: 422D F4D2      clrb.ex -2862(a5)
502A: 4EBA CEC6      jsr.ex -27910(pc)
502E: 7600           moveq #0, d3
5030: 6022           bra 34
5032: 701A           moveq #26, d0
5034: C1C3           muls d3, d0
5036: 41ED F414      lea -3052(a5), d0
503A: 41E8 0310      lea 784(a0), d0
503E: 4230 0800      clrb.ex 0(a0,d0.l)
5042: 701A           moveq #26, d0
5044: C1C3           muls d3, d0
5046: 41ED F414      lea -3052(a5), d0
504A: 41E8 0311      lea 785(a0), d0
504E: 4230 0800      clrb.ex 0(a0,d0.l)
5052: 5243           addqw #1, d3
5054: 0C43 000C      cmpiw #12, d3
5058: 6DD8           blt -40
505A: 422D F4CF      clrb.ex -2865(a5)
505E: 422D F4CE      clrb.ex -2866(a5)
5062: 7600           moveq #0, d3
5064: 6022           bra 34
5066: 7014           moveq #20, d0
5068: C1C3           muls d3, d0
506A: 41ED F414      lea -3052(a5), d0
506E: 41E8 0448      lea 1096(a0), d0
5072: 4230 0800      clrb.ex 0(a0,d0.l)
5076: 7014           moveq #20, d0
5078: C1C3           muls d3, d0
507A: 41ED F414      lea -3052(a5), d0
507E: 41E8 0449      lea 1097(a0), d0
5082: 4230 0800      clrb.ex 0(a0,d0.l)
5086: 5243           addqw #1, d3
5088: 0C43 001E      cmpiw #30, d3
508C: 6DD8           blt -40
508E: 422D F4D8      clrb.ex -2856(a5)
5092: 422D F4D7      clrb.ex -2857(a5)
5096: 7600           moveq #0, d3
5098: 6022           bra 34
509A: 7016           moveq #22, d0
509C: C1C3           muls d3, d0
509E: 41ED F414      lea -3052(a5), d0
50A2: 41E8 06A0      lea 1696(a0), d0
50A6: 4230 0800      clrb.ex 0(a0,d0.l)
50AA: 7016           moveq #22, d0
50AC: C1C3           muls d3, d0
50AE: 41ED F414      lea -3052(a5), d0
50B2: 41E8 06A1      lea 1697(a0), d0
50B6: 4230 0800      clrb.ex 0(a0,d0.l)
50BA: 5243           addqw #1, d3
50BC: 0C43 0003      cmpiw #3, d3
50C0: 6DD8           blt -40
50C2: 422D F4DA      clrb.ex -2854(a5)
50C6: 422D F4D9      clrb.ex -2855(a5)
50CA: 422D F4DB      clrb.ex -2853(a5)
50CE: 422D F4DC      clrb.ex -2852(a5)
50D2: 4EBA FD1E      jsr.ex -15878(pc)
50D6: 4EBA FE38      jsr.ex -15592(pc)
50DA: 4E4F           trap #15
50DC: A0F7           sysTrapTimGetTicks
50DE: 5C80           addql #6, d0
50E0: 2B40 F41A      movel.mx d0, -3046(a5)
50E4: 606A           bra 106
50E6: 1B6D F24B F414 moveb.emx -3509(a5), -3052(a5)
50EC: 4E4F           trap #15
50EE: A0F7           sysTrapTimGetTicks
50F0: 0680 0000 00B4 addil #180, d0
50F6: 2B40 F41A      movel.mx d0, -3046(a5)
50FA: 7000           moveq #0, d0
50FC: 102D F4C8      moveb.ex -2872(a5), d0
5100: C1FC 001E      muls.ex #30, d0
5104: 41ED F414      lea -3052(a5), d0
5108: 41E8 0044      lea 68(a0), d0
510C: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
5112: 7000           moveq #0, d0
5114: 102D F4C8      moveb.ex -2872(a5), d0
5118: C1FC 001E      muls.ex #30, d0
511C: 41ED F414      lea -3052(a5), d0
5120: 41E8 004B      lea 75(a0), d0
5124: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
512A: 7000           moveq #0, d0
512C: 102D F4C8      moveb.ex -2872(a5), d0
5130: C1FC 001E      muls.ex #30, d0
5134: 41ED F414      lea -3052(a5), d0
5138: 41E8 004C      lea 76(a0), d0
513C: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
5142: 7000           moveq #0, d0
5144: 102D F4CA      moveb.ex -2870(a5), d0
5148: 3F00           movew d0, -(a7)
514A: 4EBA F554      jsr.ex -17752(pc)
514E: 544F           addqw #2, a7
5150: 4CDF 0418      movem (a7)+, <0418>
5154: 4E5E           unlk a6
5156: 4E75           rts
5158: 8947           dc.w #35143
515A: 616D           bsr 109
515C: 6553           bcs 83
515E: 7461           moveq #97, d2
5160: 7274           moveq #116, d1
5162: 0000           dc.w #0
5164: 4E56 0000      link a6, #0
5168: 48E7 1820      movem <1820>, -(a7)
516C: 7600           moveq #0, d3
516E: 162D F43E      moveb.ex -3010(a5), d3
5172: 5343           subqw #1, d3
5174: 6012           bra 18
5176: 701E           moveq #30, d0
5178: C1C3           muls d3, d0
517A: 41ED F414      lea -3052(a5), d0
517E: 41E8 003C      lea 60(a0), d0
5182: 42B0 0800      clrl.ex 0(a0,d0.l)
5186: 5343           subqw #1, d3
5188: 4A43           tstw d3
518A: 6CEA           bge -22
518C: 4A2D F4D0      tstb.ex -2864(a5)
5190: 56C0           sne d0
5192: 4400           negb d0
5194: 4880           extw d0
5196: 1800           moveb d0, d4
5198: 7000           moveq #0, d0
519A: 102D F4D0      moveb.ex -2864(a5), d0
519E: C1FC 0026      muls.ex #38, d0
51A2: 45ED F414      lea -3052(a5), d2
51A6: D5C0           addal d0, a2
51A8: 45EA 00A4      lea 164(a2), d2
51AC: 6000 00CC      bra 204
51B0: 082A 0003 0020 btst #3, 32(a2)
51B6: 6600 00BE      bne 190
51BA: 082A 0004 0020 btst #4, 32(a2)
51C0: 6600 00B2      bne 178
51C4: 0C2A 0004 0018 cmpib.ex #4, 24(a2)
51CA: 6622           bne 34
51CC: 7000           moveq #0, d0
51CE: 102A 0014      moveb.ex 20(a2), d0
51D2: C1FC 001E      muls.ex #30, d0
51D6: 41ED F414      lea -3052(a5), d0
51DA: 41E8 003C      lea 60(a0), d0
51DE: 7240           moveq #64, d1
51E0: 2181 0800      movel.mx d1, 0(a0,d0.l)
51E4: 2F0A           movel a2, -(a7)
51E6: 4EBA C05C      jsr.ex -31156(pc)
51EA: 584F           addqw #4, a7
51EC: 6066           bra 102
51EE: 0C2A 0004 0018 cmpib.ex #4, 24(a2)
51F4: 6F34           ble 52
51F6: 0C2A 000C 0018 cmpib.ex #12, 24(a2)
51FC: 6C2C           bge 44
51FE: 4A2A 0019      tstb.ex 25(a2)
5202: 6620           bne 32
5204: 7000           moveq #0, d0
5206: 102A 0014      moveb.ex 20(a2), d0
520A: C1FC 001E      muls.ex #30, d0
520E: 41ED F414      lea -3052(a5), d0
5212: 41E8 003C      lea 60(a0), d0
5216: 7210           moveq #16, d1
5218: 2181 0800      movel.mx d1, 0(a0,d0.l)
521C: 157C 0004 0019 moveb.emx #4, 25(a2)
5222: 6030           bra 48
5224: 532A 0019      subqb.ex #1, 25(a2)
5228: 602A           bra 42
522A: 4A2A 0019      tstb.ex 25(a2)
522E: 6620           bne 32
5230: 7000           moveq #0, d0
5232: 102A 0014      moveb.ex 20(a2), d0
5236: C1FC 001E      muls.ex #30, d0
523A: 41ED F414      lea -3052(a5), d0
523E: 41E8 003C      lea 60(a0), d0
5242: 7208           moveq #8, d1
5244: 2181 0800      movel.mx d1, 0(a0,d0.l)
5248: 157C 0004 0019 moveb.emx #4, 25(a2)
524E: 6004           bra 4
5250: 532A 0019      subqb.ex #1, 25(a2)
5254: 0C6A 0100 0006 cmpiw.ex #256, 6(a2)
525A: 6D0C           blt 12
525C: 302A 0002      movew.ex 2(a2), d0
5260: D06A 000A      addrw.ex 10(a2), d0
5264: 4A40           tstw d0
5266: 6C08           bge 8
5268: 002A 0008 0020 orib.ex #8, 32(a2)
526E: 6006           bra 6
5270: 7800           moveq #0, d4
5272: 6002           bra 2
5274: 7800           moveq #0, d4
5276: 7026           moveq #38, d0
5278: 95C0           subal d0, a2
527A: 41ED F414      lea -3052(a5), d0
527E: 41E8 00CA      lea 202(a0), d0
5282: B5C8           cmpal a0, a2
5284: 6400 FF2A      bcc 65322
5288: 4A04           tstb d4
528A: 6706           beq 6
528C: 1B7C 0006 F414 moveb.emx #6, -3052(a5)
5292: 4CDF 0418      movem (a7)+, <0418>
5296: 4E5E           unlk a6
5298: 4E75           rts
529A: 9047           subrw d7, d0
529C: 616D           bsr 109
529E: 6543           bcs 67
52A0: 6F6E           ble 110
52A2: 7472           moveq #114, d2
52A4: 6F6C           ble 108
52A6: 5368 6970      subqw.ex #1, 26992(a0)
52AA: 7300           dc.w #29440
52AC: 0000           dc.w #0
52AE: 4E56 FFFC      link a6, #-4
52B2: 2F03           movel d3, -(a7)
52B4: 4A2D F246      tstb.ex -3514(a5)
52B8: 6704           beq 4
52BA: 532D F246      subqb.ex #1, -3514(a5)
52BE: 4A2D F247      tstb.ex -3513(a5)
52C2: 6704           beq 4
52C4: 532D F247      subqb.ex #1, -3513(a5)
52C8: 4A2D F248      tstb.ex -3512(a5)
52CC: 6704           beq 4
52CE: 532D F248      subqb.ex #1, -3512(a5)
52D2: 4A2D F249      tstb.ex -3511(a5)
52D6: 6704           beq 4
52D8: 532D F249      subqb.ex #1, -3511(a5)
52DC: 707A           moveq #122, d0
52DE: 2D40 FFFC      movel.mx d0, -4(a6)
52E2: 4E4F           trap #15
52E4: A2A2           sysTrapKeyCurrentState
52E6: C0AE FFFC      andrl.ex -4(a6), d0
52EA: 2600           movel d0, d3
52EC: 7018           moveq #24, d0
52EE: C083           andrl d3, d0
52F0: 7218           moveq #24, d1
52F2: B081           cmpl d1, d0
52F4: 6604           bne 4
52F6: 70E7           moveq #231, d0
52F8: C680           andrl d0, d3
52FA: 7008           moveq #8, d0
52FC: C083           andrl d3, d0
52FE: 6714           beq 20
5300: 4A2D F246      tstb.ex -3514(a5)
5304: 6608           bne 8
5306: 1B7C 0002 F246 moveb.emx #2, -3514(a5)
530C: 601E           bra 30
530E: 70F7           moveq #247, d0
5310: C680           andrl d0, d3
5312: 6018           bra 24
5314: 7010           moveq #16, d0
5316: C083           andrl d3, d0
5318: 6712           beq 18
531A: 4A2D F247      tstb.ex -3513(a5)
531E: 6608           bne 8
5320: 1B7C 0002 F247 moveb.emx #2, -3513(a5)
5326: 6004           bra 4
5328: 70EF           moveq #239, d0
532A: C680           andrl d0, d3
532C: 7040           moveq #64, d0
532E: C083           andrl d3, d0
5330: 6712           beq 18
5332: 4A2D F248      tstb.ex -3512(a5)
5336: 6608           bne 8
5338: 1B7C 0002 F248 moveb.emx #2, -3512(a5)
533E: 6004           bra 4
5340: 70BF           moveq #191, d0
5342: C680           andrl d0, d3
5344: 7020           moveq #32, d0
5346: C083           andrl d3, d0
5348: 6712           beq 18
534A: 4A2D F249      tstb.ex -3511(a5)
534E: 6608           bne 8
5350: 1B7C 0004 F249 moveb.emx #4, -3511(a5)
5356: 6004           bra 4
5358: 70DF           moveq #223, d0
535A: C680           andrl d0, d3
535C: 2003           movel d3, d0
535E: 261F           movel (a7)+, d3
5360: 4E5E           unlk a6
5362: 4E75           rts
5364: 8F43           dc.w #36675
5366: 6F6E           ble 110
5368: 736F           dc.w #29551
536A: 6C65           bge 101
536C: 4765           dc.w #18277
536E: 7449           moveq #73, d2
5370: 6E70           bgt 112
5372: 7574           dc.w #30068
5374: 0000           dc.w #0
5376: 4E56 0000      link a6, #0
537A: 48E7 1020      movem <1020>, -(a7)
537E: 246E 0008      movel.ex 8(a6), a2
5382: 262E 000C      movel.ex 12(a6), d3
5386: 022A 007F 0020 andib.ex #127, 32(a2)
538C: 022A 00BF 0020 andib.ex #191, 32(a2)
5392: 022A 00DF 0020 andib.ex #223, 32(a2)
5398: 7008           moveq #8, d0
539A: C083           andrl d3, d0
539C: 6714           beq 20
539E: 7010           moveq #16, d0
53A0: C083           andrl d3, d0
53A2: 660E           bne 14
53A4: 4A2A 000C      tstb.ex 12(a2)
53A8: 6726           beq 38
53AA: 002A 0080 0020 orib.ex #128, 32(a2)
53B0: 601E           bra 30
53B2: 7010           moveq #16, d0
53B4: C083           andrl d3, d0
53B6: 6714           beq 20
53B8: 7008           moveq #8, d0
53BA: C083           andrl d3, d0
53BC: 660E           bne 14
53BE: 4A2A 000C      tstb.ex 12(a2)
53C2: 670C           beq 12
53C4: 002A 0040 0020 orib.ex #64, 32(a2)
53CA: 6004           bra 4
53CC: 422A 000D      clrb.ex 13(a2)
53D0: 7040           moveq #64, d0
53D2: C083           andrl d3, d0
53D4: 670C           beq 12
53D6: 4A2A 000C      tstb.ex 12(a2)
53DA: 6706           beq 6
53DC: 002A 0020 0020 orib.ex #32, 32(a2)
53E2: 7020           moveq #32, d0
53E4: C083           andrl d3, d0
53E6: 6740           beq 64
53E8: 4A2A 000C      tstb.ex 12(a2)
53EC: 673A           beq 58
53EE: 102A 0018      moveb.ex 24(a2), d0
53F2: 4880           extw d0
53F4: 48C0           extl d0
53F6: D080           addrl d0, d0
53F8: 41ED FCEE      lea -786(a5), d0
53FC: 7218           moveq #24, d1
53FE: C3F0 0800      muls.ex 0(a0,d0.l), d1
5402: 3F01           movew d1, -(a7)
5404: 102A 0018      moveb.ex 24(a2), d0
5408: 4880           extw d0
540A: 48C0           extl d0
540C: D080           addrl d0, d0
540E: 41ED FCCE      lea -818(a5), d0
5412: 7218           moveq #24, d1
5414: C3F0 0800      muls.ex 0(a0,d0.l), d1
5418: 3F01           movew d1, -(a7)
541A: 2F0A           movel a2, -(a7)
541C: 1F3C 0007      moveb.ex #7, -(a7)
5420: 4EBA D9C8      jsr.ex -24078(pc)
5424: 4FEF 000A      lea 10(a7), d7
5428: 7002           moveq #2, d0
542A: C083           andrl d3, d0
542C: 672A           beq 42
542E: 4A2A 000C      tstb.ex 12(a2)
5432: 6724           beq 36
5434: 082A 0006 001E btst #6, 30(a2)
543A: 661C           bne 28
543C: 082A 0005 001E btst #5, 30(a2)
5442: 6614           bne 20
5444: 082A 0004 001E btst #4, 30(a2)
544A: 660C           bne 12
544C: 002A 0040 001E orib.ex #64, 30(a2)
5452: 157C 0002 0019 moveb.emx #2, 25(a2)
5458: 4CDF 0408      movem (a7)+, <0408>
545C: 4E5E           unlk a6
545E: 4E75           rts
5460: 8F53           ormw d7, (a3)
5462: 6869           bvc 105
5464: 7048           moveq #72, d0
5466: 616E           bsr 110
5468: 646C           bcc 108
546A: 6549           bcs 73
546C: 6E70           bgt 112
546E: 7574           dc.w #30068
5470: 0000           dc.w #0
5472: 4E56 0000      link a6, #0
5476: 2F0A           movel a2, -(a7)
5478: 246E 0008      movel.ex 8(a6), a2
547C: 302A 0008      movew.ex 8(a2), d0
5480: D152           addmw d0, (a2)
5482: 302A 000A      movew.ex 10(a2), d0
5486: D16A 0002      addmw.ex d0, 2(a2)
548A: 302A 0008      movew.ex 8(a2), d0
548E: D16A 0004      addmw.ex d0, 4(a2)
5492: 302A 000A      movew.ex 10(a2), d0
5496: D16A 0006      addmw.ex d0, 6(a2)
549A: 2F0A           movel a2, -(a7)
549C: 4EBA B82C      jsr.ex -32558(pc)
54A0: 584F           addqw #4, a7
54A2: 245F           movel (a7)+, a2
54A4: 4E5E           unlk a6
54A6: 4E75           rts
54A8: 8A4F           dc.w #35407
54AA: 626A           bhi 106
54AC: 6563           bcs 99
54AE: 744D           moveq #77, d2
54B0: 6F76           ble 118
54B2: 6500 0000      bcs 0
54B6: 4E56 FFF4      link a6, #-12
54BA: 48E7 1F20      movem <1f20>, -(a7)
54BE: 246E 0008      movel.ex 8(a6), a2
54C2: 3D7C 0014 FFF4 movew.emx #20, -12(a6)
54C8: 3012           movew (a2), d0
54CA: D06A 0004      addrw.ex 4(a2), d0
54CE: 3E00           movew d0, d7
54D0: E04F           lsriw #0, d7
54D2: EE4F           lsriw #7, d7
54D4: DE40           addrw d0, d7
54D6: E247           asriw #1, d7
54D8: 302A 0002      movew.ex 2(a2), d0
54DC: D06A 0006      addrw.ex 6(a2), d0
54E0: 3200           movew d0, d1
54E2: E049           lsriw #0, d1
54E4: EE49           lsriw #7, d1
54E6: D240           addrw d0, d1
54E8: E241           asriw #1, d1
54EA: 3D41 FFF6      movew.mx d1, -10(a6)
54EE: 486E FFF8      pea.ex -8(a6)
54F2: 4EBA B79E      jsr.ex -32614(pc)
54F6: 584F           addqw #4, a7
54F8: 6000 00BA      bra 186
54FC: 42A7           clrl -(a7)
54FE: 4E4F           trap #15
5500: A0C2           sysTrapSysRandom
5502: 322E FFFC      movew.ex -4(a6), d1
5506: 926E FFF8      subrw.ex -8(a6), d1
550A: C3C0           muls d0, d1
550C: 2001           movel d1, d0
550E: E080           asril #0, d0
5510: EC80           asril #6, d0
5512: E088           lsril #0, d0
5514: E088           lsril #0, d0
5516: E288           lsril #1, d0
5518: D081           addrl d1, d0
551A: E080           asril #0, d0
551C: EE80           asril #7, d0
551E: 306E FFF8      movew.ex -8(a6), a0
5522: D088           addrl a0, d0
5524: 3600           movew d0, d3
5526: 42A7           clrl -(a7)
5528: 4E4F           trap #15
552A: A0C2           sysTrapSysRandom
552C: 322E FFFE      movew.ex -2(a6), d1
5530: 926E FFFA      subrw.ex -6(a6), d1
5534: C3C0           muls d0, d1
5536: 2001           movel d1, d0
5538: E080           asril #0, d0
553A: EC80           asril #6, d0
553C: E088           lsril #0, d0
553E: E088           lsril #0, d0
5540: E288           lsril #1, d0
5542: D081           addrl d1, d0
5544: E080           asril #0, d0
5546: EE80           asril #7, d0
5548: 306E FFFA      movew.ex -6(a6), a0
554C: D088           addrl a0, d0
554E: 3800           movew d0, d4
5550: 4A2E 000C      tstb.ex 12(a6)
5554: 504F           addqw #0, a7
5556: 672E           beq 46
5558: 3A03           movew d3, d5
555A: 9A47           subrw d7, d5
555C: 48C5           extl d5
555E: 8BFC 000F      divs.ex #15, d5
5562: 3C04           movew d4, d6
5564: 9C6E FFF6      subrw.ex -10(a6), d6
5568: 48C6           extl d6
556A: 8DFC 000F      divs.ex #15, d6
556E: 1F3C 000F      moveb.ex #15, -(a7)
5572: 3F06           movew d6, -(a7)
5574: 3F05           movew d5, -(a7)
5576: 3F2E FFF6      movew.ex -10(a6), -(a7)
557A: 3F07           movew d7, -(a7)
557C: 4EBA BA30      jsr.ex -31818(pc)
5580: 4FEF 000A      lea 10(a7), d7
5584: 602A           bra 42
5586: 3A07           movew d7, d5
5588: 9A43           subrw d3, d5
558A: 48C5           extl d5
558C: 8BFC 000F      divs.ex #15, d5
5590: 3C2E FFF6      movew.ex -10(a6), d6
5594: 9C44           subrw d4, d6
5596: 48C6           extl d6
5598: 8DFC 000F      divs.ex #15, d6
559C: 1F3C 000F      moveb.ex #15, -(a7)
55A0: 3F06           movew d6, -(a7)
55A2: 3F05           movew d5, -(a7)
55A4: 3F04           movew d4, -(a7)
55A6: 3F03           movew d3, -(a7)
55A8: 4EBA BA04      jsr.ex -31818(pc)
55AC: 4FEF 000A      lea 10(a7), d7
55B0: 536E FFF4      subqw.ex #1, -12(a6)
55B4: 4A6E FFF4      tstw.ex -12(a6)
55B8: 6E00 FF42      bgt 65346
55BC: 4CDF 04F8      movem (a7)+, <04f8>
55C0: 4E5E           unlk a6
55C2: 4E75           rts
55C4: 9353           submw d1, (a3)
55C6: 6869           bvc 105
55C8: 7044           moveq #68, d0
55CA: 7261           moveq #97, d1
55CC: 7757           dc.w #30551
55CE: 6172           bsr 114
55D0: 7045           moveq #69, d0
55D2: 6666           bne 102
55D4: 6563           bcs 99
55D6: 7473           moveq #115, d2
55D8: 0000           dc.w #0
55DA: 4E56 0000      link a6, #0
55DE: 48E7 1E38      movem <1e38>, -(a7)
55E2: 246E 0008      movel.ex 8(a6), a2
55E6: 082A 0005 001E btst #5, 30(a2)
55EC: 6700 0236      beq 566
55F0: 4A2A 0019      tstb.ex 25(a2)
55F4: 6704           beq 4
55F6: 532A 0019      subqb.ex #1, 25(a2)
55FA: 4A2A 0019      tstb.ex 25(a2)
55FE: 6600 021E      bne 542
5602: 022A 00DF 001E andib.ex #223, 30(a2)
5608: 002A 0010 001E orib.ex #16, 30(a2)
560E: 157C 000F 0019 moveb.emx #15, 25(a2)
5614: 1F3C 0001      moveb.ex #1, -(a7)
5618: 2F0A           movel a2, -(a7)
561A: 4EBA B77E      jsr.ex -32350(pc)
561E: 202D F448      movel.ex -3000(a5), d0
5622: B0AD F416      cmpl.ex -3050(a5), d0
5626: 5C4F           addqw #6, a7
5628: 6700 00BE      beq 190
562C: 2B6D F416 F448 movel.emx -3050(a5), -3000(a5)
5632: 42A7           clrl -(a7)
5634: 4E4F           trap #15
5636: A0C2           sysTrapSysRandom
5638: 48C0           extl d0
563A: E988           lslil #4, d0
563C: 2200           movel d0, d1
563E: E081           asril #0, d1
5640: EC81           asril #6, d1
5642: E089           lsril #0, d1
5644: E089           lsril #0, d1
5646: E289           lsril #1, d1
5648: D280           addrl d0, d1
564A: E081           asril #0, d1
564C: EE81           asril #7, d1
564E: 1B41 F44E      moveb.mx d1, -2994(a5)
5652: 42A7           clrl -(a7)
5654: 4E4F           trap #15
5656: A0C2           sysTrapSysRandom
5658: C1FC 03E8      muls.ex #1000, d0
565C: 2200           movel d0, d1
565E: E081           asril #0, d1
5660: EC81           asril #6, d1
5662: E089           lsril #0, d1
5664: E089           lsril #0, d1
5666: E289           lsril #1, d1
5668: D280           addrl d0, d1
566A: E081           asril #0, d1
566C: EE81           asril #7, d1
566E: 7032           moveq #50, d0
5670: B280           cmpl d0, d1
5672: 5DC1           slt d1
5674: 4401           negb d1
5676: 4881           extw d1
5678: 1B41 F44C      moveb.mx d1, -2996(a5)
567C: 0C2D 0002 F4CA cmpib.ex #2, -2870(a5)
5682: 504F           addqw #0, a7
5684: 6562           bcs 98
5686: 7000           moveq #0, d0
5688: 102D F4D1      moveb.ex -2863(a5), d0
568C: 7800           moveq #0, d4
568E: 182D F4D7      moveb.ex -2857(a5), d4
5692: D840           addrw d0, d4
5694: 0C44 0004      cmpiw #4, d4
5698: 6C10           bge 16
569A: 3044           movew d4, a0
569C: 2008           movel a0, d0
569E: D080           addrl d0, d0
56A0: 41ED FD0E      lea -754(a5), d0
56A4: 3C30 0800      movew.ex 0(a0,d0.l), d6
56A8: 6016           bra 22
56AA: 0C44 0012      cmpiw #18, d4
56AE: 6C0E           bge 14
56B0: 3044           movew d4, a0
56B2: 2C08           movel a0, d6
56B4: 8DFC FFFD      divs.ex #65533, d6
56B8: 0646 0009      addiw #9, d6
56BC: 6002           bra 2
56BE: 7C03           moveq #3, d6
56C0: 42A7           clrl -(a7)
56C2: 4E4F           trap #15
56C4: A0C2           sysTrapSysRandom
56C6: C1C6           muls d6, d0
56C8: 2200           movel d0, d1
56CA: E081           asril #0, d1
56CC: EC81           asril #6, d1
56CE: E089           lsril #0, d1
56D0: E089           lsril #0, d1
56D2: E289           lsril #1, d1
56D4: D280           addrl d0, d1
56D6: E081           asril #0, d1
56D8: EE81           asril #7, d1
56DA: 5381           subql #1, d1
56DC: 57C1           seq d1
56DE: 4401           negb d1
56E0: 4881           extw d1
56E2: 1B41 F44D      moveb.mx d1, -2995(a5)
56E6: 584F           addqw #4, a7
56E8: 7000           moveq #0, d0
56EA: 102A 0014      moveb.ex 20(a2), d0
56EE: C1FC 001E      muls.ex #30, d0
56F2: 47ED F414      lea -3052(a5), d3
56F6: D7C0           addal d0, a3
56F8: 47EB 003C      lea 60(a3), d3
56FC: 4A2B 0013      tstb.ex 19(a3)
5700: 6646           bne 70
5702: 156D F44E 0018 moveb.emx -2994(a5), 24(a2)
5708: 4A2D F44C      tstb.ex -2996(a5)
570C: 6704           beq 4
570E: 7A40           moveq #64, d5
5710: 6002           bra 2
5712: 7A00           moveq #0, d5
5714: 102A 0018      moveb.ex 24(a2), d0
5718: 4880           extw d0
571A: 48C0           extl d0
571C: D080           addrl d0, d0
571E: 41ED FCCE      lea -818(a5), d0
5722: 3205           movew d5, d1
5724: C3F0 0800      muls.ex 0(a0,d0.l), d1
5728: 3541 0008      movew.mx d1, 8(a2)
572C: 102A 0018      moveb.ex 24(a2), d0
5730: 4880           extw d0
5732: 48C0           extl d0
5734: D080           addrl d0, d0
5736: 41ED FCEE      lea -786(a5), d0
573A: 3205           movew d5, d1
573C: C3F0 0800      muls.ex 0(a0,d0.l), d1
5740: 3541 000A      movew.mx d1, 10(a2)
5744: 7600           moveq #0, d3
5746: 6030           bra 48
5748: 7000           moveq #0, d0
574A: 102B 0012      moveb.ex 18(a3), d0
574E: 7200           moveq #0, d1
5750: 122B 0013      moveb.ex 19(a3), d1
5754: 5241           addqw #1, d1
5756: B240           cmpw d0, d1
5758: 6F14           ble 20
575A: 486D FFEA      pea.ex -22(a5)
575E: 3F3C 1B2C      movew.ex #6956, -(a7)
5762: 486D FDEA      pea.ex -534(a5)
5766: 4E4F           trap #15
5768: A084           sysTrapErrDisplayFileLineMsg
576A: 4FEF 000A      lea 10(a7), d7
576E: 2F0A           movel a2, -(a7)
5770: 4EBA C856      jsr.ex -27696(pc)
5774: 1600           moveb d0, d3
5776: 584F           addqw #4, a7
5778: 7000           moveq #0, d0
577A: 1003           moveb d3, d0
577C: D080           addrl d0, d0
577E: 204B           movel a3, a0
5780: 41E8 0014      lea 20(a0), d0
5784: 31AA 0016 0800 movew.emx 22(a2), 0(a0,d0.l)
578A: 522B 0013      addqb.ex #1, 19(a3)
578E: 4227           clrb -(a7)
5790: 2F0A           movel a2, -(a7)
5792: 4EBA FD22      jsr.ex -14146(pc)
5796: 4A2D F44D      tstb.ex -2995(a5)
579A: 5C4F           addqw #6, a7
579C: 6700 0080      beq 128
57A0: 0C2B 0004 0012 cmpib.ex #4, 18(a3)
57A6: 6476           bcc 118
57A8: 0C2D 000F F4D0 cmpib.ex #15, -2864(a5)
57AE: 646E           bcc 110
57B0: 102D F4D0      moveb.ex -2864(a5), d0
57B4: 522D F4D0      addqb.ex #1, -2864(a5)
57B8: 7200           moveq #0, d1
57BA: 1200           moveb d0, d1
57BC: C3FC 0026      muls.ex #38, d1
57C0: 49ED F414      lea -3052(a5), d4
57C4: D9C1           addal d1, a4
57C6: 49EC 00CA      lea 202(a4), d4
57CA: 4878 0026      pea.ex (0026).w
57CE: 2F0A           movel a2, -(a7)
57D0: 2F0C           movel a4, -(a7)
57D2: 4E4F           trap #15
57D4: A026           sysTrapMemMove
57D6: 4EBA C73A      jsr.ex -27878(pc)
57DA: 3940 0016      movew.mx d0, 22(a4)
57DE: 7000           moveq #0, d0
57E0: 102C 0014      moveb.ex 20(a4), d0
57E4: C1FC 001E      muls.ex #30, d0
57E8: 41ED F414      lea -3052(a5), d0
57EC: 41E8 004E      lea 78(a0), d0
57F0: 5230 0800      addqb.ex #1, 0(a0,d0.l)
57F4: 2F0C           movel a4, -(a7)
57F6: 4EBA C7D0      jsr.ex -27696(pc)
57FA: 1600           moveb d0, d3
57FC: 7000           moveq #0, d0
57FE: 1003           moveb d3, d0
5800: D080           addrl d0, d0
5802: 204B           movel a3, a0
5804: 41E8 0014      lea 20(a0), d0
5808: 31AC 0016 0800 movew.emx 22(a4), 0(a0,d0.l)
580E: 522B 0013      addqb.ex #1, 19(a3)
5812: 4227           clrb -(a7)
5814: 2F0C           movel a4, -(a7)
5816: 4EBA FC9E      jsr.ex -14146(pc)
581A: 4FEF 0016      lea 22(a7), d7
581E: 7001           moveq #1, d0
5820: 6000 009A      bra 154
5824: 082A 0004 001E btst #4, 30(a2)
582A: 671A           beq 26
582C: 4A2A 0019      tstb.ex 25(a2)
5830: 6704           beq 4
5832: 532A 0019      subqb.ex #1, 25(a2)
5836: 4A2A 0019      tstb.ex 25(a2)
583A: 6606           bne 6
583C: 022A 00EF 001E andib.ex #239, 30(a2)
5842: 7001           moveq #1, d0
5844: 6076           bra 118
5846: 082A 0006 001E btst #6, 30(a2)
584C: 676C           beq 108
584E: 4A2A 0019      tstb.ex 25(a2)
5852: 6704           beq 4
5854: 532A 0019      subqb.ex #1, 25(a2)
5858: 4A2A 0019      tstb.ex 25(a2)
585C: 665C           bne 92
585E: 022A 00BF 001E andib.ex #191, 30(a2)
5864: 002A 0020 001E orib.ex #32, 30(a2)
586A: 202D F442      movel.ex -3006(a5), d0
586E: B0AD F416      cmpl.ex -3050(a5), d0
5872: 672C           beq 44
5874: 2B6D F416 F442 movel.emx -3050(a5), -3006(a5)
587A: 42A7           clrl -(a7)
587C: 4E4F           trap #15
587E: A0C2           sysTrapSysRandom
5880: C1FC 0050      muls.ex #80, d0
5884: 2200           movel d0, d1
5886: E081           asril #0, d1
5888: EC81           asril #6, d1
588A: E089           lsril #0, d1
588C: E089           lsril #0, d1
588E: E289           lsril #1, d1
5890: D280           addrl d0, d1
5892: E081           asril #0, d1
5894: EE81           asril #7, d1
5896: 7014           moveq #20, d0
5898: D280           addrl d0, d1
589A: 1B41 F446      moveb.mx d1, -3002(a5)
589E: 584F           addqw #4, a7
58A0: 156D F446 0019 moveb.emx -3002(a5), 25(a2)
58A6: 2F0A           movel a2, -(a7)
58A8: 4EBA C68E      jsr.ex -27840(pc)
58AC: 1F3C 0001      moveb.ex #1, -(a7)
58B0: 2F0A           movel a2, -(a7)
58B2: 4EBA FC02      jsr.ex -14146(pc)
58B6: 4FEF 000A      lea 10(a7), d7
58BA: 7000           moveq #0, d0
58BC: 4CDF 1C78      movem (a7)+, <1c78>
58C0: 4E5E           unlk a6
58C2: 4E75           rts
58C4: 9553           submw d2, (a3)
58C6: 6869           bvc 105
58C8: 7048           moveq #72, d0
58CA: 616E           bsr 110
58CC: 646C           bcc 108
58CE: 6557           bcs 87
58D0: 6172           bsr 114
58D2: 7045           moveq #69, d0
58D4: 6666           bne 102
58D6: 6563           bcs 99
58D8: 7473           moveq #115, d2
58DA: 0000           dc.w #0
58DC: 4E56 FFFC      link a6, #-4
58E0: 48E7 1E38      movem <1e38>, -(a7)
58E4: 99CC           subal a4, a4
58E6: 7600           moveq #0, d3
58E8: 7000           moveq #0, d0
58EA: 102D F4D0      moveb.ex -2864(a5), d0
58EE: C1FC 0026      muls.ex #38, d0
58F2: 45ED F414      lea -3052(a5), d2
58F6: D5C0           addal d0, a2
58F8: 45EA 00A4      lea 164(a2), d2
58FC: 47ED F414      lea -3052(a5), d3
5900: 47EB 00CA      lea 202(a3), d3
5904: 6000 0392      bra 914
5908: 2F0A           movel a2, -(a7)
590A: 4EBA FCCE      jsr.ex -13854(pc)
590E: 4A00           tstb d0
5910: 584F           addqw #4, a7
5912: 6600 0380      bne 896
5916: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
591C: 6400 01A6      bcc 422
5920: 082A 0004 0020 btst #4, 32(a2)
5926: 6710           beq 16
5928: 5203           addqb #1, d3
592A: 200C           movel a4, d0
592C: 6602           bne 2
592E: 284A           movel a2, a4
5930: 7026           moveq #38, d0
5932: 95C0           subal d0, a2
5934: 6000 0362      bra 866
5938: 082A 0003 0020 btst #3, 32(a2)
593E: 6708           beq 8
5940: 7026           moveq #38, d0
5942: 95C0           subal d0, a2
5944: 6000 0352      bra 850
5948: 7000           moveq #0, d0
594A: 102A 0014      moveb.ex 20(a2), d0
594E: C1FC 001E      muls.ex #30, d0
5952: 41ED F414      lea -3052(a5), d0
5956: 2F30 083C      movel.ex 60(a0,d0.l), -(a7)
595A: 2F0A           movel a2, -(a7)
595C: 4EBA FA18      jsr.ex -14466(pc)
5960: 082A 0007 0020 btst #7, 32(a2)
5966: 504F           addqw #0, a7
5968: 6712           beq 18
596A: 522A 0018      addqb.ex #1, 24(a2)
596E: 0C2A 0010 0018 cmpib.ex #16, 24(a2)
5974: 6D1E           blt 30
5976: 422A 0018      clrb.ex 24(a2)
597A: 6018           bra 24
597C: 082A 0006 0020 btst #6, 32(a2)
5982: 6710           beq 16
5984: 532A 0018      subqb.ex #1, 24(a2)
5988: 4A2A 0018      tstb.ex 24(a2)
598C: 6C06           bge 6
598E: 157C 000F 0018 moveb.emx #15, 24(a2)
5994: 082A 0005 0020 btst #5, 32(a2)
599A: 6700 00EA      beq 234
599E: 102A 0018      moveb.ex 24(a2), d0
59A2: 4880           extw d0
59A4: 48C0           extl d0
59A6: D080           addrl d0, d0
59A8: 41ED FCCE      lea -818(a5), d0
59AC: 7205           moveq #5, d1
59AE: C3F0 0800      muls.ex 0(a0,d0.l), d1
59B2: 3001           movew d1, d0
59B4: E048           lsriw #0, d0
59B6: EE48           lsriw #7, d0
59B8: D041           addrw d1, d0
59BA: E240           asriw #1, d0
59BC: D16A 0008      addmw.ex d0, 8(a2)
59C0: 102A 0018      moveb.ex 24(a2), d0
59C4: 4880           extw d0
59C6: 48C0           extl d0
59C8: D080           addrl d0, d0
59CA: 41ED FCEE      lea -786(a5), d0
59CE: 7205           moveq #5, d1
59D0: C3F0 0800      muls.ex 0(a0,d0.l), d1
59D4: 3001           movew d1, d0
59D6: E048           lsriw #0, d0
59D8: EE48           lsriw #7, d0
59DA: D041           addrw d1, d0
59DC: E240           asriw #1, d0
59DE: D16A 000A      addmw.ex d0, 10(a2)
59E2: 4A6A 0008      tstw.ex 8(a2)
59E6: 5CC0           sge d0
59E8: 4400           negb d0
59EA: 4880           extw d0
59EC: 6706           beq 6
59EE: 3A2A 0008      movew.ex 8(a2), d5
59F2: 6006           bra 6
59F4: 3A2A 0008      movew.ex 8(a2), d5
59F8: 4445           negw d5
59FA: 7005           moveq #5, d0
59FC: C1ED FCCE      muls.ex -818(a5), d0
5A00: 3200           movew d0, d1
5A02: E049           lsriw #0, d1
5A04: EE49           lsriw #7, d1
5A06: D240           addrw d0, d1
5A08: E241           asriw #1, d1
5A0A: BA41           cmpw d1, d5
5A0C: 6C34           bge 52
5A0E: 4A6A 000A      tstw.ex 10(a2)
5A12: 5CC0           sge d0
5A14: 4400           negb d0
5A16: 4880           extw d0
5A18: 6706           beq 6
5A1A: 3C2A 000A      movew.ex 10(a2), d6
5A1E: 6006           bra 6
5A20: 3C2A 000A      movew.ex 10(a2), d6
5A24: 4446           negw d6
5A26: 7005           moveq #5, d0
5A28: C1ED FCF6      muls.ex -778(a5), d0
5A2C: 3200           movew d0, d1
5A2E: E049           lsriw #0, d1
5A30: EE49           lsriw #7, d1
5A32: D240           addrw d0, d1
5A34: E241           asriw #1, d1
5A36: BC41           cmpw d1, d6
5A38: 6C08           bge 8
5A3A: 426A 0008      clrw.ex 8(a2)
5A3E: 426A 000A      clrw.ex 10(a2)
5A42: 0C6A 0080 0008 cmpiw.ex #128, 8(a2)
5A48: 6F08           ble 8
5A4A: 357C 0080 0008 movew.emx #128, 8(a2)
5A50: 600E           bra 14
5A52: 0C6A FF80 0008 cmpiw.ex #-128, 8(a2)
5A58: 6C06           bge 6
5A5A: 357C FF80 0008 movew.emx #65408, 8(a2)
5A60: 0C6A 0080 000A cmpiw.ex #128, 10(a2)
5A66: 6F0A           ble 10
5A68: 357C 0080 000A movew.emx #128, 10(a2)
5A6E: 6000 01AE      bra 430
5A72: 0C6A FF80 000A cmpiw.ex #-128, 10(a2)
5A78: 6C00 01A4      bge 420
5A7C: 357C FF80 000A movew.emx #65408, 10(a2)
5A82: 6000 019A      bra 410
5A86: 082A 0002 001E btst #2, 30(a2)
5A8C: 6700 0190      beq 400
5A90: 4A6A 0008      tstw.ex 8(a2)
5A94: 6F06           ble 6
5A96: 536A 0008      subqw.ex #1, 8(a2)
5A9A: 600A           bra 10
5A9C: 4A6A 0008      tstw.ex 8(a2)
5AA0: 6C04           bge 4
5AA2: 526A 0008      addqw.ex #1, 8(a2)
5AA6: 4A6A 000A      tstw.ex 10(a2)
5AAA: 6F08           ble 8
5AAC: 536A 000A      subqw.ex #1, 10(a2)
5AB0: 6000 016C      bra 364
5AB4: 4A6A 000A      tstw.ex 10(a2)
5AB8: 6C00 0164      bge 356
5ABC: 526A 000A      addqw.ex #1, 10(a2)
5AC0: 6000 015C      bra 348
5AC4: 42A7           clrl -(a7)
5AC6: 4E4F           trap #15
5AC8: A0C2           sysTrapSysRandom
5ACA: C1FC 03E8      muls.ex #1000, d0
5ACE: 2200           movel d0, d1
5AD0: E081           asril #0, d1
5AD2: EC81           asril #6, d1
5AD4: E089           lsril #0, d1
5AD6: E089           lsril #0, d1
5AD8: E289           lsril #1, d1
5ADA: D280           addrl d0, d1
5ADC: E081           asril #0, d1
5ADE: EE81           asril #7, d1
5AE0: 700F           moveq #15, d0
5AE2: B280           cmpl d0, d1
5AE4: 584F           addqw #4, a7
5AE6: 6C08           bge 8
5AE8: 2F0A           movel a2, -(a7)
5AEA: 4EBA DAD0      jsr.ex -22076(pc)
5AEE: 584F           addqw #4, a7
5AF0: 7000           moveq #0, d0
5AF2: 102A 001A      moveb.ex 26(a2), d0
5AF6: 7200           moveq #0, d1
5AF8: 122D F4CA      moveb.ex -2870(a5), d1
5AFC: D241           addrw d1, d1
5AFE: C3C0           muls d0, d1
5B00: 48C1           extl d1
5B02: 2D41 FFFC      movel.mx d1, -4(a6)
5B06: 42A7           clrl -(a7)
5B08: 4E4F           trap #15
5B0A: A0C2           sysTrapSysRandom
5B0C: C1FC 03E8      muls.ex #1000, d0
5B10: 2200           movel d0, d1
5B12: E081           asril #0, d1
5B14: EC81           asril #6, d1
5B16: E089           lsril #0, d1
5B18: E089           lsril #0, d1
5B1A: E289           lsril #1, d1
5B1C: D280           addrl d0, d1
5B1E: E081           asril #0, d1
5B20: EE81           asril #7, d1
5B22: B2AE FFFC      cmpl.ex -4(a6), d1
5B26: 584F           addqw #4, a7
5B28: 6C0A           bge 10
5B2A: 42A7           clrl -(a7)
5B2C: 2F0A           movel a2, -(a7)
5B2E: 4EBA DBC4      jsr.ex -21764(pc)
5B32: 504F           addqw #0, a7
5B34: 082A 0003 0020 btst #3, 32(a2)
5B3A: 6742           beq 66
5B3C: 42A7           clrl -(a7)
5B3E: 4E4F           trap #15
5B40: A0C2           sysTrapSysRandom
5B42: C1FC 03E8      muls.ex #1000, d0
5B46: 2200           movel d0, d1
5B48: E081           asril #0, d1
5B4A: EC81           asril #6, d1
5B4C: E089           lsril #0, d1
5B4E: E089           lsril #0, d1
5B50: E289           lsril #1, d1
5B52: D280           addrl d0, d1
5B54: E081           asril #0, d1
5B56: EE81           asril #7, d1
5B58: 7064           moveq #100, d0
5B5A: B280           cmpl d0, d1
5B5C: 584F           addqw #4, a7
5B5E: 6C1E           bge 30
5B60: 102A 0022      moveb.ex 34(a2), d0
5B64: E608           lsrib #3, d0
5B66: 5D00           subqb #6, d0
5B68: E708           lslib #3, d0
5B6A: 0200 00F8      andib #248, d0
5B6E: 022A 0007 0022 andib.ex #7, 34(a2)
5B74: 812A 0022      ormb.ex d0, 34(a2)
5B78: 022A 00F7 0020 andib.ex #247, 32(a2)
5B7E: 0C2A 0002 000E cmpib.ex #2, 14(a2)
5B84: 624C           bhi 76
5B86: 0C2A 0001 000E cmpib.ex #1, 14(a2)
5B8C: 6544           bcs 68
5B8E: 0C2A 0040 001C cmpib.ex #64, 28(a2)
5B94: 643C           bcc 60
5B96: 7000           moveq #0, d0
5B98: 102A 001C      moveb.ex 28(a2), d0
5B9C: 7240           moveq #64, d1
5B9E: 9240           subrw d0, d1
5BA0: 48C1           extl d1
5BA2: 2D41 FFFC      movel.mx d1, -4(a6)
5BA6: 42A7           clrl -(a7)
5BA8: 4E4F           trap #15
5BAA: A0C2           sysTrapSysRandom
5BAC: 48C0           extl d0
5BAE: ED88           lslil #6, d0
5BB0: 2200           movel d0, d1
5BB2: E081           asril #0, d1
5BB4: EC81           asril #6, d1
5BB6: E089           lsril #0, d1
5BB8: E089           lsril #0, d1
5BBA: E289           lsril #1, d1
5BBC: D280           addrl d0, d1
5BBE: E081           asril #0, d1
5BC0: EE81           asril #7, d1
5BC2: B2AE FFFC      cmpl.ex -4(a6), d1
5BC6: 584F           addqw #4, a7
5BC8: 6C08           bge 8
5BCA: 2F0A           movel a2, -(a7)
5BCC: 4EBA B676      jsr.ex -31156(pc)
5BD0: 584F           addqw #4, a7
5BD2: 0C2A 0003 000E cmpib.ex #3, 14(a2)
5BD8: 6644           bne 68
5BDA: 0C2A 001B 001C cmpib.ex #27, 28(a2)
5BE0: 643C           bcc 60
5BE2: 7000           moveq #0, d0
5BE4: 102A 001C      moveb.ex 28(a2), d0
5BE8: 721B           moveq #27, d1
5BEA: 9240           subrw d0, d1
5BEC: 48C1           extl d1
5BEE: 2D41 FFFC      movel.mx d1, -4(a6)
5BF2: 42A7           clrl -(a7)
5BF4: 4E4F           trap #15
5BF6: A0C2           sysTrapSysRandom
5BF8: C1FC 0036      muls.ex #54, d0
5BFC: 2200           movel d0, d1
5BFE: E081           asril #0, d1
5C00: EC81           asril #6, d1
5C02: E089           lsril #0, d1
5C04: E089           lsril #0, d1
5C06: E289           lsril #1, d1
5C08: D280           addrl d0, d1
5C0A: E081           asril #0, d1
5C0C: EE81           asril #7, d1
5C0E: B2AE FFFC      cmpl.ex -4(a6), d1
5C12: 584F           addqw #4, a7
5C14: 6C08           bge 8
5C16: 2F0A           movel a2, -(a7)
5C18: 4EBA B62A      jsr.ex -31156(pc)
5C1C: 584F           addqw #4, a7
5C1E: 2F0A           movel a2, -(a7)
5C20: 4EBA F850      jsr.ex -14214(pc)
5C24: 4227           clrb -(a7)
5C26: 2F0A           movel a2, -(a7)
5C28: 4EBA E3D8      jsr.ex -19446(pc)
5C2C: 4AAA 0010      tstl.ex 16(a2)
5C30: 4FEF 000A      lea 10(a7), d7
5C34: 675E           beq 94
5C36: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
5C3C: 6556           bcs 86
5C3E: 082A 0003 0020 btst #3, 32(a2)
5C44: 664E           bne 78
5C46: 206A 0010      movel.ex 16(a2), a0
5C4A: 0C28 0007 000E cmpib.ex #7, 14(a0)
5C50: 6516           bcs 22
5C52: 206A 0010      movel.ex 16(a2), a0
5C56: 0C28 0008 000E cmpib.ex #8, 14(a0)
5C5C: 620A           bhi 10
5C5E: 206A 0010      movel.ex 16(a2), a0
5C62: 1828 0014      moveb.ex 20(a0), d4
5C66: 601A           bra 26
5C68: 206A 0010      movel.ex 16(a2), a0
5C6C: 0C28 0006 000E cmpib.ex #6, 14(a0)
5C72: 620A           bhi 10
5C74: 206A 0010      movel.ex 16(a2), a0
5C78: 1828 0014      moveb.ex 20(a0), d4
5C7C: 6004           bra 4
5C7E: 183C 00FF      moveb.ex #255, d4
5C82: 0C04 00FE      cmpib #254, d4
5C86: 670C           beq 12
5C88: 2F2A 0010      movel.ex 16(a2), -(a7)
5C8C: 2F0A           movel a2, -(a7)
5C8E: 4EBA DC1C      jsr.ex -21324(pc)
5C92: 504F           addqw #0, a7
5C94: 7026           moveq #38, d0
5C96: 95C0           subal d0, a2
5C98: B5CB           cmpal a3, a2
5C9A: 6400 FC6C      bcc 64620
5C9E: 244C           movel a4, a2
5CA0: 47ED F414      lea -3052(a5), d3
5CA4: 47EB 00CA      lea 202(a3), d3
5CA8: 602E           bra 46
5CAA: 082A 0005 001E btst #5, 30(a2)
5CB0: 6622           bne 34
5CB2: 082A 0004 001E btst #4, 30(a2)
5CB8: 661A           bne 26
5CBA: 0C2A 0004 0014 cmpib.ex #4, 20(a2)
5CC0: 6412           bcc 18
5CC2: 082A 0004 0020 btst #4, 32(a2)
5CC8: 670A           beq 10
5CCA: 5303           subqb #1, d3
5CCC: 2F0A           movel a2, -(a7)
5CCE: 4EBA C4E8      jsr.ex -27200(pc)
5CD2: 584F           addqw #4, a7
5CD4: 7026           moveq #38, d0
5CD6: 95C0           subal d0, a2
5CD8: 4A03           tstb d3
5CDA: 6704           beq 4
5CDC: B5CB           cmpal a3, a2
5CDE: 64CA           bcc -54
5CE0: 4CDF 1C78      movem (a7)+, <1c78>
5CE4: 4E5E           unlk a6
5CE6: 4E75           rts
5CE8: 8B53           ormw d5, (a3)
5CEA: 6869           bvc 105
5CEC: 704D           moveq #77, d0
5CEE: 6F76           ble 118
5CF0: 6541           bcs 65
5CF2: 6C6C           bge 108
5CF4: 0000           dc.w #0
5CF6: 4E56 0000      link a6, #0
5CFA: 48E7 1038      movem <1038>, -(a7)
5CFE: 7000           moveq #0, d0
5D00: 102D F4CE      moveb.ex -2866(a5), d0
5D04: C1FC 001A      muls.ex #26, d0
5D08: 45ED F414      lea -3052(a5), d2
5D0C: D5C0           addal d0, a2
5D0E: 45EA 02EA      lea 746(a2), d2
5D12: 49ED F414      lea -3052(a5), d4
5D16: 49EC 0304      lea 772(a4), d4
5D1A: 6000 00B2      bra 178
5D1E: 532A 0018      subqb.ex #1, 24(a2)
5D22: 4A2A 0018      tstb.ex 24(a2)
5D26: 6700 009A      beq 154
5D2A: 0C2A 0008 000E cmpib.ex #8, 14(a2)
5D30: 667C           bne 124
5D32: 102A 0018      moveb.ex 24(a2), d0
5D36: 0240 0007      andiw #7, d0
5D3A: 5940           subqw #4, d0
5D3C: 6670           bne 112
5D3E: 2F0A           movel a2, -(a7)
5D40: 4EBA C0AE      jsr.ex -28168(pc)
5D44: 2648           movel a0, a3
5D46: 200B           movel a3, d0
5D48: 584F           addqw #4, a7
5D4A: 6716           beq 22
5D4C: 486A 000A      pea.ex 10(a2)
5D50: 486A 0008      pea.ex 8(a2)
5D54: 2F0B           movel a3, -(a7)
5D56: 2F0A           movel a2, -(a7)
5D58: 4EBA CDEA      jsr.ex -24756(pc)
5D5C: 4FEF 0010      lea 16(a7), d7
5D60: 604C           bra 76
5D62: 42A7           clrl -(a7)
5D64: 4E4F           trap #15
5D66: A0C2           sysTrapSysRandom
5D68: 48C0           extl d0
5D6A: E988           lslil #4, d0
5D6C: 2200           movel d0, d1
5D6E: E081           asril #0, d1
5D70: EC81           asril #6, d1
5D72: E089           lsril #0, d1
5D74: E089           lsril #0, d1
5D76: E289           lsril #1, d1
5D78: D280           addrl d0, d1
5D7A: E081           asril #0, d1
5D7C: EE81           asril #7, d1
5D7E: 1601           moveb d1, d3
5D80: 1003           moveb d3, d0
5D82: 4880           extw d0
5D84: 48C0           extl d0
5D86: D080           addrl d0, d0
5D88: 41ED FCCE      lea -818(a5), d0
5D8C: 7218           moveq #24, d1
5D8E: C3F0 0800      muls.ex 0(a0,d0.l), d1
5D92: 3541 0008      movew.mx d1, 8(a2)
5D96: 1003           moveb d3, d0
5D98: 4880           extw d0
5D9A: 48C0           extl d0
5D9C: D080           addrl d0, d0
5D9E: 41ED FCEE      lea -786(a5), d0
5DA2: 7218           moveq #24, d1
5DA4: C3F0 0800      muls.ex 0(a0,d0.l), d1
5DA8: 3541 000A      movew.mx d1, 10(a2)
5DAC: 584F           addqw #4, a7
5DAE: 2F0A           movel a2, -(a7)
5DB0: 4EBA F6C0      jsr.ex -14214(pc)
5DB4: 4227           clrb -(a7)
5DB6: 2F0A           movel a2, -(a7)
5DB8: 4EBA E248      jsr.ex -19446(pc)
5DBC: 4FEF 000A      lea 10(a7), d7
5DC0: 6008           bra 8
5DC2: 2F0A           movel a2, -(a7)
5DC4: 4EBA D28C      jsr.ex -23462(pc)
5DC8: 584F           addqw #4, a7
5DCA: 701A           moveq #26, d0
5DCC: 95C0           subal d0, a2
5DCE: B5CC           cmpal a4, a2
5DD0: 6400 FF4C      bcc 65356
5DD4: 4CDF 1C08      movem (a7)+, <1c08>
5DD8: 4E5E           unlk a6
5DDA: 4E75           rts
5DDC: 8C53           orrw (a3), d6
5DDE: 686F           bvc 111
5DE0: 7473           moveq #115, d2
5DE2: 4D6F           dc.w #19823
5DE4: 7665           moveq #101, d3
5DE6: 416C           dc.w #16748
5DE8: 6C00 0000      bge 0
5DEC: 4E56 0000      link a6, #0
5DF0: 48E7 0030      movem <0030>, -(a7)
5DF4: 47ED F414      lea -3052(a5), d3
5DF8: 47EB 043C      lea 1084(a3), d3
5DFC: 7000           moveq #0, d0
5DFE: 102D F4D7      moveb.ex -2857(a5), d0
5E02: C1FC 0014      muls.ex #20, d0
5E06: 45ED F414      lea -3052(a5), d2
5E0A: D5C0           addal d0, a2
5E0C: 45EA 0428      lea 1064(a2), d2
5E10: 6016           bra 22
5E12: 2F0A           movel a2, -(a7)
5E14: 4EBA F65C      jsr.ex -14214(pc)
5E18: 4227           clrb -(a7)
5E1A: 2F0A           movel a2, -(a7)
5E1C: 4EBA E1E4      jsr.ex -19446(pc)
5E20: 7014           moveq #20, d0
5E22: 95C0           subal d0, a2
5E24: 4FEF 000A      lea 10(a7), d7
5E28: B5CB           cmpal a3, a2
5E2A: 64E6           bcc -26
5E2C: 45ED F414      lea -3052(a5), d2
5E30: 45EA 043C      lea 1084(a2), d2
5E34: 4A2A 000C      tstb.ex 12(a2)
5E38: 672A           beq 42
5E3A: 0C6A 0010 0004 cmpiw.ex #16, 4(a2)
5E40: 6E0E           bgt 14
5E42: 4A6A 0008      tstw.ex 8(a2)
5E46: 6608           bne 8
5E48: 356A 000A 0008 movew.emx 10(a2), 8(a2)
5E4E: 6014           bra 20
5E50: 0C6A 0010 0006 cmpiw.ex #16, 6(a2)
5E56: 6E0C           bgt 12
5E58: 4A6A 000A      tstw.ex 10(a2)
5E5C: 6606           bne 6
5E5E: 356A 0008 000A movew.emx 8(a2), 10(a2)
5E64: 4CDF 0C00      movem (a7)+, <0c00>
5E68: 4E5E           unlk a6
5E6A: 4E75           rts
5E6C: 8B52           ormw d5, (a2)
5E6E: 6F63           ble 99
5E70: 6B4D           bmi 77
5E72: 6F76           ble 118
5E74: 6541           bcs 65
5E76: 6C6C           bge 108
5E78: 0000           dc.w #0
5E7A: 4E56 0000      link a6, #0
5E7E: 7000           moveq #0, d0
5E80: 102D F4DB      moveb.ex -2853(a5), d0
5E84: C1FC 000A      muls.ex #10, d0
5E88: 41ED F414      lea -3052(a5), d0
5E8C: D1C0           addal d0, a0
5E8E: 41E8 06CC      lea 1740(a0), d0
5E92: 43ED F414      lea -3052(a5), d1
5E96: 43E9 06D6      lea 1750(a1), d1
5E9A: 6016           bra 22
5E9C: 5328 0008      subqb.ex #1, 8(a0)
5EA0: 3028 0004      movew.ex 4(a0), d0
5EA4: D150           addmw d0, (a0)
5EA6: 3028 0006      movew.ex 6(a0), d0
5EAA: D168 0002      addmw.ex d0, 2(a0)
5EAE: 700A           moveq #10, d0
5EB0: 91C0           subal d0, a0
5EB2: B1C9           cmpal a1, a0
5EB4: 64E6           bcc -26
5EB6: 4E5E           unlk a6
5EB8: 4E75           rts
5EBA: 8D53           ormw d6, (a3)
5EBC: 7061           moveq #97, d0
5EBE: 726B           moveq #107, d1
5EC0: 734D           dc.w #29517
5EC2: 6F76           ble 118
5EC4: 6541           bcs 65
5EC6: 6C6C           bge 108
5EC8: 0000           dc.w #0
5ECA: 4E56 0000      link a6, #0
5ECE: 7000           moveq #0, d0
5ED0: 102D F4DC      moveb.ex -2852(a5), d0
5ED4: E988           lslil #4, d0
5ED6: 41ED F414      lea -3052(a5), d0
5EDA: D1C0           addal d0, a0
5EDC: 41E8 0856      lea 2134(a0), d0
5EE0: 43ED F414      lea -3052(a5), d1
5EE4: 43E9 0866      lea 2150(a1), d1
5EE8: 6008           bra 8
5EEA: 5328 0008      subqb.ex #1, 8(a0)
5EEE: 7010           moveq #16, d0
5EF0: 91C0           subal d0, a0
5EF2: B1C9           cmpal a1, a0
5EF4: 64F4           bcc -12
5EF6: 4E5E           unlk a6
5EF8: 4E75           rts
5EFA: 8D53           ormw d6, (a3)
5EFC: 636F           bls 111
5EFE: 7265           moveq #101, d1
5F00: 7357           dc.w #29527
5F02: 6169           bsr 105
5F04: 7441           moveq #65, d2
5F06: 6C6C           bge 108
5F08: 0000           dc.w #0
5F0A: 4E56 0000      link a6, #0
5F0E: 48E7 0030      movem <0030>, -(a7)
5F12: 7000           moveq #0, d0
5F14: 102D F4D9      moveb.ex -2855(a5), d0
5F18: C1FC 0016      muls.ex #22, d0
5F1C: 45ED F414      lea -3052(a5), d2
5F20: D5C0           addal d0, a2
5F22: 45EA 067E      lea 1662(a2), d2
5F26: 47ED F414      lea -3052(a5), d3
5F2A: 47EB 0694      lea 1684(a3), d3
5F2E: 6012           bra 18
5F30: 532A 0014      subqb.ex #1, 20(a2)
5F34: 4227           clrb -(a7)
5F36: 2F0A           movel a2, -(a7)
5F38: 4EBA E0C8      jsr.ex -19446(pc)
5F3C: 7016           moveq #22, d0
5F3E: 95C0           subal d0, a2
5F40: 5C4F           addqw #6, a7
5F42: B5CB           cmpal a3, a2
5F44: 64EA           bcc -22
5F46: 4CDF 0C00      movem (a7)+, <0c00>
5F4A: 4E5E           unlk a6
5F4C: 4E75           rts
5F4E: 8E42           orrw d2, d7
5F50: 6F6E           ble 110
5F52: 7573           dc.w #30067
5F54: 6573           bcs 115
5F56: 5761           subqw #3, -(a1)
5F58: 6974           bvs 116
5F5A: 416C           dc.w #16748
5F5C: 6C00 0000      bge 0
5F60: 4E56 0000      link a6, #0
5F64: 48E7 1800      movem <1800>, -(a7)
5F68: 7801           moveq #1, d4
5F6A: 7600           moveq #0, d3
5F6C: 162D F43E      moveb.ex -3010(a5), d3
5F70: 5343           subqw #1, d3
5F72: 6000 00B6      bra 182
5F76: 701E           moveq #30, d0
5F78: C1C3           muls d3, d0
5F7A: 41ED F414      lea -3052(a5), d0
5F7E: 41E8 004A      lea 74(a0), d0
5F82: 4A30 0800      tstb.ex 0(a0,d0.l)
5F86: 6700 008C      beq 140
5F8A: 701E           moveq #30, d0
5F8C: C1C3           muls d3, d0
5F8E: 41ED F414      lea -3052(a5), d0
5F92: 41E8 004D      lea 77(a0), d0
5F96: 721E           moveq #30, d1
5F98: C3C3           muls d3, d1
5F9A: 43ED F414      lea -3052(a5), d1
5F9E: 43E9 004E      lea 78(a1), d1
5FA2: 1231 1800      moveb.ex 0(a1,d1.l), d1
5FA6: B230 0800      cmpb.ex 0(a0,d0.l), d1
5FAA: 6464           bcc 100
5FAC: 701E           moveq #30, d0
5FAE: C1C3           muls d3, d0
5FB0: 41ED F414      lea -3052(a5), d0
5FB4: 41E8 004A      lea 74(a0), d0
5FB8: 4A30 0800      tstb.ex 0(a0,d0.l)
5FBC: 6752           beq 82
5FBE: 701E           moveq #30, d0
5FC0: C1C3           muls d3, d0
5FC2: 41ED F414      lea -3052(a5), d0
5FC6: 41E8 0058      lea 88(a0), d0
5FCA: 4A30 0800      tstb.ex 0(a0,d0.l)
5FCE: 6630           bne 48
5FD0: 1F03           moveb d3, -(a7)
5FD2: 4EBA C35C      jsr.ex -26824(pc)
5FD6: 4A00           tstb d0
5FD8: 544F           addqw #2, a7
5FDA: 6734           beq 52
5FDC: 701E           moveq #30, d0
5FDE: C1C3           muls d3, d0
5FE0: 41ED F414      lea -3052(a5), d0
5FE4: 41E8 004A      lea 74(a0), d0
5FE8: 5330 0800      subqb.ex #1, 0(a0,d0.l)
5FEC: 701E           moveq #30, d0
5FEE: C1C3           muls d3, d0
5FF0: 41ED F414      lea -3052(a5), d0
5FF4: 41E8 004B      lea 75(a0), d0
5FF8: 11BC 0001 0800 moveb.emx #1, 0(a0,d0.l)
5FFE: 6010           bra 16
6000: 701E           moveq #30, d0
6002: C1C3           muls d3, d0
6004: 41ED F414      lea -3052(a5), d0
6008: 41E8 0058      lea 88(a0), d0
600C: 5330 0800      subqb.ex #1, 0(a0,d0.l)
6010: 7800           moveq #0, d4
6012: 6014           bra 20
6014: 701E           moveq #30, d0
6016: C1C3           muls d3, d0
6018: 41ED F414      lea -3052(a5), d0
601C: 41E8 004E      lea 78(a0), d0
6020: 4A30 0800      tstb.ex 0(a0,d0.l)
6024: 6702           beq 2
6026: 7800           moveq #0, d4
6028: 5343           subqw #1, d3
602A: 4A43           tstw d3
602C: 6C00 FF48      bge 65352
6030: 4A04           tstb d4
6032: 6712           beq 18
6034: 4A6D F4CC      tstw.ex -2868(a5)
6038: 6F06           ble 6
603A: 536D F4CC      subqw.ex #1, -2868(a5)
603E: 6006           bra 6
6040: 1B7C 0006 F414 moveb.emx #6, -3052(a5)
6046: 4CDF 0018      movem (a7)+, <0018>
604A: 4E5E           unlk a6
604C: 4E75           rts
604E: 8E50           orrw (a0), d7
6050: 6C61           bge 97
6052: 7965           dc.w #31077
6054: 7273           moveq #115, d1
6056: 4164           dc.w #16740
6058: 6453           bcc 83
605A: 6869           bvc 105
605C: 7000           moveq #0, d0
605E: 0000           dc.w #0
6060: 4E56 FFFA      link a6, #-6
6064: 2F03           movel d3, -(a7)
6066: 426E FFFE      clrw.ex -2(a6)
606A: 52AD F416      addql.ex #1, -3050(a5)
606E: 0C2D 0007 F414 cmpib.ex #7, -3052(a5)
6074: 6700 02F0      beq 752
6078: 0C2D 0006 F414 cmpib.ex #6, -3052(a5)
607E: 6656           bne 86
6080: 0C2D 0019 F4CA cmpib.ex #25, -2870(a5)
6086: 6616           bne 22
6088: 4A2D F4D2      tstb.ex -2862(a5)
608C: 6610           bne 16
608E: 1B7C 001D F4CA moveb.emx #29, -2870(a5)
6094: 3F3C 03ED      movew.ex #1005, -(a7)
6098: 4E4F           trap #15
609A: A195           sysTrapFrmHelp
609C: 544F           addqw #2, a7
609E: 0C2D 001A F4CA cmpib.ex #26, -2870(a5)
60A4: 660A           bne 10
60A6: 3F3C 03EC      movew.ex #1004, -(a7)
60AA: 4E4F           trap #15
60AC: A195           sysTrapFrmHelp
60AE: 544F           addqw #2, a7
60B0: 1B7C 0007 F414 moveb.emx #7, -3052(a5)
60B6: 7000           moveq #0, d0
60B8: 102D F4C8      moveb.ex -2872(a5), d0
60BC: C1FC 001E      muls.ex #30, d0
60C0: 41ED F414      lea -3052(a5), d0
60C4: 2F30 0840      movel.ex 64(a0,d0.l), -(a7)
60C8: 4EBA 07EE      jsr.ex -9024(pc)
60CC: 4EBA A83E      jsr.ex 32020(pc)
60D0: 584F           addqw #4, a7
60D2: 6000 0292      bra 658
60D6: 4A2D F43C      tstb.ex -3012(a5)
60DA: 664A           bne 74
60DC: 4A2D F4D7      tstb.ex -2857(a5)
60E0: 6644           bne 68
60E2: 4A2D F43D      tstb.ex -3011(a5)
60E6: 663E           bne 62
60E8: 4A2D F4D1      tstb.ex -2863(a5)
60EC: 6728           beq 40
60EE: 0C2D 0005 F4CA cmpib.ex #5, -2870(a5)
60F4: 6730           beq 48
60F6: 0C2D 000A F4CA cmpib.ex #10, -2870(a5)
60FC: 6728           beq 40
60FE: 0C2D 000F F4CA cmpib.ex #15, -2870(a5)
6104: 6720           beq 32
6106: 0C2D 0014 F4CA cmpib.ex #20, -2870(a5)
610C: 6718           beq 24
610E: 0C2D 0019 F4CA cmpib.ex #25, -2870(a5)
6114: 6710           beq 16
6116: 0C2D 0003 F414 cmpib.ex #3, -3052(a5)
611C: 6608           bne 8
611E: 4EBA EDCC      jsr.ex -15628(pc)
6122: 6000 0242      bra 578
6126: 202D F41A      movel.ex -3046(a5), d0
612A: 5C80           addql #6, d0
612C: 2B40 F41A      movel.mx d0, -3046(a5)
6130: 4E4F           trap #15
6132: A0F7           sysTrapTimGetTicks
6134: 2600           movel d0, d3
6136: 202D F41A      movel.ex -3046(a5), d0
613A: 9083           subrl d3, d0
613C: 4A80           tstl d0
613E: 6E08           bgt 8
6140: 2003           movel d3, d0
6142: 5C80           addql #6, d0
6144: 2B40 F41A      movel.mx d0, -3046(a5)
6148: 0C2D 0005 F414 cmpib.ex #5, -3052(a5)
614E: 6630           bne 48
6150: 4EBA FE0E      jsr.ex -11416(pc)
6154: 4EBA F00E      jsr.ex -14996(pc)
6158: 4EBA AD1A      jsr.ex -32132(pc)
615C: 4EBA F77E      jsr.ex -13084(pc)
6160: 4EBA FB94      jsr.ex -12034(pc)
6164: 4EBA FD14      jsr.ex -11646(pc)
6168: 4EBA CE6A      jsr.ex -23588(pc)
616C: 4EBA AF20      jsr.ex -31594(pc)
6170: 4A2D F4DB      tstb.ex -2853(a5)
6174: 6706           beq 6
6176: 1B7C 0005 F414 moveb.emx #5, -3052(a5)
617C: 6000 01E8      bra 488
6180: 0C2D 0004 F414 cmpib.ex #4, -3052(a5)
6186: 6608           bne 8
6188: 536D F4CC      subqw.ex #1, -2868(a5)
618C: 6000 0172      bra 370
6190: 0C2D 0005 F4CA cmpib.ex #5, -2870(a5)
6196: 6722           beq 34
6198: 0C2D 000A F4CA cmpib.ex #10, -2870(a5)
619E: 671A           beq 26
61A0: 0C2D 000F F4CA cmpib.ex #15, -2870(a5)
61A6: 6712           beq 18
61A8: 0C2D 0014 F4CA cmpib.ex #20, -2870(a5)
61AE: 670A           beq 10
61B0: 0C2D 0019 F4CA cmpib.ex #25, -2870(a5)
61B6: 6600 009C      bne 156
61BA: 4A2D F4D2      tstb.ex -2862(a5)
61BE: 6714           beq 20
61C0: 7000           moveq #0, d0
61C2: 102D F4D2      moveb.ex -2862(a5), d0
61C6: 7200           moveq #0, d1
61C8: 122D F4D1      moveb.ex -2863(a5), d1
61CC: 9240           subrw d0, d1
61CE: 0C41 0002      cmpiw #2, d1
61D2: 6D6A           blt 106
61D4: 4A2D F43D      tstb.ex -3011(a5)
61D8: 6700 00C8      beq 200
61DC: 0C2D 0001 F4D1 cmpib.ex #1, -2863(a5)
61E2: 655A           bcs 90
61E4: 7000           moveq #0, d0
61E6: 102D F4CA      moveb.ex -2870(a5), d0
61EA: 48C0           extl d0
61EC: 81FC 000A      divs.ex #10, d0
61F0: 5640           addqw #3, d0
61F2: 7200           moveq #0, d1
61F4: 122D F4D1      moveb.ex -2863(a5), d1
61F8: B240           cmpw d0, d1
61FA: 6C00 00A6      bge 166
61FE: 7000           moveq #0, d0
6200: 102D F4CA      moveb.ex -2870(a5), d0
6204: 3200           movew d0, d1
6206: E241           asriw #1, d1
6208: E049           lsriw #0, d1
620A: EC49           lsriw #6, d1
620C: D240           addrw d0, d1
620E: E441           asriw #2, d1
6210: 0641 000E      addiw #14, d1
6214: 48C1           extl d1
6216: 2D41 FFFA      movel.mx d1, -6(a6)
621A: 42A7           clrl -(a7)
621C: 4E4F           trap #15
621E: A0C2           sysTrapSysRandom
6220: C1FC 03E8      muls.ex #1000, d0
6224: 2200           movel d0, d1
6226: E081           asril #0, d1
6228: EC81           asril #6, d1
622A: E089           lsril #0, d1
622C: E089           lsril #0, d1
622E: E289           lsril #1, d1
6230: D280           addrl d0, d1
6232: E081           asril #0, d1
6234: EE81           asril #7, d1
6236: B2AE FFFA      cmpl.ex -6(a6), d1
623A: 584F           addqw #4, a7
623C: 6C64           bge 100
623E: 1F3C 0004      moveb.ex #4, -(a7)
6242: 4EBA D298      jsr.ex -22300(pc)
6246: 544F           addqw #2, a7
6248: 6058           bra 88
624A: 1F3C 0004      moveb.ex #4, -(a7)
624E: 4EBA D28C      jsr.ex -22300(pc)
6252: 544F           addqw #2, a7
6254: 0C2D 0005 F4D7 cmpib.ex #5, -2857(a5)
625A: 6446           bcc 70
625C: 0C2D 0002 F4CA cmpib.ex #2, -2870(a5)
6262: 653E           bcs 62
6264: 7000           moveq #0, d0
6266: 102D F4CA      moveb.ex -2870(a5), d0
626A: 48C0           extl d0
626C: 81FC 0006      divs.ex #6, d0
6270: 5840           addqw #4, d0
6272: 48C0           extl d0
6274: 2D40 FFFA      movel.mx d0, -6(a6)
6278: 42A7           clrl -(a7)
627A: 4E4F           trap #15
627C: A0C2           sysTrapSysRandom
627E: C1FC 03E8      muls.ex #1000, d0
6282: 2200           movel d0, d1
6284: E081           asril #0, d1
6286: EC81           asril #6, d1
6288: E089           lsril #0, d1
628A: E089           lsril #0, d1
628C: E289           lsril #1, d1
628E: D280           addrl d0, d1
6290: E081           asril #0, d1
6292: EE81           asril #7, d1
6294: B2AE FFFA      cmpl.ex -6(a6), d1
6298: 584F           addqw #4, a7
629A: 6DAE           blt -82
629C: 6004           bra 4
629E: 4EBA B25C      jsr.ex -30460(pc)
62A2: 7000           moveq #0, d0
62A4: 102D F4CA      moveb.ex -2870(a5), d0
62A8: 3200           movew d0, d1
62AA: E049           lsriw #0, d1
62AC: EE49           lsriw #7, d1
62AE: D240           addrw d0, d1
62B0: E241           asriw #1, d1
62B2: 48C1           extl d1
62B4: 2D41 FFFA      movel.mx d1, -6(a6)
62B8: 42A7           clrl -(a7)
62BA: 4E4F           trap #15
62BC: A0C2           sysTrapSysRandom
62BE: C1FC 03E8      muls.ex #1000, d0
62C2: 2200           movel d0, d1
62C4: E081           asril #0, d1
62C6: EC81           asril #6, d1
62C8: E089           lsril #0, d1
62CA: E089           lsril #0, d1
62CC: E289           lsril #1, d1
62CE: D280           addrl d0, d1
62D0: E081           asril #0, d1
62D2: EE81           asril #7, d1
62D4: B2AE FFFA      cmpl.ex -6(a6), d1
62D8: 584F           addqw #4, a7
62DA: 6C10           bge 16
62DC: 0C2D 0002 F4CA cmpib.ex #2, -2870(a5)
62E2: 6508           bcs 8
62E4: 0C2D 0019 F4CA cmpib.ex #25, -2870(a5)
62EA: 65B2           bcs -78
62EC: 4A2D F225      tstb.ex -3547(a5)
62F0: 670E           beq 14
62F2: 532D F217      subqb.ex #1, -3561(a5)
62F6: 4A2D F217      tstb.ex -3561(a5)
62FA: 6604           bne 4
62FC: 422D F225      clrb.ex -3547(a5)
6300: 4EBA EFAC      jsr.ex -14666(pc)
6304: 7200           moveq #0, d1
6306: 122D F4C8      moveb.ex -2872(a5), d1
630A: C3FC 001E      muls.ex #30, d1
630E: 41ED F414      lea -3052(a5), d0
6312: 41E8 003C      lea 60(a0), d0
6316: 2180 1800      movel.mx d0, 0(a0,d1.l)
631A: 4EBA FC44      jsr.ex -11416(pc)
631E: 4EBA AB54      jsr.ex -32132(pc)
6322: 4A2D F43C      tstb.ex -3012(a5)
6326: 6704           beq 4
6328: 4EBA B55A      jsr.ex -29556(pc)
632C: 4EBA FABE      jsr.ex -11788(pc)
6330: 4EBA F9C4      jsr.ex -12034(pc)
6334: 4EBA F5A6      jsr.ex -13084(pc)
6338: 4EBA FB40      jsr.ex -11646(pc)
633C: 4EBA FB8C      jsr.ex -11566(pc)
6340: 4EBA FBC8      jsr.ex -11502(pc)
6344: 4A2D F43F      tstb.ex -3009(a5)
6348: 6704           beq 4
634A: 4EBA E104      jsr.ex -18344(pc)
634E: 4EBA B6E0      jsr.ex -29128(pc)
6352: 4EBA CC80      jsr.ex -23588(pc)
6356: 4EBA C1EE      jsr.ex -26290(pc)
635A: 4EBA AD32      jsr.ex -31594(pc)
635E: 4EBA B144      jsr.ex -30548(pc)
6362: 4EBA B33C      jsr.ex -30040(pc)
6366: 261F           movel (a7)+, d3
6368: 4E5E           unlk a6
636A: 4E75           rts
636C: 8F47           dc.w #36679
636E: 616D           bsr 109
6370: 6553           bcs 83
6372: 7461           moveq #97, d2
6374: 7465           moveq #101, d2
6376: 456C           dc.w #17772
6378: 6170           bsr 112
637A: 7365           dc.w #29541
637C: 0000           dc.w #0
637E: 4E56 FFF6      link a6, #-10
6382: 4A2D FCCA      tstb.ex -822(a5)
6386: 6754           beq 84
6388: 1D7C 0001 FFF6 moveb.emx #1, -10(a6)
638E: 7000           moveq #0, d0
6390: 102D FCCA      moveb.ex -822(a5), d0
6394: E788           lslil #3, d0
6396: 41ED FD16      lea -746(a5), d0
639A: 5488           addql #2, a0
639C: 2D70 0800 FFF8 movel.emx 0(a0,d0.l), -8(a6)
63A2: 7000           moveq #0, d0
63A4: 102D FCCA      moveb.ex -822(a5), d0
63A8: E788           lslil #3, d0
63AA: 41ED FD16      lea -746(a5), d0
63AE: 5C88           addql #6, a0
63B0: 3D70 0800 FFFC movew.emx 0(a0,d0.l), -4(a6)
63B6: 3D6D F214 FFFE movew.emx -3564(a5), -2(a6)
63BC: 1F3C 0001      moveb.ex #1, -(a7)
63C0: 486E FFF6      pea.ex -10(a6)
63C4: 42A7           clrl -(a7)
63C6: 4E4F           trap #15
63C8: A233           sysTrapSndDoCmd
63CA: 532D FCCB      subqb.ex #1, -821(a5)
63CE: 4A2D FCCB      tstb.ex -821(a5)
63D2: 4FEF 000A      lea 10(a7), d7
63D6: 6E04           bgt 4
63D8: 422D FCCA      clrb.ex -822(a5)
63DC: 4E5E           unlk a6
63DE: 4E75           rts
63E0: 8E47           orrw d7, d7
63E2: 616D           bsr 109
63E4: 6550           bcs 80
63E6: 6C61           bge 97
63E8: 7953           dc.w #31059
63EA: 6F75           ble 117
63EC: 6E64           bgt 100
63EE: 7300           dc.w #29440
63F0: 0000           dc.w #0
63F2: 4E56 0000      link a6, #0
63F6: 48E7 0030      movem <0030>, -(a7)
63FA: 4E4F           trap #15
63FC: A173           sysTrapFrmGetActiveForm
63FE: 2648           movel a0, a3
6400: 3F3C 0514      movew.ex #1300, -(a7)
6404: 4E4F           trap #15
6406: A16F           sysTrapFrmInitForm
6408: 2448           movel a0, a2
640A: 2F0A           movel a2, -(a7)
640C: 4E4F           trap #15
640E: A174           sysTrapFrmSetActiveForm
6410: 2F0A           movel a2, -(a7)
6412: 4E4F           trap #15
6414: A193           sysTrapFrmDoDialog
6416: 2F0A           movel a2, -(a7)
6418: 4E4F           trap #15
641A: A170           sysTrapFrmDeleteForm
641C: 2F0B           movel a3, -(a7)
641E: 4E4F           trap #15
6420: A174           sysTrapFrmSetActiveForm
6422: 4FEF 0012      lea 18(a7), d7
6426: 4CDF 0C00      movem (a7)+, <0c00>
642A: 4E5E           unlk a6
642C: 4E75           rts
642E: 8B49           dc.w #35657
6430: 6E66           bgt 102
6432: 6F44           ble 68
6434: 6973           bvs 115
6436: 706C           moveq #108, d0
6438: 6179           bsr 121
643A: 0000           dc.w #0
643C: 4E56 FFEE      link a6, #-18
6440: 48E7 1F38      movem <1f38>, -(a7)
6444: 4E4F           trap #15
6446: A173           sysTrapFrmGetActiveForm
6448: 2E08           movel a0, d7
644A: 3F3C 044C      movew.ex #1100, -(a7)
644E: 4E4F           trap #15
6450: A16F           sysTrapFrmInitForm
6452: 2C08           movel a0, d6
6454: 2F06           movel d6, -(a7)
6456: 4E4F           trap #15
6458: A174           sysTrapFrmSetActiveForm
645A: 2F06           movel d6, -(a7)
645C: 4E4F           trap #15
645E: A171           sysTrapFrmDrawForm
6460: 1F3C 0001      moveb.ex #1, -(a7)
6464: 4E4F           trap #15
6466: A164           sysTrapFntSetFont
6468: 1D40 FFEF      moveb.mx d0, -17(a6)
646C: 3F3C 044C      movew.ex #1100, -(a7)
6470: 2F3C 7453 5452 movel.exl #1951618130, -(a7)
6476: 4E4F           trap #15
6478: A05F           sysTrapDmGetResource
647A: 2648           movel a0, a3
647C: 2F0B           movel a3, -(a7)
647E: 4E4F           trap #15
6480: A021           sysTrapMemHandleLock
6482: 2448           movel a0, a2
6484: 2F3C 0011 0010 movel.exl #1114128, -(a7)
648A: 2F0A           movel a2, -(a7)
648C: 4E4F           trap #15
648E: A0C7           sysTrapStrLen
6490: 584F           addqw #4, a7
6492: 3F00           movew d0, -(a7)
6494: 2F0A           movel a2, -(a7)
6496: 4E4F           trap #15
6498: A220           sysTrapWinDrawChars
649A: 2F0A           movel a2, -(a7)
649C: 4E4F           trap #15
649E: A035           sysTrapMemPtrUnlock
64A0: 3F3C 044D      movew.ex #1101, -(a7)
64A4: 2F3C 7453 5452 movel.exl #1951618130, -(a7)
64AA: 4E4F           trap #15
64AC: A05F           sysTrapDmGetResource
64AE: 2648           movel a0, a3
64B0: 2F0B           movel a3, -(a7)
64B2: 4E4F           trap #15
64B4: A021           sysTrapMemHandleLock
64B6: 2448           movel a0, a2
64B8: 3F3C 0010      movew.ex #16, -(a7)
64BC: 2F0A           movel a2, -(a7)
64BE: 4E4F           trap #15
64C0: A0C7           sysTrapStrLen
64C2: 584F           addqw #4, a7
64C4: 3F00           movew d0, -(a7)
64C6: 2F0A           movel a2, -(a7)
64C8: 4E4F           trap #15
64CA: A16B           sysTrapFntCharsWidth
64CC: 7272           moveq #114, d1
64CE: 9240           subrw d0, d1
64D0: 5C4F           addqw #6, a7
64D2: 3F01           movew d1, -(a7)
64D4: 2F0A           movel a2, -(a7)
64D6: 4E4F           trap #15
64D8: A0C7           sysTrapStrLen
64DA: 584F           addqw #4, a7
64DC: 3F00           movew d0, -(a7)
64DE: 2F0A           movel a2, -(a7)
64E0: 4E4F           trap #15
64E2: A220           sysTrapWinDrawChars
64E4: 2F0A           movel a2, -(a7)
64E6: 4E4F           trap #15
64E8: A035           sysTrapMemPtrUnlock
64EA: 3F3C 044E      movew.ex #1102, -(a7)
64EE: 2F3C 7453 5452 movel.exl #1951618130, -(a7)
64F4: 4E4F           trap #15
64F6: A05F           sysTrapDmGetResource
64F8: 2648           movel a0, a3
64FA: 2F0B           movel a3, -(a7)
64FC: 4E4F           trap #15
64FE: A021           sysTrapMemHandleLock
6500: 2448           movel a0, a2
6502: 3F3C 0010      movew.ex #16, -(a7)
6506: 2F0A           movel a2, -(a7)
6508: 4E4F           trap #15
650A: A0C7           sysTrapStrLen
650C: 584F           addqw #4, a7
650E: 3F00           movew d0, -(a7)
6510: 2F0A           movel a2, -(a7)
6512: 4E4F           trap #15
6514: A16B           sysTrapFntCharsWidth
6516: 323C 0099      movew.ex #153, d1
651A: 9240           subrw d0, d1
651C: 5C4F           addqw #6, a7
651E: 3F01           movew d1, -(a7)
6520: 2F0A           movel a2, -(a7)
6522: 4E4F           trap #15
6524: A0C7           sysTrapStrLen
6526: 584F           addqw #4, a7
6528: 3F00           movew d0, -(a7)
652A: 2F0A           movel a2, -(a7)
652C: 4E4F           trap #15
652E: A220           sysTrapWinDrawChars
6530: 2F0A           movel a2, -(a7)
6532: 4E4F           trap #15
6534: A035           sysTrapMemPtrUnlock
6536: 2F3C 0099 001B movel.exl #10027035, -(a7)
653C: 2F3C 0011 001B movel.exl #1114139, -(a7)
6542: 4E4F           trap #15
6544: A213           sysTrapWinDrawLine
6546: 7600           moveq #0, d3
6548: 4FEF 005C      lea 92(a7), d7
654C: 6000 0204      bra 516
6550: 780C           moveq #12, d4
6552: C9C3           muls d3, d4
6554: 0644 001C      addiw #28, d4
6558: 7000           moveq #0, d0
655A: 102D F176      moveb.ex -3722(a5), d0
655E: B640           cmpw d0, d3
6560: 660C           bne 12
6562: 1F3C 0001      moveb.ex #1, -(a7)
6566: 4E4F           trap #15
6568: A164           sysTrapFntSetFont
656A: 544F           addqw #2, a7
656C: 6008           bra 8
656E: 4227           clrb -(a7)
6570: 4E4F           trap #15
6572: A164           sysTrapFntSetFont
6574: 544F           addqw #2, a7
6576: 3003           movew d3, d0
6578: 5240           addqw #1, d0
657A: 48C0           extl d0
657C: 2F00           movel d0, -(a7)
657E: 486E FFF0      pea.ex -16(a6)
6582: 4E4F           trap #15
6584: A0C9           sysTrapStrIToA
6586: 486D FFF8      pea.ex -8(a5)
658A: 486E FFF0      pea.ex -16(a6)
658E: 4E4F           trap #15
6590: A0C6           sysTrapStrCat
6592: 3F04           movew d4, -(a7)
6594: 486E FFF0      pea.ex -16(a6)
6598: 4E4F           trap #15
659A: A0C7           sysTrapStrLen
659C: 584F           addqw #4, a7
659E: 3F00           movew d0, -(a7)
65A0: 486E FFF0      pea.ex -16(a6)
65A4: 4E4F           trap #15
65A6: A16B           sysTrapFntCharsWidth
65A8: 7211           moveq #17, d1
65AA: 9240           subrw d0, d1
65AC: 5C4F           addqw #6, a7
65AE: 3F01           movew d1, -(a7)
65B0: 486E FFF0      pea.ex -16(a6)
65B4: 4E4F           trap #15
65B6: A0C7           sysTrapStrLen
65B8: 584F           addqw #4, a7
65BA: 3F00           movew d0, -(a7)
65BC: 486E FFF0      pea.ex -16(a6)
65C0: 4E4F           trap #15
65C2: A220           sysTrapWinDrawChars
65C4: 3F04           movew d4, -(a7)
65C6: 3F3C 0011      movew.ex #17, -(a7)
65CA: 7016           moveq #22, d0
65CC: C1C3           muls d3, d0
65CE: 41ED F0B0      lea -3920(a5), d0
65D2: 4870 0800      pea.ex 0(a0,d0.l)
65D6: 4E4F           trap #15
65D8: A0C7           sysTrapStrLen
65DA: 584F           addqw #4, a7
65DC: 3F00           movew d0, -(a7)
65DE: 7016           moveq #22, d0
65E0: C1C3           muls d3, d0
65E2: 41ED F0B0      lea -3920(a5), d0
65E6: 4870 0800      pea.ex 0(a0,d0.l)
65EA: 4E4F           trap #15
65EC: A220           sysTrapWinDrawChars
65EE: 7016           moveq #22, d0
65F0: C1C3           muls d3, d0
65F2: 41ED F0B0      lea -3920(a5), d0
65F6: 2F30 0810      movel.ex 16(a0,d0.l), -(a7)
65FA: 486E FFF0      pea.ex -16(a6)
65FE: 4E4F           trap #15
6600: A0C9           sysTrapStrIToA
6602: 3F04           movew d4, -(a7)
6604: 486E FFF0      pea.ex -16(a6)
6608: 4E4F           trap #15
660A: A0C7           sysTrapStrLen
660C: 584F           addqw #4, a7
660E: 3F00           movew d0, -(a7)
6610: 486E FFF0      pea.ex -16(a6)
6614: 4E4F           trap #15
6616: A16B           sysTrapFntCharsWidth
6618: 7272           moveq #114, d1
661A: 9240           subrw d0, d1
661C: 5C4F           addqw #6, a7
661E: 3F01           movew d1, -(a7)
6620: 486E FFF0      pea.ex -16(a6)
6624: 4E4F           trap #15
6626: A0C7           sysTrapStrLen
6628: 584F           addqw #4, a7
662A: 3F00           movew d0, -(a7)
662C: 486E FFF0      pea.ex -16(a6)
6630: 4E4F           trap #15
6632: A220           sysTrapWinDrawChars
6634: 7016           moveq #22, d0
6636: C1C3           muls d3, d0
6638: 41ED F0B0      lea -3920(a5), d0
663C: 41E8 0014      lea 20(a0), d0
6640: 0C70 001B 0800 cmpiw.ex #27, 0(a0,d0.l)
6646: 4FEF 0036      lea 54(a7), d7
664A: 6C52           bge 82
664C: 7016           moveq #22, d0
664E: C1C3           muls d3, d0
6650: 41ED F0B0      lea -3920(a5), d0
6654: 3270 0814      movew.ex 20(a0,d0.l), a1
6658: 2F09           movel a1, -(a7)
665A: 486E FFF0      pea.ex -16(a6)
665E: 4E4F           trap #15
6660: A0C9           sysTrapStrIToA
6662: 3F04           movew d4, -(a7)
6664: 486E FFF0      pea.ex -16(a6)
6668: 4E4F           trap #15
666A: A0C7           sysTrapStrLen
666C: 584F           addqw #4, a7
666E: 3F00           movew d0, -(a7)
6670: 486E FFF0      pea.ex -16(a6)
6674: 4E4F           trap #15
6676: A16B           sysTrapFntCharsWidth
6678: 323C 0099      movew.ex #153, d1
667C: 9240           subrw d0, d1
667E: 5C4F           addqw #6, a7
6680: 3F01           movew d1, -(a7)
6682: 486E FFF0      pea.ex -16(a6)
6686: 4E4F           trap #15
6688: A0C7           sysTrapStrLen
668A: 584F           addqw #4, a7
668C: 3F00           movew d0, -(a7)
668E: 486E FFF0      pea.ex -16(a6)
6692: 4E4F           trap #15
6694: A220           sysTrapWinDrawChars
6696: 4FEF 0012      lea 18(a7), d7
669A: 6000 00B4      bra 180
669E: 7016           moveq #22, d0
66A0: C1C3           muls d3, d0
66A2: 41ED F0B0      lea -3920(a5), d0
66A6: 41E8 0014      lea 20(a0), d0
66AA: 0C70 001E 0800 cmpiw.ex #30, 0(a0,d0.l)
66B0: 6650           bne 80
66B2: 3F3C 044F      movew.ex #1103, -(a7)
66B6: 2F3C 7453 5452 movel.exl #1951618130, -(a7)
66BC: 4E4F           trap #15
66BE: A05F           sysTrapDmGetResource
66C0: 5C4F           addqw #6, a7
66C2: 2F08           movel a0, -(a7)
66C4: 4E4F           trap #15
66C6: A021           sysTrapMemHandleLock
66C8: 2848           movel a0, a4
66CA: 3F04           movew d4, -(a7)
66CC: 2F0C           movel a4, -(a7)
66CE: 4E4F           trap #15
66D0: A0C7           sysTrapStrLen
66D2: 584F           addqw #4, a7
66D4: 3F00           movew d0, -(a7)
66D6: 2F0C           movel a4, -(a7)
66D8: 4E4F           trap #15
66DA: A16B           sysTrapFntCharsWidth
66DC: 323C 0099      movew.ex #153, d1
66E0: 9240           subrw d0, d1
66E2: 5C4F           addqw #6, a7
66E4: 3F01           movew d1, -(a7)
66E6: 2F0C           movel a4, -(a7)
66E8: 4E4F           trap #15
66EA: A0C7           sysTrapStrLen
66EC: 584F           addqw #4, a7
66EE: 3F00           movew d0, -(a7)
66F0: 2F0C           movel a4, -(a7)
66F2: 4E4F           trap #15
66F4: A220           sysTrapWinDrawChars
66F6: 2F0C           movel a4, -(a7)
66F8: 4E4F           trap #15
66FA: A035           sysTrapMemPtrUnlock
66FC: 4FEF 0012      lea 18(a7), d7
6700: 604E           bra 78
6702: 3F3C 0450      movew.ex #1104, -(a7)
6706: 2F3C 7453 5452 movel.exl #1951618130, -(a7)
670C: 4E4F           trap #15
670E: A05F           sysTrapDmGetResource
6710: 5C4F           addqw #6, a7
6712: 2F08           movel a0, -(a7)
6714: 4E4F           trap #15
6716: A021           sysTrapMemHandleLock
6718: 2A08           movel a0, d5
671A: 3F04           movew d4, -(a7)
671C: 2F05           movel d5, -(a7)
671E: 4E4F           trap #15
6720: A0C7           sysTrapStrLen
6722: 584F           addqw #4, a7
6724: 3F00           movew d0, -(a7)
6726: 2F05           movel d5, -(a7)
6728: 4E4F           trap #15
672A: A16B           sysTrapFntCharsWidth
672C: 323C 0099      movew.ex #153, d1
6730: 9240           subrw d0, d1
6732: 5C4F           addqw #6, a7
6734: 3F01           movew d1, -(a7)
6736: 2F05           movel d5, -(a7)
6738: 4E4F           trap #15
673A: A0C7           sysTrapStrLen
673C: 584F           addqw #4, a7
673E: 3F00           movew d0, -(a7)
6740: 2F05           movel d5, -(a7)
6742: 4E4F           trap #15
6744: A220           sysTrapWinDrawChars
6746: 2F05           movel d5, -(a7)
6748: 4E4F           trap #15
674A: A035           sysTrapMemPtrUnlock
674C: 4FEF 0012      lea 18(a7), d7
6750: 5243           addqw #1, d3
6752: 0C43 0009      cmpiw #9, d3
6756: 6C14           bge 20
6758: 7016           moveq #22, d0
675A: C1C3           muls d3, d0
675C: 41ED F0B0      lea -3920(a5), d0
6760: 5088           addql #0, a0
6762: 5088           addql #0, a0
6764: 4AB0 0800      tstl.ex 0(a0,d0.l)
6768: 6E00 FDE6      bgt 64998
676C: 1F2E FFEF      moveb.ex -17(a6), -(a7)
6770: 4E4F           trap #15
6772: A164           sysTrapFntSetFont
6774: 2F06           movel d6, -(a7)
6776: 4E4F           trap #15
6778: A193           sysTrapFrmDoDialog
677A: 2F06           movel d6, -(a7)
677C: 4E4F           trap #15
677E: A170           sysTrapFrmDeleteForm
6780: 2F07           movel d7, -(a7)
6782: 4E4F           trap #15
6784: A174           sysTrapFrmSetActiveForm
6786: 4FEF 000E      lea 14(a7), d7
678A: 4CDF 1CF8      movem (a7)+, <1cf8>
678E: 4E5E           unlk a6
6790: 4E75           rts
6792: 9148           subxmw -(a0), -(a0)
6794: 6967           bvs 103
6796: 6853           bvc 83
6798: 636F           bls 111
679A: 7265           moveq #101, d1
679C: 7344           dc.w #29508
679E: 6973           bvs 115
67A0: 706C           moveq #108, d0
67A2: 6179           bsr 121
67A4: 0000           dc.w #0
67A6: 4E56 0000      link a6, #0
67AA: 48E7 1E20      movem <1e20>, -(a7)
67AE: 246E 0008      movel.ex 8(a6), a2
67B2: 282E 000C      movel.ex 12(a6), d4
67B6: 3A2E 0010      movew.ex 16(a6), d5
67BA: 1C2E 0012      moveb.ex 18(a6), d6
67BE: 7609           moveq #9, d3
67C0: 6002           bra 2
67C2: 5343           subqw #1, d3
67C4: 4A43           tstw d3
67C6: 6F16           ble 22
67C8: 3003           movew d3, d0
67CA: 5340           subqw #1, d0
67CC: C1FC 0016      muls.ex #22, d0
67D0: 41ED F0B0      lea -3920(a5), d0
67D4: 5088           addql #0, a0
67D6: 5088           addql #0, a0
67D8: B8B0 0800      cmpl.ex 0(a0,d0.l), d4
67DC: 6EE4           bgt -28
67DE: 0C43 0009      cmpiw #9, d3
67E2: 6C00 00B6      bge 182
67E6: 4A06           tstb d6
67E8: 674C           beq 76
67EA: 4A43           tstw d3
67EC: 6F48           ble 72
67EE: 3003           movew d3, d0
67F0: 5340           subqw #1, d0
67F2: C1FC 0016      muls.ex #22, d0
67F6: 41ED F0B0      lea -3920(a5), d0
67FA: 4870 0800      pea.ex 0(a0,d0.l)
67FE: 2F0A           movel a2, -(a7)
6800: 4E4F           trap #15
6802: A0C8           sysTrapStrCompare
6804: 4A40           tstw d0
6806: 504F           addqw #0, a7
6808: 662C           bne 44
680A: 3003           movew d3, d0
680C: 5340           subqw #1, d0
680E: C1FC 0016      muls.ex #22, d0
6812: 41ED F0B0      lea -3920(a5), d0
6816: 5088           addql #0, a0
6818: 5088           addql #0, a0
681A: B8B0 0800      cmpl.ex 0(a0,d0.l), d4
681E: 6616           bne 22
6820: 3003           movew d3, d0
6822: 5340           subqw #1, d0
6824: C1FC 0016      muls.ex #22, d0
6828: 41ED F0B0      lea -3920(a5), d0
682C: 41E8 0014      lea 20(a0), d0
6830: BA70 0800      cmpw.ex 0(a0,d0.l), d5
6834: 6764           beq 100
6836: 7008           moveq #8, d0
6838: 9043           subrw d3, d0
683A: C0FC 0016      mulu.ex #22, d0
683E: 2F00           movel d0, -(a7)
6840: 7016           moveq #22, d0
6842: C1C3           muls d3, d0
6844: 41ED F0B0      lea -3920(a5), d0
6848: D1C0           addal d0, a0
684A: 4850           pea (a0)
684C: 3003           movew d3, d0
684E: 5240           addqw #1, d0
6850: C1FC 0016      muls.ex #22, d0
6854: 41ED F0B0      lea -3920(a5), d0
6858: D1C0           addal d0, a0
685A: 4850           pea (a0)
685C: 4E4F           trap #15
685E: A026           sysTrapMemMove
6860: 7016           moveq #22, d0
6862: C1C3           muls d3, d0
6864: 41ED F0B0      lea -3920(a5), d0
6868: 5088           addql #0, a0
686A: 5088           addql #0, a0
686C: 2184 0800      movel.mx d4, 0(a0,d0.l)
6870: 7016           moveq #22, d0
6872: C1C3           muls d3, d0
6874: 41ED F0B0      lea -3920(a5), d0
6878: 41E8 0014      lea 20(a0), d0
687C: 3185 0800      movew.mx d5, 0(a0,d0.l)
6880: 2F0A           movel a2, -(a7)
6882: 7016           moveq #22, d0
6884: C1C3           muls d3, d0
6886: 41ED F0B0      lea -3920(a5), d0
688A: 4870 0800      pea.ex 0(a0,d0.l)
688E: 4E4F           trap #15
6890: A0C5           sysTrapStrCopy
6892: 1B43 F176      moveb.mx d3, -3722(a5)
6896: 4FEF 0014      lea 20(a7), d7
689A: 4CDF 0478      movem (a7)+, <0478>
689E: 4E5E           unlk a6
68A0: 4E75           rts
68A2: 9248           subrw a0, d1
68A4: 6967           bvs 103
68A6: 6853           bvc 83
68A8: 636F           bls 111
68AA: 7265           moveq #101, d1
68AC: 7341           dc.w #29505
68AE: 6464           bcc 100
68B0: 5363           subqw #1, -(a3)
68B2: 6F72           ble 114
68B4: 6500 0000      bcs 0
68B8: 4E56 0000      link a6, #0
68BC: 48E7 1F38      movem <1f38>, -(a7)
68C0: 7609           moveq #9, d3
68C2: 6002           bra 2
68C4: 5343           subqw #1, d3
68C6: 4A43           tstw d3
68C8: 6F1A           ble 26
68CA: 3003           movew d3, d0
68CC: 5340           subqw #1, d0
68CE: C1FC 0016      muls.ex #22, d0
68D2: 41ED F0B0      lea -3920(a5), d0
68D6: 5088           addql #0, a0
68D8: 5088           addql #0, a0
68DA: 222E 0008      movel.ex 8(a6), d1
68DE: B2B0 0800      cmpl.ex 0(a0,d0.l), d1
68E2: 6EE0           bgt -32
68E4: 0C43 0009      cmpiw #9, d3
68E8: 6C00 011C      bge 284
68EC: 4878 0029      pea.ex (0029).w
68F0: 4E4F           trap #15
68F2: A01E           sysTrapMemHandleNew
68F4: 2848           movel a0, a4
68F6: 2F0C           movel a4, -(a7)
68F8: 4E4F           trap #15
68FA: A021           sysTrapMemHandleLock
68FC: 2448           movel a0, a2
68FE: 0C2D 0009 F176 cmpib.ex #9, -3722(a5)
6904: 504F           addqw #0, a7
6906: 671C           beq 28
6908: 7000           moveq #0, d0
690A: 102D F176      moveb.ex -3722(a5), d0
690E: C1FC 0016      muls.ex #22, d0
6912: 41ED F0B0      lea -3920(a5), d0
6916: 4870 0800      pea.ex 0(a0,d0.l)
691A: 2F0A           movel a2, -(a7)
691C: 4E4F           trap #15
691E: A0C5           sysTrapStrCopy
6920: 504F           addqw #0, a7
6922: 6036           bra 54
6924: 486D FFFB      pea.ex -5(a5)
6928: 2F0A           movel a2, -(a7)
692A: 4E4F           trap #15
692C: A0C5           sysTrapStrCopy
692E: 42A7           clrl -(a7)
6930: 42A7           clrl -(a7)
6932: 2F0A           movel a2, -(a7)
6934: 42A7           clrl -(a7)
6936: 42A7           clrl -(a7)
6938: 42A7           clrl -(a7)
693A: 4E4F           trap #15
693C: A2A9           sysTrapDlkGetSyncInfo
693E: 3F3C 0020      movew.ex #32, -(a7)
6942: 2F0A           movel a2, -(a7)
6944: 4E4F           trap #15
6946: A0CC           sysTrapStrChr
6948: 2A08           movel a0, d5
694A: 4A85           tstl d5
694C: 4FEF 0026      lea 38(a7), d7
6950: 6704           beq 4
6952: 2045           movel d5, a0
6954: 4210           clrb (a0)
6956: 422A 000F      clrb.ex 15(a2)
695A: 2F0A           movel a2, -(a7)
695C: 4E4F           trap #15
695E: A035           sysTrapMemPtrUnlock
6960: 1B43 F176      moveb.mx d3, -3722(a5)
6964: 4E4F           trap #15
6966: A173           sysTrapFrmGetActiveForm
6968: 2E08           movel a0, d7
696A: 3F3C 04B0      movew.ex #1200, -(a7)
696E: 4E4F           trap #15
6970: A16F           sysTrapFrmInitForm
6972: 2648           movel a0, a3
6974: 3F3C 04B3      movew.ex #1203, -(a7)
6978: 2F0B           movel a3, -(a7)
697A: 4E4F           trap #15
697C: A180           sysTrapFrmGetObjectIndex
697E: 3800           movew d0, d4
6980: 2F0C           movel a4, -(a7)
6982: 3F04           movew d4, -(a7)
6984: 2F0B           movel a3, -(a7)
6986: 4E4F           trap #15
6988: A183           sysTrapFrmGetObjectPtr
698A: 5C4F           addqw #6, a7
698C: 2F08           movel a0, -(a7)
698E: 4E4F           trap #15
6990: A158           sysTrapFldSetTextHandle
6992: 3F04           movew d4, -(a7)
6994: 2F0B           movel a3, -(a7)
6996: 4E4F           trap #15
6998: A179           sysTrapFrmSetFocus
699A: 1F3C 0001      moveb.ex #1, -(a7)
699E: 4227           clrb -(a7)
69A0: 4227           clrb -(a7)
69A2: 4E4F           trap #15
69A4: A281           sysTrapGrfSetState
69A6: 2F0B           movel a3, -(a7)
69A8: 4E4F           trap #15
69AA: A193           sysTrapFrmDoDialog
69AC: 3C00           movew d0, d6
69AE: 42A7           clrl -(a7)
69B0: 3F04           movew d4, -(a7)
69B2: 2F0B           movel a3, -(a7)
69B4: 4E4F           trap #15
69B6: A183           sysTrapFrmGetObjectPtr
69B8: 5C4F           addqw #6, a7
69BA: 2F08           movel a0, -(a7)
69BC: 4E4F           trap #15
69BE: A158           sysTrapFldSetTextHandle
69C0: 2F0B           movel a3, -(a7)
69C2: 4E4F           trap #15
69C4: A170           sysTrapFrmDeleteForm
69C6: 2F07           movel d7, -(a7)
69C8: 4E4F           trap #15
69CA: A174           sysTrapFrmSetActiveForm
69CC: 0C46 04B4      cmpiw #1204, d6
69D0: 4FEF 0034      lea 52(a7), d7
69D4: 661E           bne 30
69D6: 4A12           tstb (a2)
69D8: 671A           beq 26
69DA: 4227           clrb -(a7)
69DC: 7000           moveq #0, d0
69DE: 102D F4CA      moveb.ex -2870(a5), d0
69E2: 5240           addqw #1, d0
69E4: 3F00           movew d0, -(a7)
69E6: 2F2E 0008      movel.ex 8(a6), -(a7)
69EA: 2F0A           movel a2, -(a7)
69EC: 4EBA FDB8      jsr.ex -9298(pc)
69F0: 4FEF 000C      lea 12(a7), d7
69F4: 2F0C           movel a4, -(a7)
69F6: 4E4F           trap #15
69F8: A02B           sysTrapMemHandleFree
69FA: 0C46 04B4      cmpiw #1204, d6
69FE: 584F           addqw #4, a7
6A00: 6604           bne 4
6A02: 4EBA FA38      jsr.ex -10172(pc)
6A06: 4CDF 1CF8      movem (a7)+, <1cf8>
6A0A: 4E5E           unlk a6
6A0C: 4E75           rts
6A0E: 9448           subrw a0, d2
6A10: 6967           bvs 103
6A12: 6853           bvc 83
6A14: 636F           bls 111
6A16: 7265           moveq #101, d1
6A18: 7343           dc.w #29507
6A1A: 6865           bvc 101
6A1C: 636B           bls 107
6A1E: 5363           subqw #1, -(a3)
6A20: 6F72           ble 114
6A22: 6500 0000      bcs 0
6A26: 4E56 0000      link a6, #0
6A2A: 302E 0008      movew.ex 8(a6), d0
6A2E: 0440 0064      subiw #100, d0
6A32: 670E           beq 14
6A34: 5340           subqw #1, d0
6A36: 671A           beq 26
6A38: 5340           subqw #1, d0
6A3A: 6722           beq 34
6A3C: 5540           subqw #2, d0
6A3E: 670C           beq 12
6A40: 6020           bra 32
6A42: 4EBA E4FE      jsr.ex -15542(pc)
6A46: 4EBA DD72      jsr.ex -17470(pc)
6A4A: 6016           bra 22
6A4C: 4EBA F9A4      jsr.ex -10246(pc)
6A50: 6010           bra 16
6A52: 3F3C 03E8      movew.ex #1000, -(a7)
6A56: 4E4F           trap #15
6A58: A195           sysTrapFrmHelp
6A5A: 544F           addqw #2, a7
6A5C: 6004           bra 4
6A5E: 4EBA F9DC      jsr.ex -10172(pc)
6A62: 7001           moveq #1, d0
6A64: 4E5E           unlk a6
6A66: 4E75           rts
6A68: 914D           subxmw -(a5), -(a0)
6A6A: 6169           bsr 105
6A6C: 6E56           bgt 86
6A6E: 6965           bvs 101
6A70: 7744           dc.w #30532
6A72: 6F43           ble 67
6A74: 6F6D           ble 109
6A76: 6D61           blt 97
6A78: 6E64           bgt 100
6A7A: 0000           dc.w #0
6A7C: 4E56 FFE0      link a6, #-32
6A80: 48E7 1830      movem <1830>, -(a7)
6A84: 246E 0008      movel.ex 8(a6), a2
6A88: 7600           moveq #0, d3
6A8A: 4A52           tstw (a2)
6A8C: 6700 011E      beq 286
6A90: 0C52 0004      cmpiw #4, (a2)
6A94: 6600 00CE      bne 206
6A98: 0C6A 0074 0008 cmpiw.ex #116, 8(a2)
6A9E: 6600 00A8      bne 168
6AA2: 4E4F           trap #15
6AA4: A0F7           sysTrapTimGetTicks
6AA6: 90AD F424      subrl.ex -3036(a5), d0
6AAA: D0AD F178      addrl.ex -3720(a5), d0
6AAE: 2D40 FFE0      movel.mx d0, -32(a6)
6AB2: 4E4F           trap #15
6AB4: A2E9           sysTrapSysTicksPerSecond
6AB6: 7200           moveq #0, d1
6AB8: 3200           movew d0, d1
6ABA: 202E FFE0      movel.ex -32(a6), d0
6ABE: 4EBA 9908      jsr.ex 30672(pc)
6AC2: 2800           movel d0, d4
6AC4: 486E FFE4      pea.ex -28(a6)
6AC8: 2F04           movel d4, -(a7)
6ACA: 4E4F           trap #15
6ACC: A0FC           sysTrapTimSecondsToDateTime
6ACE: 486E FFF2      pea.ex -14(a6)
6AD2: 1F3C 0002      moveb.ex #2, -(a7)
6AD6: 1F2E FFE7      moveb.ex -25(a6), -(a7)
6ADA: 1F2E FFE9      moveb.ex -23(a6), -(a7)
6ADE: 4E4F           trap #15
6AE0: A268           sysTrapTimeToAscii
6AE2: 486D FFFC      pea.ex -4(a5)
6AE6: 486E FFF2      pea.ex -14(a6)
6AEA: 4E4F           trap #15
6AEC: A0C6           sysTrapStrCat
6AEE: 0C6E 000A FFE4 cmpiw.ex #10, -28(a6)
6AF4: 4FEF 001A      lea 26(a7), d7
6AF8: 6C0E           bge 14
6AFA: 486D FFFE      pea.ex -2(a5)
6AFE: 486E FFF2      pea.ex -14(a6)
6B02: 4E4F           trap #15
6B04: A0C6           sysTrapStrCat
6B06: 504F           addqw #0, a7
6B08: 306E FFE4      movew.ex -28(a6), a0
6B0C: 2F08           movel a0, -(a7)
6B0E: 486E FFF2      pea.ex -14(a6)
6B12: 4E4F           trap #15
6B14: A0C7           sysTrapStrLen
6B16: 7200           moveq #0, d1
6B18: 3200           movew d0, d1
6B1A: 41EE FFF2      lea -14(a6), d0
6B1E: 584F           addqw #4, a7
6B20: 4870 1800      pea.ex 0(a0,d1.l)
6B24: 4E4F           trap #15
6B26: A0C9           sysTrapStrIToA
6B28: 2F3C 0046 0082 movel.exl #4587650, -(a7)
6B2E: 486E FFF2      pea.ex -14(a6)
6B32: 4E4F           trap #15
6B34: A0C7           sysTrapStrLen
6B36: 584F           addqw #4, a7
6B38: 3F00           movew d0, -(a7)
6B3A: 486E FFF2      pea.ex -14(a6)
6B3E: 4E4F           trap #15
6B40: A220           sysTrapWinDrawChars
6B42: 4FEF 0012      lea 18(a7), d7
6B46: 6018           bra 24
6B48: 0C6A 000C 0008 cmpiw.ex #12, 8(a2)
6B4E: 6610           bne 16
6B50: 0C2D 0007 F414 cmpib.ex #7, -3052(a5)
6B56: 6608           bne 8
6B58: 4EBA E3E8      jsr.ex -15542(pc)
6B5C: 4EBA DC5C      jsr.ex -17470(pc)
6B60: 7001           moveq #1, d0
6B62: 604A           bra 74
6B64: 0C52 0015      cmpiw #21, (a2)
6B68: 660E           bne 14
6B6A: 3F2A 0008      movew.ex 8(a2), -(a7)
6B6E: 4EBA FEB6      jsr.ex -8658(pc)
6B72: 7001           moveq #1, d0
6B74: 544F           addqw #2, a7
6B76: 6036           bra 54
6B78: 0C52 001C      cmpiw #28, (a2)
6B7C: 672E           beq 46
6B7E: 0C52 0018      cmpiw #24, (a2)
6B82: 661E           bne 30
6B84: 4E4F           trap #15
6B86: A173           sysTrapFrmGetActiveForm
6B88: 2648           movel a0, a3
6B8A: 2F0B           movel a3, -(a7)
6B8C: 4E4F           trap #15
6B8E: A171           sysTrapFrmDrawForm
6B90: 4EBA E3B0      jsr.ex -15542(pc)
6B94: 4EBA DE72      jsr.ex -16880(pc)
6B98: 4EBA DC20      jsr.ex -17470(pc)
6B9C: 7601           moveq #1, d3
6B9E: 584F           addqw #4, a7
6BA0: 600A           bra 10
6BA2: 0C52 001A      cmpiw #26, (a2)
6BA6: 6604           bne 4
6BA8: 4EBA DDF0      jsr.ex -16990(pc)
6BAC: 1003           moveb d3, d0
6BAE: 4CDF 0C18      movem (a7)+, <0c18>
6BB2: 4E5E           unlk a6
6BB4: 4E75           rts
6BB6: 934D           subxmw -(a5), -(a1)
6BB8: 6169           bsr 105
6BBA: 6E56           bgt 86
6BBC: 6965           bvs 101
6BBE: 7748           dc.w #30536
6BC0: 616E           bsr 110
6BC2: 646C           bcc 108
6BC4: 6545           bcs 69
6BC6: 7665           moveq #101, d3
6BC8: 6E74           bgt 116
6BCA: 0000           dc.w #0
6BCC: 4E56 0000      link a6, #0
6BD0: 48E7 1030      movem <1030>, -(a7)
6BD4: 266E 0008      movel.ex 8(a6), a3
6BD8: 0C53 0017      cmpiw #23, (a3)
6BDC: 662E           bne 46
6BDE: 362B 0008      movew.ex 8(a3), d3
6BE2: 3F03           movew d3, -(a7)
6BE4: 4E4F           trap #15
6BE6: A16F           sysTrapFrmInitForm
6BE8: 2448           movel a0, a2
6BEA: 2F0A           movel a2, -(a7)
6BEC: 4E4F           trap #15
6BEE: A174           sysTrapFrmSetActiveForm
6BF0: 5C4F           addqw #6, a7
6BF2: 3003           movew d3, d0
6BF4: 0440 03E8      subiw #1000, d0
6BF8: 6702           beq 2
6BFA: 600C           bra 12
6BFC: 487A FE7E      pea.ex -8572(pc)
6C00: 2F0A           movel a2, -(a7)
6C02: 4E4F           trap #15
6C04: A19F           sysTrapFrmSetEventHandler
6C06: 504F           addqw #0, a7
6C08: 7001           moveq #1, d0
6C0A: 6002           bra 2
6C0C: 7000           moveq #0, d0
6C0E: 4CDF 0C08      movem (a7)+, <0c08>
6C12: 4E5E           unlk a6
6C14: 4E75           rts
6C16: 9641           subrw d1, d3
6C18: 7070           moveq #112, d0
6C1A: 6C69           bge 105
6C1C: 6361           bls 97
6C1E: 7469           moveq #105, d2
6C20: 6F6E           ble 110
6C22: 4861           dc.w #18529
6C24: 6E64           bgt 100
6C26: 6C65           bge 101
6C28: 4576           dc.w #17782
6C2A: 656E           bcs 110
6C2C: 7400           moveq #0, d2
6C2E: 0000           dc.w #0
6C30: 4E56 FFDE      link a6, #-34
6C34: 4EBA 982A      jsr.ex 30824(pc)
6C38: 2F00           movel d0, -(a7)
6C3A: 486E FFE8      pea.ex -24(a6)
6C3E: 4E4F           trap #15
6C40: A11D           sysTrapEvtGetEvent
6C42: 0C6E 0006 FFE8 cmpiw.ex #6, -24(a6)
6C48: 504F           addqw #0, a7
6C4A: 6624           bne 36
6C4C: 3F3C 03E8      movew.ex #1000, -(a7)
6C50: 4E4F           trap #15
6C52: A17E           sysTrapFrmGetFormPtr
6C54: B1EE FFF4      cmpal.ex -12(a6), a0
6C58: 544F           addqw #2, a7
6C5A: 6600 0098      bne 152
6C5E: 1B7C 0001 F41E moveb.emx #1, -3042(a5)
6C64: 4E4F           trap #15
6C66: A0F7           sysTrapTimGetTicks
6C68: 2B40 F420      movel.mx d0, -3040(a5)
6C6C: 6000 0086      bra 134
6C70: 0C6E 0005 FFE8 cmpiw.ex #5, -24(a6)
6C76: 6658           bne 88
6C78: 3F3C 03E8      movew.ex #1000, -(a7)
6C7C: 4E4F           trap #15
6C7E: A17E           sysTrapFrmGetFormPtr
6C80: B1EE FFF0      cmpal.ex -16(a6), a0
6C84: 544F           addqw #2, a7
6C86: 666C           bne 108
6C88: 4E4F           trap #15
6C8A: A197           sysTrapFrmGetFirstForm
6C8C: B1EE FFF0      cmpal.ex -16(a6), a0
6C90: 6662           bne 98
6C92: 4A2D F41E      tstb.ex -3042(a5)
6C96: 660C           bne 12
6C98: 4E4F           trap #15
6C9A: A0F7           sysTrapTimGetTicks
6C9C: 5C80           addql #6, d0
6C9E: 2B40 F41A      movel.mx d0, -3046(a5)
6CA2: 601C           bra 28
6CA4: 422D F41E      clrb.ex -3042(a5)
6CA8: 4E4F           trap #15
6CAA: A0F7           sysTrapTimGetTicks
6CAC: 90AD F420      subrl.ex -3040(a5), d0
6CB0: D1AD F41A      addml.ex d0, -3046(a5)
6CB4: 4E4F           trap #15
6CB6: A0F7           sysTrapTimGetTicks
6CB8: 90AD F420      subrl.ex -3040(a5), d0
6CBC: D1AD F424      addml.ex d0, -3036(a5)
6CC0: 4A2D F0AF      tstb.ex -3921(a5)
6CC4: 672E           beq 46
6CC6: 4EBA DCD2      jsr.ex -16990(pc)
6CCA: 422D F0AF      clrb.ex -3921(a5)
6CCE: 6024           bra 36
6CD0: 4EBA 978E      jsr.ex 30824(pc)
6CD4: 4A80           tstl d0
6CD6: 661C           bne 28
6CD8: 4EBA DAE0      jsr.ex -17470(pc)
6CDC: 4EBA E232      jsr.ex -15592(pc)
6CE0: 4EBA F37E      jsr.ex -11160(pc)
6CE4: 4EBA DD22      jsr.ex -16880(pc)
6CE8: 4EBA F694      jsr.ex -10362(pc)
6CEC: 4A6E FFE8      tstw.ex -24(a6)
6CF0: 6700 0092      beq 146
6CF4: 0C6E 0004 FFE8 cmpiw.ex #4, -24(a6)
6CFA: 664C           bne 76
6CFC: 0C6E 0204 FFF0 cmpiw.ex #516, -16(a6)
6D02: 651A           bcs 26
6D04: 0C6E 0207 FFF0 cmpiw.ex #519, -16(a6)
6D0A: 6212           bhi 18
6D0C: 0C2D 0007 F414 cmpib.ex #7, -3052(a5)
6D12: 670A           beq 10
6D14: 302E FFF4      movew.ex -12(a6), d0
6D18: 0240 0100      andiw #256, d0
6D1C: 6766           beq 102
6D1E: 0C6E 0108 FFF0 cmpiw.ex #264, -16(a6)
6D24: 6522           bcs 34
6D26: 486E FFDE      pea.ex -34(a6)
6D2A: 486E FFE4      pea.ex -28(a6)
6D2E: 4267           clrw -(a7)
6D30: 4E4F           trap #15
6D32: A00B           sysTrapMemHeapFreeBytes
6D34: 0CAE 0000 0FA0 FFE4 cmpil.ex #4000, -28(a6)
6D3C: 4FEF 000A      lea 10(a7), d7
6D40: 6206           bhi 6
6D42: 1B7C 0001 F0AF moveb.emx #1, -3921(a5)
6D48: 486E FFE8      pea.ex -24(a6)
6D4C: 4E4F           trap #15
6D4E: A0A9           sysTrapSysHandleEvent
6D50: 4A00           tstb d0
6D52: 584F           addqw #4, a7
6D54: 662E           bne 46
6D56: 486E FFE2      pea.ex -30(a6)
6D5A: 486E FFE8      pea.ex -24(a6)
6D5E: 42A7           clrl -(a7)
6D60: 4E4F           trap #15
6D62: A1BF           sysTrapMenuHandleEvent
6D64: 4A00           tstb d0
6D66: 4FEF 000C      lea 12(a7), d7
6D6A: 6618           bne 24
6D6C: 486E FFE8      pea.ex -24(a6)
6D70: 4EBA FE5A      jsr.ex -8236(pc)
6D74: 4A00           tstb d0
6D76: 584F           addqw #4, a7
6D78: 660A           bne 10
6D7A: 486E FFE8      pea.ex -24(a6)
6D7E: 4E4F           trap #15
6D80: A1A0           sysTrapFrmDispatchEvent
6D82: 584F           addqw #4, a7
6D84: 0C6E 0016 FFE8 cmpiw.ex #22, -24(a6)
6D8A: 6600 FEA8      bne 65192
6D8E: 4E5E           unlk a6
6D90: 4E75           rts
6D92: 8945           dc.w #35141
6D94: 7665           moveq #101, d3
6D96: 6E74           bgt 116
6D98: 4C6F           dc.w #19567
6D9A: 6F70           ble 112
6D9C: 0000           dc.w #0
6D9E: 4E56 0000      link a6, #0
6DA2: 48E7 1C00      movem <1c00>, -(a7)
6DA6: 3A2E 0008      movew.ex 8(a6), d5
6DAA: 382E 000E      movew.ex 14(a6), d4
6DAE: 3F04           movew d4, -(a7)
6DB0: 2F3C 0200 0000 movel.exl #33554432, -(a7)
6DB6: 4EBA 9A46      jsr.ex 31750(pc)
6DBA: 3600           movew d0, d3
6DBC: 4A43           tstw d3
6DBE: 5C4F           addqw #6, a7
6DC0: 6706           beq 6
6DC2: 3043           movew d3, a0
6DC4: 2008           movel a0, d0
6DC6: 6022           bra 34
6DC8: 4A45           tstw d5
6DCA: 661C           bne 28
6DCC: 4EBA 96EA      jsr.ex 30912(pc)
6DD0: 3600           movew d0, d3
6DD2: 3F3C 03E8      movew.ex #1000, -(a7)
6DD6: 4E4F           trap #15
6DD8: A19B           sysTrapFrmGotoForm
6DDA: 4A43           tstw d3
6DDC: 544F           addqw #2, a7
6DDE: 6604           bne 4
6DE0: 4EBA FE4E      jsr.ex -8136(pc)
6DE4: 4EBA 9944      jsr.ex 31538(pc)
6DE8: 7000           moveq #0, d0
6DEA: 4CDF 0038      movem (a7)+, <0038>
6DEE: 4E5E           unlk a6
6DF0: 4E75           rts
6DF2: 8950           ormw d4, (a0)
6DF4: 696C           bvs 108
6DF6: 6F74           ble 116
6DF8: 4D61           dc.w #19809
6DFA: 696E           bvs 110
6DFC: 0000           dc.w #0

