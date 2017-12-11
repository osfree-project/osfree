019A: 0000 0001      orib #1, d0
019E: 487A 0004      pea.ex 30124(pc)
01A2: 0697 0000 0006 addil #6, (a7)
01A8: 4E75           rts
01AA: 4E56 FFF4      link a6, #-12
01AE: 48E7 1800      movem <1800>, -(a7)
01B2: 486E FFF4      pea.ex -12(a6)
01B6: 486E FFF8      pea.ex -8(a6)
01BA: 486E FFFC      pea.ex -4(a6)
01BE: 4E4F           trap #15
01C0: A08F           sysTrapSysAppStartup
01C2: 3800           movew d0, d4
01C4: 4A44           tstw d4
01C6: 4FEF 000C      lea 12(a7), d7
01CA: 671C           beq 28
01CC: 41FA 0066      lea 30268(pc), d0
01D0: 4850           pea (a0)
01D2: 3F3C 005C      movew.ex #92, -(a7)
01D6: 41FA 0078      lea 30296(pc), d0
01DA: 4850           pea (a0)
01DC: 4E4F           trap #15
01DE: A084           sysTrapErrDisplayFileLineMsg
01E0: 7000           moveq #0, d0
01E2: 4FEF 000A      lea 10(a7), d7
01E6: 6036           bra 54
01E8: 206E FFFC      movel.ex -4(a6), a0
01EC: 3F28 0006      movew.ex 6(a0), -(a7)
01F0: 2F28 0002      movel.ex 2(a0), -(a7)
01F4: 3F10           movew (a0), -(a7)
01F6: 487A 000E      pea.ex 30222(pc)
01FA: 487A 0004      pea.ex 30216(pc)
01FE: 0697 0000 697C addil #27004, (a7)
0204: 4E75           rts
0206: 2600           movel d0, d3
0208: 2F2E FFF4      movel.ex -12(a6), -(a7)
020C: 2F2E FFF8      movel.ex -8(a6), -(a7)
0210: 2F2E FFFC      movel.ex -4(a6), -(a7)
0214: 4E4F           trap #15
0216: A090           sysTrapSysAppExit
0218: 2003           movel d3, d0
021A: 4FEF 0014      lea 20(a7), d7
021E: 4CDF 0018      movem (a7)+, <0018>
0222: 4E5E           unlk a6
0224: 4E75           rts
0226: 8B5F           ormw d5, (a7)+
0228: 5F53           subqw #7, (a3)
022A: 7461           moveq #97, d2
022C: 7274           moveq #116, d1
022E: 7570           dc.w #30064
0230: 5F5F           subqw #7, (a7)+
0232: 002A           dc.w #42
0234: 4572           dc.w #17778
0236: 726F           moveq #111, d1
0238: 7220           moveq #32, d1
023A: 6C61           bge 97
023C: 756E           dc.w #30062
023E: 6368           bls 104
0240: 696E           bvs 110
0242: 6720           beq 32
0244: 6170           bsr 112
0246: 706C           moveq #108, d0
0248: 6963           bvs 99
024A: 6174           bsr 116
024C: 696F           bvs 111
024E: 6E00 5374      bgt 21364
0252: 6172           bsr 114
0254: 7475           moveq #117, d2
0256: 7043           moveq #67, d0
0258: 6F64           ble 100
025A: 652E           bcs 46
025C: 6300 4E56      bls 20054
0260: FFAC           dc.w #65452
0262: 48E7 1820      movem <1820>, -(a7)
0266: 7600           moveq #0, d3
0268: 486E FFB4      pea.ex -76(a6)
026C: 4E4F           trap #15
026E: A22C           sysTrapPrefGetPreferences
0270: 1B6E FFC0 FFCD moveb.emx -64(a6), -51(a5)
0276: 4A2E FFC0      tstb.ex -64(a6)
027A: 584F           addqw #4, a7
027C: 6704           beq 4
027E: 7803           moveq #3, d4
0280: 6002           bra 2
0282: 7813           moveq #19, d4
0284: 3F04           movew d4, -(a7)
0286: 2F3C 6164 6472 movel.exl #1633969266, -(a7)
028C: 2F3C 4441 5441 movel.exl #1145132097, -(a7)
0292: 4E4F           trap #15
0294: A075           sysTrapDmOpenDatabaseByTypeCreator
0296: 2B48 FFDE      movel.mx a0, -34(a5)
029A: 4AAD FFDE      tstl.ex -34(a5)
029E: 4FEF 000A      lea 10(a7), d7
02A2: 6600 00E4      bne 228
02A6: 4227           clrb -(a7)
02A8: 2F3C 4441 5441 movel.exl #1145132097, -(a7)
02AE: 2F3C 6164 6472 movel.exl #1633969266, -(a7)
02B4: 41FA 015C      lea 30746(pc), d0
02B8: 4850           pea (a0)
02BA: 4267           clrw -(a7)
02BC: 4E4F           trap #15
02BE: A041           sysTrapDmCreateDatabase
02C0: 3600           movew d0, d3
02C2: 4A43           tstw d3
02C4: 4FEF 0010      lea 16(a7), d7
02C8: 6706           beq 6
02CA: 3003           movew d3, d0
02CC: 6000 0128      bra 296
02D0: 3F04           movew d4, -(a7)
02D2: 2F3C 6164 6472 movel.exl #1633969266, -(a7)
02D8: 2F3C 4441 5441 movel.exl #1145132097, -(a7)
02DE: 4E4F           trap #15
02E0: A075           sysTrapDmOpenDatabaseByTypeCreator
02E2: 2B48 FFDE      movel.mx a0, -34(a5)
02E6: 4AAD FFDE      tstl.ex -34(a5)
02EA: 4FEF 000A      lea 10(a7), d7
02EE: 6606           bne 6
02F0: 7001           moveq #1, d0
02F2: 6000 0102      bra 258
02F6: 42A7           clrl -(a7)
02F8: 486E FFB2      pea.ex -78(a6)
02FC: 42A7           clrl -(a7)
02FE: 42A7           clrl -(a7)
0300: 486E FFAE      pea.ex -82(a6)
0304: 2F2D FFDE      movel.ex -34(a5), -(a7)
0308: 4E4F           trap #15
030A: A04C           sysTrapDmOpenDatabaseInfo
030C: 42A7           clrl -(a7)
030E: 42A7           clrl -(a7)
0310: 42A7           clrl -(a7)
0312: 42A7           clrl -(a7)
0314: 42A7           clrl -(a7)
0316: 42A7           clrl -(a7)
0318: 42A7           clrl -(a7)
031A: 42A7           clrl -(a7)
031C: 42A7           clrl -(a7)
031E: 486E FFAC      pea.ex -84(a6)
0322: 42A7           clrl -(a7)
0324: 2F2E FFAE      movel.ex -82(a6), -(a7)
0328: 3F2E FFB2      movew.ex -78(a6), -(a7)
032C: 4E4F           trap #15
032E: A046           sysTrapDmDatabaseInfo
0330: 006E 0008 FFAC oriw.ex #8, -84(a6)
0336: 42A7           clrl -(a7)
0338: 42A7           clrl -(a7)
033A: 42A7           clrl -(a7)
033C: 42A7           clrl -(a7)
033E: 42A7           clrl -(a7)
0340: 42A7           clrl -(a7)
0342: 42A7           clrl -(a7)
0344: 42A7           clrl -(a7)
0346: 42A7           clrl -(a7)
0348: 486E FFAC      pea.ex -84(a6)
034C: 42A7           clrl -(a7)
034E: 2F2E FFAE      movel.ex -82(a6), -(a7)
0352: 3F2E FFB2      movew.ex -78(a6), -(a7)
0356: 4E4F           trap #15
0358: A047           sysTrapDmSetDatabaseInfo
035A: 2F2D FFDE      movel.ex -34(a5), -(a7)
035E: 4EBA 701E      jsr.ex -6266(pc)
0362: 3600           movew d0, d3
0364: 4A43           tstw d3
0366: 4FEF 0080      lea 128(a7), d7
036A: 671C           beq 28
036C: 2F2D FFDE      movel.ex -34(a5), -(a7)
0370: 4E4F           trap #15
0372: A04A           sysTrapDmCloseDatabase
0374: 2F2E FFAE      movel.ex -82(a6), -(a7)
0378: 3F2E FFB2      movew.ex -78(a6), -(a7)
037C: 4E4F           trap #15
037E: A042           sysTrapDmDeleteDatabase
0380: 3003           movew d3, d0
0382: 4FEF 000A      lea 10(a7), d7
0386: 606E           bra 110
0388: 2F2D FFDE      movel.ex -34(a5), -(a7)
038C: 4EBA 6CCC      jsr.ex -7070(pc)
0390: 2448           movel a0, a2
0392: 200A           movel a2, d0
0394: 584F           addqw #4, a7
0396: 6618           bne 24
0398: 41FA 0082      lea 30756(pc), d0
039C: 4850           pea (a0)
039E: 3F3C 0168      movew.ex #360, -(a7)
03A2: 41FA 0090      lea 30780(pc), d0
03A6: 4850           pea (a0)
03A8: 4E4F           trap #15
03AA: A084           sysTrapErrDisplayFileLineMsg
03AC: 4FEF 000A      lea 10(a7), d7
03B0: 102A 027A      moveb.ex 634(a2), d0
03B4: B02E FFB6      cmpb.ex -74(a6), d0
03B8: 6708           beq 8
03BA: 2F0A           movel a2, -(a7)
03BC: 4EBA 6D20      jsr.ex -6938(pc)
03C0: 584F           addqw #4, a7
03C2: 486D FF5E      pea.ex -162(a5)
03C6: 2F0A           movel a2, -(a7)
03C8: 4EBA 64AC      jsr.ex -9090(pc)
03CC: 7001           moveq #1, d0
03CE: C02A 027C      andrb.ex 636(a2), d0
03D2: 1B40 FFCC      moveb.mx d0, -52(a5)
03D6: 2F0A           movel a2, -(a7)
03D8: 4EBA 6990      jsr.ex -7822(pc)
03DC: 4E4F           trap #15
03DE: A0F7           sysTrapTimGetTicks
03E0: 2B40 FF12      movel.mx d0, -238(a5)
03E4: 4E4F           trap #15
03E6: A2A2           sysTrapKeyCurrentState
03E8: 4680           notl d0
03EA: 2F00           movel d0, -(a7)
03EC: 4E4F           trap #15
03EE: A2CF           sysTrapKeySetMask
03F0: 3003           movew d3, d0
03F2: 4FEF 0010      lea 16(a7), d7
03F6: 4CDF 0418      movem (a7)+, <0418>
03FA: 4E5E           unlk a6
03FC: 4E75           rts
03FE: 9053           subrw (a3), d0
0400: 7461           moveq #97, d2
0402: 7274           moveq #116, d1
0404: 4170           dc.w #16752
0406: 706C           moveq #108, d0
0408: 6963           bvs 99
040A: 6174           bsr 116
040C: 696F           bvs 111
040E: 6E00 002C      bgt 44
0412: 4164           dc.w #16740
0414: 6472           bcc 114
0416: 6573           bcs 115
0418: 7344           dc.w #29508
041A: 4200           clrb d0
041C: 4D69           dc.w #19817
041E: 7373           dc.w #29555
0420: 696E           bvs 110
0422: 6720           beq 32
0424: 6170           bsr 112
0426: 7020           moveq #32, d0
0428: 696E           bvs 110
042A: 666F           bne 111
042C: 2062           movel -(a2), a0
042E: 6C6F           bge 111
0430: 636B           bls 107
0432: 0000           dc.w #0
0434: 4164           dc.w #16740
0436: 6472           bcc 114
0438: 6573           bcs 115
043A: 732E           dc.w #29486
043C: 6300 4E56      bls 20054
0440: 0000           dc.w #0
0442: 4EBA 6A22      jsr.ex -7570(pc)
0446: 4E4F           trap #15
0448: A1A2           sysTrapFrmSaveAllForms
044A: 4E4F           trap #15
044C: A1A1           sysTrapFrmCloseAllForms
044E: 2F2D FFDE      movel.ex -34(a5), -(a7)
0452: 4E4F           trap #15
0454: A04A           sysTrapDmCloseDatabase
0456: 4E5E           unlk a6
0458: 4E75           rts
045A: 8F53           ormw d7, (a3)
045C: 746F           moveq #111, d2
045E: 7041           moveq #65, d0
0460: 7070           moveq #112, d0
0462: 6C69           bge 105
0464: 6361           bls 97
0466: 7469           moveq #105, d2
0468: 6F6E           ble 110
046A: 0000           dc.w #0
046C: 4E56 0000      link a6, #0
0470: 48E7 1030      movem <1030>, -(a7)
0474: 7603           moveq #3, d3
0476: 3F03           movew d3, -(a7)
0478: 2F3C 6164 6472 movel.exl #1633969266, -(a7)
047E: 2F3C 4441 5441 movel.exl #1145132097, -(a7)
0484: 4E4F           trap #15
0486: A075           sysTrapDmOpenDatabaseByTypeCreator
0488: 2448           movel a0, a2
048A: 200A           movel a2, d0
048C: 4FEF 000A      lea 10(a7), d7
0490: 6726           beq 38
0492: 2F0A           movel a2, -(a7)
0494: 4EBA 6BC4      jsr.ex -7070(pc)
0498: 2648           movel a0, a3
049A: 7001           moveq #1, d0
049C: C02B 027C      andrb.ex 636(a3), d0
04A0: 1F00           moveb d0, -(a7)
04A2: 2F0A           movel a2, -(a7)
04A4: 4EBA 7B42      jsr.ex -3088(pc)
04A8: 2F0B           movel a3, -(a7)
04AA: 4E4F           trap #15
04AC: A035           sysTrapMemPtrUnlock
04AE: 2F0A           movel a2, -(a7)
04B0: 4E4F           trap #15
04B2: A04A           sysTrapDmCloseDatabase
04B4: 4FEF 0012      lea 18(a7), d7
04B8: 4CDF 0C08      movem (a7)+, <0c08>
04BC: 4E5E           unlk a6
04BE: 4E75           rts
04C0: 8D41           dc.w #36161
04C2: 7070           moveq #112, d0
04C4: 4861           dc.w #18529
04C6: 6E64           bgt 100
04C8: 6C65           bge 101
04CA: 5379 6E63 0000 subqw.ex #1, (6e630000).l
04D0: 4E56 0000      link a6, #0
04D4: 48E7 1020      movem <1020>, -(a7)
04D8: 246E 0008      movel.ex 8(a6), a2
04DC: 200A           movel a2, d0
04DE: 6604           bne 4
04E0: 7000           moveq #0, d0
04E2: 6014           bra 20
04E4: 2F0A           movel a2, -(a7)
04E6: 4EBA 6E96      jsr.ex -6266(pc)
04EA: 3600           movew d0, d3
04EC: 4A43           tstw d3
04EE: 584F           addqw #4, a7
04F0: 6704           beq 4
04F2: 7000           moveq #0, d0
04F4: 6002           bra 2
04F6: 7001           moveq #1, d0
04F8: 4CDF 0408      movem (a7)+, <0408>
04FC: 4E5E           unlk a6
04FE: 4E75           rts
0500: 9841           subrw d1, d4
0502: 7070           moveq #112, d0
0504: 4C61           dc.w #19553
0506: 756E           dc.w #30062
0508: 6368           bls 104
050A: 436D           dc.w #17261
050C: 6444           bcc 68
050E: 6174           bsr 116
0510: 6162           bsr 98
0512: 6173           bsr 115
0514: 6549           bcs 73
0516: 6E69           bgt 105
0518: 7400           moveq #0, d2
051A: 0000           dc.w #0
051C: 4E56 FF6C      link a6, #-148
0520: 48E7 1F38      movem <1f38>, -(a7)
0524: 246E 0008      movel.ex 8(a6), a2
0528: 42AE FFA6      clrl.ex -90(a6)
052C: 426E FFA4      clrw.ex -92(a6)
0530: 486E FFA0      pea.ex -96(a6)
0534: 486E FFA4      pea.ex -92(a6)
0538: 1F3C 0001      moveb.ex #1, -(a7)
053C: 2F3C 6164 6472 movel.exl #1633969266, -(a7)
0542: 2F3C 4441 5441 movel.exl #1145132097, -(a7)
0548: 486E FF6C      pea.ex -148(a6)
054C: 1F3C 0001      moveb.ex #1, -(a7)
0550: 4E4F           trap #15
0552: A078           sysTrapDmGetNextDatabaseByTypeCreator
0554: 3D40 FF96      movew.mx d0, -106(a6)
0558: 4A6E FF96      tstw.ex -106(a6)
055C: 4FEF 0018      lea 24(a7), d7
0560: 6708           beq 8
0562: 422A 0004      clrb.ex 4(a2)
0566: 6000 01A8      bra 424
056A: 3F12           movew (a2), -(a7)
056C: 2F2E FFA0      movel.ex -96(a6), -(a7)
0570: 3F2E FFA4      movew.ex -92(a6), -(a7)
0574: 4E4F           trap #15
0576: A049           sysTrapDmOpenDatabase
0578: 2648           movel a0, a3
057A: 200B           movel a3, d0
057C: 504F           addqw #0, a7
057E: 6608           bne 8
0580: 422A 0004      clrb.ex 4(a2)
0584: 6000 018A      bra 394
0588: 3F3C 0064      movew.ex #100, -(a7)
058C: 2F3C 7453 5452 movel.exl #1951618130, -(a7)
0592: 4E4F           trap #15
0594: A05F           sysTrapDmGetResource
0596: 2C08           movel a0, d6
0598: 2F06           movel d6, -(a7)
059A: 4E4F           trap #15
059C: A021           sysTrapMemHandleLock
059E: 2E08           movel a0, d7
05A0: 2F07           movel d7, -(a7)
05A2: 2F0A           movel a2, -(a7)
05A4: 4E4F           trap #15
05A6: A26D           sysTrapFindDrawHeader
05A8: 1800           moveb d0, d4
05AA: 2F06           movel d6, -(a7)
05AC: 4E4F           trap #15
05AE: A022           sysTrapMemHandleUnlock
05B0: 2F06           movel d6, -(a7)
05B2: 4E4F           trap #15
05B4: A061           sysTrapDmReleaseResource
05B6: 4A04           tstb d4
05B8: 4FEF 001A      lea 26(a7), d7
05BC: 6600 014A      bne 330
05C0: 3D6A 0002 FFFE movew.emx 2(a2), -2(a6)
05C6: 302E FFFE      movew.ex -2(a6), d0
05CA: 0240 000F      andiw #15, d0
05CE: 6618           bne 24
05D0: 1F3C 0001      moveb.ex #1, -(a7)
05D4: 4E4F           trap #15
05D6: A2CD           sysTrapEvtSysEventAvail
05D8: 4A00           tstb d0
05DA: 544F           addqw #2, a7
05DC: 670A           beq 10
05DE: 157C 0001 0004 moveb.emx #1, 4(a2)
05E4: 6000 0112      bra 274
05E8: 3F3C 00FF      movew.ex #255, -(a7)
05EC: 486E FFFE      pea.ex -2(a6)
05F0: 2F0B           movel a3, -(a7)
05F2: 4E4F           trap #15
05F4: A070           sysTrapDmQueryNextInCategory
05F6: 2D48 FFFA      movel.mx a0, -6(a6)
05FA: 4AAE FFFA      tstl.ex -6(a6)
05FE: 4FEF 000A      lea 10(a7), d7
0602: 6608           bne 8
0604: 422A 0004      clrb.ex 4(a2)
0608: 6000 00EE      bra 238
060C: 486E FFFA      pea.ex -6(a6)
0610: 486E FFAA      pea.ex -86(a6)
0614: 3F2E FFFE      movew.ex -2(a6), -(a7)
0618: 2F0B           movel a3, -(a7)
061A: 4EBA 7830      jsr.ex -3500(pc)
061E: 7A00           moveq #0, d5
0620: 7600           moveq #0, d3
0622: 4FEF 000E      lea 14(a7), d7
0626: 6038           bra 56
0628: 7000           moveq #0, d0
062A: 3003           movew d3, d0
062C: E588           lslil #2, d0
062E: 41EE FFAA      lea -86(a6), d0
0632: 5888           addql #4, a0
0634: 4AB0 0800      tstl.ex 0(a0,d0.l)
0638: 6724           beq 36
063A: 486E FF8C      pea.ex -116(a6)
063E: 486A 0016      pea.ex 22(a2)
0642: 7000           moveq #0, d0
0644: 3003           movew d3, d0
0646: E588           lslil #2, d0
0648: 41EE FFAA      lea -86(a6), d0
064C: 2F30 0804      movel.ex 4(a0,d0.l), -(a7)
0650: 4E4F           trap #15
0652: A26A           sysTrapFindStrInStr
0654: 1A00           moveb d0, d5
0656: 4A05           tstb d5
0658: 4FEF 000C      lea 12(a7), d7
065C: 6608           bne 8
065E: 5243           addqw #1, d3
0660: 0C43 0013      cmpiw #19, d3
0664: 65C2           bcs -62
0666: 4A05           tstb d5
0668: 677C           beq 124
066A: 2F2E FFA0      movel.ex -96(a6), -(a7)
066E: 3F2E FFA4      movew.ex -92(a6), -(a7)
0672: 42A7           clrl -(a7)
0674: 3F03           movew d3, -(a7)
0676: 3F2E FF8C      movew.ex -116(a6), -(a7)
067A: 3F2E FFFE      movew.ex -2(a6), -(a7)
067E: 2F0A           movel a2, -(a7)
0680: 4E4F           trap #15
0682: A26B           sysTrapFindSaveMatch
0684: 1800           moveb d0, d4
0686: 4A04           tstb d4
0688: 4FEF 0014      lea 20(a7), d7
068C: 670C           beq 12
068E: 2F2E FFFA      movel.ex -6(a6), -(a7)
0692: 4E4F           trap #15
0694: A022           sysTrapMemHandleUnlock
0696: 584F           addqw #4, a7
0698: 605E           bra 94
069A: 486E FF8E      pea.ex -114(a6)
069E: 2F0A           movel a2, -(a7)
06A0: 4E4F           trap #15
06A2: A26C           sysTrapFindGetLineBounds
06A4: 2F0B           movel a3, -(a7)
06A6: 4EBA 69B2      jsr.ex -7070(pc)
06AA: 2848           movel a0, a4
06AC: 486E FF98      pea.ex -104(a6)
06B0: 2F0C           movel a4, -(a7)
06B2: 4EBA 61C2      jsr.ex -9090(pc)
06B6: 4227           clrb -(a7)
06B8: 4E4F           trap #15
06BA: A164           sysTrapFntSetFont
06BC: 486E FFA6      pea.ex -90(a6)
06C0: 7001           moveq #1, d0
06C2: C02C 027C      andrb.ex 636(a4), d0
06C6: 1F00           moveb d0, -(a7)
06C8: 486E FF98      pea.ex -104(a6)
06CC: 486E FF8E      pea.ex -114(a6)
06D0: 486E FFAA      pea.ex -86(a6)
06D4: 4EBA 4FFC      jsr.ex -13606(pc)
06D8: 2F0C           movel a4, -(a7)
06DA: 4E4F           trap #15
06DC: A035           sysTrapMemPtrUnlock
06DE: 526A 002A      addqw.ex #1, 42(a2)
06E2: 4FEF 002C      lea 44(a7), d7
06E6: 2F2E FFFA      movel.ex -6(a6), -(a7)
06EA: 4E4F           trap #15
06EC: A022           sysTrapMemHandleUnlock
06EE: 526E FFFE      addqw.ex #1, -2(a6)
06F2: 584F           addqw #4, a7
06F4: 6000 FED0      bra 65232
06F8: 4AAE FFA6      tstl.ex -90(a6)
06FC: 670A           beq 10
06FE: 2F2E FFA6      movel.ex -90(a6), -(a7)
0702: 4E4F           trap #15
0704: A035           sysTrapMemPtrUnlock
0706: 584F           addqw #4, a7
0708: 2F0B           movel a3, -(a7)
070A: 4E4F           trap #15
070C: A04A           sysTrapDmCloseDatabase
070E: 584F           addqw #4, a7
0710: 4CDF 1CF8      movem (a7)+, <1cf8>
0714: 4E5E           unlk a6
0716: 4E75           rts
0718: 8653           orrw (a3), d3
071A: 6561           bcs 97
071C: 7263           moveq #99, d1
071E: 6800 0000      bvc 0
0722: 4E56 FFE0      link a6, #-32
0726: 48E7 1820      movem <1820>, -(a7)
072A: 246E 0008      movel.ex 8(a6), a2
072E: 182E 000C      moveb.ex 12(a6), d4
0732: 3D6A 0008 FFE6 movew.emx 8(a2), -26(a6)
0738: 42A7           clrl -(a7)
073A: 486E FFE0      pea.ex -32(a6)
073E: 486E FFE4      pea.ex -28(a6)
0742: 3F2E FFE6      movew.ex -26(a6), -(a7)
0746: 2F2D FFDE      movel.ex -34(a5), -(a7)
074A: 4E4F           trap #15
074C: A050           sysTrapDmRecordInfo
074E: 0C6D 00FF FFE6 cmpiw.ex #255, -26(a5)
0754: 4FEF 0012      lea 18(a7), d7
0758: 670C           beq 12
075A: 302E FFE4      movew.ex -28(a6), d0
075E: 0240 000F      andiw #15, d0
0762: 3B40 FFE6      movew.mx d0, -26(a5)
0766: 4A04           tstb d4
0768: 6618           bne 24
076A: 4E4F           trap #15
076C: A1A1           sysTrapFrmCloseAllForms
076E: 486E FFE6      pea.ex -26(a6)
0772: 2F2E FFE0      movel.ex -32(a6), -(a7)
0776: 2F2D FFDE      movel.ex -34(a5), -(a7)
077A: 4E4F           trap #15
077C: A07B           sysTrapDmFindRecordByID
077E: 4FEF 000C      lea 12(a7), d7
0782: 3B6E FFE6 FFE2 movew.emx -26(a6), -30(a5)
0788: 3B7C 03E8 FF68 movew.emx #1000, -152(a5)
078E: 0C6A 0012 000C cmpiw.ex #18, 12(a2)
0794: 6606           bne 6
0796: 363C 2A94      movew.ex #10900, d3
079A: 6004           bra 4
079C: 363C 06A4      movew.ex #1700, d3
07A0: 4227           clrb -(a7)
07A2: 4878 0018      pea.ex (0018).w
07A6: 486E FFE8      pea.ex -24(a6)
07AA: 4E4F           trap #15
07AC: A027           sysTrapMemSet
07AE: 3D7C 0017 FFE8 movew.emx #23, -24(a6)
07B4: 3D43 FFF0      movew.mx d3, -16(a6)
07B8: 486E FFE8      pea.ex -24(a6)
07BC: 4E4F           trap #15
07BE: A11B           sysTrapEvtAddEventToQueue
07C0: 3D7C 0019 FFE8 movew.emx #25, -24(a6)
07C6: 3D43 FFF0      movew.mx d3, -16(a6)
07CA: 3D6E FFE6 FFF2 movew.emx -26(a6), -14(a6)
07D0: 3D6A 000A FFF4 movew.emx 10(a2), -12(a6)
07D6: 3D52 FFF6      movew.mx (a2), -10(a6)
07DA: 3D6A 000C FFF8 movew.emx 12(a2), -8(a6)
07E0: 486E FFE8      pea.ex -24(a6)
07E4: 4E4F           trap #15
07E6: A11B           sysTrapEvtAddEventToQueue
07E8: 4FEF 0012      lea 18(a7), d7
07EC: 4CDF 0418      movem (a7)+, <0418>
07F0: 4E5E           unlk a6
07F2: 4E75           rts
07F4: 8847           orrw d7, d4
07F6: 6F54           ble 84
07F8: 6F49           ble 73
07FA: 7465           moveq #101, d2
07FC: 6D00 0000      blt 0
0800: 4E56 0000      link a6, #0
0804: 4E4F           trap #15
0806: A0F5           sysTrapTimGetSeconds
0808: 3B40 FF22      movew.mx d0, -222(a5)
080C: 3B7C 0001 FF24 movew.emx #1, -220(a5)
0812: 4E5E           unlk a6
0814: 4E75           rts
0816: 8F52           ormw d7, (a2)
0818: 6573           bcs 115
081A: 6574           bcs 116
081C: 5363           subqw #1, -(a3)
081E: 726F           moveq #111, d1
0820: 6C6C           bge 108
0822: 5261           addqw #1, -(a1)
0824: 7465           moveq #101, d2
0826: 0000           dc.w #0
0828: 4E56 0000      link a6, #0
082C: 2F03           movel d3, -(a7)
082E: 4E4F           trap #15
0830: A0F5           sysTrapTimGetSeconds
0832: 3600           movew d0, d3
0834: 0C6D 0005 FF24 cmpiw.ex #5, -220(a5)
083A: 6414           bcc 20
083C: 3003           movew d3, d0
083E: 906D FF22      subrw.ex -222(a5), d0
0842: 0C40 0002      cmpiw #2, d0
0846: 6308           bls 8
0848: 3B43 FF22      movew.mx d3, -222(a5)
084C: 546D FF24      addqw.ex #2, -220(a5)
0850: 261F           movel (a7)+, d3
0852: 4E5E           unlk a6
0854: 4E75           rts
0856: 9041           subrw d1, d0
0858: 646A           bcc 106
085A: 7573           dc.w #30067
085C: 7453           moveq #83, d2
085E: 6372           bls 114
0860: 6F6C           ble 108
0862: 6C52           bge 82
0864: 6174           bsr 116
0866: 6500 0000      bcs 0
086A: 4E56 0000      link a6, #0
086E: 48E7 1F30      movem <1f30>, -(a7)
0872: 266E 0008      movel.ex 8(a6), a3
0876: 1F2E 000C      moveb.ex 12(a6), -(a7)
087A: 4E4F           trap #15
087C: A164           sysTrapFntSetFont
087E: 1E00           moveb d0, d7
0880: 7600           moveq #0, d3
0882: 7800           moveq #0, d4
0884: 544F           addqw #2, a7
0886: 6036           bra 54
0888: 3444           movew d4, a2
088A: 200A           movel a2, d0
088C: E988           lslil #4, d0
088E: 244B           movel a3, a2
0890: D5C0           addal d0, a2
0892: 45EA 011A      lea 282(a2), d2
0896: 2F0A           movel a2, -(a7)
0898: 4E4F           trap #15
089A: A0C7           sysTrapStrLen
089C: 584F           addqw #4, a7
089E: 3F00           movew d0, -(a7)
08A0: 2F0A           movel a2, -(a7)
08A2: 4E4F           trap #15
08A4: A16B           sysTrapFntCharsWidth
08A6: 3A00           movew d0, d5
08A8: B645           cmpw d5, d3
08AA: 52C0           shi d0
08AC: 4400           negb d0
08AE: 4880           extw d0
08B0: 5C4F           addqw #6, a7
08B2: 6704           beq 4
08B4: 3C03           movew d3, d6
08B6: 6002           bra 2
08B8: 3C05           movew d5, d6
08BA: 3606           movew d6, d3
08BC: 5244           addqw #1, d4
08BE: 0C44 0016      cmpiw #22, d4
08C2: 6DC4           blt -60
08C4: 1F3C 003A      moveb.ex #58, -(a7)
08C8: 4E4F           trap #15
08CA: A16A           sysTrapFntCharWidth
08CC: 5240           addqw #1, d0
08CE: D640           addrw d0, d3
08D0: 1F07           moveb d7, -(a7)
08D2: 4E4F           trap #15
08D4: A164           sysTrapFntSetFont
08D6: 0C43 0050      cmpiw #80, d3
08DA: 584F           addqw #4, a7
08DC: 6302           bls 2
08DE: 7650           moveq #80, d3
08E0: 3003           movew d3, d0
08E2: 4CDF 0CF8      movem (a7)+, <0cf8>
08E6: 4E5E           unlk a6
08E8: 4E75           rts
08EA: 9347           subxrw d7, d1
08EC: 6574           bcs 116
08EE: 4C61           dc.w #19553
08F0: 6265           bhi 101
08F2: 6C43           bge 67
08F4: 6F6C           ble 108
08F6: 756D           dc.w #30061
08F8: 6E57           bgt 87
08FA: 6964           bvs 100
08FC: 7468           moveq #104, d2
08FE: 0000           dc.w #0
0900: 4E56 0000      link a6, #0
0904: 2F0A           movel a2, -(a7)
0906: 4E4F           trap #15
0908: A173           sysTrapFrmGetActiveForm
090A: 2448           movel a0, a2
090C: 2F0A           movel a2, -(a7)
090E: 4E4F           trap #15
0910: A172           sysTrapFrmEraseForm
0912: 2F0A           movel a2, -(a7)
0914: 4E4F           trap #15
0916: A170           sysTrapFrmDeleteForm
0918: 4E4F           trap #15
091A: A197           sysTrapFrmGetFirstForm
091C: 2F08