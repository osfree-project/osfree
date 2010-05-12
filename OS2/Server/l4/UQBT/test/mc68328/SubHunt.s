01EA: 0000 0001      orib #1, d0
01EE: 487A 0004      pea.ex 30204(pc)
01F2: 0697 0000 3638 addil #13880, (a7)
01F8: 4E75           rts
01FA: 4E56 0000      link a6, #0
01FE: 2F03           movel d3, -(a7)
0200: 0C2D 0001 F714 cmpib.ex #1, -2284(a5)
0206: 6714           beq 20
0208: 4A2D F714      tstb.ex -2284(a5)
020C: 670E           beq 14
020E: 0C2D 0006 F714 cmpib.ex #6, -2284(a5)
0214: 6706           beq 6
0216: 4A2D F71A      tstb.ex -2278(a5)
021A: 6704           beq 4
021C: 70FF           moveq #255, d0
021E: 6014           bra 20
0220: 4E4F           trap #15
0222: A0F7           sysTrapTimGetTicks
0224: 222D F716      movel.ex -2282(a5), d1
0228: 9280           subrl d0, d1
022A: 2601           movel d1, d3
022C: 4A83           tstl d3
022E: 6C02           bge 2
0230: 7600           moveq #0, d3
0232: 2003           movel d3, d0
0234: 261F           movel (a7)+, d3
0236: 4E5E           unlk a6
0238: 4E75           rts
023A: 9454           subrw (a4), d2
023C: 696D           bvs 109
023E: 6555           bcs 85
0240: 6E74           bgt 116
0242: 696C           bvs 108
0244: 6C4E           bge 78
0246: 6578           bcs 120
0248: 7450           moveq #80, d2
024A: 6572           bcs 114
024C: 696F           bvs 111
024E: 6400 0000      bcc 0
0252: 4E56 FFFC      link a6, #-4
0256: 48E7 1020      movem <1020>, -(a7)
025A: 1F3C 001F      moveb.ex #31, -(a7)
025E: 4E4F           trap #15
0260: A2D1           sysTrapPrefGetPreference
0262: 3B40 F62C      movew.mx d0, -2516(a5)
0266: 486D F62A      pea.ex -2518(a5)
026A: 486D F628      pea.ex -2520(a5)
026E: 486D F626      pea.ex -2522(a5)
0272: 486D F624      pea.ex -2524(a5)
0276: 4227           clrb -(a7)
0278: 4E4F           trap #15
027A: A2A4           sysTrapKeyRates
027C: 4E4F           trap #15
027E: A1FE           sysTrapWinGetDrawWindow
0280: 2448           movel a0, a2
0282: 7600           moveq #0, d3
0284: 4FEF 0014      lea 20(a7), d7
0288: 6000 00E0      bra 224
028C: 3003           movew d3, d0
028E: 0640 03E9      addiw #1001, d0
0292: 3F00           movew d0, -(a7)
0294: 2F3C 5462 6D70 movel.exl #1415736688, -(a7)
029A: 4E4F           trap #15
029C: A05F           sysTrapDmGetResource
029E: 3243           movew d3, a1
02A0: 2009           movel a1, d0
02A2: E588           lslil #2, d0
02A4: 43ED F6C8      lea -2360(a5), d1
02A8: 2388 0800      movel.mx a0, 0(a1,d0.l)
02AC: 3043           movew d3, a0
02AE: 2008           movel a0, d0
02B0: E588           lslil #2, d0
02B2: 2049           movel a1, a0
02B4: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
02B8: 4E4F           trap #15
02BA: A021           sysTrapMemHandleLock
02BC: 3243           movew d3, a1
02BE: 2009           movel a1, d0
02C0: E588           lslil #2, d0
02C2: 43ED F67C      lea -2436(a5), d1
02C6: 2388 0800      movel.mx a0, 0(a1,d0.l)
02CA: 0C43 03E7      cmpiw #999, d3
02CE: 4FEF 000A      lea 10(a7), d7
02D2: 6612           bne 18
02D4: 3043           movew d3, a0
02D6: 2008           movel a0, d0
02D8: E588           lslil #2, d0
02DA: 41ED F630      lea -2512(a5), d0
02DE: 42B0 0800      clrl.ex 0(a0,d0.l)
02E2: 6000 0084      bra 132
02E6: 486E FFFC      pea.ex -4(a6)
02EA: 4227           clrb -(a7)
02EC: 3043           movew d3, a0
02EE: 2008           movel a0, d0
02F0: E588           lslil #2, d0
02F2: 2049           movel a1, a0
02F4: 2070 0800      movel.ex 0(a0,d0.l), a0
02F8: 3F28 0002      movew.ex 2(a0), -(a7)
02FC: 3043           movew d3, a0
02FE: 2008           movel a0, d0
0300: E588           lslil #2, d0
0302: 2049           movel a1, a0
0304: 2070 0800      movel.ex 0(a0,d0.l), a0
0308: 3F10           movew (a0), -(a7)
030A: 4E4F           trap #15
030C: A1F7           sysTrapWinCreateOffscreenWindow
030E: 3243           movew d3, a1
0310: 2009           movel a1, d0
0312: E588           lslil #2, d0
0314: 43ED F630      lea -2512(a5), d1
0318: 2388 0800      movel.mx a0, 0(a1,d0.l)
031C: 4A6E FFFC      tstw.ex -4(a6)
0320: 4FEF 000A      lea 10(a7), d7
0324: 6718           beq 24
0326: 41FA 019E      lea 30926(pc), d0
032A: 4850           pea (a0)
032C: 3F3C 02F3      movew.ex #755, -(a7)
0330: 41FA 01AA      lea 30948(pc), d0
0334: 4850           pea (a0)
0336: 4E4F           trap #15
0338: A084           sysTrapErrDisplayFileLineMsg
033A: 4FEF 000A      lea 10(a7), d7
033E: 3043           movew d3, a0
0340: 2008           movel a0, d0
0342: E588           lslil #2, d0
0344: 41ED F630      lea -2512(a5), d0
0348: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
034C: 4E4F           trap #15
034E: A1FD           sysTrapWinSetDrawWindow
0350: 42A7           clrl -(a7)
0352: 3043           movew d3, a0
0354: 2008           movel a0, d0
0356: E588           lslil #2, d0
0358: 41ED F67C      lea -2436(a5), d0
035C: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
0360: 4E4F           trap #15
0362: A226           sysTrapWinDrawBitmap
0364: 4FEF 000C      lea 12(a7), d7
0368: 5243           addqw #1, d3
036A: 0C43 0013      cmpiw #19, d3
036E: 6D00 FF1C      blt 65308
0372: 2F0A           movel a2, -(a7)
0374: 4E4F           trap #15
0376: A1FD           sysTrapWinSetDrawWindow
0378: 3D7C 00CC FFFE movew.emx #204, -2(a6)
037E: 1F3C 0001      moveb.ex #1, -(a7)
0382: 486E FFFE      pea.ex -2(a6)
0386: 486D F556      pea.ex -2730(a5)
038A: 4267           clrw -(a7)
038C: 2F3C 7368 646B movel.exl #1936221291, -(a7)
0392: 4E4F           trap #15
0394: A2D3           sysTrapPrefGetAppPreferences
0396: 5240           addqw #1, d0
0398: 4FEF 0014      lea 20(a7), d7
039C: 6600 00BA      bne 186
03A0: 7600           moveq #0, d3
03A2: 602C           bra 44
03A4: 7016           moveq #22, d0
03A6: C1C3           muls d3, d0
03A8: 41ED F556      lea -2730(a5), d0
03AC: 4230 0800      clrb.ex 0(a0,d0.l)
03B0: 7016           moveq #22, d0
03B2: C1C3           muls d3, d0
03B4: 5088           addql #0, a0
03B6: 5088           addql #0, a0
03B8: 42B0 0800      clrl.ex 0(a0,d0.l)
03BC: 7016           moveq #22, d0
03BE: C1C3           muls d3, d0
03C0: 41ED F556      lea -2730(a5), d0
03C4: 41E8 0014      lea 20(a0), d0
03C8: 31BC 0001 0800 movew.emx #1, 0(a0,d0.l)
03CE: 5243           addqw #1, d3
03D0: 0C43 0009      cmpiw #9, d3
03D4: 6DCE           blt -50
03D6: 1F3C 0001      moveb.ex #1, -(a7)
03DA: 3F3C 0014      movew.ex #20, -(a7)
03DE: 4878 5BA0      pea.ex (5ba0).w
03E2: 41FA 0102      lea 30958(pc), d0
03E6: 4850           pea (a0)
03E8: 4EBA 2CD2      jsr.ex -23356(pc)
03EC: 1F3C 0001      moveb.ex #1, -(a7)
03F0: 3F3C 0013      movew.ex #19, -(a7)
03F4: 4878 4F2C      pea.ex (4f2c).w
03F8: 41FA 00F0      lea 30962(pc), d0
03FC: 4850           pea (a0)
03FE: 4EBA 2CBC      jsr.ex -23356(pc)
0402: 1F3C 0001      moveb.ex #1, -(a7)
0406: 3F3C 0012      movew.ex #18, -(a7)
040A: 4878 4AF9      pea.ex (4af9).w
040E: 41FA 00E4      lea 30972(pc), d0
0412: 4850           pea (a0)
0414: 4EBA 2CA6      jsr.ex -23356(pc)
0418: 1F3C 0001      moveb.ex #1, -(a7)
041C: 3F3C 000E      movew.ex #14, -(a7)
0420: 4878 308E      pea.ex (308e).w
0424: 41FA 00D8      lea 30982(pc), d0
0428: 4850           pea (a0)
042A: 4EBA 2C90      jsr.ex -23356(pc)
042E: 1F3C 0001      moveb.ex #1, -(a7)
0432: 3F3C 0019      movew.ex #25, -(a7)
0436: 2F3C 0000 A700 movel.exl #42752, -(a7)
043C: 41FA 00CC      lea 30994(pc), d0
0440: 4850           pea (a0)
0442: 4EBA 2C78      jsr.ex -23356(pc)
0446: 1B7C 0009 F61C moveb.emx #9, -2532(a5)
044C: 422D F61D      clrb.ex -2531(a5)
0450: 42AD F61E      clrl.ex -2530(a5)
0454: 4FEF 003C      lea 60(a7), d7
0458: 3D7C 08A8 FFFE movew.emx #2216, -2(a6)
045E: 4227           clrb -(a7)
0460: 486E FFFE      pea.ex -2(a6)
0464: 486D F714      pea.ex -2284(a5)
0468: 4267           clrw -(a7)
046A: 2F3C 7368 646B movel.exl #1936221291, -(a7)
0470: 4E4F           trap #15
0472: A2D3           sysTrapPrefGetAppPreferences
0474: 5240           addqw #1, d0
0476: 4FEF 0010      lea 16(a7), d7
047A: 6608           bne 8
047C: 1B7C 0001 F714 moveb.emx #1, -2284(a5)
0482: 6012           bra 18
0484: 0C2D 0006 F714 cmpib.ex #6, -2284(a5)
048A: 670A           beq 10
048C: 1B6D F714 F62F moveb.emx -2284(a5), -2513(a5)
0492: 422D F714      clrb.ex -2284(a5)
0496: 4E4F           trap #15
0498: A0F7           sysTrapTimGetTicks
049A: 2B40 F720      movel.mx d0, -2272(a5)
049E: 2F2D F720      movel.ex -2272(a5), -(a7)
04A2: 4E4F           trap #15
04A4: A0C2           sysTrapSysRandom
04A6: 7000           moveq #0, d0
04A8: 584F           addqw #4, a7
04AA: 4CDF 0408      movem (a7)+, <0408>
04AE: 4E5E           unlk a6
04B0: 4E75           rts
04B2: 9053           subrw (a3), d0
04B4: 7461           moveq #97, d2
04B6: 7274           moveq #116, d1
04B8: 4170           dc.w #16752
04BA: 706C           moveq #108, d0
04BC: 6963           bvs 99
04BE: 6174           bsr 116
04C0: 696F           bvs 111
04C2: 6E00 004A      bgt 74
04C6: 4572           dc.w #17778
04C8: 726F           moveq #111, d1
04CA: 7220           moveq #32, d1
04CC: 6C6F           bge 111
04CE: 6164           bsr 100
04D0: 696E           bvs 110
04D2: 6720           beq 32
04D4: 696D           bvs 109
04D6: 6167           bsr 103
04D8: 6573           bcs 115
04DA: 0000           dc.w #0
04DC: 5375 6248      subqw.ex #1, 72(a5,d6.w)
04E0: 756E           dc.w #30062
04E2: 742E           moveq #46, d2
04E4: 6300 4169      bls 16745
04E8: 6E00 5368      bgt 21352
04EC: 656C           bcs 108
04EE: 6C64           bge 100
04F0: 6F6E           ble 110
04F2: 0000           dc.w #0
04F4: 7468           moveq #104, d2
04F6: 6520           bcs 32
04F8: 4A6F 6465      tstw.ex 25701(a7)
04FC: 0000           dc.w #0
04FE: 526F 636B      addqw.ex #1, 25451(a7)
0502: 6574           bcs 116
0504: 2042           movel d2, a0
0506: 6F79           ble 121
0508: 0000           dc.w #0
050A: 4D72           dc.w #19826
050C: 2E20           movel -(a0), d7
050E: 5000           addqb #0, d0
0510: 4E56 0000      link a6, #0
0514: 2F03           movel d3, -(a7)
0516: 7600           moveq #0, d3
0518: 604E           bra 78
051A: 3043           movew d3, a0
051C: 2008           movel a0, d0
051E: E588           lslil #2, d0
0520: 41ED F67C      lea -2436(a5), d0
0524: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
0528: 4E4F           trap #15
052A: A035           sysTrapMemPtrUnlock
052C: 3043           movew d3, a0
052E: 2008           movel a0, d0
0530: E588           lslil #2, d0
0532: 41ED F6C8      lea -2360(a5), d0
0536: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
053A: 4E4F           trap #15
053C: A061           sysTrapDmReleaseResource
053E: 3043           movew d3, a0
0540: 2008           movel a0, d0
0542: E588           lslil #2, d0
0544: 41ED F630      lea -2512(a5), d0
0548: 4AB0 0800      tstl.ex 0(a0,d0.l)
054C: 504F           addqw #0, a7
054E: 6716           beq 22
0550: 4227           clrb -(a7)
0552: 3043           movew d3, a0
0554: 2008           movel a0, d0
0556: E588           lslil #2, d0
0558: 41ED F630      lea -2512(a5), d0
055C: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
0560: 4E4F           trap #15
0562: A1F8           sysTrapWinDeleteWindow
0564: 5C4F           addqw #6, a7
0566: 5243           addqw #1, d3
0568: 0C43 0013      cmpiw #19, d3
056C: 6DAC           blt -84
056E: 4E4F           trap #15
0570: A0F7           sysTrapTimGetTicks
0572: 90AD F720      subrl.ex -2272(a5), d0
0576: D1AD F61E      addml.ex d0, -2530(a5)
057A: 4A2D F714      tstb.ex -2284(a5)
057E: 6606           bne 6
0580: 1B6D F62F F714 moveb.emx -2513(a5), -2284(a5)
0586: 1F3C 0001      moveb.ex #1, -(a7)
058A: 3F3C 00CC      movew.ex #204, -(a7)
058E: 486D F556      pea.ex -2730(a5)
0592: 4878 0002      pea.ex (0002).w
0596: 2F3C 7368 646B movel.exl #1936221291, -(a7)
059C: 4E4F           trap #15
059E: A2D4           sysTrapPrefSetAppPreferences
05A0: 4227           clrb -(a7)
05A2: 3F3C 08A8      movew.ex #2216, -(a7)
05A6: 486D F714      pea.ex -2284(a5)
05AA: 4878 0002      pea.ex (0002).w
05AE: 2F3C 7368 646B movel.exl #1936221291, -(a7)
05B4: 4E4F           trap #15
05B6: A2D4           sysTrapPrefSetAppPreferences
05B8: 4FEF 0020      lea 32(a7), d7
05BC: 261F           movel (a7)+, d3
05BE: 4E5E           unlk a6
05C0: 4E75           rts
05C2: 8F53           ormw d7, (a3)
05C4: 746F           moveq #111, d2
05C6: 7041           moveq #65, d0
05C8: 7070           moveq #112, d0
05CA: 6C69           bge 105
05CC: 6361           bls 97
05CE: 7469           moveq #105, d2
05D0: 6F6E           ble 110
05D2: 0000           dc.w #0
05D4: 4E56 FFD6      link a6, #-42
05D8: 48E7 1C00      movem <1c00>, -(a7)
05DC: 2A2E 0008      movel.ex 8(a6), d5
05E0: 382E 000C      movew.ex 12(a6), d4
05E4: 486E FFF8      pea.ex -8(a6)
05E8: 3F3C 0001      movew.ex #1, -(a7)
05EC: 2F3C 7073 7973 movel.exl #1886615923, -(a7)
05F2: 4E4F           trap #15
05F4: A27B           sysTrapFtrGet
05F6: BAAE FFF8      cmpl.ex -8(a6), d5
05FA: 4FEF 000A      lea 10(a7), d7
05FE: 6300 00AA      bls 170
0602: 3004           movew d4, d0
0604: 0240 000C      andiw #12, d0
0608: 0C40 000C      cmpiw #12, d0
060C: 6600 0096      bne 150
0610: 3F3C 03E9      movew.ex #1001, -(a7)
0614: 4E4F           trap #15
0616: A192           sysTrapFrmAlert
0618: 0CAE 0200 0000 FFF8 cmpil.ex #33554432, -8(a6)
0620: 544F           addqw #2, a7
0622: 6400 0080      bcc 128
0626: 486E FFFC      pea.ex -4(a6)
062A: 486E FFF6      pea.ex -10(a6)
062E: 1F3C 0001      moveb.ex #1, -(a7)
0632: 2F3C 7072 6566 movel.exl #1886545254, -(a7)
0638: 2F3C 6170 706C movel.exl #1634758764, -(a7)
063E: 486E FFD6      pea.ex -42(a6)
0642: 1F3C 0001      moveb.ex #1, -(a7)
0646: 4E4F           trap #15
0648: A078           sysTrapDmGetNextDatabaseByTypeCreator
064A: 4AAE FFFC      tstl.ex -4(a6)
064E: 4FEF 0018      lea 24(a7), d7
0652: 6618           bne 24
0654: 41FA 0076      lea 31444(pc), d0
0658: 4850           pea (a0)
065A: 3F3C 0396      movew.ex #918, -(a7)
065E: 41FA 0080      lea 31464(pc), d0
0662: 4850           pea (a0)
0664: 4E4F           trap #15
0666: A084           sysTrapErrDisplayFileLineMsg
0668: 4FEF 000A      lea 10(a7), d7
066C: 4AAE FFFC      tstl.ex -4(a6)
0670: 6732           beq 50
0672: 42A7           clrl -(a7)
0674: 4267           clrw -(a7)
0676: 2F2E FFFC      movel.ex -4(a6), -(a7)
067A: 3F2E FFF6      movew.ex -10(a6), -(a7)
067E: 4E4F           trap #15
0680: A0A7           sysTrapSysUIAppSwitch
0682: 3600           movew d0, d3
0684: 4A43           tstw d3
0686: 4FEF 000C      lea 12(a7), d7
068A: 6718           beq 24
068C: 41FA 005C      lea 31474(pc), d0
0690: 4850           pea (a0)
0692: 3F3C 0396      movew.ex #918, -(a7)
0696: 41FA 0068      lea 31496(pc), d0
069A: 4850           pea (a0)
069C: 4E4F           trap #15
069E: A084           sysTrapErrDisplayFileLineMsg
06A0: 4FEF 000A      lea 10(a7), d7
06A4: 303C 050C      movew.ex #1292, d0
06A8: 6002           bra 2
06AA: 7000           moveq #0, d0
06AC: 4CDF 0038      movem (a7)+, <0038>
06B0: 4E5E           unlk a6
06B2: 4E75           rts
06B4: 9452           subrw (a2), d2
06B6: 6F6D           ble 109
06B8: 5665           addqw #3, -(a5)
06BA: 7273           moveq #115, d1
06BC: 696F           bvs 111
06BE: 6E43           bgt 67
06C0: 6F6D           ble 109
06C2: 7061           moveq #97, d0
06C4: 7469           moveq #105, d2
06C6: 626C           bhi 108
06C8: 6500 003E      bcs 62
06CC: 436F           dc.w #17263
06CE: 756C           dc.w #30060
06D0: 6420           bcc 32
06D2: 6E6F           bgt 111
06D4: 7420           moveq #32, d2
06D6: 6669           bne 105
06D8: 6E64           bgt 100
06DA: 2061           movel -(a1), a0
06DC: 7070           moveq #112, d0
06DE: 0000           dc.w #0
06E0: 5375 6248      subqw.ex #1, 72(a5,d6.w)
06E4: 756E           dc.w #30062
06E6: 742E           moveq #46, d2
06E8: 6300 436F      bls 17263
06EC: 756C           dc.w #30060
06EE: 6420           bcc 32
06F0: 6E6F           bgt 111
06F2: 7420           moveq #32, d2
06F4: 6C61           bge 97
06F6: 756E           dc.w #30062
06F8: 6368           bls 104
06FA: 2061           movel -(a1), a0
06FC: 7070           moveq #112, d0
06FE: 0000           dc.w #0
0700: 5375 6248      subqw.ex #1, 72(a5,d6.w)
0704: 756E           dc.w #30062
0706: 742E           moveq #46, d2
0708: 6300 4E56      bls 20054
070C: 0000           dc.w #0
070E: 48E7 0600      movem <0600>, -(a7)
0712: 206E 0008      movel.ex 8(a6), a0
0716: 1C2E 000C      moveb.ex 12(a6), d6
071A: 342E 000E      movew.ex 14(a6), d2
071E: 3A2E 0010      movew.ex 16(a6), d5
0722: 7200           moveq #0, d1
0724: 6002           bra 2
0726: 5241           addqw #1, d1
0728: 7000           moveq #0, d0
072A: 3001           movew d1, d0
072C: BC30 0800      cmpb.ex 0(a0,d0.l), d6
0730: 6704           beq 4
0732: B242           cmpw d2, d1
0734: 65F0           bcs -16
0736: B242           cmpw d2, d1
0738: 6504           bcs 4
073A: 3005           movew d5, d0
073C: 6002           bra 2
073E: 3001           movew d1, d0
0740: 4CDF 0060      movem (a7)+, <0060>
0744: 4E5E           unlk a6
0746: 4E75           rts
0748: 8D4D           dc.w #36173
074A: 6170           bsr 112
074C: 546F 506F      addqw.ex #2, 20591(a7)
0750: 7369           dc.w #29545
0752: 7469           moveq #105, d2
0754: 6F6E           ble 110
0756: 0000           dc.w #0
0758: 4E56 0000      link a6, #0
075C: 48E7 1C30      movem <1c30>, -(a7)
0760: 3A2E 0008      movew.ex 8(a6), d5
0764: 382E 000A      movew.ex 10(a6), d4
0768: 362E 000C      movew.ex 12(a6), d3
076C: 3F05           movew d5, -(a7)
076E: 2F3C 5462 6D70 movel.exl #1415736688, -(a7)
0774: 4E4F           trap #15
0776: A05F           sysTrapDmGetResource
0778: 2448           movel a0, a2
077A: 200A           movel a2, d0
077C: 5C4F           addqw #6, a7
077E: 6618           bne 24
0780: 41FA 004E      lea 31704(pc), d0
0784: 4850           pea (a0)
0786: 3F3C 03F6      movew.ex #1014, -(a7)
078A: 41FA 0054      lea 31720(pc), d0
078E: 4850           pea (a0)
0790: 4E4F           trap #15
0792: A084           sysTrapErrDisplayFileLineMsg
0794: 4FEF 000A      lea 10(a7), d7
0798: 2F0A           movel a2, -(a7)
079A: 4E4F           trap #15
079C: A021           sysTrapMemHandleLock
079E: 2648           movel a0, a3
07A0: 3F03           movew d3, -(a7)
07A2: 3F04           movew d4, -(a7)
07A4: 2F0B           movel a3, -(a7)
07A6: 4E4F           trap #15
07A8: A226           sysTrapWinDrawBitmap
07AA: 2F0B           movel a3, -(a7)
07AC: 4E4F           trap #15
07AE: A035           sysTrapMemPtrUnlock
07B0: 2F0A           movel a2, -(a7)
07B2: 4E4F           trap #15
07B4: A061           sysTrapDmReleaseResource
07B6: 4FEF 0014      lea 20(a7), d7
07BA: 4CDF 0C38      movem (a7)+, <0c38>
07BE: 4E5E           unlk a6
07C0: 4E75           rts
07C2: 8A44           orrw d4, d5
07C4: 7261           moveq #97, d1
07C6: 7742           dc.w #30530
07C8: 6974           bvs 116
07CA: 6D61           blt 97
07CC: 7000           moveq #0, d0
07CE: 001A           dc.w #26
07D0: 4D69           dc.w #19817
07D2: 7373           dc.w #29555
07D4: 696E           bvs 110
07D6: 6720           beq 32
07D8: 6269           bhi 105
07DA: 746D           moveq #109, d2
07DC: 6170           bsr 112
07DE: 0000           dc.w #0
07E0: 5375 6248      subqw.ex #1, 72(a5,d6.w)
07E4: 756E           dc.w #30062
07E6: 742E           moveq #46, d2
07E8: 6300 4E56      bls 20054
07EC: FFFA           dc.w #65530
07EE: 4878 FF87      pea.ex (ff87).w
07F2: 4E4F           trap #15
07F4: A2CF           sysTrapKeySetMask
07F6: 4E5E           unlk a6
07F8: 4E75           rts
07FA: 8C47           orrw d7, d6
07FC: 616D           bsr 109
07FE: 654D           bcs 77
0800: 6173           bsr 115
0802: 6B4B           bmi 75
0804: 6579           bcs 121
0806: 7300           dc.w #29440
0808: 0000           dc.w #0
080A: 4E56 0000      link a6, #0
080E: 4878 FFFF      pea.ex (ffff).w
0812: 4E4F           trap #15
0814: A2CF           sysTrapKeySetMask
0816: 4E5E           unlk a6
0818: 4E75           rts
081A: 8E47           orrw d7, d7
081C: 616D           bsr 109
081E: 6555           bcs 85
0820: 6E6D           bgt 109
0822: 6173           bsr 115
0824: 6B4B           bmi 75
0826: 6579           bcs 121
0828: 7300           dc.w #29440
082A: 0000           dc.w #0
082C: 4E56 FFF8      link a6, #-8
0830: 48E7 1E00      movem <1e00>, -(a7)
0834: 362E 0008      movew.ex 8(a6), d3
0838: 3C2E 000A      movew.ex 10(a6), d6
083C: 3A2E 000C      movew.ex 12(a6), d5
0840: 182E 000E      moveb.ex 14(a6), d4
0844: 3043           movew d3, a0
0846: 2008           movel a0, d0
0848: E588           lslil #2, d0
084A: 41ED F630      lea -2512(a5), d0
084E: 4AB0 0800      tstl.ex 0(a0,d0.l)
0852: 6618           bne 24
0854: 41FA 006C      lea 31946(pc), d0
0858: 4850           pea (a0)
085A: 3F3C 0459      movew.ex #1113, -(a7)
085E: 41FA 007A      lea 31970(pc), d0
0862: 4850           pea (a0)
0864: 4E4F           trap #15
0866: A084           sysTrapErrDisplayFileLineMsg
0868: 4FEF 000A      lea 10(a7), d7
086C: 4878 0008      pea.ex (0008).w
0870: 3043           movew d3, a0
0872: 2008           movel a0, d0
0874: E588           lslil #2, d0
0876: 41ED F630      lea -2512(a5), d0
087A: 2070 0800      movel.ex 0(a0,d0.l), a0
087E: 4868 000A      pea.ex 10(a0)
0882: 486E FFF8      pea.ex -8(a6)
0886: 4E4F           trap #15
0888: A026           sysTrapMemMove
088A: 1F04           moveb d4, -(a7)
088C: 3F05           movew d5, -(a7)
088E: 3F06           movew d6, -(a7)
0890: 486E FFF8      pea.ex -8(a6)
0894: 42A7           clrl -(a7)
0896: 3043           movew d3, a0
0898: 2008           movel a0, d0
089A: E588           lslil #2, d0
089C: 41ED F630      lea -2512(a5), d0
08A0: 2F30 0800      movel.ex 0(a0,d0.l), -(a7)
08A4: 4E4F           trap #15
08A6: A209           sysTrapWinCopyRectangle
08A8: 4FEF 001E      lea 30(a7), d7
08AC: 4CDF 0078      movem (a7)+, <0078>
08B0: 4E5E           unlk a6
08B2: 4E75           rts
08B4: 8A44           orrw d4, d5
08B6: 7261           moveq #97, d1
08B8: 774F           dc.w #30543
08BA: 626A           bhi 106
08BC: 6563           bcs 99
08BE: 7400           moveq #0, d2
08C0: 0022           dc.w #34
08C2: 556E 6861      subqw.ex #2, 26721(a6)
08C6: 6E64           bgt 100
08C8: 6C65           bge 101
08CA: 6420           bcc 32
08CC: 6F62           ble 98
08CE: 6A65           bpl 101
08D0: 6374           bls 116
08D2: 2069 6D61      movel.ex 28001(a1), a0
08D6: 6765           beq 101
08D8: 0000           dc.w #0
08DA: 5375 6248      subqw.ex #1, 72(a5,d6.w)
08DE: 756E           dc.w #30062
08E0: 742E           moveq #46, d2
08E2: 6300 4E56      bls 20054
08E6: 0000           dc.w #0
08E8: 142E 0008      moveb.ex 8(a6), d2
08EC: 7000           moveq #0, d0
08EE: 102D FFB6      moveb.ex -74(a5), d0
08F2: E788           lslil #3, d0
08F4: 41ED FFBC      lea -68(a5), d0
08F8: 7200           moveq #0, d1
08FA: 1202           moveb d2, d1
08FC: E789           lslil #3, d1
08FE: 2248           movel a0, a1
0900: 1231 1800      moveb.ex 0(a1,d1.l), d1
0904: B230 0800      cmpb.ex 0(a0,d0.l), d1
0908: 6512           bcs 18
090A: 1B42 FFB6      moveb.mx d2, -74(a5)
090E: 7000           moveq #0, d0
0910: 1002           moveb d2, d0
0912: E788           lslil #3, d0
0914: 5288           addql #1, a0
0916: 1B70 0800 FFB7 moveb.emx 0(a0,d0.l), -73(a5)
091C: 4E5E           unlk a6
091E: 4E75           rts
0920: 9047           subrw d7, d0
0922: 616D           bsr 109
0924: 6552           bcs 82
0926: 6571           bcs 113
0928: 7565           dc.w #30053
092A: 7374           dc.w #29556
092C: 536F 756E      subqw.ex #1, 30062(a7)
0930: 6400 0000      bcc 0
0934: 4E56 0000      link a6, #0
0938: 202E 0008      movel.ex 8(a6), d0
093C: D1AD FB70      addml.ex d0, -1168(a5)
0940: 4A2D FFB9      tstb.ex -71(a5)
0944: 6626           bne 38
0946: 202D FB70      movel.ex -1168(a5), d0
094A: B0AD F616      cmpl.ex -2538(a5), d0
094E: 6F1C           ble 28
0950: 4AAD F616      tstl.ex -2538(a5)
0954: 6F16           ble 22
0956: 1B7C 0001 FFB9 moveb.emx #1, -71(a5)
095C: 4AAD FB70      tstl.ex -1168(a5)
0960: 6F0A           ble 10
0962: 1F3C 0007      moveb.ex #7, -(a7)
0966: 4EBA FF7C      jsr.ex 31980(pc)
096A: 544F           addqw #2, a7
096C: 4A2D FFBA      tstb.ex -70(a5)
0970: 6626           bne 38
0972: 202D FB70      movel.ex -1168(a5), d0
0976: B0AD F566      cmpl.ex -2714(a5), d0
097A: 6F1C           ble 28
097C: 4AAD F566      tstl.ex -2714(a5)
0980: 6F16           ble 22
0982: 1B7C 0001 FFBA moveb.emx #1, -70(a5)
0988: 4AAD FB70      tstl.ex -1168(a5)
098C: 6F0A           ble 10
098E: 1F3C 0007      moveb.ex #7, -(a7)
0992: 4EBA FF50      jsr.ex 31980(pc)
0996: 544F           addqw #2, a7
0998: 4E5E           unlk a6
099A: 4E75           rts
099C: 8D49           dc.w #36169
099E: 6E63           bgt 99
09A0: 7265           moveq #101, d1
09A2: 6173           bsr 115
09A4: 6553           bcs 83
09A6: 636F           bls 111
09A8: 7265           moveq #101, d1
09AA: 0000           dc.w #0
09AC: 4E56 0000      link a6, #0
09B0: 1B7C 0001 FB7F moveb.emx #1, -1153(a5)
09B6: 1B7C 0001 FB7E moveb.emx #1, -1154(a5)
09BC: 422D FB80      clrb.ex -1152(a5)
09C0: 422D F72D      clrb.ex -2259(a5)
09C4: 3B7C 003E FB76 movew.emx #62, -1162(a5)
09CA: 3B7C 0001 FB78 movew.emx #1, -1160(a5)
09D0: 7024           moveq #36, d0
09D2: D06D FB76      addrw.ex -1162(a5), d0
09D6: 3B40 FB7A      movew.mx d0, -1158(a5)
09DA: 700A           moveq #10, d0
09DC: D06D FB78      addrw.ex -1160(a5), d0
09E0: 3B40 FB7C      movew.mx d0, -1156(a5)
09E4: 1B7C 0005 FFAF moveb.emx #5, -81(a5)
09EA: 422D FFB1      clrb.ex -79(a5)
09EE: 1B7C 0046 FFB3 moveb.emx #70, -77(a5)
09F4: 1B7C 0005 FFAE moveb.emx #5, -82(a5)
09FA: 422D FFB0      clrb.ex -80(a5)
09FE: 1B7C 0046 FFB2 moveb.emx #70, -78(a5)
0A04: 4E5E           unlk a6
0A06: 4E75           rts
0A08: 8753           ormw d3, (a3)
0A0A: 6869           bvc 105
0A0C: 7041           moveq #65, d0
0A0E: 6464           bcc 100
0A10: 0000           dc.w #0
0A12: 4E56 0000      link a6, #0
0A16: 522D F72C      addqb.ex #1, -2260(a5)
0A1A: 1B7C 0001 FB7F moveb.emx #1, -1153(a5)
0A20: 0C2D 0003 F72C cmpib.ex #3, -2260(a5)
0A26: 6508           bcs 8
0A28: 522D F72D      addqb.ex #1, -2259(a5)
0A2C: 422D FB88      clrb.ex -1144(a5)
0A30: 1F3C 0005      moveb.ex #5, -(a7)
0A34: 4EBA FEAE      jsr.ex 31980(pc)
0A38: 4E5E           unlk a6
0A3A: 4E75           rts
0A3C: 8753           ormw d3, (a3)
0A3E: 6869           bvc 105
0A40: 7048           moveq #72, d0
0A42: 6974           bvs 116
0A44: 0000           dc.w #0
0A46: 4E56 0000      link a6, #0
0A4A: 2F05           movel d5, -(a7)
0A4C: 1A2E 0008      moveb.ex 8(a6), d5
0A50: 122E 000A      moveb.ex 10(a6), d1
0A54: 0C2D 0014 FB74 cmpib.ex #20, -1164(a5)
0A5A: 6506           bcs 6
0A5C: 7000           moveq #0, d0
0A5E: 6000 0088      bra 136
0A62: 102D FB74      moveb.ex -1164(a5), d0
0A66: 522D FB74      addqb.ex #1, -1164(a5)
0A6A: 7400           moveq #0, d2
0A6C: 1400           moveb d0, d2
0A6E: 700E           moveq #14, d0
0A70: C1C2           muls d2, d0
0A72: 43ED F714      lea -2284(a5), d1
0A76: D3C0           addal d0, a1
0A78: 43E9 0552      lea 1362(a1), d1
0A7C: 137C 0001 0008 moveb.emx #1, 8(a1)
0A82: 137C 0001 0009 moveb.emx #1, 9(a1)
0A88: 4229 000A      clrb.ex 10(a1)
0A8C: 1345 000B      moveb.mx d5, 11(a1)
0A90: 0C01 0001      cmpib #1, d1
0A94: 660A           bne 10
0A96: 302D FB76      movew.ex -1162(a5), d0
0A9A: 5B40           subqw #5, d0
0A9C: 3280           movew d0, (a1)
0A9E: 6008           bra 8
0AA0: 302D FB7A      movew.ex -1158(a5), d0
0AA4: 5640           addqw #3, d0
0AA6: 3280           movew d0, (a1)
0AA8: 336D FB7C 0002 movew.emx -1156(a5), 2(a1)
0AAE: 3011           movew (a1), d0
0AB0: 5440           addqw #2, d0
0AB2: 3340 0004      movew.mx d0, 4(a1)
0AB6: 3029 0002      movew.ex 2(a1), d0
0ABA: 5440           addqw #2, d0
0ABC: 3340 0006      movew.mx d0, 6(a1)
0AC0: 4229 000C      clrb.ex 12(a1)
0AC4: 7000           moveq #0, d0
0AC6: 1001           moveb d1, d0
0AC8: 41ED F714      lea -2284(a5), d0
0ACC: 41E8 089A      lea 2202(a0), d0
0AD0: 5330 0000      subqb.ex #1, 0(a0,d0.w)
0AD4: 7000           moveq #0, d0
0AD6: 1001           moveb d1, d0
0AD8: 41ED F714      lea -2284(a5), d0
0ADC: 41E8 089C      lea 2204(a0), d0
0AE0: 11BC 0014 0000 moveb.emx #20, 0(a0,d0.w)
0AE6: 7001           moveq #1, d0
0AE8: 2A1F           movel (a7)+, d5
0AEA: 4E5E           unlk a6
0AEC: 4E75           rts
0AEE: 8E44           orrw d4, d7
0AF0: 6570           bcs 112
0AF2: 7468           moveq #104, d2
0AF4: 4368           dc.w #17256
0AF6: 6172           bsr 114
0AF8: 6765           beq 101
0AFA: 4164           dc.w #16740
0AFC: 6400 0000      bcc 0
0B00: 4E56 0000      link a6, #0
0B04: 2F0A           movel a2, -(a7)
0B06: 700E           moveq #14, d0
0B08: C1EE 0008      muls.ex 8(a6), d0
0B0C: 45ED F714      lea -2284(a5), d2
0B10: D5C0           addal d0, a2
0B12: 45EA 0552      lea 1362(a2), d2
0B16: 5B52           subqw #5, (a2)
0B18: 5B6A 0002      subqw.ex #5, 2(a2)
0B1C: 700C           moveq #12, d0
0B1E: D052           addrw (a2), d0
0B20: 3540 0004      movew.mx d0, 4(a2)
0B24: 700C           moveq #12, d0
0B26: D06A 0002      addrw.ex 2(a2), d0
0B2A: 3540 0006      movew.mx d0, 6(a2)
0B2E: 157C 0001 000A moveb.emx #1, 10(a2)
0B34: 157C 000A 000C moveb.emx #10, 12(a2)
0B3A: 157C 0001 0009 moveb.emx #1, 9(a2)
0B40: 1F3C 0003      moveb.ex #3, -(a7)
0B44: 4EBA FD9E      jsr.ex 31980(pc)
0B48: 544F           addqw #2, a7
0B4A: 245F           movel (a7)+, a2
0B4C: 4E5E           unlk a6
0B4E: 4E75           rts
0B50: 9244           subrw d4, d1
0B52: 6570           bcs 112
0B54: 7468           moveq #104, d2
0B56: 4368           dc.w #17256
0B58: 6172           bsr 114
0B5A: 6765           beq 101
0B5C: 4578           dc.w #17784
0B5E: 706C           moveq #108, d0
0B60: 6F64           ble 100
0B62: 6500 0000      bcs 0
0B66: 4E56 0000      link a6, #0
0B6A: 2F03           movel d3, -(a7)
0B6C: 362E 0008      movew.ex 8(a6), d3
0B70: 7000           moveq #0, d0
0B72: 102D FB74      moveb.ex -1164(a5), d0
0B76: B640           cmpw d0, d3
0B78: 6D18           blt 24
0B7A: 41FA 010E      lea -32622(pc), d0
0B7E: 4850           pea (a0)
0B80: 3F3C 056C      movew.ex #1388, -(a7)
0B84: 41FA 012A      lea -32584(pc), d0
0B88: 4850           pea (a0)
0B8A: 4E4F           trap #15
0B8C: A084           sysTrapErrDisplayFileLineMsg
0B8E: 4FEF 000A      lea 10(a7), d7
0B92: 7000           moveq #0, d0
0B94: 102D FB74      moveb.ex -1164(a5), d0
0B98: 9043           subrw d3, d0
0B9A: 0C40 0001      cmpiw #1, d0
0B9E: 6F6C           ble 108
0BA0: 7000           moveq #0, d0
0BA2: 102D FB74      moveb.ex -1164(a5), d0
0BA6: 5340           subqw #1, d0
0BA8: 9043           subrw d3, d0
0BAA: C0FC 000E      mulu.ex #14, d0
0BAE: 2F00           movel d0, -(a7)
0BB0: 3003           movew d3, d0
0BB2: 5240           addqw #1, d0
0BB4: C1FC 000E      muls.ex #14, d0
0BB8: 41ED F714      lea -2284(a5), d0
0BBC: D1C0           addal d0, a0
0BBE: 4868 0552      pea.ex 1362(a0)
0BC2: 700E           moveq #14, d0
0BC4: C1C3           muls d3, d0
0BC6: 41ED F714      lea -2284(a5), d0
0BCA: D1C0           addal d0, a0
0BCC: 4868 0552      pea.ex 1362(a0)
0BD0: 4E4F           trap #15
0BD2: A026           sysTrapMemMove
0BD4: 7000           moveq #0, d0
0BD6: 102D F736      moveb.ex -2250(a5), d0
0BDA: 5340           subqw #1, d0
0BDC: 9043           subrw d3, d0
0BDE: C0FC 000E      mulu.ex #14, d0
0BE2: 2F00           movel d0, -(a7)
0BE4: 3003           movew d3, d0
0BE6: 5240           addqw #1, d0
0BE8: C1FC 000E      muls.ex #14, d0
0BEC: 41ED F714      lea -2284(a5), d0
0BF0: D1C0           addal d0, a0
0BF2: 4868 0114      pea.ex 276(a0)
0BF6: 700E           moveq #14, d0
0BF8: C1C3           muls d3, d0
0BFA: 41ED F714      lea -2284(a5), d0
0BFE: D1C0           addal d0, a0
0C00: 4868 0114      pea.ex 276(a0)
0C04: 4E4F           trap #15
0C06: A026           sysTrapMemMove
0C08: 4FEF 0018      lea 24(a7), d7
0C0C: 7000           moveq #0, d0
0C0E: 102D FB74      moveb.ex -1164(a5), d0
0C12: 5340           subqw #1, d0
0C14: C1FC 000E      muls.ex #14, d0
0C18: 41ED F714      lea -2284(a5), d0
0C1C: 41E8 055A      lea 1370(a0), d0
0C20: 4230 0800      clrb.ex 0(a0,d0.l)
0C24: 7000           moveq #0, d0
0C26: 102D FB74      moveb.ex -1164(a5), d0
0C2A: 5340           subqw #1, d0
0C2C: C1FC 000E      muls.ex #14, d0
0C30: 41ED F714      lea -2284(a5), d0
0C34: 41E8 055B      lea 1371(a0), d0
0C38: 4230 0800      clrb.ex 0(a0,d0.l)
0C3C: 7000           moveq #0, d0
0C3E: 102D FB74      moveb.ex -1164(a5), d0
0C42: 5340           subqw #1, d0
0C44: C1FC 000E      muls.ex #14, d0
0C48: 41ED F714      lea -2284(a5), d0
0C4C: 41E8 011C      lea 284(a0), d0
0C50: 4230 0800      clrb.ex 0(a0,d0.l)
0C54: 7000           moveq #0, d0
0C56: 102D FB74      moveb.ex -1164(a5), d0
0C5A: 5340           subqw #1, d0
0C5C: C1FC 000E      muls.ex #14, d0
0C60: 41ED F714      lea -2284(a5), d0
0C64: 41E8 011D      lea 285(a0), d0
0C68: 4230 0800      clrb.ex 0(a0,d0.l)
0C6C: 532D FB74      subqb.ex #1, -1164(a5)
0C70: 261F           movel (a7)+, d3
0C72: 4E5E           unlk a6
0C74: 4E75           rts
0C76: 9144           subxrw d4, d0
0C78: 6570           bcs 112
0C7A: 7468           moveq #104, d2
0C7C: 4368           dc.w #17256
0C7E: 6172           bsr 114
0C80: 6765           beq 101
0C82: 5265           addqw #1, -(a5)
0C84: 6D6F           blt 111
0C86: 7665           moveq #101, d3
0C88: 0030           dc.w #48
0C8A: 5265           addqw #1, -(a5)
0C8C: 6D6F           blt 111
0C8E: 7669           moveq #105, d3
0C90: 6E67           bgt 103
0C92: 2061           movel -(a1), a0
0C94: 2063           movel -(a3), a0
0C96: 6861           bvc 97
0C98: 7267           moveq #103, d1
0C9A: 6520           bcs 32
0C9C: 7468           moveq #104, d2
0C9E: 6174           bsr 116
0CA0: 2064           movel -(a4), a0
0CA2: 6F65           ble 101
0CA4: 736E           dc.w #29550
0CA6: 2774 2065 7869 movel.emx 101(a4,d2.w), 30825(a3)
0CAC: 7374           dc.w #29556
0CAE: 0000           dc.w #0
0CB0: 5375 6248      subqw.ex #1, 72(a5,d6.w)
0CB4: 756E           dc.w #30062
0CB6: 742E           moveq #46, d2
0CB8: 6300 4E56      bls 20054
0CBC: FFFE           dc.w #65534
0CBE: 48E7 1F20      movem <1f20>, -(a7)
0CC2: 382E 000A      movew.ex 10(a6), d4
0CC6: 3C2E 000C      movew.ex 12(a6), d6
0CCA: 7000           moveq #0, d0
0CCC: 102E 0008      moveb.ex 8(a6), d0
0CD0: 5340           subqw #1, d0
0CD2: 670A           beq 10
0CD4: 5340           subqw #1, d0
0CD6: 670C           beq 12
0CD8: 5340           subqw #1, d0
0CDA: 670E           beq 14
0CDC: 6010           bra 16
0CDE: 7A03           moveq #3, d5
0CE0: 7E03           moveq #3, d7
0CE2: 600A           bra 10
0CE4: 7A03           moveq #3, d5
0CE6: 7E03           moveq #3, d7
0CE8: 6004           bra 4
0CEA: 7A04           moveq #4, d5
0CEC: 7E04           moveq #4, d7
0CEE: 0C2E 0001 000E cmpib.ex #1, 14(a6)
0CF4: 57C0           seq d0
0CF6: 4400           negb d0
0CF8: 4880           extw d0
0CFA: 6708           beq 8
0CFC: 3D7C 0028 FFFE movew.emx #40, -2(a6)
0D02: 6006           bra 6
0D04: 3D7C 001E FFFE movew.emx #30, -2(a6)
0D0A: 7000           moveq #0, d0
0D0C: 102D FB75      moveb.ex -1163(a5), d0
0D10: B06E FFFE      cmpw.ex -2(a6), d0
0D14: 6C0E           bge 14
0D16: 3004           movew d4, d0
0D18: D045           addrw d5, d0
0D1A: 4A40           tstw d0
0D1C: 6D06           blt 6
0D1E: 0C44 00A0      cmpiw #160, d4
0D22: 6F06           ble 6
0D24: 7000           moveq #0, d0
0D26: 6000 013E      bra 318
0D2A: 7600           moveq #0, d3
0D2C: 162D FB75      moveb.ex -1163(a5), d3
0D30: 5343           subqw #1, d3
0D32: 6000 0088      bra 136
0D36: 700E           moveq #14, d0
0D38: C1C3           muls d3, d0
0D3A: 45ED F714      lea -2284(a5), d2
0D3E: D5C0           addal d0, a2
0D40: 45EA 066A      lea 1642(a2), d2
0D44: BC6A 0002      cmpw.ex 2(a2), d6
0D48: 6D70           blt 112
0D4A: BC6A 0006      cmpw.ex 6(a2), d6
0D4E: 6C10           bge 16
0D50: B852           cmpw (a2), d4
0D52: 6D0C           blt 12
0D54: B86A 0004      cmpw.ex 4(a2), d4
0D58: 6C06           bge 6
0D5A: 7000           moveq #0, d0
0D5C: 6000 0108      bra 264
0D60: 5243           addqw #1, d3
0D62: 700E           moveq #14, d0
0D64: C1C3           muls d3, d0
0D66: 45ED F714      lea -2284(a5), d2
0D6A: D5C0           addal d0, a2
0D6C: 45EA 066A      lea 1642(a2), d2
0D70: BC6A 0002      cmpw.ex 2(a2), d6
0D74: 6D06           blt 6
0D76: BC6A 0006      cmpw.ex 6(a2), d6
0D7A: 6F18           ble 24
0D7C: 3006           movew d6, d0
0D7E: D047           addrw d7, d0
0D80: 5340           subqw #1, d0
0D82: B06A 0002      cmpw.ex 2(a2), d0
0D86: 6D3A           blt 58
0D88: 3006           movew d6, d0
0D8A: D047           addrw d7, d0
0D8C: 5340           subqw #1, d0
0D8E: B06A 0006      cmpw.ex 6(a2), d0
0D92: 6E2E           bgt 46
0D94: B852           cmpw (a2), d4
0D96: 6D06           blt 6
0D98: B86A 0004      cmpw.ex 4(a2), d4
0D9C: 6D16           blt 22
0D9E: 3004           movew d4, d0
0DA0: D045           addrw d5, d0
0DA2: 5340           subqw #1, d0
0DA4: B052           cmpw (a2), d0
0DA6: 6D1A           blt 26
0DA8: 3004           movew d4, d0
0DAA: D045           addrw d5, d0
0DAC: 5340           subqw #1, d0
0DAE: B06A 0004      cmpw.ex 4(a2), d0
0DB2: 6C0E           bge 14
0DB4: 7000           moveq #0, d0
0DB6: 6000 00AE      bra 174
0DBA: 5343           subqw #1, d3
0DBC: 4A43           tstw d3
0DBE: 6C00 FF76      bge 65398
0DC2: 4A43           tstw d3
0DC4: 6C0A           bge 10
0DC6: 7600           moveq #0, d3
0DC8: 45ED F714      lea -2284(a5), d2
0DCC: 45EA 066A      lea 1642(a2), d2
0DD0: 4A2D FB75      tstb.ex -1163(a5)
0DD4: 6758           beq 88
0DD6: 7000           moveq #0, d0
0DD8: 102D FB75      moveb.ex -1163(a5), d0
0DDC: B043           cmpw d3, d0
0DDE: 6F4E           ble 78
0DE0: 7000           moveq #0, d0
0DE2: 102D FB75      moveb.ex -1163(a5), d0
0DE6: 9043           subrw d3, d0
0DE8: C0FC 000E      mulu.ex #14, d0
0DEC: 2F00           movel d0, -(a7)
0DEE: 2F0A           movel a2, -(a7)
0DF0: 486A 000E      pea.ex 14(a2)
0DF4: 4E4F           trap #15
0DF6: A026           sysTrapMemMove
0DF8: 7000           moveq #0, d0
0DFA: 102D F737      moveb.ex -2249(a5), d0
0DFE: 9043           subrw d3, d0
0E00: C0FC 000E      mulu.ex #14, d0
0E04: 2F00           movel d0, -(a7)
0E06: 700E           moveq #14, d0
0E08: C1C3           muls d3, d0
0E0A: 41ED F714      lea -2284(a5), d0
0E0E: D1C0           addal d0, a0
0E10: 4868 022C      pea.ex 556(a0)
0E14: 3003           movew d3, d0
0E16: 5240           addqw #1, d0
0E18: C1FC 000E      muls.ex #14, d0
0E1C: 41ED F714      lea -2284(a5), d0
0E20: D1C0           addal d0, a0
0E22: 4868 022C      pea.ex 556(a0)
0E26: 4E4F           trap #15
0E28: A026           sysTrapMemMove
0E2A: 4FEF 0018      lea 24(a7), d7
0E2E: 157C 0001 0008 moveb.emx #1, 8(a2)
0E34: 157C 0001 0009 moveb.emx #1, 9(a2)
0E3A: 422A 000A      clrb.ex 10(a2)
0E3E: 156E 0008 000B moveb.emx 8(a6), 11(a2)
0E44: 3484           movew d4, (a2)
0E46: 3546 0002      movew.mx d6, 2(a2)
0E4A: 3012           movew (a2), d0
0E4C: D045           addrw d5, d0
0E4E: 3540 0004      movew.mx d0, 4(a2)
0E52: 302A 0002      movew.ex 2(a2), d0
0E56: D047           addrw d7, d0
0E58: 3540 0006      movew.mx d0, 6(a2)
0E5C: 422A 000C      clrb.ex 12(a2)
0E60: 522D FB75      addqb.ex #1, -1163(a5)
0E64: 7001           moveq #1, d0
0E66: 4CDF 04F8      movem (a7)+, <04f8>
0E6A: 4E5E           unlk a6
0E6C: 4E75           rts
0E6E: 874D           dc.w #34637
0E70: 696E           bvs 110
0E72: 6541           bcs 65
0E74: 6464           bcc 100
0E76: 0000           dc.w #0
0E78: 4E56 0000      link a6, #0
0E7C: 2F0A           movel a2, -(a7)
0E7E: 700E           moveq #14, d0
0E80: C1EE 0008      muls.ex 8(a6), d0
0E84: 45ED F714      lea -2284(a5), d2
0E88: D5C0           addal d0, a2
0E8A: 45EA 066A      lea 1642(a2), d2
0E8E: 157C 0001 000A moveb.emx #1, 10(a2)
0E94: 157C 000A 000C moveb.emx #10, 12(a2)
0E9A: 157C 0001 0009 moveb.emx #1, 9(a2)
0EA0: 3012           movew (a2), d0
0EA2: B06D FB76      cmpw.ex -1162(a5), d0
0EA6: 6D44           blt 68
0EA8: 302A 0004      movew.ex 4(a2), d0
0EAC: B06D FB7A      cmpw.ex -1158(a5), d0
0EB0: 6E3A           bgt 58
0EB2: 302A 0004      movew.ex 4(a2), d0
0EB6: 9052           subrw (a2), d0
0EB8: 0640 FFF4      addiw #-12, d0
0EBC: 3200           movew d0, d1
0EBE: E049           lsriw #0, d1
0EC0: EE49           lsriw #7, d1
0EC2: D240           addrw d0, d1
0EC4: E241           asriw #1, d1
0EC6: D352           addmw d1, (a2)
0EC8: 046A 000C 0002 subiw.ex #12, 2(a2)
0ECE: 700C           moveq #12, d0
0ED0: D052           addrw (a2), d0
0ED2: 3540 0004      movew.mx d0, 4(a2)
0ED6: 700C           moveq #12, d0
0ED8: D06A 0002      addrw.ex 2(a2), d0
0EDC: 3540 0006      movew.mx d0, 6(a2)
0EE0: 157C 0005 000B moveb.emx #5, 11(a2)
0EE6: 4EBA FB2A      jsr.ex 32282(pc)
0EEA: 603C           bra 60
0EEC: 302A 0004      movew.ex 4(a2), d0
0EF0: 9052           subrw (a2), d0
0EF2: 5B40           subqw #5, d0
0EF4: 3200           movew d0, d1
0EF6: E049           lsriw #0, d1
0EF8: EE49           lsriw #7, d1
0EFA: D240           addrw d0, d1
0EFC: E241           asriw #1, d1
0EFE: D352           addmw d1, (a2)
0F00: 046A 000A 0002 subiw.ex #10, 2(a2)
0F06: 3012           movew (a2), d0
0F08: 5A40           addqw #5, d0
0F0A: 3540 0004      movew.mx d0, 4(a2)
0F0E: 700A           moveq #10, d0
0F10: D06A 0002      addrw.ex 2(a2), d0
0F14: 3540 0006      movew.mx d0, 6(a2)
0F18: 157C 0004 000B moveb.emx #4, 11(a2)
0F1E: 1F3C 0004      moveb.ex #4, -(a7)
0F22: 4EBA F9C0      jsr.ex 31980(pc)
0F26: 544F           addqw #2, a7
0F28: 245F           movel (a7)+, a2
0F2A: 4E5E           unlk a6
0F2C: 4E75           rts
0F2E: 8B4D           dc.w #35661
0F30: 696E           bvs 110
0F32: 6545           bcs 69
0F34: 7870           moveq #112, d4
0F36: 6C6F           bge 111
0F38: 6465           bcc 101
0F3A: 0000           dc.w #0
0F3C: 4E56 0000      link a6, #0
0F40: 2F03           movel d3, -(a7)
0F42: 362E 0008      movew.ex 8(a6), d3
0F46: 7000           moveq #0, d0
0F48: 102D FB75      moveb.ex -1163(a5), d0
0F4C: B640           cmpw d0, d3
0F4E: 6D18           blt 24
0F50: 41FA 0108      lea -31646(pc), d0
0F54: 4850           pea (a0)
0F56: 3F3C 0666      movew.ex #1638, -(a7)
0F5A: 41FA 0122      lea -31610(pc), d0
0F5E: 4850           pea (a0)
0F60: 4E4F           trap #15
0F62: A084           sysTrapErrDisplayFileLineMsg
0F64: 4FEF 000A      lea 10(a7), d7
0F68: 7000           moveq #0, d0
0F6A: 102D FB75      moveb.ex -1163(a5), d0
0F6E: 9043           subrw d3, d0
0F70: 0C40 0001      cmpiw #1, d0
0F74: 6F6C           ble 108
0F76: 7000           moveq #0, d0
0F78: 102D FB75      moveb.ex -1163(a5), d0
0F7C: 5340           subqw #1, d0
0F7E: 9043           subrw d3, d0
0F80: C0FC 000E      mulu.ex #14, d0
0F84: 2F00           movel d0, -(a7)
0F86: 3003           movew d3, d0
0F88: 5240           addqw #1, d0
0F8A: C1FC 000E      muls.ex #14, d0
0F8E: 41ED F714      lea -2284(a5), d0
0F92: D1C0           addal d0, a0
0F94: 4868 066A      pea.ex 1642(a0)
0F98: 700E           moveq #14, d0
0F9A: C1C3           muls d3, d0
0F9C: 41ED F714      lea -2284(a5), d0
0FA0: D1C0           addal d0, a0
0FA2: 4868 066A      pea.ex 1642(a0)
0FA6: 4E4F           trap #15
0FA8: A026           sysTrapMemMove
0FAA: 7000           moveq #0, d0
0FAC: 102D F737      moveb.ex -2249(a5), d0
0FB0: 5340           subqw #1, d0
0FB2: 9043           subrw d3, d0
0FB4: C0FC 000E      mulu.ex #14, d0
0FB8: 2F00           movel d0, -(a7)
0FBA: 3003           movew d3, d0
0FBC: 5240           addqw #1, d0
0FBE: C1FC 000E      muls.ex #14, d0
0FC2: 41ED F714      lea -2284(a5), d0
0FC6: D1C0           addal d0, a0
0FC8: 4868 022C      pea.ex 556(a0)
0FCC: 700E           moveq #14, d0
0FCE: C1C3           muls d3, d0
0FD0: 41ED F714      lea -2284(a5), d0
0FD4: D1C0           addal d0, a0
0FD6: 4868 022C      pea.ex 556(a0)
0FDA: 4E4F           trap #15
0FDC: A026           sysTrapMemMove
0FDE: 4FEF 0018      lea 24(a7), d7
0FE2: 7000           moveq #0, d0
0FE4: 102D FB75      moveb.ex -1163(a5), d0
0FE8: 5340           subqw #1, d0
0FEA: C1FC 000E      muls.ex #14, d0
0FEE: 41ED F714      lea -2284(a5), d0
0FF2: 41E8 0672      lea 1650(a0), d0
0FF6: 4230 0800      clrb.ex 0(a0,d0.l)
0FFA: 7000           moveq #0, d0
0FFC: 102D FB75      moveb.ex -1163(a5), d0
1000: 5340           subqw #1, d0
1002: C1FC 000E      muls.ex #14, d0
1006: 41ED F714      lea -2284(a5), d0
100A: 41E8 0673      lea 1651(a0), d0
100E: 4230 0800      clrb.ex 0(a0,d0.l)
1012: 7000           moveq #0, d0
1014: 102D FB75      moveb.ex -1163(a5), d0
1018: 5340           subqw #1, d0
101A: C1FC 000E      muls.ex #14, d0
101E: 41ED F714      lea -2284(a5), d0
1022: 41E8 0234      lea 564(a0), d0
1026: 4230 0800      clrb.ex 0(a0,d0.l)
102A: 7000           moveq #0, d0
102C: 102D FB75      moveb.ex -1163(a5), d0
1030: 5340           subqw #1, d0
1032: C1FC 000E      muls.ex #14, d0
1036: 41ED F714      lea -2284(a5), d0
103A: 41E8 0235      lea 565(a0), d0
103E: 4230 0800      clrb.ex 0(a0,d0.l)
1042: 532D FB75      subqb.ex #1, -1163(a5)
1046: 261F           movel (a7)+, d3
1048: 4E5E           unlk a6
104A: 4E75           rts
104C: 8A4D           dc.w #35405
104E: 696E           bvs 110
1050: 6552           bcs 82
1052: 656D           bcs 109
1054: 6F76           ble 118
1056: 6500 002E      bcs 46
105A: 5265           addqw #1, -(a5)
105C: 6D6F           blt 111
105E: 7669           moveq #105, d3
1060: 6E67           bgt 103
1062: 2061           movel -(a1), a0
1064: 206D 696E      movel.ex 26990(a5), a0
1068: 6520           bcs 32
106A: 7468           moveq #104, d2
106C: 6174           bsr 116
106E: 2064           movel -(a4), a0
1070: 6F65           ble 101
1072: 736E           dc.w #29550
1074: 2774 2065 7869 movel.emx 101(a4,d2.w), 30825(a3)
107A: 7374           dc.w #29556
107C: 0000           dc.w #0
107E: 5375 6248      subqw.ex #1, 72(a5,d6.w)
1082: 756E           dc.w #30062
1084: 742E           moveq #46, d2
1086: 6300 4E56      bls 20054
108A: FFFC           dc.w #65532
108C: 48E7 1F20      movem <1f20>, -(a7)
1090: 1E2E 0008      moveb.ex 8(a6), d7
1094: 42A7           clrl -(a7)
1096: 4E4F           trap #15
1098: A0C2           sysTrapSysRandom
109A: C1FC 0009      muls.ex #9, d0
109E: 2200           movel d0, d1
10A0: E081           asril #0, d1
10A2: EC81           asril #6, d1
10A4: E089           lsril #0, d1
10A6: E089           lsril #0, d1
10A8: E289           lsril #1, d1
10AA: D280           addrl d0, d1
10AC: E081           asril #0, d1
10AE: EE81           asril #7, d1
10B0: 3601           movew d1, d3
10B2: 7018           moveq #24, d0
10B4: C1C3           muls d3, d0
10B6: 45ED F714      lea -2284(a5), d2
10BA: D5C0           addal d0, a2
10BC: 45EA 047A      lea 1146(a2), d2
10C0: 4A2A 0008      tstb.ex 8(a2)
10C4: 584F           addqw #4, a7
10C6: 6606           bne 6
10C8: 4A2A 0009      tstb.ex 9(a2)
10CC: 6704           beq 4
10CE: 6000 025A      bra 602
10D2: 0C43 0008      cmpiw #8, d3
10D6: 6730           beq 48
10D8: 3003           movew d3, d0
10DA: 5240           addqw #1, d0
10DC: C1FC 0018      muls.ex #24, d0
10E0: 41ED F714      lea -2284(a5), d0
10E4: 41E8 048A      lea 1162(a0), d0
10E8: 1030 0800      moveb.ex 0(a0,d0.l), d0
10EC: 4880           extw d0
10EE: B043           cmpw d3, d0
10F0: 661A           bne 26
10F2: 3003           movew d3, d0
10F4: 5240           addqw #1, d0
10F6: C1FC 0018      muls.ex #24, d0
10FA: 41ED F714      lea -2284(a5), d0
10FE: 41E8 0482      lea 1154(a0), d0
1102: 4A30 0800      tstb.ex 0(a0,d0.l)
1106: 6704           beq 4
1108: 6000 0220      bra 544
110C: 4A2D F729      tstb.ex -2263(a5)
1110: 6700 0218      beq 536
1114: 532D F729      subqb.ex #1, -2263(a5)
1118: 522D F72A      addqb.ex #1, -2262(a5)
111C: 7C00           moveq #0, d6
111E: 0C2D 0005 F72E cmpib.ex #5, -2258(a5)
1124: 6542           bcs 66
1126: 7000           moveq #0, d0
1128: 102D F72E      moveb.ex -2258(a5), d0
112C: 5B40           subqw #5, d0
112E: 3200           movew d0, d1
1130: E241           asriw #1, d1
1132: E049           lsriw #0, d1
1134: EC49           lsriw #6, d1
1136: D240           addrw d0, d1
1138: E441           asriw #2, d1
113A: 5A41           addqw #5, d1
113C: 48C1           extl d1
113E: 2D41 FFFC      movel.mx d1, -4(a6)
1142: 42A7           clrl -(a7)
1144: 4E4F           trap #15
1146: A0C2           sysTrapSysRandom
1148: C1FC 0064      muls.ex #100, d0
114C: 2200           movel d0, d1
114E: E081           asril #0, d1
1150: EC81           asril #6, d1
1152: E089           lsril #0, d1
1154: E089           lsril #0, d1
1156: E289           lsril #1, d1
1158: D280           addrl d0, d1
115A: E081           asril #0, d1
115C: EE81           asril #7, d1
115E: B2AE FFFC      cmpl.ex -4(a6), d1
1162: 584F           addqw #4, a7
1164: 6C02           bge 2
1166: 7C02           moveq #2, d6
1168: 0C2D 0008 F72E cmpib.ex #8, -2258(a5)
116E: 6540           bcs 64
1170: 7000           moveq #0, d0
1172: 102D F72E      moveb.ex -2258(a5), d0
1176: 5140           subqw #0, d0
1178: 3200           movew d0, d1
117A: E049           lsriw #0, d1
117C: EE49           lsriw #7, d1
117E: D240           addrw d0, d1
1180: E241           asriw #1, d1
1182: 5641           addqw #3, d1
1184: 48C1           extl d1
1186: 2D41 FFFC      movel.mx d1, -4(a6)
118A: 42A7           clrl -(a7)
118C: 4E4F           trap #15
118E: A0C2           sysTrapSysRandom
1190: C1FC 0064      muls.ex #100, d0
1194: 2200           movel d0, d1
1196: E081           asril #0, d1
1198: EC81           asril #6, d1
119A: E089           lsril #0, d1
119C: E089           lsril #0, d1
119E: E289           lsril #1, d1
11A0: D280           addrl d0, d1
11A2: E081           asril #0, d1
11A4: EE81           asril #7, d1
11A6: B2AE FFFC      cmpl.ex -4(a6), d1
11AA: 584F           addqw #4, a7
11AC: 6C02           bge 2
11AE: 7C01           moveq #1, d6
11B0: 157C 0001 0008 moveb.emx #1, 8(a2)
11B6: 157C 0001 0009 moveb.emx #1, 9(a2)
11BC: 422A 000A      clrb.ex 10(a2)
11C0: 422A 000B      clrb.ex 11(a2)
11C4: 1546 000C      moveb.mx d6, 12(a2)
11C8: 7000           moveq #0, d0
11CA: 1006           moveb d6, d0
11CC: 6714           beq 20
11CE: 5340           subqw #1, d0
11D0: 676A           beq 106
11D2: 5340           subqw #1, d0
11D4: 6700 00B6      beq 182
11D8: 5340           subqw #1, d0
11DA: 6700 00E0      beq 224
11DE: 6000 010A      bra 266
11E2: 7821           moveq #33, d4
11E4: 7A0C           moveq #12, d5
11E6: 42A7           clrl -(a7)
11E8: 4E4F           trap #15
11EA: A0C2           sysTrapSysRandom
11EC: C1FC 000A      muls.ex #10, d0
11F0: 2200           movel d0, d1
11F2: E081           asril #0, d1
11F4: EC81           asril #6, d1
11F6: E089           lsril #0, d1
11F8: E089           lsril #0, d1
11FA: E289           lsril #1, d1
11FC: D280           addrl d0, d1
11FE: E081           asril #0, d1
1200: EE81           asril #7, d1
1202: 5281           addql #1, d1
1204: 1541 000E      moveb.mx d1, 14(a2)
1208: 7000           moveq #0, d0
120A: 102A 000E      moveb.ex 14(a2), d0
120E: 3203           movew d3, d1
1210: 5241           addqw #1, d1
1212: C3C0           muls d0, d1
1214: 1541 0011      moveb.mx d1, 17(a2)
1218: 7000           moveq #0, d0
121A: 102A 0011      moveb.ex 17(a2), d0
121E: 2F00           movel d0, -(a7)
1220: 486A 0013      pea.ex 19(a2)
1224: 4E4F           trap #15
1226: A0C9           sysTrapStrIToA
1228: 486A 0013      pea.ex 19(a2)
122C: 4E4F           trap #15
122E: A0C7           sysTrapStrLen
1230: 1540 0017      moveb.mx d0, 23(a2)
1234: 4FEF 0010      lea 16(a7), d7
1238: 6000 00B0      bra 176
123C: 7829           moveq #41, d4
123E: 7A0C           moveq #12, d5
1240: 42A7           clrl -(a7)
1242: 4E4F           trap #15
1244: A0C2           sysTrapSysRandom
1246: 48C0           extl d0
1248: D080           addrl d0, d0
124A: 2200           movel d0, d1
124C: E081           asril #0, d1
124E: EC81           asril #6, d1
1250: E089           lsril #0, d1
1252: E089           lsril #0, d1
1254: E289           lsril #1, d1
1256: D280           addrl d0, d1
1258: E081           asril #0, d1
125A: EE81           asril #7, d1
125C: 7009           moveq #9, d0
125E: D280           addrl d0, d1
1260: 1541 000E      moveb.mx d1, 14(a2)
1264: 157C 0096 0011 moveb.emx #150, 17(a2)
126A: 7000           moveq #0, d0
126C: 102A 0011      moveb.ex 17(a2), d0
1270: 2F00           movel d0, -(a7)
1272: 486A 0013      pea.ex 19(a2)
1276: 4E4F           trap #15
1278: A0C9           sysTrapStrIToA
127A: 486A 0013      pea.ex 19(a2)
127E: 4E4F           trap #15
1280: A0C7           sysTrapStrLen
1282: 1540 0017      moveb.mx d0, 23(a2)
1286: 4FEF 0010      lea 16(a7), d7
128A: 605E           bra 94
128C: 7821           moveq #33, d4
128E: 7A0B           moveq #11, d5
1290: 42A7           clrl -(a7)
1292: 4E4F           trap #15
1294: A0C2           sysTrapSysRandom
1296: C1FC 0003      muls.ex #3, d0
129A: 2200           movel d0, d1
129C: E081           asril #0, d1
129E: EC81           asril #6, d1
12A0: E089           lsril #0, d1
12A2: E089           lsril #0, d1
12A4: E289           lsril #1, d1
12A6: D280           addrl d0, d1
12A8: E081           asril #0, d1
12AA: EE81           asril #7, d1
12AC: 5881           addql #4, d1
12AE: 1541 000E      moveb.mx d1, 14(a2)
12B2: 157C 0064 0011 moveb.emx #100, 17(a2)
12B8: 584F           addqw #4, a7
12BA: 602E           bra 46
12BC: 7821           moveq #33, d4
12BE: 7A09           moveq #9, d5
12C0: 42A7           clrl -(a7)
12C2: 4E4F           trap #15
12C4: A0C2           sysTrapSysRandom
12C6: C1FC 0003      muls.ex #3, d0
12CA: 2200           movel d0, d1
12CC: E081           asril #0, d1
12CE: EC81           asril #6, d1
12D0: E089           lsril #0, d1
12D2: E089           lsril #0, d1
12D4: E289           lsril #1, d1
12D6: D280           addrl d0, d1
12D8: E081           asril #0, d1
12DA: EE81           asril #7, d1
12DC: 5881           addql #4, d1
12DE: 1541 000E      moveb.mx d1, 14(a2)
12E2: 157C 004B 0011 moveb.emx #75, 17(a2)
12E8: 584F           addqw #4, a7
12EA: 156A 000E 000F moveb.emx 14(a2), 15(a2)
12F0: 1543 0010      moveb.mx d3, 16(a2)
12F4: 4A07           tstb d7
12F6: 6608           bne 8
12F8: 3004           movew d4, d0
12FA: 4440           negw d0
12FC: 3480           movew d0, (a2)
12FE: 6004           bra 4
1300: 34BC 00A0      movew.ex #160, (a2)
1304: 700D           moveq #13, d0
1306: C1C3           muls d3, d0
1308: 0640 0015      addiw #21, d0
130C: 3540 0002      movew.mx d0, 2(a2)
1310: 3012           movew (a2), d0
1312: D044           addrw d4, d0
1314: 3540 0004      movew.mx d0, 4(a2)
1318: 302A 0002      movew.ex 2(a2), d0
131C: D045           addrw d5, d0
131E: 3540 0006      movew.mx d0, 6(a2)
1322: 1547 000D      moveb.mx d7, 13(a2)
1326: 422A 0012      clrb.ex 18(a2)
132A: 4CDF 04F8      movem (a7)+, <04f8>
132E: 4E5E           unlk a6
1330: 4E75           rts
1332: 8653           orrw (a3), d3
1334: 7562           dc.w #30050
1336: 4164           dc.w #16740
1338: 6400 0000      bcc 0
133C: 4E56 0000      link a6, #0
1340: 532D F72A      subqb.ex #1, -2262(a5)
1344: 7018           moveq #24, d0
1346: C1EE 0008      muls.ex 8(a6), d0
134A: 41ED F714      lea -2284(a5), d0
134E: D1C0           addal d0, a0
1350: 41E8 047A      lea 1146(a0), d0
1354: 4228 0008      clrb.ex 8(a0)
1358: 117C 0001 0009 moveb.emx #1, 9(a0)
135E: 4E5E           unlk a6
1360: 4E75           rts
1362: 8953           ormw d4, (a3)
1364: 7562           dc.w #30050
1366: 5265           addqw #1, -(a5)
1368: 6D6F           blt 111
136A: 7665           moveq #101, d3
136C: 0000           dc.w #0
136E: 4E56 0000      link a6, #0
1372: 48E7 1C20      movem <1c20>, -(a7)
1376: 3A2E 0008      movew.ex 8(a6), d5
137A: 382E 000A      movew.ex 10(a6), d4
137E: 7018           moveq #24, d0
1380: C1C4           muls d4, d0
1382: 45ED F714      lea -2284(a5), d2
1386: D5C0           addal d0, a2
1388: 45EA 047A      lea 1146(a2), d2
138C: 0C2A 0002 000C cmpib.ex #2, 12(a2)
1392: 656E           bcs 110
1394: 4A2A 000B      tstb.ex 11(a2)
1398: 6668           bne 104
139A: 157C 0001 000B moveb.emx #1, 11(a2)
13A0: 0C6A 0015 0002 cmpiw.ex #21, 2(a2)
13A6: 6C24           bge 36
13A8: 42A7           clrl -(a7)
13AA: 4E4F           trap #15
13AC: A0C2           sysTrapSysRandom
13AE: C1FC 0064      muls.ex #100, d0
13B2: 2200           movel d0, d1
13B4: E081           asril #0, d1
13B6: EC81           asril #6, d1
13B8: E089           lsril #0, d1
13BA: E089           lsril #0, d1
13BC: E289           lsril #1, d1
13BE: D280           addrl d0, d1
13C0: E081           asril #0, d1
13C2: EE81           asril #7, d1
13C4: 704B           moveq #75, d0
13C6: B280           cmpl d0, d1
13C8: 584F           addqw #4, a7
13CA: 6C5A           bge 90
13CC: 0C2A 0001 000D cmpib.ex #1, 13(a2)
13D2: 57C0           seq d0
13D4: 4400           negb d0
13D6: 4880           extw d0
13D8: 6704           beq 4
13DA: 7600           moveq #0, d3
13DC: 6002           bra 2
13DE: 7601           moveq #1, d3
13E0: 1543 000D      moveb.mx d3, 13(a2)
13E4: 157C 0009 000E moveb.emx #9, 14(a2)
13EA: 157C 0009 000F moveb.emx #9, 15(a2)
13F0: 422A 0012      clrb.ex 18(a2)
13F4: 0C6A 0015 0002 cmpiw.ex #21, 2(a2)
13FA: 6C2A           bge 42
13FC: 422A 0010      clrb.ex 16(a2)
1400: 6024           bra 36
1402: 4A2A 000A      tstb.ex 10(a2)
1406: 661E           bne 30
1408: 157C 0001 000A moveb.emx #1, 10(a2)
140E: 157C 0012 0012 moveb.emx #18, 18(a2)
1414: 7000           moveq #0, d0
1416: 102A 0011      moveb.ex 17(a2), d0
141A: 2F00           movel d0, -(a7)
141C: 4EBA F516      jsr.ex 32060(pc)
1420: 522D F72B      addqb.ex #1, -2261(a5)
1424: 584F           addqw #4, a7
1426: 3F05           movew d5, -(a7)
1428: 4EBA F6D6      jsr.ex 32520(pc)
142C: 544F           addqw #2, a7
142E: 4CDF 0438      movem (a7)+, <0438>
1432: 4E5E           unlk a6
1434: 4E75           rts
1436: 8A53           orrw (a3), d5
1438: 7562           dc.w #30050
143A: 4465           negw -(a5)
143C: 7374           dc.w #29556
143E: 726F           moveq #111, d1
1440: 7900           dc.w #30976
1442: 0000           dc.w #0
1444: 4E56 0000      link a6, #0
1448: 48E7 1E00      movem <1e00>, -(a7)
144C: 362E 0008      movew.ex 8(a6), d3
1450: 7801           moveq #1, d4
1452: 7A00           moveq #0, d5
1454: 7018           moveq #24, d0
1456: C1C3           muls d3, d0
1458: 41ED F714      lea -2284(a5), d0
145C: 41E8 047A      lea 1146(a0), d0
1460: 3030 0800      movew.ex 0(a0,d0.l), d0
1464: B06D FB76      cmpw.ex -1162(a5), d0
1468: 5CC0           sge d0
146A: 4400           negb d0
146C: 4880           extw d0
146E: 671E           beq 30
1470: 7018           moveq #24, d0
1472: C1C3           muls d3, d0
1474: 41ED F714      lea -2284(a5), d0
1478: 41E8 047A      lea 1146(a0), d0
147C: 3030 0800      movew.ex 0(a0,d0.l), d0
1480: B06D FB7A      cmpw.ex -1158(a5), d0
1484: 5DC0           slt d0
1486: 4400           negb d0
1488: 4880           extw d0
148A: 6702           beq 2
148C: 7A01           moveq #1, d5
148E: 4A45           tstw d5
1490: 6642           bne 66
1492: 7C00           moveq #0, d6
1494: 7018           moveq #24, d0
1496: C1C3           muls d3, d0
1498: 41ED F714      lea -2284(a5), d0
149C: 41E8 047E      lea 1150(a0), d0
14A0: 3030 0800      movew.ex 0(a0,d0.l), d0
14A4: B06D FB7A      cmpw.ex -1158(a5), d0
14A8: 5DC0           slt d0
14AA: 4400           negb d0
14AC: 4880           extw d0
14AE: 671E           beq 30
14B0: 7018           moveq #24, d0
14B2: C1C3           muls d3, d0
14B4: 41ED F714      lea -2284(a5), d0
14B8: 41E8 047E      lea 1150(a0), d0
14BC: 3030 0800      movew.ex 0(a0,d0.l), d0
14C0: B06D FB76      cmpw.ex -1162(a5), d0
14C4: 5CC0           sge d0
14C6: 4400           negb d0
14C8: 4880           extw d0
14CA: 6702           beq 2
14CC: 7C01           moveq #1, d6
14CE: 4A46           tstw d6
14D0: 6602           bne 2
14D2: 7800           moveq #0, d4
14D4: 1004           moveb d4, d0
14D6: 4CDF 0078      movem (a7)+, <0078>
14DA: 4E5E           unlk a6
14DC: 4E75           rts
14DE: 8E49           dc.w #36425
14E0: 7353           dc.w #29523
14E2: 7562           dc.w #30050
14E4: 556E 6465      subqw.ex #2, 25701(a6)
14E8: 7253           moveq #83, d1
14EA: 6869           bvc 105
14EC: 7000           moveq #0, d0
14EE: 0000           dc.w #0
14F0: 4E56 FFF8      link a6, #-8
14F4: 48E7 1800      movem <1800>, -(a7)
14F8: 7800           moveq #0, d4
14FA: 607A           bra 122
14FC: 7600           moveq #0, d3
14FE: 606E           bra 110
1500: 0C44 0001      cmpiw #1, d4
1504: 6614           bne 20
1506: 3003           movew d3, d0
1508: E548           lsliw #2, d0
150A: 0640 0035      addiw #53, d0
150E: 3D40 FFF8      movew.mx d0, -8(a6)
1512: 3D7C 0006 FFFA movew.emx #6, -6(a6)
1518: 6012           bra 18
151A: 3003           movew d3, d0
151C: E548           lsliw #2, d0
151E: 7277           moveq #119, d1
1520: 9240           subrw d0, d1
1522: 3D41 FFF8      movew.mx d1, -8(a6)
1526: 3D7C 0006 FFFA movew.emx #6, -6(a6)
152C: 41ED F714      lea -2284(a5), d0
1530: 41E8 089A      lea 2202(a0), d0
1534: 7000           moveq #0, d0
1536: 1030 4000      moveb.ex 0(a0,d4.w), d0
153A: B043           cmpw d3, d0
153C: 6F16           ble 22
153E: 4227           clrb -(a7)
1540: 3F2E FFFA      movew.ex -6(a6), -(a7)
1544: 3F2E FFF8      movew.ex -8(a6), -(a7)
1548: 3F3C 000B      movew.ex #11, -(a7)
154C: 4EBA F2DE      jsr.ex 31796(pc)
1550: 504F           addqw #0, a7
1552: 6018           bra 24
1554: 3D7C 0002 FFFC movew.emx #2, -4(a6)
155A: 3D7C 0002 FFFE movew.emx #2, -2(a6)
1560: 4267           clrw -(a7)
1562: 486E FFF8      pea.ex -8(a6)
1566: 4E4F           trap #15
1568: A219           sysTrapWinEraseRectangle
156A: 5C4F           addqw #6, a7
156C: 5243           addqw #1, d3
156E: 0C43 0005      cmpiw #5, d3
1572: 6D8C           blt -116
1574: 5244           addqw #1, d4
1576: 0C44 0001      cmpiw #1, d4
157A: 6F80           ble -128
157C: 4CDF 0018      movem (a7)+, <0018>
1580: 4E5E           unlk a6
1582: 4E75           rts
1584: 9347           subxrw d7, d1
1586: 616D           bsr 109
1588: 6544           bcs 68
158A: 7261           moveq #97, d1
158C: 7743           dc.w #30531
158E: 6861           bvc 97
1590: 7267           moveq #103, d1
1592: 6547           bcs 71
1594: 6175           bsr 117
1596: 6765           beq 101
1598: 0000           dc.w #0
159A: 4E56 FFFA      link a6, #-6
159E: 48E7 1C00      movem <1c00>, -(a7)
15A2: 2A2E 0008      movel.ex 8(a6), d5
15A6: 1F3C 0001      moveb.ex #1, -(a7)
15AA: 4E4F           trap #15
15AC: A164           sysTrapFntSetFont
15AE: 1800           moveb d0, d4
15B0: 4A85           tstl d5
15B2: 544F           addqw #2, a7
15B4: 6F0E           ble 14
15B6: 2F05           movel d5, -(a7)
15B8: 486E FFFA      pea.ex -6(a6)
15BC: 4E4F           trap #15
15BE: A0C9           sysTrapStrIToA
15C0: 504F           addqw #0, a7
15C2: 6030           bra 48
15C4: 1F3C 0080      moveb.ex #128, -(a7)
15C8: 4878 0005      pea.ex (0005).w
15CC: 486E FFFA      pea.ex -6(a6)
15D0: 4E4F           trap #15
15D2: A027           sysTrapMemSet
15D4: 3F3C 0002      movew.ex #2, -(a7)
15D8: 2F3C 0005 004A movel.exl #327754, -(a7)
15DE: 486E FFFA      pea.ex -6(a6)
15E2: 4E4F           trap #15
15E4: A220           sysTrapWinDrawChars
15E6: 1D7C 0030 FFFA moveb.emx #48, -6(a6)
15EC: 422E FFFB      clrb.ex -5(a6)
15F0: 4FEF 0014      lea 20(a7), d7
15F4: 486E FFFA      pea.ex -6(a6)
15F8: 4E4F           trap #15
15FA: A0C7           sysTrapStrLen
15FC: 3600           movew d0, d3
15FE: 3F3C 0002      movew.ex #2, -(a7)
1602: 7005           moveq #5, d0
1604: C1C3           muls d3, d0
1606: 7218           moveq #24, d1
1608: 9240           subrw d0, d1
160A: 3001           movew d1, d0
160C: E048           lsriw #0, d0
160E: EE48           lsriw #7, d0
1610: D041           addrw d1, d0
1612: E240           asriw #1, d0
1614: 0640 004A      addiw #74, d0
1618: 3F00           movew d0, -(a7)
161A: 3F03           movew d3, -(a7)
161C: 486E FFFA      pea.ex -6(a6)
1620: 4E4F           trap #15
1622: A220           sysTrapWinDrawChars
1624: 1F04           moveb d4, -(a7)
1626: 4E4F           trap #15
1628: A164           sysTrapFntSetFont
162A: 4FEF 0010      lea 16(a7), d7
162E: 4CDF 0038      movem (a7)+, <0038>
1632: 4E5E           unlk a6
1634: 4E75           rts
1636: 9247           subrw d7, d1
1638: 616D           bsr 109
163A: 6544           bcs 68
163C: 7261           moveq #97, d1
163E: 7753           dc.w #30547
1640: 636F           bls 111
1642: 7265           moveq #101, d1
1644: 4761           dc.w #18273
1646: 7567           dc.w #30055
1648: 6500 0000      bcs 0
164C: 4E56 FFFC      link a6, #-4
1650: 48E7 1C00      movem <1c00>, -(a7)
1654: 3A2E 0008      movew.ex 8(a6), d5
1658: 1F3C 0001      moveb.ex #1, -(a7)
165C: 4E4F           trap #15
165E: A164           sysTrapFntSetFont
1660: 1600           moveb d0, d3
1662: 4AAD FB70      tstl.ex -1168(a5)
1666: 544F           addqw #2, a7
1668: 6626           bne 38
166A: 1F3C 0080      moveb.ex #128, -(a7)
166E: 4878 0002      pea.ex (0002).w
1672: 486E FFFC      pea.ex -4(a6)
1676: 4E4F           trap #15
1678: A027           sysTrapMemSet
167A: 3F3C 0002      movew.ex #2, -(a7)
167E: 2F3C 0002 0094 movel.exl #131220, -(a7)
1684: 486E FFFC      pea.ex -4(a6)
1688: 4E4F           trap #15
168A: A220           sysTrapWinDrawChars
168C: 4FEF 0014      lea 20(a7), d7
1690: 3045           movew d5, a0
1692: 2F08           movel a0, -(a7)
1694: 486E FFFC      pea.ex -4(a6)
1698: 4E4F           trap #15
169A: A0C9           sysTrapStrIToA
169C: 486E FFFC      pea.ex -4(a6)
16A0: 4E4F           trap #15
16A2: A0C7           sysTrapStrLen
16A4: 3800           movew d0, d4
16A6: 2F3C 0094 0002 movel.exl #9699330, -(a7)
16AC: 3F04           movew d4, -(a7)
16AE: 486E FFFC      pea.ex -4(a6)
16B2: 4E4F           trap #15
16B4: A220           sysTrapWinDrawChars
16B6: 1F03           moveb d3, -(a7)
16B8: 4E4F           trap #15
16BA: A164           sysTrapFntSetFont
16BC: 4FEF 0018      lea 24(a7), d7
16C0: 4CDF 0038      movem (a7)+, <0038>
16C4: 4E5E           unlk a6
16C6: 4E75           rts
16C8: 9247           subrw d7, d1
16CA: 616D           bsr 109
16CC: 6544           bcs 68
16CE: 7261           moveq #97, d1
16D0: 774C           dc.w #30540
16D2: 6576           bcs 118
16D4: 656C           bcs 108
16D6: 4761           dc.w #18273
16D8: 7567           dc.w #30055
16DA: 6500 0000      bcs 0
16DE: 4E56 0000      link a6, #0
16E2: 48E7 1C00      movem <1c00>, -(a7)
16E6: 3F3C 000F      movew.ex #15, -(a7)
16EA: 2F3C 03E8 0000 movel.exl #65536000, -(a7)
16F0: 4EBA F066      jsr.ex 31584(pc)
16F4: 422D FFB8      clrb.ex -72(a5)
16F8: 4A2D F740      tstb.ex -2240(a5)
16FC: 5C4F           addqw #6, a7
16FE: 6742           beq 66
1700: 7000           moveq #0, d0
1702: 102D F72C      moveb.ex -2260(a5), d0
1706: 7200           moveq #0, d1
1708: 122D F744      moveb.ex -2236(a5), d1
170C: D240           addrw d0, d1
170E: 0C41 0002      cmpiw #2, d1
1712: 5DC1           slt d1
1714: 4401           negb d1
1716: 4881           extw d1
1718: 6710           beq 16
171A: 7000           moveq #0, d0
171C: 102D F72C      moveb.ex -2260(a5), d0
1720: 7800           moveq #0, d4
1722: 182D F744      moveb.ex -2236(a5), d4
1726: D840           addrw d0, d4
1728: 6002           bra 2
172A: 7802           moveq #2, d4
172C: 4227           clrb -(a7)
172E: 700F           moveq #15, d0
1730: D06D F73A      addrw.ex -2246(a5), d0
1734: 3F00           movew d0, -(a7)
1736: 3F2D F738      movew.ex -2248(a5), -(a7)
173A: 3F04           movew d4, -(a7)
173C: 4EBA F0EE      jsr.ex 31796(pc)
1740: 504F           addqw #0, a7
1742: 7608           moveq #8, d3
1744: 6070           bra 112
1746: 7018           moveq #24, d0
1748: C1C3           muls d3, d0
174A: 41ED F714      lea -2284(a5), d0
174E: 41E8 0044      lea 68(a0), d0
1752: 4A30 0800      tstb.ex 0(a0,d0.l)
1756: 675C           beq 92
1758: 7018           moveq #24, d0
175A: C1C3           muls d3, d0
175C: 41ED F714      lea -2284(a5), d0
1760: 41E8 047C      lea 1148(a0), d0
1764: 0C70 0015 0800 cmpiw.ex #21, 0(a0,d0.l)
176A: 5DC1           slt d1
176C: 4401           negb d1
176E: 4881           extw d1
1770: 6704           beq 4
1772: 7A04           moveq #4, d5
1774: 6002           bra 2
1776: 7A00           moveq #0, d5
1778: 1F05           moveb d5, -(a7)
177A: 7018           moveq #24, d0
177C: C1C3           muls d3, d0
177E: 41ED F714      lea -2284(a5), d0
1782: 720F           moveq #15, d1
1784: D270 083E      addrw.ex 62(a0,d0.l), d1
1788: 3F01           movew d1, -(a7)
178A: 7018           moveq #24, d0
178C: C1C3           muls d3, d0
178E: 3F30 083C      movew.ex 60(a0,d0.l), -(a7)
1792: 7018           moveq #24, d0
1794: C1C3           muls d3, d0
1796: 7200           moveq #0, d1
1798: 1230 0849      moveb.ex 73(a0,d0.l), d1
179C: 7018           moveq #24, d0
179E: C1C3           muls d3, d0
17A0: 7400           moveq #0, d2
17A2: 1430 0848      moveb.ex 72(a0,d0.l), d2
17A6: D442           addrw d2, d2
17A8: 5642           addqw #3, d2
17AA: D441           addrw d1, d2
17AC: 3F02           movew d2, -(a7)
17AE: 4EBA F07C      jsr.ex 31796(pc)
17B2: 504F           addqw #0, a7
17B4: 5343           subqw #1, d3
17B6: 4A43           tstw d3
17B8: 6C8C           bge -116
17BA: 7600           moveq #0, d3
17BC: 162D FB74      moveb.ex -1164(a5), d3
17C0: 5343           subqw #1, d3
17C2: 6052           bra 82
17C4: 700E           moveq #14, d0
17C6: C1C3           muls d3, d0
17C8: 41ED F714      lea -2284(a5), d0
17CC: 41E8 011C      lea 284(a0), d0
17D0: 4A30 0800      tstb.ex 0(a0,d0.l)
17D4: 673E           beq 62
17D6: 4227           clrb -(a7)
17D8: 700E           moveq #14, d0
17DA: C1C3           muls d3, d0
17DC: 41ED F714      lea -2284(a5), d0
17E0: D1C0           addal d0, a0
17E2: 700F           moveq #15, d0
17E4: D068 0116      addrw.ex 278(a0), d0
17E8: 3F00           movew d0, -(a7)
17EA: 700E           moveq #14, d0
17EC: C1C3           muls d3, d0
17EE: 41ED F714      lea -2284(a5), d0
17F2: D1C0           addal d0, a0
17F4: 3F28 0114      movew.ex 276(a0), -(a7)
17F8: 700E           moveq #14, d0
17FA: C1C3           muls d3, d0
17FC: 41ED F714      lea -2284(a5), d0
1800: D1C0           addal d0, a0
1802: 7000           moveq #0, d0
1804: 1028 011F      moveb.ex 287(a0), d0
1808: 0640 000B      addiw #11, d0
180C: 3F00           movew d0, -(a7)
180E: 4EBA F01C      jsr.ex 31796(pc)
1812: 504F           addqw #0, a7
1814: 5343           subqw #1, d3
1816: 4A43           tstw d3
1818: 6CAA           bge -86
181A: 7600           moveq #0, d3
181C: 162D FB75      moveb.ex -1163(a5), d3
1820: 5343           subqw #1, d3
1822: 6054           bra 84
1824: 700E           moveq #14, d0
1826: C1C3           muls d3, d0
1828: 41ED F714      lea -2284(a5), d0
182C: 41E8 0234      lea 564(a0), d0
1830: 4A30 0800      tstb.ex 0(a0,d0.l)
1834: 6740           beq 64
1836: 1F3C 0004      moveb.ex #4, -(a7)
183A: 700E           moveq #14, d0
183C: C1C3           muls d3, d0
183E: 41ED F714      lea -2284(a5), d0
1842: D1C0           addal d0, a0
1844: 700F           moveq #15, d0
1846: D068 022E      addrw.ex 558(a0), d0
184A: 3F00           movew d0, -(a7)
184C: 700E           moveq #14, d0
184E: C1C3           muls d3, d0
1850: 41ED F714      lea -2284(a5), d0
1854: D1C0           addal d0, a0
1856: 3F28 022C      movew.ex 556(a0), -(a7)
185A: 700E           moveq #14, d0
185C: C1C3           muls d3, d0
185E: 41ED F714      lea -2284(a5), d0
1862: D1C0           addal d0, a0
1864: 7000           moveq #0, d0
1866: 1028 0237      moveb.ex 567(a0), d0
186A: 0640 000B      addiw #11, d0
186E: 3F00           movew d0, -(a7)
1870: 4EBA EFBA      jsr.ex 31796(pc)
1874: 504F           addqw #0, a7
1876: 5343           subqw #1, d3
1878: 4A43           tstw d3
187A: 6CA8           bge -88
187C: 2F2D F732      movel.ex -2254(a5), -(a7)
1880: 4EBA FD18      jsr.ex -30302(pc)
1884: 4EBA FC6A      jsr.ex -30472(pc)
1888: 7000           moveq #0, d0
188A: 102D F72E      moveb.ex -2258(a5), d0
188E: 5240           addqw #1, d0
1890: 3F00           movew d0, -(a7)
1892: 4EBA FDB8      jsr.ex -30124(pc)
1896: 5C4F           addqw #6, a7
1898: 4CDF 0038      movem (a7)+, <0038>
189C: 4E5E           unlk a6
189E: 4E75           rts
18A0: 8D47           dc.w #36167
18A2: 616D           bsr 109
18A4: 6553           bcs 83
18A6: 7461           moveq #97, d2
18A8: 7465           moveq #101, d2
18AA: 4472 6177      negw.ex 119(a2,d6.w)
18AE: 0000           dc.w #0
18B0: 4E56 FFEC      link a6, #-20
18B4: 48E7 1F38      movem <1f38>, -(a7)
18B8: 1F3C 0001      moveb.ex #1, -(a7)
18BC: 4E4F           trap #15
18BE: A164           sysTrapFntSetFont
18C0: 1D40 FFEC      moveb.mx d0, -20(a6)
18C4: 4A2D FFB8      tstb.ex -72(a5)
18C8: 544F           addqw #2, a7
18CA: 6712           beq 18
18CC: 4227           clrb -(a7)
18CE: 3F3C 001A      movew.ex #26, -(a7)
18D2: 2F3C 0012 0000 movel.exl #1179648, -(a7)
18D8: 4EBA EF52      jsr.ex 31796(pc)
18DC: 504F           addqw #0, a7
18DE: 41ED F714      lea -2284(a5), d0
18E2: 41E8 0024      lea 36(a0), d0
18E6: 2E08           movel a0, d7
18E8: 47ED F714      lea -2284(a5), d3
18EC: 47EB 0462      lea 1122(a3), d3
18F0: 2047           movel d7, a0
18F2: 4A28 0008      tstb.ex 8(a0)
18F6: 6738           beq 56
18F8: 4A2B 0009      tstb.ex 9(a3)
18FC: 6732           beq 50
18FE: 3D50 FFF8      movew.mx (a0), -8(a6)
1902: 700F           moveq #15, d0
1904: D068 0002      addrw.ex 2(a0), d0
1908: 3D40 FFFA      movew.mx d0, -6(a6)
190C: 2247           movel d7, a1
190E: 3029 0004      movew.ex 4(a1), d0
1912: 9050           subrw (a0), d0
1914: 3D40 FFFC      movew.mx d0, -4(a6)
1918: 3029 0006      movew.ex 6(a1), d0
191C: 9068 0002      subrw.ex 2(a0), d0
1920: 3D40 FFFE      movew.mx d0, -2(a6)
1924: 4267           clrw -(a7)
1926: 486E FFF8      pea.ex -8(a6)
192A: 4E4F           trap #15
192C: A219           sysTrapWinEraseRectangle
192E: 5C4F           addqw #6, a7
1930: 4A2B 0008      tstb.ex 8(a3)
1934: 6706           beq 6
1936: 4A2B 0009      tstb.ex 9(a3)
193A: 6608           bne 8
193C: 4A2D FFB8      tstb.ex -72(a5)
1940: 6700 0096      beq 150
1944: 4A2D F72D      tstb.ex -2259(a5)
1948: 672E           beq 46
194A: 3D53 FFF8      movew.mx (a3), -8(a6)
194E: 700F           moveq #15, d0
1950: D06B 0002      addrw.ex 2(a3), d0
1954: 3D40 FFFA      movew.mx d0, -6(a6)
1958: 302B 0004      movew.ex 4(a3), d0
195C: 9053           subrw (a3), d0
195E: 3D40 FFFC      movew.mx d0, -4(a6)
1962: 302B 0006      movew.ex 6(a3), d0
1966: 906B 0002      subrw.ex 2(a3), d0
196A: 3D40 FFFE      movew.mx d0, -2(a6)
196E: 486E FFF8      pea.ex -8(a6)
1972: 4E4F           trap #15
1974: A210           sysTrapWinSetClip
1976: 584F           addqw #4, a7
1978: 7000           moveq #0, d0
197A: 102D F72C      moveb.ex -2260(a5), d0
197E: 7200           moveq #0, d1
1980: 122B 000C      moveb.ex 12(a3), d1
1984: D240           addrw d0, d1
1986: 0C41 0002      cmpiw #2, d1
198A: 5DC1           slt d1
198C: 4401           negb d1
198E: 4881           extw d1
1990: 6714           beq 20
1992: 7000           moveq #0, d0
1994: 102D F72C      moveb.ex -2260(a5), d0
1998: 7200           moveq #0, d1
199A: 122B 000C      moveb.ex 12(a3), d1
199E: D240           addrw d0, d1
19A0: 3D41 FFF2      movew.mx d1, -14(a6)
19A4: 6006           bra 6
19A6: 3D7C 0002 FFF2 movew.emx #2, -14(a6)
19AC: 4227           clrb -(a7)
19AE: 7000           moveq #0, d0
19B0: 102D F72D      moveb.ex -2259(a5), d0
19B4: 720F           moveq #15, d1
19B6: D26B 0002      addrw.ex 2(a3), d1
19BA: D240           addrw d0, d1
19BC: 3F01           movew d1, -(a7)
19BE: 3F13           movew (a3), -(a7)
19C0: 3F2E FFF2      movew.ex -14(a6), -(a7)
19C4: 4EBA EE66      jsr.ex 31796(pc)
19C8: 4A2D F72D      tstb.ex -2259(a5)
19CC: 504F           addqw #0, a7
19CE: 6704           beq 4
19D0: 4E4F           trap #15
19D2: A211           sysTrapWinResetClip
19D4: 422D FFB8      clrb.ex -72(a5)
19D8: 7808           moveq #8, d4
19DA: 6000 010A      bra 266
19DE: 7018           moveq #24, d0
19E0: C1C4           muls d4, d0
19E2: 41ED F714      lea -2284(a5), d0
19E6: D1C0           addal d0, a0
19E8: 41E8 003C      lea 60(a0), d0
19EC: 2A08           movel a0, d5
19EE: 7018           moveq #24, d0
19F0: C1C4           muls d4, d0
19F2: 45ED F714      lea -2284(a5), d2
19F6: D5C0           addal d0, a2
19F8: 45EA 047A      lea 1146(a2), d2
19FC: 2045           movel d5, a0
19FE: 4A28 0008      tstb.ex 8(a0)
1A02: 6738           beq 56
1A04: 4A2A 0009      tstb.ex 9(a2)
1A08: 6732           beq 50
1A0A: 3D50 FFF8      movew.mx (a0), -8(a6)
1A0E: 700F           moveq #15, d0
1A10: D068 0002      addrw.ex 2(a0), d0
1A14: 3D40 FFFA      movew.mx d0, -6(a6)
1A18: 2245           movel d5, a1
1A1A: 3029 0004      movew.ex 4(a1), d0
1A1E: 9050           subrw (a0), d0
1A20: 3D40 FFFC      movew.mx d0, -4(a6)
1A24: 3029 0006      movew.ex 6(a1), d0
1A28: 9068 0002      subrw.ex 2(a0), d0
1A2C: 3D40 FFFE      movew.mx d0, -2(a6)
1A30: 4267           clrw -(a7)
1A32: 486E FFF8      pea.ex -8(a6)
1A36: 4E4F           trap #15
1A38: A219           sysTrapWinEraseRectangle
1A3A: 5C4F           addqw #6, a7
1A3C: 4A2A 0008      tstb.ex 8(a2)
1A40: 6700 00A2      beq 162
1A44: 4A2A 0009      tstb.ex 9(a2)
1A48: 6700 009A      beq 154
1A4C: 0C6A 0015 0002 cmpiw.ex #21, 2(a2)
1A52: 5DC0           slt d0
1A54: 4400           negb d0
1A56: 4880           extw d0
1A58: 6708           beq 8
1A5A: 1D7C 0004 FFF1 moveb.emx #4, -15(a6)
1A60: 6004           bra 4
1A62: 422E FFF1      clrb.ex -15(a6)
1A66: 1F2E FFF1      moveb.ex -15(a6), -(a7)
1A6A: 700F           moveq #15, d0
1A6C: D06A 0002      addrw.ex 2(a2), d0
1A70: 3F00           movew d0, -(a7)
1A72: 3F12           movew (a2), -(a7)
1A74: 7000           moveq #0, d0
1A76: 102A 000D      moveb.ex 13(a2), d0
1A7A: 7200           moveq #0, d1
1A7C: 122A 000C      moveb.ex 12(a2), d1
1A80: D241           addrw d1, d1
1A82: 5641           addqw #3, d1
1A84: D240           addrw d0, d1
1A86: 3F01           movew d1, -(a7)
1A88: 4EBA EDA2      jsr.ex 31796(pc)
1A8C: 0C2A 0001 000C cmpib.ex #1, 12(a2)
1A92: 504F           addqw #0, a7
1A94: 624E           bhi 78
1A96: 2045           movel d5, a0
1A98: 0C28 0001 000D cmpib.ex #1, 13(a0)
1A9E: 57C0           seq d0
1AA0: 4400           negb d0
1AA2: 4880           extw d0
1AA4: 6708           beq 8
1AA6: 3D7C 0007 FFEE movew.emx #7, -18(a6)
1AAC: 6012           bra 18
1AAE: 7000           moveq #0, d0
1AB0: 102A 0017      moveb.ex 23(a2), d0
1AB4: C1FC 0005      muls.ex #5, d0
1AB8: 7219           moveq #25, d1
1ABA: 9240           subrw d0, d1
1ABC: 3D41 FFEE      movew.mx d1, -18(a6)
1AC0: 302A 0006      movew.ex 6(a2), d0
1AC4: 5A40           addqw #5, d0
1AC6: 3F00           movew d0, -(a7)
1AC8: 3012           movew (a2), d0
1ACA: D06E FFEE      addrw.ex -18(a6), d0
1ACE: 3F00           movew d0, -(a7)
1AD0: 7000           moveq #0, d0
1AD2: 102A 0017      moveb.ex 23(a2), d0
1AD6: 3F00           movew d0, -(a7)
1AD8: 486A 0013      pea.ex 19(a2)
1ADC: 4E4F           trap #15
1ADE: A221           sysTrapWinEraseChars
1AE0: 4FEF 000A      lea 10(a7), d7
1AE4: 5344           subqw #1, d4
1AE6: 4A44           tstw d4
1AE8: 6C00 FEF4      bge 65268
1AEC: 2045           movel d5, a0
1AEE: 4A28 0008      tstb.ex 8(a0)
1AF2: 6714           beq 20
1AF4: 4A2A 0009      tstb.ex 9(a2)
1AF8: 670E           beq 14
1AFA: 0C68 000E 0002 cmpiw.ex #14, 2(a0)
1B00: 6C06           bge 6
1B02: 1B7C 0001 FFB8 moveb.emx #1, -72(a5)
1B08: 7800           moveq #0, d4
1B0A: 182D FB74      moveb.ex -1164(a5), d4
1B0E: 5344           subqw #1, d4
1B10: 6000 00F8      bra 248
1B14: 700E           moveq #14, d0
1B16: C1C4           muls d4, d0
1B18: 49ED F714      lea -2284(a5), d4
1B1C: D9C0           addal d0, a4
1B1E: 49EC 0114      lea 276(a4), d4
1B22: 700E           moveq #14, d0
1B24: C1C4           muls d4, d0
1B26: 41ED F714      lea -2284(a5), d0
1B2A: D1C0           addal d0, a0
1B2C: 41E8 0552      lea 1362(a0), d0
1B30: 2608           movel a0, d3
1B32: 4A2C 0008      tstb.ex 8(a4)
1B36: 6766           beq 102
1B38: 2043           movel d3, a0
1B3A: 4A28 0009      tstb.ex 9(a0)
1B3E: 675E           beq 94
1B40: 4A2C 000A      tstb.ex 10(a4)
1B44: 6640           bne 64
1B46: 0C6C 000E 0002 cmpiw.ex #14, 2(a4)
1B4C: 6606           bne 6
1B4E: 1B7C 0001 FFB8 moveb.emx #1, -72(a5)
1B54: 3D54 FFF8      movew.mx (a4), -8(a6)
1B58: 700F           moveq #15, d0
1B5A: D06C 0002      addrw.ex 2(a4), d0
1B5E: 3D40 FFFA      movew.mx d0, -6(a6)
1B62: 302C 0004      movew.ex 4(a4), d0
1B66: 9054           subrw (a4), d0
1B68: 3D40 FFFC      movew.mx d0, -4(a6)
1B6C: 302C 0006      movew.ex 6(a4), d0
1B70: 906C 0002      subrw.ex 2(a4), d0
1B74: 3D40 FFFE      movew.mx d0, -2(a6)
1B78: 4267           clrw -(a7)
1B7A: 486E FFF8      pea.ex -8(a6)
1B7E: 4E4F           trap #15
1B80: A219           sysTrapWinEraseRectangle
1B82: 5C4F           addqw #6, a7
1B84: 6018           bra 24
1B86: 1F3C 0002      moveb.ex #2, -(a7)
1B8A: 700F           moveq #15, d0
1B8C: D06C 0002      addrw.ex 2(a4), d0
1B90: 3F00           movew d0, -(a7)
1B92: 3F14           movew (a4), -(a7)
1B94: 3F3C 0011      movew.ex #17, -(a7)
1B98: 4EBA EC92      jsr.ex 31796(pc)
1B9C: 504F           addqw #0, a7
1B9E: 2043           movel d3, a0
1BA0: 4A28 0008      tstb.ex 8(a0)
1BA4: 6762           beq 98
1BA6: 2043           movel d3, a0
1BA8: 4A28 0009      tstb.ex 9(a0)
1BAC: 675A           beq 90
1BAE: 2043           movel d3, a0
1BB0: 4A28 000A      tstb.ex 10(a0)
1BB4: 6620           bne 32
1BB6: 4227           clrb -(a7)
1BB8: 700F           moveq #15, d0
1BBA: D068 0002      addrw.ex 2(a0), d0
1BBE: 3F00           movew d0, -(a7)
1BC0: 3F10           movew (a0), -(a7)
1BC2: 7000           moveq #0, d0
1BC4: 1028 000B      moveb.ex 11(a0), d0
1BC8: 0640 000B      addiw #11, d0
1BCC: 3F00           movew d0, -(a7)
1BCE: 4EBA EC5C      jsr.ex 31796(pc)
1BD2: 504F           addqw #0, a7
1BD4: 6032           bra 50
1BD6: 1F3C 0002      moveb.ex #2, -(a7)
1BDA: 700F           moveq #15, d0
1BDC: D068 0002      addrw.ex 2(a0), d0
1BE0: 3F00           movew d0, -(a7)
1BE2: 3F10           movew (a0), -(a7)
1BE4: 3F3C 0011      movew.ex #17, -(a7)
1BE8: 4EBA EC42      jsr.ex 31796(pc)
1BEC: 1F3C 0004      moveb.ex #4, -(a7)
1BF0: 2043           movel d3, a0
1BF2: 700F           moveq #15, d0
1BF4: D068 0002      addrw.ex 2(a0), d0
1BF8: 3F00           movew d0, -(a7)
1BFA: 3F10           movew (a0), -(a7)
1BFC: 3F3C 0010      movew.ex #16, -(a7)
1C00: 4EBA EC2A      jsr.ex 31796(pc)
1C04: 4FEF 0010      lea 16(a7), d7
1C08: 5344           subqw #1, d4
1C0A: 4A44           tstw d4
1C0C: 6C00 FF06      bge 65286
1C10: 7800           moveq #0, d4
1C12: 182D FB75      moveb.ex -1163(a5), d4
1C16: 5344           subqw #1, d4
1C18: 6000 00D2      bra 210
1C1C: 700E           moveq #14, d0
1C1E: C1C4           muls d4, d0
1C20: 41ED F714      lea -2284(a5), d0
1C24: D1C0           addal d0, a0
1C26: 41E8 022C      lea 556(a0), d0
1C2A: 2C08           movel a0, d6
1C2C: 700E           moveq #14, d0
1C2E: C1C4           muls d4, d0
1C30: 41ED F714      lea -2284(a5), d0
1C34: D1C0           addal d0, a0
1C36: 41E8 066A      lea 1642(a0), d0
1C3A: 2D48 FFF4      movel.mx a0, -12(a6)
1C3E: 2046           movel d6, a0
1C40: 4A28 0008      tstb.ex 8(a0)
1C44: 674E           beq 78
1C46: 206E FFF4      movel.ex -12(a6), a0
1C4A: 4A28 0009      tstb.ex 9(a0)
1C4E: 6744           beq 68
1C50: 2046           movel d6, a0
1C52: 0C68 000E 0002 cmpiw.ex #14, 2(a0)
1C58: 6C06           bge 6
1C5A: 1B7C 0001 FFB8 moveb.emx #1, -72(a5)
1C60: 2046           movel d6, a0
1C62: 3D50 FFF8      movew.mx (a0), -8(a6)
1C66: 700F           moveq #15, d0
1C68: D068 0002      addrw.ex 2(a0), d0
1C6C: 3D40 FFFA      movew.mx d0, -6(a6)
1C70: 2246           movel d6, a1
1C72: 3029 0004      movew.ex 4(a1), d0
1C76: 9050           subrw (a0), d0
1C78: 3D40 FFFC      movew.mx d0, -4(a6)
1C7C: 3029 0006      movew.ex 6(a1), d0
1C80: 9068 0002      subrw.ex 2(a0), d0
1C84: 3D40 FFFE      movew.mx d0, -2(a6)
1C88: 4267           clrw -(a7)
1C8A: 486E FFF8      pea.ex -8(a6)
1C8E: 4E4F           trap #15
1C90: A219           sysTrapWinEraseRectangle
1C92: 5C4F           addqw #6, a7
1C94: 206E FFF4      movel.ex -12(a6), a0
1C98: 4A28 0008      tstb.ex 8(a0)
1C9C: 674C           beq 76
1C9E: 2048           movel a0, a0
1CA0: 4A28 0009      tstb.ex 9(a0)
1CA4: 6744           beq 68
1CA6: 206E FFF4      movel.ex -12(a6), a0
1CAA: 0C28 0005 000B cmpib.ex #5, 11(a0)
1CB0: 57C0           seq d0
1CB2: 4400           negb d0
1CB4: 4880           extw d0
1CB6: 6708           beq 8
1CB8: 1D7C 0002 FFED moveb.emx #2, -19(a6)
1CBE: 6006           bra 6
1CC0: 1D7C 0004 FFED moveb.emx #4, -19(a6)
1CC6: 1F2E FFED      moveb.ex -19(a6), -(a7)
1CCA: 206E FFF4      movel.ex -12(a6), a0
1CCE: 700F           moveq #15, d0
1CD0: D068 0002      addrw.ex 2(a0), d0
1CD4: 3F00           movew d0, -(a7)
1CD6: 3F10           movew (a0), -(a7)
1CD8: 7000           moveq #0, d0
1CDA: 1028 000B      moveb.ex 11(a0), d0
1CDE: 0640 000B      addiw #11, d0
1CE2: 3F00           movew d0, -(a7)
1CE4: 4EBA EB46      jsr.ex 31796(pc)
1CE8: 504F           addqw #0, a7
1CEA: 5344           subqw #1, d4
1CEC: 4A44           tstw d4
1CEE: 6C00 FF2C      bge 65324
1CF2: 202D F732      movel.ex -2254(a5), d0
1CF6: B0AD FB70      cmpl.ex -1168(a5), d0
1CFA: 670A           beq 10
1CFC: 2F2D FB70      movel.ex -1168(a5), -(a7)
1D00: 4EBA F898      jsr.ex -30302(pc)
1D04: 584F           addqw #4, a7
1D06: 1F2E FFEC      moveb.ex -20(a6), -(a7)
1D0A: 4E4F           trap #15
1D0C: A164           sysTrapFntSetFont
1D0E: 544F           addqw #2, a7
1D10: 4CDF 1CF8      movem (a7)+, <1cf8>
1D14: 4E5E           unlk a6
1D16: 4E75           rts
1D18: 9447           subrw d7, d2
1D1A: 616D           bsr 109
1D1C: 6553           bcs 83
1D1E: 7461           moveq #97, d2
1D20: 7465           moveq #101, d2
1D22: 4472 6177      negw.ex 119(a2,d6.w)
1D26: 4368           dc.w #17256
1D28: 616E           bsr 110
1D2A: 6765           beq 101
1D2C: 7300           dc.w #29440
1D2E: 0000           dc.w #0
1D30: 4E56 0000      link a6, #0
1D34: 2F03           movel d3, -(a7)
1D36: 7000           moveq #0, d0
1D38: 102D F72E      moveb.ex -2258(a5), d0
1D3C: 5240           addqw #1, d0
1D3E: 3F00           movew d0, -(a7)
1D40: 4EBA F90A      jsr.ex -30124(pc)
1D44: 4EBA F998      jsr.ex -29978(pc)
1D48: 1B7C 0003 F714 moveb.emx #3, -2284(a5)
1D4E: 7000           moveq #0, d0
1D50: 102D F72E      moveb.ex -2258(a5), d0
1D54: C1FC 0005      muls.ex #5, d0
1D58: 5A40           addqw #5, d0
1D5A: 1B40 F728      moveb.mx d0, -2264(a5)
1D5E: 1B40 F729      moveb.mx d0, -2263(a5)
1D62: 422D F72A      clrb.ex -2262(a5)
1D66: 422D F72B      clrb.ex -2261(a5)
1D6A: 7608           moveq #8, d3
1D6C: 544F           addqw #2, a7
1D6E: 6022           bra 34
1D70: 7018           moveq #24, d0
1D72: C1C3           muls d3, d0
1D74: 41ED F714      lea -2284(a5), d0
1D78: 41E8 0482      lea 1154(a0), d0
1D7C: 4230 0800      clrb.ex 0(a0,d0.l)
1D80: 7018           moveq #24, d0
1D82: C1C3           muls d3, d0
1D84: 41ED F714      lea -2284(a5), d0
1D88: 41E8 0483      lea 1155(a0), d0
1D8C: 4230 0800      clrb.ex 0(a0,d0.l)
1D90: 5343           subqw #1, d3
1D92: 4A43           tstw d3
1D94: 6CDA           bge -38
1D96: 261F           movel (a7)+, d3
1D98: 4E5E           unlk a6
1D9A: 4E75           rts
1D9C: 8D47           dc.w #36167
1D9E: 616D           bsr 109
1DA0: 6549           bcs 73
1DA2: 6E69           bgt 105
1DA4: 744C           moveq #76, d2
1DA6: 6576           bcs 118
1DA8: 656C           bcs 108
1DAA: 0000           dc.w #0
1DAC: 4E56 FFFA      link a6, #-6
1DB0: 48E7 1820      movem <1820>, -(a7)
1DB4: 1B7C 0004 F714 moveb.emx #4, -2284(a5)
1DBA: 7000           moveq #0, d0
1DBC: 102D F72E      moveb.ex -2258(a5), d0
1DC0: C1FC 0014      muls.ex #20, d0
1DC4: 0640 007D      addiw #125, d0
1DC8: 3B40 F730      movew.mx d0, -2256(a5)
1DCC: 1F3C 0002      moveb.ex #2, -(a7)
1DD0: 4E4F           trap #15
1DD2: A164           sysTrapFntSetFont
1DD4: 1800           moveb d0, d4
1DD6: 7000           moveq #0, d0
1DD8: 102D F728      moveb.ex -2264(a5), d0
1DDC: 7200           moveq #0, d1
1DDE: 122D F72B      moveb.ex -2261(a5), d1
1DE2: C3FC 0064      muls.ex #100, d1
1DE6: 48C1           extl d1
1DE8: 83C0           divs d0, d1
1DEA: 48C1           extl d1
1DEC: 2F01           movel d1, -(a7)
1DEE: 486E FFFA      pea.ex -6(a6)
1DF2: 4E4F           trap #15
1DF4: A0C9           sysTrapStrIToA
1DF6: 41FA 0092      lea -28014(pc), d0
1DFA: 4850           pea (a0)
1DFC: 486E FFFA      pea.ex -6(a6)
1E00: 4E4F           trap #15
1E02: A0C6           sysTrapStrCat
1E04: 486E FFFA      pea.ex -6(a6)
1E08: 4E4F           trap #15
1E0A: A0C7           sysTrapStrLen
1E0C: 3600           movew d0, d3
1E0E: 3F3C 03E9      movew.ex #1001, -(a7)
1E12: 2F3C 7453 5452 movel.exl #1951618130, -(a7)
1E18: 4E4F           trap #15
1E1A: A05F           sysTrapDmGetResource
1E1C: 5C4F           addqw #6, a7
1E1E: 2F08           movel a0, -(a7)
1E20: 4E4F           trap #15
1E22: A021           sysTrapMemHandleLock
1E24: 2448           movel a0, a2
1E26: 2F3C 001E 0050 movel.exl #1966160, -(a7)
1E2C: 2F0A           movel a2, -(a7)
1E2E: 4E4F           trap #15
1E30: A0C7           sysTrapStrLen
1E32: 584F           addqw #4, a7
1E34: 3F00           movew d0, -(a7)
1E36: 2F0A           movel a2, -(a7)
1E38: 4E4F           trap #15
1E3A: A220           sysTrapWinDrawChars
1E3C: 3F3C 0050      movew.ex #80, -(a7)
1E40: 2F0A           movel a2, -(a7)
1E42: 4E4F           trap #15
1E44: A0C7           sysTrapStrLen
1E46: 584F           addqw #4, a7
1E48: 3F00           movew d0, -(a7)
1E4A: 2F0A           movel a2, -(a7)
1E4C: 4E4F           trap #15
1E4E: A16B           sysTrapFntCharsWidth
1E50: 0640 001E      addiw #30, d0
1E54: 5C4F           addqw #6, a7
1E56: 3F00           movew d0, -(a7)
1E58: 3F03           movew d3, -(a7)
1E5A: 486E FFFA      pea.ex -6(a6)
1E5E: 4E4F           trap #15
1E60: A220           sysTrapWinDrawChars
1E62: 2F0A           movel a2, -(a7)
1E64: 4E4F           trap #15
1E66: A035           sysTrapMemPtrUnlock
1E68: 1F04           moveb d4, -(a7)
1E6A: 4E4F           trap #15
1E6C: A164           sysTrapFntSetFont
1E6E: 4FEF 0034      lea 52(a7), d7
1E72: 4CDF 0418      movem (a7)+, <0418>
1E76: 4E5E           unlk a6
1E78: 4E75           rts
1E7A: 8C47           orrw d7, d6
1E7C: 616D           bsr 109
1E7E: 6545           bcs 69
1E80: 6E64           bgt 100
1E82: 4C65           dc.w #19557
1E84: 7665           moveq #101, d3
1E86: 6C00 0002      bge 2
1E8A: 2500           movel d0, -(a2)
1E8C: 4E56 0000      link a6, #0
1E90: 4878 043E      pea.ex (043e).w
1E94: 486D FB70      pea.ex -1168(a5)
1E98: 486D F732      pea.ex -2254(a5)
1E9C: 4E4F           trap #15
1E9E: A026           sysTrapMemMove
1EA0: 0C2D 0004 F714 cmpib.ex #4, -2284(a5)
1EA6: 4FEF 000C      lea 12(a7), d7
1EAA: 660E           bne 14
1EAC: 4A6D F730      tstw.ex -2256(a5)
1EB0: 6608           bne 8
1EB2: 522D F72E      addqb.ex #1, -2258(a5)
1EB6: 4EBA FE78      jsr.ex -28360(pc)
1EBA: 4E5E           unlk a6
1EBC: 4E75           rts
1EBE: 9047           subrw d7, d0
1EC0: 616D           bsr 109
1EC2: 6553           bcs 83
1EC4: 7461           moveq #97, d2
1EC6: 7465           moveq #101, d2
1EC8: 4164           dc.w #16740
1ECA: 7661           moveq #97, d3
1ECC: 6E63           bgt 99
1ECE: 6500 0000      bcs 0
1ED2: 4E56 0000      link a6, #0
1ED6: 2F03           movel d3, -(a7)
1ED8: 4EBA E910      jsr.ex 31730(pc)
1EDC: 422D F71A      clrb.ex -2278(a5)
1EE0: 42AD F71C      clrl.ex -2276(a5)
1EE4: 4A2D F714      tstb.ex -2284(a5)
1EE8: 6700 0094      beq 148
1EEC: 42AD FB70      clrl.ex -1168(a5)
1EF0: 422D FFB6      clrb.ex -74(a5)
1EF4: 1B6D F61D F72E moveb.emx -2531(a5), -2258(a5)
1EFA: 422D FFB9      clrb.ex -71(a5)
1EFE: 422D FFBA      clrb.ex -70(a5)
1F02: 422D F72C      clrb.ex -2260(a5)
1F06: 422D FB74      clrb.ex -1164(a5)
1F0A: 422D FB75      clrb.ex -1163(a5)
1F0E: 7600           moveq #0, d3
1F10: 6022           bra 34
1F12: 700E           moveq #14, d0
1F14: C1C3           muls d3, d0
1F16: 41ED F714      lea -2284(a5), d0
1F1A: 41E8 055A      lea 1370(a0), d0
1F1E: 4230 0800      clrb.ex 0(a0,d0.l)
1F22: 700E           moveq #14, d0
1F24: C1C3           muls d3, d0
1F26: 41ED F714      lea -2284(a5), d0
1F2A: 41E8 055B      lea 1371(a0), d0
1F2E: 4230 0800      clrb.ex 0(a0,d0.l)
1F32: 5243           addqw #1, d3
1F34: 0C43 0014      cmpiw #20, d3
1F38: 6DD8           blt -40
1F3A: 7600           moveq #0, d3
1F3C: 6022           bra 34
1F3E: 700E           moveq #14, d0
1F40: C1C3           muls d3, d0
1F42: 41ED F714      lea -2284(a5), d0
1F46: 41E8 0672      lea 1650(a0), d0
1F4A: 4230 0800      clrb.ex 0(a0,d0.l)
1F4E: 700E           moveq #14, d0
1F50: C1C3           muls d3, d0
1F52: 41ED F714      lea -2284(a5), d0
1F56: 41E8 0673      lea 1651(a0), d0
1F5A: 4230 0800      clrb.ex 0(a0,d0.l)
1F5E: 5243           addqw #1, d3
1F60: 0C43 0028      cmpiw #40, d3
1F64: 6DD8           blt -40
1F66: 4EBA FDC8      jsr.ex -28360(pc)
1F6A: 4EBA EA40      jsr.ex 32180(pc)
1F6E: 4EBA FF1C      jsr.ex -28012(pc)
1F72: 4E4F           trap #15
1F74: A0F7           sysTrapTimGetTicks
1F76: 5880           addql #4, d0
1F78: 2B40 F716      movel.mx d0, -2282(a5)
1F7C: 6014           bra 20
1F7E: 1B6D F62F F714 moveb.emx -2513(a5), -2284(a5)
1F84: 4E4F           trap #15
1F86: A0F7           sysTrapTimGetTicks
1F88: 0680 0000 012C addil #300, d0
1F8E: 2B40 F716      movel.mx d0, -2282(a5)
1F92: 261F           movel (a7)+, d3
1F94: 4E5E           unlk a6
1F96: 4E75           rts
1F98: 8947           dc.w #35143
1F9A: 616D           bsr 109
1F9C: 6553           bcs 83
1F9E: 7461           moveq #97, d2
1FA0: 7274           moveq #116, d1
1FA2: 0000           dc.w #0
1FA4: 4E56 0000      link a6, #0
1FA8: 7001           moveq #1, d0
1FAA: 4E5E           unlk a6
1FAC: 4E75           rts
1FAE: 8023           orrb -(a3), d0
1FB0: 4368           dc.w #17256
1FB2: 6563           bcs 99
1FB4: 6B43           bmi 67
1FB6: 6861           bvc 97
1FB8: 7267           moveq #103, d1
1FBA: 6541           bcs 65
1FBC: 6E64           bgt 100
1FBE: 5375 6243      subqw.ex #1, 67(a5,d6.w)
1FC2: 6F6C           ble 108
1FC4: 6C69           bge 105
1FC6: 7369           dc.w #29545
1FC8: 6F6E           ble 110
1FCA: 4361           dc.w #17249
1FCC: 7265           moveq #101, d1
1FCE: 6675           bne 117
1FD0: 6C6C           bge 108
1FD2: 7900           dc.w #30976
1FD4: 0000           dc.w #0
1FD6: 4E56 0000      link a6, #0
1FDA: 48E7 1830      movem <1830>, -(a7)
1FDE: 7600           moveq #0, d3
1FE0: 162D FB74      moveb.ex -1164(a5), d3
1FE4: 5343           subqw #1, d3
1FE6: 7800           moveq #0, d4
1FE8: 700E           moveq #14, d0
1FEA: C1C3           muls d3, d0
1FEC: 45ED F714      lea -2284(a5), d2
1FF0: D5C0           addal d0, a2
1FF2: 45EA 0552      lea 1362(a2), d2
1FF6: 7018           moveq #24, d0
1FF8: C1C4           muls d4, d0
1FFA: 47ED F714      lea -2284(a5), d3
1FFE: D7C0           addal d0, a3
2000: 47EB 047A      lea 1146(a3), d3
2004: 6000 00CA      bra 202
2008: 4A2A 000A      tstb.ex 10(a2)
200C: 6714           beq 20
200E: 5343           subqw #1, d3
2010: 700E           moveq #14, d0
2012: C1C3           muls d3, d0
2014: 45ED F714      lea -2284(a5), d2
2018: D5C0           addal d0, a2
201A: 45EA 0552      lea 1362(a2), d2
201E: 6000 00B0      bra 176
2022: 4A2B 0008      tstb.ex 8(a3)
2026: 6700 0098      beq 152
202A: 302A 0002      movew.ex 2(a2), d0
202E: B06B 0006      cmpw.ex 6(a3), d0
2032: 6F14           ble 20
2034: 5244           addqw #1, d4
2036: 7018           moveq #24, d0
2038: C1C4           muls d4, d0
203A: 47ED F714      lea -2284(a5), d3
203E: D7C0           addal d0, a3
2040: 47EB 047A      lea 1146(a3), d3
2044: 6000 008A      bra 138
2048: 302A 0006      movew.ex 6(a2), d0
204C: B06B 0002      cmpw.ex 2(a3), d0
2050: 6C12           bge 18
2052: 5343           subqw #1, d3
2054: 700E           moveq #14, d0
2056: C1C3           muls d3, d0
2058: 45ED F714      lea -2284(a5), d2
205C: D5C0           addal d0, a2
205E: 45EA 0552      lea 1362(a2), d2
2062: 606C           bra 108
2064: 302A 0004      movew.ex 4(a2), d0
2068: B053           cmpw (a3), d0
206A: 6D42           blt 66
206C: 3012           movew (a2), d0
206E: B06B 0004      cmpw.ex 4(a3), d0
2072: 6E3A           bgt 58
2074: 2F0B           movel a3, -(a7)
2076: 2F0A           movel a2, -(a7)
2078: 4EBA FF2A      jsr.ex -27732(pc)
207C: 4A00           tstb d0
207E: 504F           addqw #0, a7
2080: 672C           beq 44
2082: 3F04           movew d4, -(a7)
2084: 3F03           movew d3, -(a7)
2086: 4EBA F2E6      jsr.ex -30858(pc)
208A: 5244           addqw #1, d4
208C: 7018           moveq #24, d0
208E: C1C4           muls d4, d0
2090: 47ED F714      lea -2284(a5), d3
2094: D7C0           addal d0, a3
2096: 47EB 047A      lea 1146(a3), d3
209A: 5343           subqw #1, d3
209C: 700E           moveq #14, d0
209E: C1C3           muls d3, d0
20A0: 45ED F714      lea -2284(a5), d2
20A4: D5C0           addal d0, a2
20A6: 45EA 0552      lea 1362(a2), d2
20AA: 584F           addqw #4, a7
20AC: 6022           bra 34
20AE: 5343           subqw #1, d3
20B0: 700E           moveq #14, d0
20B2: C1C3           muls d3, d0
20B4: 45ED F714      lea -2284(a5), d2
20B8: D5C0           addal d0, a2
20BA: 45EA 0552      lea 1362(a2), d2
20BE: 6010           bra 16
20C0: 5244           addqw #1, d4
20C2: 7018           moveq #24, d0
20C4: C1C4           muls d4, d0
20C6: 47ED F714      lea -2284(a5), d3
20CA: D7C0           addal d0, a3
20CC: 47EB 047A      lea 1146(a3), d3
20D0: 0C44 0009      cmpiw #9, d4
20D4: 6C06           bge 6
20D6: 4A43           tstw d3
20D8: 6C00 FF2E      bge 65326
20DC: 4CDF 0C18      movem (a7)+, <0c18>
20E0: 4E5E           unlk a6
20E2: 4E75           rts
20E4: 9C43           subrw d3, d6
20E6: 6865           bvc 101
20E8: 636B           bls 107
20EA: 4368           dc.w #17256
20EC: 6172           bsr 114
20EE: 6765           beq 101
20F0: 5769 7468      subqw.ex #3, 29800(a1)
20F4: 5375 6243      subqw.ex #1, 67(a5,d6.w)
20F8: 6F6C           ble 108
20FA: 6C69           bge 105
20FC: 7369           dc.w #29545
20FE: 6F6E           ble 110
2100: 7300           dc.w #29440
2102: 0000           dc.w #0
2104: 4E56 0000      link a6, #0
2108: 48E7 1E20      movem <1e20>, -(a7)
210C: 282E 0008      movel.ex 8(a6), d4
2110: 422D FFB4      clrb.ex -76(a5)
2114: 422D FFB5      clrb.ex -75(a5)
2118: 7602           moveq #2, d3
211A: 45ED F714      lea -2284(a5), d2
211E: 45EA 0462      lea 1122(a2), d2
2122: 7008           moveq #8, d0
2124: C084           andrl d4, d0
2126: 6746           beq 70
2128: 7040           moveq #64, d0
212A: C084           andrl d4, d0
212C: 6640           bne 64
212E: 4A2A 0008      tstb.ex 8(a2)
2132: 6700 008E      beq 142
2136: 4A2D F72D      tstb.ex -2259(a5)
213A: 6600 0086      bne 134
213E: 3012           movew (a2), d0
2140: 5B40           subqw #5, d0
2142: B640           cmpw d0, d3
2144: 5DC0           slt d0
2146: 4400           negb d0
2148: 4880           extw d0
214A: 6704           beq 4
214C: 3A03           movew d3, d5
214E: 6004           bra 4
2150: 3A12           movew (a2), d5
2152: 5B45           subqw #5, d5
2154: 3605           movew d5, d3
2156: 4A43           tstw d3
2158: 6F68           ble 104
215A: 9752           submw d3, (a2)
215C: 976A 0004      submw.ex d3, 4(a2)
2160: 157C 0001 0009 moveb.emx #1, 9(a2)
2166: 1B7C 0001 FFB4 moveb.emx #1, -76(a5)
216C: 6054           bra 84
216E: 7040           moveq #64, d0
2170: C084           andrl d4, d0
2172: 674A           beq 74
2174: 7008           moveq #8, d0
2176: C084           andrl d4, d0
2178: 6644           bne 68
217A: 4A2A 0008      tstb.ex 8(a2)
217E: 6742           beq 66
2180: 4A2D F72D      tstb.ex -2259(a5)
2184: 663C           bne 60
2186: 303C 009B      movew.ex #155, d0
218A: 906A 0004      subrw.ex 4(a2), d0
218E: B640           cmpw d0, d3
2190: 5DC0           slt d0
2192: 4400           negb d0
2194: 4880           extw d0
2196: 6704           beq 4
2198: 3C03           movew d3, d6
219A: 6008           bra 8
219C: 3C3C 009B      movew.ex #155, d6
21A0: 9C6A 0004      subrw.ex 4(a2), d6
21A4: 3606           movew d6, d3
21A6: 4A43           tstw d3
21A8: 6F18           ble 24
21AA: D752           addmw d3, (a2)
21AC: D76A 0004      addmw.ex d3, 4(a2)
21B0: 157C 0001 0009 moveb.emx #1, 9(a2)
21B6: 1B7C 0001 FFB5 moveb.emx #1, -75(a5)
21BC: 6004           bra 4
21BE: 422A 0009      clrb.ex 9(a2)
21C2: 4CDF 0478      movem (a7)+, <0478>
21C6: 4E5E           unlk a6
21C8: 4E75           rts
21CA: 8853           orrw (a3), d4
21CC: 6869           bvc 105
21CE: 704D           moveq #77, d0
21D0: 6F76           ble 118
21D2: 6500 0000      bcs 0
21D6: 4E56 FFEC      link a6, #-20
21DA: 48E7 1F20      movem <1f20>, -(a7)
21DE: 7608           moveq #8, d3
21E0: 6000 06E0      bra 1760
21E4: 7018           moveq #24, d0
21E6: C1C3           muls d3, d0
21E8: 45ED F714      lea -2284(a5), d2
21EC: D5C0           addal d0, a2
21EE: 45EA 047A      lea 1146(a2), d2
21F2: 4A2A 0008      tstb.ex 8(a2)
21F6: 660C           bne 12
21F8: 422A 0008      clrb.ex 8(a2)
21FC: 422A 0009      clrb.ex 9(a2)
2200: 6000 06BE      bra 1726
2204: 532A 0012      subqb.ex #1, 18(a2)
2208: 4A2A 0012      tstb.ex 18(a2)
220C: 6E00 06AE      bgt 1710
2210: 4A2A 000A      tstb.ex 10(a2)
2214: 670C           beq 12
2216: 3F03           movew d3, -(a7)
2218: 4EBA F122      jsr.ex -30908(pc)
221C: 544F           addqw #2, a7
221E: 6000 06A0      bra 1696
2222: 4A43           tstw d3
2224: 6670           bne 112
2226: 0C6A 0015 0002 cmpiw.ex #21, 2(a2)
222C: 6D68           blt 104
222E: 0C2A 0001 000C cmpib.ex #1, 12(a2)
2234: 6624           bne 36
2236: 42A7           clrl -(a7)
2238: 4E4F           trap #15
223A: A0C2           sysTrapSysRandom
223C: C1FC 03E8      muls.ex #1000, d0
2240: 2200           movel d0, d1
2242: E081           asril #0, d1
2244: EC81           asril #6, d1
2246: E089           lsril #0, d1
2248: E089           lsril #0, d1
224A: E289           lsril #1, d1
224C: D280           addrl d0, d1
224E: E081           asril #0, d1
2250: EE81           asril #7, d1
2252: 7020           moveq #32, d0
2254: B280           cmpl d0, d1
2256: 584F           addqw #4, a7
2258: 6D2C           blt 44
225A: 0C2A 0001 000C cmpib.ex #1, 12(a2)
2260: 6334           bls 52
2262: 42A7           clrl -(a7)
2264: 4E4F           trap #15
2266: A0C2           sysTrapSysRandom
2268: C1FC 03E8      muls.ex #1000, d0
226C: 2200           movel d0, d1
226E: E081           asril #0, d1
2270: EC81           asril #6, d1
2272: E089           lsril #0, d1
2274: E089           lsril #0, d1
2276: E289           lsril #1, d1
2278: D280           addrl d0, d1
227A: E081           asril #0, d1
227C: EE81           asril #7, d1
227E: 7030           moveq #48, d0
2280: B280           cmpl d0, d1
2282: 584F           addqw #4, a7
2284: 6C10           bge 16
2286: 157C 000A 000F moveb.emx #10, 15(a2)
228C: 157C FFFF 0010 moveb.emx Nonzero high byte, 16(a2)
2292: 6000 0130      bra 304
2296: 102A 000F      moveb.ex 15(a2), d0
229A: 4880           extw d0
229C: 7200           moveq #0, d1
229E: 122A 000E      moveb.ex 14(a2), d1
22A2: B240           cmpw d0, d1
22A4: 6600 011E      bne 286
22A8: 0C6A 0015 0002 cmpiw.ex #21, 2(a2)
22AE: 6D00 0114      blt 276
22B2: 0C2A 0001 000C cmpib.ex #1, 12(a2)
22B8: 6624           bne 36
22BA: 42A7           clrl -(a7)
22BC: 4E4F           trap #15
22BE: A0C2           sysTrapSysRandom
22C0: C1FC 03E8      muls.ex #1000, d0
22C4: 2200           movel d0, d1
22C6: E081           asril #0, d1
22C8: EC81           asril #6, d1
22CA: E089           lsril #0, d1
22CC: E089           lsril #0, d1
22CE: E289           lsril #1, d1
22D0: D280           addrl d0, d1
22D2: E081           asril #0, d1
22D4: EE81           asril #7, d1
22D6: 7010           moveq #16, d0
22D8: B280           cmpl d0, d1
22DA: 584F           addqw #4, a7
22DC: 6D30           blt 48
22DE: 0C2A 0002 000C cmpib.ex #2, 12(a2)
22E4: 6500 00DE      bcs 222
22E8: 42A7           clrl -(a7)
22EA: 4E4F           trap #15
22EC: A0C2           sysTrapSysRandom
22EE: C1FC 03E8      muls.ex #1000, d0
22F2: 2200           movel d0, d1
22F4: E081           asril #0, d1
22F6: EC81           asril #6, d1
22F8: E089           lsril #0, d1
22FA: E089           lsril #0, d1
22FC: E289           lsril #1, d1
22FE: D280           addrl d0, d1
2300: E081           asril #0, d1
2302: EE81           asril #7, d1
2304: 7014           moveq #20, d0
2306: B280           cmpl d0, d1
2308: 584F           addqw #4, a7
230A: 6C00 00B8      bge 184
230E: 7000           moveq #0, d0
2310: 102A 000C      moveb.ex 12(a2), d0
2314: 5340           subqw #1, d0
2316: 670A           beq 10
2318: 5340           subqw #1, d0
231A: 672E           beq 46
231C: 5340           subqw #1, d0
231E: 6750           beq 80
2320: 6072           bra 114
2322: 42A7           clrl -(a7)
2324: 4E4F           trap #15
2326: A0C2           sysTrapSysRandom
2328: 48C0           extl d0
232A: D080           addrl d0, d0
232C: 2200           movel d0, d1
232E: E081           asril #0, d1
2330: EC81           asril #6, d1
2332: E089           lsril #0, d1
2334: E089           lsril #0, d1
2336: E289           lsril #1, d1
2338: D280           addrl d0, d1
233A: E081           asril #0, d1
233C: EE81           asril #7, d1
233E: 7009           moveq #9, d0
2340: D280           addrl d0, d1
2342: 1541 000F      moveb.mx d1, 15(a2)
2346: 584F           addqw #4, a7
2348: 604A           bra 74
234A: 42A7           clrl -(a7)
234C: 4E4F           trap #15
234E: A0C2           sysTrapSysRandom
2350: C1FC 0003      muls.ex #3, d0
2354: 2200           movel d0, d1
2356: E081           asril #0, d1
2358: EC81           asril #6, d1
235A: E089           lsril #0, d1
235C: E089           lsril #0, d1
235E: E289           lsril #1, d1
2360: D280           addrl d0, d1
2362: E081           asril #0, d1
2364: EE81           asril #7, d1
2366: 5881           addql #4, d1
2368: 1541 000F      moveb.mx d1, 15(a2)
236C: 584F           addqw #4, a7
236E: 6024           bra 36
2370: 42A7           clrl -(a7)
2372: 4E4F           trap #15
2374: A0C2           sysTrapSysRandom
2376: C1FC 0003      muls.ex #3, d0
237A: 2200           movel d0, d1
237C: E081           asril #0, d1
237E: EC81           asril #6, d1
2380: E089           lsril #0, d1
2382: E089           lsril #0, d1
2384: E289           lsril #1, d1
2386: D280           addrl d0, d1
2388: E081           asril #0, d1
238A: EE81           asril #7, d1
238C: 5881           addql #4, d1
238E: 1541 000F      moveb.mx d1, 15(a2)
2392: 584F           addqw #4, a7
2394: 42A7           clrl -(a7)
2396: 4E4F           trap #15
2398: A0C2           sysTrapSysRandom
239A: C1FC 0064      muls.ex #100, d0
239E: 2200           movel d0, d1
23A0: E081           asril #0, d1
23A2: EC81           asril #6, d1
23A4: E089           lsril #0, d1
23A6: E089           lsril #0, d1
23A8: E289           lsril #1, d1
23AA: D280           addrl d0, d1
23AC: E081           asril #0, d1
23AE: EE81           asril #7, d1
23B0: 704B           moveq #75, d0
23B2: B280           cmpl d0, d1
23B4: 584F           addqw #4, a7
23B6: 6C0C           bge 12
23B8: 102A 000F      moveb.ex 15(a2), d0
23BC: 4880           extw d0
23BE: 4440           negw d0
23C0: 1540 000F      moveb.mx d0, 15(a2)
23C4: 102A 000F      moveb.ex 15(a2), d0
23C8: 4880           extw d0
23CA: 7200           moveq #0, d1
23CC: 122A 000E      moveb.ex 14(a2), d1
23D0: B240           cmpw d0, d1
23D2: 6756           beq 86
23D4: 7000           moveq #0, d0
23D6: 102A 000E      moveb.ex 14(a2), d0
23DA: 122A 000F      moveb.ex 15(a2), d1
23DE: 4881           extw d1
23E0: B240           cmpw d0, d1
23E2: 6F08           ble 8
23E4: 542A 000E      addqb.ex #2, 14(a2)
23E8: 6000 0144      bra 324
23EC: 0C2A 0002 000E cmpib.ex #2, 14(a2)
23F2: 622E           bhi 46
23F4: 157C 0001 000E moveb.emx #1, 14(a2)
23FA: 0C2A 0001 000D cmpib.ex #1, 13(a2)
2400: 57C0           seq d0
2402: 4400           negb d0
2404: 4880           extw d0
2406: 6704           beq 4
2408: 7C00           moveq #0, d6
240A: 6002           bra 2
240C: 7C01           moveq #1, d6
240E: 1546 000D      moveb.mx d6, 13(a2)
2412: 102A 000F      moveb.ex 15(a2), d0
2416: 4880           extw d0
2418: 4440           negw d0
241A: 1540 000F      moveb.mx d0, 15(a2)
241E: 6000 010E      bra 270
2422: 552A 000E      subqb.ex #2, 14(a2)
2426: 6000 0106      bra 262
242A: 102A 0010      moveb.ex 16(a2), d0
242E: 4880           extw d0
2430: B043           cmpw d3, d0
2432: 6600 00FA      bne 250
2436: 0C6A 0015 0002 cmpiw.ex #21, 2(a2)
243C: 6D00 00F0      blt 240
2440: 4A43           tstw d3
2442: 6F00 00EA      ble 234
2446: 3003           movew d3, d0
2448: 5340           subqw #1, d0
244A: C1FC 0018      muls.ex #24, d0
244E: 41ED F714      lea -2284(a5), d0
2452: 41E8 0482      lea 1154(a0), d0
2456: 4A30 0800      tstb.ex 0(a0,d0.l)
245A: 6600 00D2      bne 210
245E: 3003           movew d3, d0
2460: 5340           subqw #1, d0
2462: C1FC 0018      muls.ex #24, d0
2466: 41ED F714      lea -2284(a5), d0
246A: 41E8 0483      lea 1155(a0), d0
246E: 4A30 0800      tstb.ex 0(a0,d0.l)
2472: 6600 00BA      bne 186
2476: 0C2A 0001 000C cmpib.ex #1, 12(a2)
247C: 6626           bne 38
247E: 42A7           clrl -(a7)
2480: 4E4F           trap #15
2482: A0C2           sysTrapSysRandom
2484: C1FC 03E8      muls.ex #1000, d0
2488: 2200           movel d0, d1
248A: E081           asril #0, d1
248C: EC81           asril #6, d1
248E: E089           lsril #0, d1
2490: E089           lsril #0, d1
2492: E289           lsril #1, d1
2494: D280           addrl d0, d1
2496: E081           asril #0, d1
2498: EE81           asril #7, d1
249A: 7010           moveq #16, d0
249C: B280           cmpl d0, d1
249E: 584F           addqw #4, a7
24A0: 6D00 0088      blt 136
24A4: 7E00           moveq #0, d7
24A6: 0C2A 0002 000C cmpib.ex #2, 12(a2)
24AC: 54C0           scc d0
24AE: 4400           negb d0
24B0: 4880           extw d0
24B2: 6760           beq 96
24B4: 426E FFFE      clrw.ex -2(a6)
24B8: 3012           movew (a2), d0
24BA: B06D FB7A      cmpw.ex -1158(a5), d0
24BE: 5DC0           slt d0
24C0: 4400           negb d0
24C2: 4880           extw d0
24C4: 6716           beq 22
24C6: 302A 0004      movew.ex 4(a2), d0
24CA: B06D FB76      cmpw.ex -1162(a5), d0
24CE: 5CC0           sge d0
24D0: 4400           negb d0
24D2: 4880           extw d0
24D4: 6706           beq 6
24D6: 3D7C 0001 FFFE movew.emx #1, -2(a6)
24DC: 701E           moveq #30, d0
24DE: D06E FFFE      addrw.ex -2(a6), d0
24E2: 48C0           extl d0
24E4: 2D40 FFEC      movel.mx d0, -20(a6)
24E8: 42A7           clrl -(a7)
24EA: 4E4F           trap #15
24EC: A0C2           sysTrapSysRandom
24EE: C1FC 03E8      muls.ex #1000, d0
24F2: 2200           movel d0, d1
24F4: E081           asril #0, d1
24F6: EC81           asril #6, d1
24F8: E089           lsril #0, d1
24FA: E089           lsril #0, d1
24FC: E289           lsril #1, d1
24FE: D280           addrl d0, d1
2500: E081           asril #0, d1
2502: EE81           asril #7, d1
2504: B2AE FFEC      cmpl.ex -20(a6), d1
2508: 5DC1           slt d1
250A: 4401           negb d1
250C: 4881           extw d1
250E: 584F           addqw #4, a7
2510: 6702           beq 2
2512: 7E01           moveq #1, d7
2514: 4A47           tstw d7
2516: 6708           beq 8
2518: 3D7C 0064 FFFC movew.emx #100, -4(a6)
251E: 6004           bra 4
2520: 426E FFFC      clrw.ex -4(a6)
2524: 4A6E FFFC      tstw.ex -4(a6)
2528: 6704           beq 4
252A: 532A 0010      subqb.ex #1, 16(a2)
252E: 7000           moveq #0, d0
2530: 102A 000E      moveb.ex 14(a2), d0
2534: 720B           moveq #11, d1
2536: 9240           subrw d0, d1
2538: 1541 0012      moveb.mx d1, 18(a2)
253C: 157C 0001 0009 moveb.emx #1, 9(a2)
2542: 0C2A 0001 000D cmpib.ex #1, 13(a2)
2548: 6616           bne 22
254A: 5352           subqw #1, (a2)
254C: 536A 0004      subqw.ex #1, 4(a2)
2550: 4A6A 0004      tstw.ex 4(a2)
2554: 6C1E           bge 30
2556: 422A 0008      clrb.ex 8(a2)
255A: 532D F72A      subqb.ex #1, -2262(a5)
255E: 6014           bra 20
2560: 5252           addqw #1, (a2)
2562: 526A 0004      addqw.ex #1, 4(a2)
2566: 0C52 00A0      cmpiw #160, (a2)
256A: 6F08           ble 8
256C: 422A 0008      clrb.ex 8(a2)
2570: 532D F72A      subqb.ex #1, -2262(a5)
2574: 4A43           tstw d3
2576: 6600 016E      bne 366
257A: 0C2A           dc.w #3114
257C: FFFF           dc.w #65535
257E: 0010           dc.w #16
2580: 6612           bne 18
2582: 0C6A 0009 0002 cmpiw.ex #9, 2(a2)
2588: 6F0A           ble 10
258A: 536A 0002      subqw.ex #1, 2(a2)
258E: 536A 0006      subqw.ex #1, 6(a2)
2592: 6016           bra 22
2594: 4A2A 0010      tstb.ex 16(a2)
2598: 6610           bne 16
259A: 0C6A 0015 0002 cmpiw.ex #21, 2(a2)
25A0: 6C08           bge 8
25A2: 526A 0002      addqw.ex #1, 2(a2)
25A6: 526A 0006      addqw.ex #1, 6(a2)
25AA: 0C6A 0009 0002 cmpiw.ex #9, 2(a2)
25B0: 6638           bne 56
25B2: 0C2A 0001 000C cmpib.ex #1, 12(a2)
25B8: 6630           bne 48
25BA: 0C2A 0001 000D cmpib.ex #1, 13(a2)
25C0: 660A           bne 10
25C2: 302D FB7A      movew.ex -1158(a5), d0
25C6: 5940           subqw #4, d0
25C8: B052           cmpw (a2), d0
25CA: 6712           beq 18
25CC: 4A2A 000D      tstb.ex 13(a2)
25D0: 6618           bne 24
25D2: 302D FB76      movew.ex -1162(a5), d0
25D6: 5940           subqw #4, d0
25D8: B06A 0004      cmpw.ex 4(a2), d0
25DC: 660C           bne 12
25DE: 4EBA E432      jsr.ex 32282(pc)
25E2: 422A 0010      clrb.ex 16(a2)
25E6: 6000 017A      bra 378
25EA: 0C6A 0009 0002 cmpiw.ex #9, 2(a2)
25F0: 6E00 0170      bgt 368
25F4: 0C2A 0002 000C cmpib.ex #2, 12(a2)
25FA: 6500 0166      bcs 358
25FE: 0C2A 0009 000E cmpib.ex #9, 14(a2)
2604: 6700 015C      beq 348
2608: 302D FB7A      movew.ex -1158(a5), d0
260C: 5740           subqw #3, d0
260E: B052           cmpw (a2), d0
2610: 6F00 00A0      ble 160
2614: 302D FB76      movew.ex -1162(a5), d0
2618: 5640           addqw #3, d0
261A: B06A 0004      cmpw.ex 4(a2), d0
261E: 6E00 0092      bgt 146
2622: 4EBA E3EE      jsr.ex 32282(pc)
2626: 157C 003C 0012 moveb.emx #60, 18(a2)
262C: 596A 0002      subqw.ex #4, 2(a2)
2630: 596A 0006      subqw.ex #4, 6(a2)
2634: 7800           moveq #0, d4
2636: 182D F72D      moveb.ex -2259(a5), d4
263A: D86D FB78      addrw.ex -1160(a5), d4
263E: 986A 0002      subrw.ex 2(a2), d4
2642: 4A44           tstw d4
2644: 6F08           ble 8
2646: D96A 0002      addmw.ex d4, 2(a2)
264A: D96A 0006      addmw.ex d4, 6(a2)
264E: 0C2A 0001 000D cmpib.ex #1, 13(a2)
2654: 6628           bne 40
2656: 302D FB7A      movew.ex -1158(a5), d0
265A: B06A 0004      cmpw.ex 4(a2), d0
265E: 6C1E           bge 30
2660: 422A 000D      clrb.ex 13(a2)
2664: 3012           movew (a2), d0
2666: 906D FB76      subrw.ex -1162(a5), d0
266A: 3800           movew d0, d4
266C: E04C           lsriw #0, d4
266E: EE4C           lsriw #7, d4
2670: D840           addrw d0, d4
2672: E244           asriw #1, d4
2674: 9952           submw d4, (a2)
2676: 996A 0004      submw.ex d4, 4(a2)
267A: 6000 00E6      bra 230
267E: 4A2A 000D      tstb.ex 13(a2)
2682: 6600 00DE      bne 222
2686: 302D FB76      movew.ex -1162(a5), d0
268A: B052           cmpw (a2), d0
268C: 6F00 00D4      ble 212
2690: 157C 0001 000D moveb.emx #1, 13(a2)
2696: 302D FB7A      movew.ex -1158(a5), d0
269A: 906A 0004      subrw.ex 4(a2), d0
269E: 3800           movew d0, d4
26A0: E04C           lsriw #0, d4
26A2: EE4C           lsriw #7, d4
26A4: D840           addrw d0, d4
26A6: E244           asriw #1, d4
26A8: D952           addmw d4, (a2)
26AA: D96A 0004      addmw.ex d4, 4(a2)
26AE: 6000 00B2      bra 178
26B2: 0C6A 0009 0002 cmpiw.ex #9, 2(a2)
26B8: 6C08           bge 8
26BA: 586A 0002      addqw.ex #4, 2(a2)
26BE: 586A 0006      addqw.ex #4, 6(a2)
26C2: 3012           movew (a2), d0
26C4: B06D FB7A      cmpw.ex -1158(a5), d0
26C8: 6D0A           blt 10
26CA: 157C 0001 000D moveb.emx #1, 13(a2)
26D0: 6000 0090      bra 144
26D4: 302A 0004      movew.ex 4(a2), d0
26D8: B06D FB76      cmpw.ex -1162(a5), d0
26DC: 6E00 0084      bgt 132
26E0: 422A 000D      clrb.ex 13(a2)
26E4: 607C           bra 124
26E6: 102A 0010      moveb.ex 16(a2), d0
26EA: 4880           extw d0
26EC: B043           cmpw d3, d0
26EE: 6C72           bge 114
26F0: 536A 0002      subqw.ex #1, 2(a2)
26F4: 536A 0006      subqw.ex #1, 6(a2)
26F8: 102A 0010      moveb.ex 16(a2), d0
26FC: 4880           extw d0
26FE: C1FC 000D      muls.ex #13, d0
2702: 0640 0015      addiw #21, d0
2706: B06A 0002      cmpw.ex 2(a2), d0
270A: 6656           bne 86
270C: 4878 0018      pea.ex (0018).w
2710: 2F0A           movel a2, -(a7)
2712: 3003           movew d3, d0
2714: 5340           subqw #1, d0
2716: C1FC 0018      muls.ex #24, d0
271A: 41ED F714      lea -2284(a5), d0
271E: D1C0           addal d0, a0
2720: 4868 047A      pea.ex 1146(a0)
2724: 4E4F           trap #15
2726: A026           sysTrapMemMove
2728: 4878 0018      pea.ex (0018).w
272C: 7018           moveq #24, d0
272E: C1C3           muls d3, d0
2730: 41ED F714      lea -2284(a5), d0
2734: D1C0           addal d0, a0
2736: 4868 003C      pea.ex 60(a0)
273A: 3003           movew d3, d0
273C: 5340           subqw #1, d0
273E: C1FC 0018      muls.ex #24, d0
2742: 41ED F714      lea -2284(a5), d0
2746: D1C0           addal d0, a0
2748: 4868 003C      pea.ex 60(a0)
274C: 4E4F           trap #15
274E: A026           sysTrapMemMove
2750: 422A 0008      clrb.ex 8(a2)
2754: 422A 0009      clrb.ex 9(a2)
2758: 5343           subqw #1, d3
275A: 4FEF 0018      lea 24(a7), d7
275E: 6000 0160      bra 352
2762: 0C2A 0001 000C cmpib.ex #1, 12(a2)
2768: 6200 0156      bhi 342
276C: 0C6A 0015 0002 cmpiw.ex #21, 2(a2)
2772: 6D00 014C      blt 332
2776: 426E FFFA      clrw.ex -6(a6)
277A: 0C2A 0001 000C cmpib.ex #1, 12(a2)
2780: 57C0           seq d0
2782: 4400           negb d0
2784: 4880           extw d0
2786: 6712           beq 18
2788: 3F03           movew d3, -(a7)
278A: 4EBA ECB8      jsr.ex -30644(pc)
278E: 4A00           tstb d0
2790: 544F           addqw #2, a7
2792: 6706           beq 6
2794: 3D7C 0001 FFFA movew.emx #1, -6(a6)
279A: 4A6E FFFA      tstw.ex -6(a6)
279E: 6708           beq 8
27A0: 3D7C 000C FFF8 movew.emx #12, -8(a6)
27A6: 6006           bra 6
27A8: 3D7C 0006 FFF8 movew.emx #6, -8(a6)
27AE: 7000           moveq #0, d0
27B0: 102D F72E      moveb.ex -2258(a5), d0
27B4: C1FC 0006      muls.ex #6, d0
27B8: C1EE FFF8      muls.ex -8(a6), d0
27BC: 3200           movew d0, d1
27BE: E241           asriw #1, d1
27C0: E049           lsriw #0, d1
27C2: EC49           lsriw #6, d1
27C4: D240           addrw d0, d1
27C6: E441           asriw #2, d1
27C8: 0C41 00FA      cmpiw #250, d1
27CC: 5EC1           sgt d1
27CE: 4401           negb d1
27D0: 4881           extw d1
27D2: 6708           beq 8
27D4: 3D7C 00FA FFF6 movew.emx #250, -10(a6)
27DA: 6056           bra 86
27DC: 426E FFF4      clrw.ex -12(a6)
27E0: 0C2A 0001 000C cmpib.ex #1, 12(a2)
27E6: 57C0           seq d0
27E8: 4400           negb d0
27EA: 4880           extw d0
27EC: 6712           beq 18
27EE: 3F03           movew d3, -(a7)
27F0: 4EBA EC52      jsr.ex -30644(pc)
27F4: 4A00           tstb d0
27F6: 544F           addqw #2, a7
27F8: 6706           beq 6
27FA: 3D7C 0001 FFF4 movew.emx #1, -12(a6)
2800: 4A6E FFF4      tstw.ex -12(a6)
2804: 6708           beq 8
2806: 3D7C 000C FFF2 movew.emx #12, -14(a6)
280C: 6006           bra 6
280E: 3D7C 0006 FFF2 movew.emx #6, -14(a6)
2814: 7000           moveq #0, d0
2816: 102D F72E      moveb.ex -2258(a5), d0
281A: C1FC 0006      muls.ex #6, d0
281E: C1EE FFF2      muls.ex -14(a6), d0
2822: 3200           movew d0, d1
2824: E241           asriw #1, d1
2826: E049           lsriw #0, d1
2828: EC49           lsriw #6, d1
282A: D240           addrw d0, d1
282C: E441           asriw #2, d1
282E: 3D41 FFF6      movew.mx d1, -10(a6)
2832: 42A7           clrl -(a7)
2834: 4E4F           trap #15
2836: A0C2           sysTrapSysRandom
2838: C1FC 03E8      muls.ex #1000, d0
283C: 2200           movel d0, d1
283E: E081           asril #0, d1
2840: EC81           asril #6, d1
2842: E089           lsril #0, d1
2844: E089           lsril #0, d1
2846: E289           lsril #1, d1
2848: D280           addrl d0, d1
284A: E081           asril #0, d1
284C: EE81           asril #7, d1
284E: 3D41 FFF0      movew.mx d1, -16(a6)
2852: B26E FFF6      cmpw.ex -10(a6), d1
2856: 584F           addqw #4, a7
2858: 6C66           bge 102
285A: 0C2A 0001 000C cmpib.ex #1, 12(a2)
2860: 6634           bne 52
2862: 0C2D 000E F72E cmpib.ex #14, -2258(a5)
2868: 6528           bcs 40
286A: 42A7           clrl -(a7)
286C: 4E4F           trap #15
286E: A0C2           sysTrapSysRandom
2870: C1FC 0064      muls.ex #100, d0
2874: 2200           movel d0, d1
2876: E081           asril #0, d1
2878: EC81           asril #6, d1
287A: E089           lsril #0, d1
287C: E089           lsril #0, d1
287E: E289           lsril #1, d1
2880: D280           addrl d0, d1
2882: E081           asril #0, d1
2884: EE81           asril #7, d1
2886: 700A           moveq #10, d0
2888: B280           cmpl d0, d1
288A: 584F           addqw #4, a7
288C: 6C04           bge 4
288E: 7A03           moveq #3, d5
2890: 6006           bra 6
2892: 7A02           moveq #2, d5
2894: 6002           bra 2
2896: 7A01           moveq #1, d5
2898: 1F2A 000C      moveb.ex 12(a2), -(a7)
289C: 3F2A 0002      movew.ex 2(a2), -(a7)
28A0: 302A 0004      movew.ex 4(a2), d0
28A4: D052           addrw (a2), d0
28A6: 3200           movew d0, d1
28A8: E049           lsriw #0, d1
28AA: EE49           lsriw #7, d1
28AC: D240           addrw d0, d1
28AE: E241           asriw #1, d1
28B0: 3F01           movew d1, -(a7)
28B2: 1F05           moveb d5, -(a7)
28B4: 4EBA E404      jsr.ex -32574(pc)
28B8: 504F           addqw #0, a7
28BA: 6004           bra 4
28BC: 422A 0009      clrb.ex 9(a2)
28C0: 5343           subqw #1, d3
28C2: 4A43           tstw d3
28C4: 6C00 F91E      bge 63774
28C8: 4CDF 04F8      movem (a7)+, <04f8>
28CC: 4E5E           unlk a6
28CE: 4E75           rts
28D0: 8A53           orrw (a3), d5
28D2: 7562           dc.w #30050
28D4: 4D6F           dc.w #19823
28D6: 7665           moveq #101, d3
28D8: 416C           dc.w #16748
28DA: 6C00 0000      bge 0
28DE: 4E56 0000      link a6, #0
28E2: 48E7 1020      movem <1020>, -(a7)
28E6: 7600           moveq #0, d3
28E8: 162D FB74      moveb.ex -1164(a5), d3
28EC: 5343           subqw #1, d3
28EE: 6068           bra 104
28F0: 700E           moveq #14, d0
28F2: C1C3           muls d3, d0
28F4: 45ED F714      lea -2284(a5), d2
28F8: D5C0           addal d0, a2
28FA: 45EA 0552      lea 1362(a2), d2
28FE: 4A2A 0008      tstb.ex 8(a2)
2902: 660A           bne 10
2904: 3F03           movew d3, -(a7)
2906: 4EBA E25E      jsr.ex 32622(pc)
290A: 544F           addqw #2, a7
290C: 6048           bra 72
290E: 532A 000C      subqb.ex #1, 12(a2)
2912: 4A2A 000C      tstb.ex 12(a2)
2916: 6E3A           bgt 58
2918: 4A2A 000A      tstb.ex 10(a2)
291C: 670C           beq 12
291E: 157C 0001 0009 moveb.emx #1, 9(a2)
2924: 422A 0008      clrb.ex 8(a2)
2928: 602C           bra 44
292A: 157C 0002 000C moveb.emx #2, 12(a2)
2930: 157C 0001 0009 moveb.emx #1, 9(a2)
2936: 526A 0002      addqw.ex #1, 2(a2)
293A: 526A 0006      addqw.ex #1, 6(a2)
293E: 0C6A 008C 0002 cmpiw.ex #140, 2(a2)
2944: 6F10           ble 16
2946: 157C 0001 0009 moveb.emx #1, 9(a2)
294C: 422A 0008      clrb.ex 8(a2)
2950: 6004           bra 4
2952: 422A 0009      clrb.ex 9(a2)
2956: 5343           subqw #1, d3
2958: 4A43           tstw d3
295A: 6C94           bge -108
295C: 4CDF 0408      movem (a7)+, <0408>
2960: 4E5E           unlk a6
2962: 4E75           rts
2964: 8E43           orrw d3, d7
2966: 6861           bvc 97
2968: 7267           moveq #103, d1
296A: 6573           bcs 115
296C: 4D6F           dc.w #19823
296E: 7665           moveq #101, d3
2970: 416C           dc.w #16748
2972: 6C00 0000      bge 0
2976: 4E56 0000      link a6, #0
297A: 48E7 1020      movem <1020>, -(a7)
297E: 7600           moveq #0, d3
2980: 162D FB75      moveb.ex -1163(a5), d3
2984: 5343           subqw #1, d3
2986: 6000 00E2      bra 226
298A: 700E           moveq #14, d0
298C: C1C3           muls d3, d0
298E: 45ED F714      lea -2284(a5), d2
2992: D5C0           addal d0, a2
2994: 45EA 066A      lea 1642(a2), d2
2998: 4A2A 0008      tstb.ex 8(a2)
299C: 660C           bne 12
299E: 3F03           movew d3, -(a7)
29A0: 4EBA E59A      jsr.ex -31932(pc)
29A4: 544F           addqw #2, a7
29A6: 6000 00C0      bra 192
29AA: 532A 000C      subqb.ex #1, 12(a2)
29AE: 4A2A 000C      tstb.ex 12(a2)
29B2: 6E00 00B0      bgt 176
29B6: 4A2A 000A      tstb.ex 10(a2)
29BA: 670E           beq 14
29BC: 157C 0001 0009 moveb.emx #1, 9(a2)
29C2: 422A 0008      clrb.ex 8(a2)
29C6: 6000 00A0      bra 160
29CA: 7000           moveq #0, d0
29CC: 102A 000B      moveb.ex 11(a2), d0
29D0: 5540           subqw #2, d0
29D2: 6702           beq 2
29D4: 6008           bra 8
29D6: 157C 0004 000C moveb.emx #4, 12(a2)
29DC: 6006           bra 6
29DE: 157C 0007 000C moveb.emx #7, 12(a2)
29E4: 157C 0001 0009 moveb.emx #1, 9(a2)
29EA: 536A 0002      subqw.ex #1, 2(a2)
29EE: 536A 0006      subqw.ex #1, 6(a2)
29F2: 0C2A 0003 000B cmpib.ex #3, 11(a2)
29F8: 6658           bne 88
29FA: 42A7           clrl -(a7)
29FC: 4E4F           trap #15
29FE: A0C2           sysTrapSysRandom
2A00: C1FC 0064      muls.ex #100, d0
2A04: 2200           movel d0, d1
2A06: E081           asril #0, d1
2A08: EC81           asril #6, d1
2A0A: E089           lsril #0, d1
2A0C: E089           lsril #0, d1
2A0E: E289           lsril #1, d1
2A10: D280           addrl d0, d1
2A12: E081           asril #0, d1
2A14: EE81           asril #7, d1
2A16: 7042           moveq #66, d0
2A18: B280           cmpl d0, d1
2A1A: 584F           addqw #4, a7
2A1C: 6C34           bge 52
2A1E: 3012           movew (a2), d0
2A20: D06A 0004      addrw.ex 4(a2), d0
2A24: 3200           movew d0, d1
2A26: E049           lsriw #0, d1
2A28: EE49           lsriw #7, d1
2A2A: D240           addrw d0, d1
2A2C: E241           asriw #1, d1
2A2E: 302D FB76      movew.ex -1162(a5), d0
2A32: D06D FB7A      addrw.ex -1158(a5), d0
2A36: 3400           movew d0, d2
2A38: E04A           lsriw #0, d2
2A3A: EE4A           lsriw #7, d2
2A3C: D440           addrw d0, d2
2A3E: E242           asriw #1, d2
2A40: B441           cmpw d1, d2
2A42: 6F08           ble 8
2A44: 5252           addqw #1, (a2)
2A46: 526A 0004      addqw.ex #1, 4(a2)
2A4A: 6006           bra 6
2A4C: 5352           subqw #1, (a2)
2A4E: 536A 0004      subqw.ex #1, 4(a2)
2A52: 0C6A 000E 0002 cmpiw.ex #14, 2(a2)
2A58: 6E0E           bgt 14
2A5A: 3F03           movew d3, -(a7)
2A5C: 4EBA E41A      jsr.ex -32128(pc)
2A60: 544F           addqw #2, a7
2A62: 6004           bra 4
2A64: 422A 0009      clrb.ex 9(a2)
2A68: 5343           subqw #1, d3
2A6A: 4A43           tstw d3
2A6C: 6C00 FF1C      bge 65308
2A70: 4CDF 0408      movem (a7)+, <0408>
2A74: 4E5E           unlk a6
2A76: 4E75           rts
2A78: 8B4D           dc.w #35661
2A7A: 696E           bvs 110
2A7C: 654D           bcs 77
2A7E: 6F76           ble 118
2A80: 6541           bcs 65
2A82: 6C6C           bge 108
2A84: 0000           dc.w #0
2A86: 4E56 FFF6      link a6, #-10
2A8A: 48E7 1F00      movem <1f00>, -(a7)
2A8E: 426E FFFE      clrw.ex -2(a6)
2A92: 7800           moveq #0, d4
2A94: 0C2D 0006 F714 cmpib.ex #6, -2284(a5)
2A9A: 6700 0224      beq 548
2A9E: 0C2D 0005 F714 cmpib.ex #5, -2284(a5)
2AA4: 6616           bne 22
2AA6: 422D FFB6      clrb.ex -74(a5)
2AAA: 1B7C 0006 F714 moveb.emx #6, -2284(a5)
2AB0: 4EBA 071C      jsr.ex -23082(pc)
2AB4: 4EBA DD54      jsr.ex 31762(pc)
2AB8: 6000 0206      bra 518
2ABC: 4A2D F729      tstb.ex -2263(a5)
2AC0: 6616           bne 22
2AC2: 4A2D F72A      tstb.ex -2262(a5)
2AC6: 6610           bne 16
2AC8: 0C2D 0003 F714 cmpib.ex #3, -2284(a5)
2ACE: 6608           bne 8
2AD0: 4EBA F2DA      jsr.ex -28236(pc)
2AD4: 6000 01EA      bra 490
2AD8: 202D F716      movel.ex -2282(a5), d0
2ADC: 5880           addql #4, d0
2ADE: 2B40 F716      movel.mx d0, -2282(a5)
2AE2: 0C2D 0004 F714 cmpib.ex #4, -2284(a5)
2AE8: 662C           bne 44
2AEA: 536D F730      subqw.ex #1, -2256(a5)
2AEE: 6064           bra 100
2AF0: 42A7           clrl -(a7)
2AF2: 4E4F           trap #15
2AF4: A0C2           sysTrapSysRandom
2AF6: 48C0           extl d0
2AF8: D080           addrl d0, d0
2AFA: 2200           movel d0, d1
2AFC: E081           asril #0, d1
2AFE: EC81           asril #6, d1
2B00: E089           lsril #0, d1
2B02: E089           lsril #0, d1
2B04: E289           lsril #1, d1
2B06: D280           addrl d0, d1
2B08: E081           asril #0, d1
2B0A: EE81           asril #7, d1
2B0C: 584F           addqw #4, a7
2B0E: 1F01           moveb d1, -(a7)
2B10: 4EBA E576      jsr.ex -31600(pc)
2B14: 544F           addqw #2, a7
2B16: 7000           moveq #0, d0
2B18: 102D F72E      moveb.ex -2258(a5), d0
2B1C: 3200           movew d0, d1
2B1E: E049           lsriw #0, d1
2B20: EE49           lsriw #7, d1
2B22: D240           addrw d0, d1
2B24: E241           asriw #1, d1
2B26: 5441           addqw #2, d1
2B28: 3D41 FFF6      movew.mx d1, -10(a6)
2B2C: 42A7           clrl -(a7)
2B2E: 4E4F           trap #15
2B30: A0C2           sysTrapSysRandom
2B32: C1FC 0064      muls.ex #100, d0
2B36: 2200           movel d0, d1
2B38: E081           asril #0, d1
2B3A: EC81           asril #6, d1
2B3C: E089           lsril #0, d1
2B3E: E089           lsril #0, d1
2B40: E289           lsril #1, d1
2B42: D280           addrl d0, d1
2B44: E081           asril #0, d1
2B46: EE81           asril #7, d1
2B48: 3D41 FFFC      movew.mx d1, -4(a6)
2B4C: B26E FFF6      cmpw.ex -10(a6), d1
2B50: 584F           addqw #4, a7
2B52: 6D9C           blt -100
2B54: 7078           moveq #120, d0
2B56: 2D40 FFF8      movel.mx d0, -8(a6)
2B5A: 4E4F           trap #15
2B5C: A2A2           sysTrapKeyCurrentState
2B5E: C0AE FFF8      andrl.ex -8(a6), d0
2B62: 2A00           movel d0, d5
2B64: 2F05           movel d5, -(a7)
2B66: 4EBA F59C      jsr.ex -27380(pc)
2B6A: 4A2D F72D      tstb.ex -2259(a5)
2B6E: 584F           addqw #4, a7
2B70: 6732           beq 50
2B72: 532D FB88      subqb.ex #1, -1144(a5)
2B76: 4A2D FB88      tstb.ex -1144(a5)
2B7A: 6E28           bgt 40
2B7C: 522D F72D      addqb.ex #1, -2259(a5)
2B80: 1B7C 0001 FB7F moveb.emx #1, -1153(a5)
2B86: 1B7C 0028 FB88 moveb.emx #40, -1144(a5)
2B8C: 0C2D 000A F72D cmpib.ex #10, -2259(a5)
2B92: 6506           bcs 6
2B94: 1B7C 0005 F714 moveb.emx #5, -2284(a5)
2B9A: 1F3C 0006      moveb.ex #6, -(a7)
2B9E: 4EBA DD44      jsr.ex 31980(pc)
2BA2: 544F           addqw #2, a7
2BA4: 4EBA F630      jsr.ex -27170(pc)
2BA8: 4EBA FD34      jsr.ex -25370(pc)
2BAC: 7600           moveq #0, d3
2BAE: 6062           bra 98
2BB0: 41ED F714      lea -2284(a5), d0
2BB4: 41E8 089C      lea 2204(a0), d0
2BB8: 4A30 3000      tstb.ex 0(a0,d3.w)
2BBC: 670C           beq 12
2BBE: 41ED F714      lea -2284(a5), d0
2BC2: 41E8 089C      lea 2204(a0), d0
2BC6: 5330 3000      subqb.ex #1, 0(a0,d3.w)
2BCA: 41ED F714      lea -2284(a5), d0
2BCE: 41E8 089A      lea 2202(a0), d0
2BD2: 0C30 0005 3000 cmpib.ex #5, 0(a0,d3.w)
2BD8: 6436           bcc 54
2BDA: 41ED F714      lea -2284(a5), d0
2BDE: 41E8 089E      lea 2206(a0), d0
2BE2: 5330 3000      subqb.ex #1, 0(a0,d3.w)
2BE6: 41ED F714      lea -2284(a5), d0
2BEA: 41E8 089E      lea 2206(a0), d0
2BEE: 4A30 3000      tstb.ex 0(a0,d3.w)
2BF2: 661C           bne 28
2BF4: 41ED F714      lea -2284(a5), d0
2BF8: 41E8 089A      lea 2202(a0), d0
2BFC: 5230 3000      addqb.ex #1, 0(a0,d3.w)
2C00: 41ED F714      lea -2284(a5), d0
2C04: 41E8 089E      lea 2206(a0), d0
2C08: 11BC 0046 3000 moveb.emx #70, 0(a0,d3.w)
2C0E: 7801           moveq #1, d4
2C10: 5243           addqw #1, d3
2C12: 0C43 0001      cmpiw #1, d3
2C16: 6F98           ble -104
2C18: 7010           moveq #16, d0
2C1A: C085           andrl d5, d0
2C1C: 673E           beq 62
2C1E: 4A2D FB7E      tstb.ex -1154(a5)
2C22: 6738           beq 56
2C24: 0C2D 0005 F72D cmpib.ex #5, -2259(a5)
2C2A: 6430           bcc 48
2C2C: 0C6D 0005 FB76 cmpiw.ex #5, -1162(a5)
2C32: 6D28           blt 40
2C34: 4A2D FFAF      tstb.ex -81(a5)
2C38: 6722           beq 34
2C3A: 4A2D FFB1      tstb.ex -79(a5)
2C3E: 661C           bne 28
2C40: 7C01           moveq #1, d6
2C42: 1F3C 0001      moveb.ex #1, -(a7)
2C46: 4227           clrb -(a7)
2C48: 4EBA DDFC      jsr.ex 32334(pc)
2C4C: 4A00           tstb d0
2C4E: 584F           addqw #4, a7
2C50: 6606           bne 6
2C52: 4A04           tstb d4
2C54: 6602           bne 2
2C56: 7C00           moveq #0, d6
2C58: 1806           moveb d6, d4
2C5A: 6040           bra 64
2C5C: 7020           moveq #32, d0
2C5E: C085           andrl d5, d0
2C60: 673A           beq 58
2C62: 4A2D FB7E      tstb.ex -1154(a5)
2C66: 6734           beq 52
2C68: 0C2D 0005 F72D cmpib.ex #5, -2259(a5)
2C6E: 642C           bcc 44
2C70: 0C6D 009B FB7A cmpiw.ex #155, -1158(a5)
2C76: 6E24           bgt 36
2C78: 4A2D FFAE      tstb.ex -82(a5)
2C7C: 671E           beq 30
2C7E: 4A2D FFB0      tstb.ex -80(a5)
2C82: 6618           bne 24
2C84: 7E01           moveq #1, d7
2C86: 4227           clrb -(a7)
2C88: 4227           clrb -(a7)
2C8A: 4EBA DDBA      jsr.ex 32334(pc)
2C8E: 4A00           tstb d0
2C90: 584F           addqw #4, a7
2C92: 6606           bne 6
2C94: 4A04           tstb d4
2C96: 6602           bne 2
2C98: 7E00           moveq #0, d7
2C9A: 1807           moveb d7, d4
2C9C: 4EBA FCD8      jsr.ex -25218(pc)
2CA0: 4A04           tstb d4
2CA2: 6704           beq 4
2CA4: 4EBA E84A      jsr.ex -30472(pc)
2CA8: 4EBA F32C      jsr.ex -27682(pc)
2CAC: 4A2D FB7F      tstb.ex -1153(a5)
2CB0: 670E           beq 14
2CB2: 0C6D 0015 FB90 cmpiw.ex #21, -1136(a5)
2CB8: 6C06           bge 6
2CBA: 1B7C 0001 FB97 moveb.emx #1, -1129(a5)
2CC0: 4CDF 00F8      movem (a7)+, <00f8>
2CC4: 4E5E           unlk a6
2CC6: 4E75           rts
2CC8: 8F47           dc.w #36679
2CCA: 616D           bsr 109
2CCC: 6553           bcs 83
2CCE: 7461           moveq #97, d2
2CD0: 7465           moveq #101, d2
2CD2: 456C           dc.w #17772
2CD4: 6170           bsr 112
2CD6: 7365           dc.w #29541
2CD8: 0000           dc.w #0
2CDA: 4E56 FFF6      link a6, #-10
2CDE: 4A2D FFB6      tstb.ex -74(a5)
2CE2: 6754           beq 84
2CE4: 1D7C 0003 FFF6 moveb.emx #3, -10(a6)
2CEA: 7000           moveq #0, d0
2CEC: 102D FFB6      moveb.ex -74(a5), d0
2CF0: E788           lslil #3, d0
2CF2: 41ED FFBC      lea -68(a5), d0
2CF6: 5488           addql #2, a0
2CF8: 2D70 0800 FFF8 movel.emx 0(a0,d0.l), -8(a6)
2CFE: 7000           moveq #0, d0
2D00: 102D FFB6      moveb.ex -74(a5), d0
2D04: E788           lslil #3, d0
2D06: 41ED FFBC      lea -68(a5), d0
2D0A: 5C88           addql #6, a0
2D0C: 3D70 0800 FFFC movew.emx 0(a0,d0.l), -4(a6)
2D12: 3D6D F62C FFFE movew.emx -2516(a5), -2(a6)
2D18: 1F3C 0001      moveb.ex #1, -(a7)
2D1C: 486E FFF6      pea.ex -10(a6)
2D20: 42A7           clrl -(a7)
2D22: 4E4F           trap #15
2D24: A233           sysTrapSndDoCmd
2D26: 532D FFB7      subqb.ex #1, -73(a5)
2D2A: 4A2D FFB7      tstb.ex -73(a5)
2D2E: 4FEF 000A      lea 10(a7), d7
2D32: 6E04           bgt 4
2D34: 422D FFB6      clrb.ex -74(a5)
2D38: 4E5E           unlk a6
2D3A: 4E75           rts
2D3C: 8E47           orrw d7, d7
2D3E: 616D           bsr 109
2D40: 6550           bcs 80
2D42: 6C61           bge 97
2D44: 7953           dc.w #31059
2D46: 6F75           ble 117
2D48: 6E64           bgt 100
2D4A: 7300           dc.w #29440
2D4C: 0000           dc.w #0
2D4E: 4E56 FFF8      link a6, #-8
2D52: 2F03           movel d3, -(a7)
2D54: 7600           moveq #0, d3
2D56: 602C           bra 44
2D58: 7016           moveq #22, d0
2D5A: C1C3           muls d3, d0
2D5C: 41ED F556      lea -2730(a5), d0
2D60: 4230 0800      clrb.ex 0(a0,d0.l)
2D64: 7016           moveq #22, d0
2D66: C1C3           muls d3, d0
2D68: 5088           addql #0, a0
2D6A: 5088           addql #0, a0
2D6C: 42B0 0800      clrl.ex 0(a0,d0.l)
2D70: 7016           moveq #22, d0
2D72: C1C3           muls d3, d0
2D74: 41ED F556      lea -2730(a5), d0
2D78: 41E8 0014      lea 20(a0), d0
2D7C: 31BC 0001 0800 movew.emx #1, 0(a0,d0.l)
2D82: 5243           addqw #1, d3
2D84: 0C43 0009      cmpiw #9, d3
2D88: 6DCE           blt -50
2D8A: 426E FFF8      clrw.ex -8(a6)
2D8E: 3D7C 001C FFFA movew.emx #28, -6(a6)
2D94: 3D7C 0095 FFFC movew.emx #149, -4(a6)
2D9A: 3D7C 006C FFFE movew.emx #108, -2(a6)
2DA0: 4267           clrw -(a7)
2DA2: 486E FFF8      pea.ex -8(a6)
2DA6: 4E4F           trap #15
2DA8: A219           sysTrapWinEraseRectangle
2DAA: 5C4F           addqw #6, a7
2DAC: 261F           movel (a7)+, d3
2DAE: 4E5E           unlk a6
2DB0: 4E75           rts
2DB2: 8F48           dc.w #36680
2DB4: 6967           bvs 103
2DB6: 6853           bvc 83
2DB8: 636F           bls 111
2DBA: 7265           moveq #101, d1
2DBC: 7343           dc.w #29507
2DBE: 6C65           bge 101
2DC0: 6172           bsr 114
2DC2: 0000           dc.w #0
2DC4: 4E56 0000      link a6, #0
2DC8: 2F0A           movel a2, -(a7)
2DCA: 246E 0008      movel.ex 8(a6), a2
2DCE: 0C52 0009      cmpiw #9, (a2)
2DD2: 661E           bne 30
2DD4: 0C6A 044F 0008 cmpiw.ex #1103, 8(a2)
2DDA: 6616           bne 22
2DDC: 3F3C 03E8      movew.ex #1000, -(a7)
2DE0: 4E4F           trap #15
2DE2: A192           sysTrapFrmAlert
2DE4: 4A40           tstw d0
2DE6: 544F           addqw #2, a7
2DE8: 6604           bne 4
2DEA: 4EBA FF62      jsr.ex -24234(pc)
2DEE: 7001           moveq #1, d0
2DF0: 6002           bra 2
2DF2: 7000           moveq #0, d0
2DF4: 245F           movel (a7)+, a2
2DF6: 4E5E           unlk a6
2DF8: 4E75           rts
2DFA: 9648           subrw a0, d3
2DFC: 6967           bvs 103
2DFE: 6853           bvc 83
2E00: 636F           bls 111
2E02: 7265           moveq #101, d1
2E04: 7345           dc.w #29509
2E06: 7665           moveq #101, d3
2E08: 6E74           bgt 116
2E0A: 4861           dc.w #18529
2E0C: 6E64           bgt 100
2E0E: 6C65           bge 101
2E10: 7200           moveq #0, d1
2E12: 0000           dc.w #0
2E14: 4E56 FFF0      link a6, #-16
2E18: 48E7 1E38      movem <1e38>, -(a7)
2E1C: 4E4F           trap #15
2E1E: A173           sysTrapFrmGetActiveForm
2E20: 2A08           movel a0, d5
2E22: 3F3C 044C      movew.ex #1100, -(a7)
2E26: 4E4F           trap #15
2E28: A16F           sysTrapFrmInitForm
2E2A: 2648           movel a0, a3
2E2C: 2F0B           movel a3, -(a7)
2E2E: 4E4F           trap #15
2E30: A174           sysTrapFrmSetActiveForm
2E32: 2F0B           movel a3, -(a7)
2E34: 4E4F           trap #15
2E36: A171           sysTrapFrmDrawForm
2E38: 1F3C 0001      moveb.ex #1, -(a7)
2E3C: 4E4F           trap #15
2E3E: A164           sysTrapFntSetFont
2E40: 1C00           moveb d0, d6
2E42: 3F3C 044C      movew.ex #1100, -(a7)
2E46: 2F3C 7453 5452 movel.exl #1951618130, -(a7)
2E4C: 4E4F           trap #15
2E4E: A05F           sysTrapDmGetResource
2E50: 2848           movel a0, a4
2E52: 2F0C           movel a4, -(a7)
2E54: 4E4F           trap #15
2E56: A021           sysTrapMemHandleLock
2E58: 2448           movel a0, a2
2E5A: 2F3C 0011 0010 movel.exl #1114128, -(a7)
2E60: 2F0A           movel a2, -(a7)
2E62: 4E4F           trap #15
2E64: A0C7           sysTrapStrLen
2E66: 584F           addqw #4, a7
2E68: 3F00           movew d0, -(a7)
2E6A: 2F0A           movel a2, -(a7)
2E6C: 4E4F           trap #15
2E6E: A220           sysTrapWinDrawChars
2E70: 2F0A           movel a2, -(a7)
2E72: 4E4F           trap #15
2E74: A035           sysTrapMemPtrUnlock
2E76: 3F3C 044D      movew.ex #1101, -(a7)
2E7A: 2F3C 7453 5452 movel.exl #1951618130, -(a7)
2E80: 4E4F           trap #15
2E82: A05F           sysTrapDmGetResource
2E84: 2848           movel a0, a4
2E86: 2F0C           movel a4, -(a7)
2E88: 4E4F           trap #15
2E8A: A021           sysTrapMemHandleLock
2E8C: 2448           movel a0, a2
2E8E: 3F3C 0010      movew.ex #16, -(a7)
2E92: 2F0A           movel a2, -(a7)
2E94: 4E4F           trap #15
2E96: A0C7           sysTrapStrLen
2E98: 584F           addqw #4, a7
2E9A: 3F00           movew d0, -(a7)
2E9C: 2F0A           movel a2, -(a7)
2E9E: 4E4F           trap #15
2EA0: A16B           sysTrapFntCharsWidth
2EA2: 7277           moveq #119, d1
2EA4: 9240           subrw d0, d1
2EA6: 5C4F           addqw #6, a7
2EA8: 3F01           movew d1, -(a7)
2EAA: 2F0A           movel a2, -(a7)
2EAC: 4E4F           trap #15
2EAE: A0C7           sysTrapStrLen
2EB0: 584F           addqw #4, a7
2EB2: 3F00           movew d0, -(a7)
2EB4: 2F0A           movel a2, -(a7)
2EB6: 4E4F           trap #15
2EB8: A220           sysTrapWinDrawChars
2EBA: 2F0A           movel a2, -(a7)
2EBC: 4E4F           trap #15
2EBE: A035           sysTrapMemPtrUnlock
2EC0: 3F3C 044E      movew.ex #1102, -(a7)
2EC4: 2F3C 7453 5452 movel.exl #1951618130, -(a7)
2ECA: 4E4F           trap #15
2ECC: A05F           sysTrapDmGetResource
2ECE: 2848           movel a0, a4
2ED0: 2F0C           movel a4, -(a7)
2ED2: 4E4F           trap #15
2ED4: A021           sysTrapMemHandleLock
2ED6: 2448           movel a0, a2
2ED8: 3F3C 0010      movew.ex #16, -(a7)
2EDC: 2F0A           movel a2, -(a7)
2EDE: 4E4F           trap #15
2EE0: A0C7           sysTrapStrLen
2EE2: 584F           addqw #4, a7
2EE4: 3F00           movew d0, -(a7)
2EE6: 2F0A           movel a2, -(a7)
2EE8: 4E4F           trap #15
2EEA: A16B           sysTrapFntCharsWidth
2EEC: 323C 0095      movew.ex #149, d1
2EF0: 9240           subrw d0, d1
2EF2: 5C4F           addqw #6, a7
2EF4: 3F01           movew d1, -(a7)
2EF6: 2F0A           movel a2, -(a7)
2EF8: 4E4F           trap #15
2EFA: A0C7           sysTrapStrLen
2EFC: 584F           addqw #4, a7
2EFE: 3F00           movew d0, -(a7)
2F00: 2F0A           movel a2, -(a7)
2F02: 4E4F           trap #15
2F04: A220           sysTrapWinDrawChars
2F06: 2F0A           movel a2, -(a7)
2F08: 4E4F           trap #15
2F0A: A035           sysTrapMemPtrUnlock
2F0C: 2F3C 0095 001B movel.exl #9764891, -(a7)
2F12: 2F3C 0011 001B movel.exl #1114139, -(a7)
2F18: 4E4F           trap #15
2F1A: A213           sysTrapWinDrawLine
2F1C: 7600           moveq #0, d3
2F1E: 4FEF 005C      lea 92(a7), d7
2F22: 6000 0138      bra 312
2F26: 780C           moveq #12, d4
2F28: C9C3           muls d3, d4
2F2A: 0644 001C      addiw #28, d4
2F2E: 7000           moveq #0, d0
2F30: 102D F61C      moveb.ex -2532(a5), d0
2F34: B640           cmpw d0, d3
2F36: 660C           bne 12
2F38: 1F3C 0001      moveb.ex #1, -(a7)
2F3C: 4E4F           trap #15
2F3E: A164           sysTrapFntSetFont
2F40: 544F           addqw #2, a7
2F42: 6008           bra 8
2F44: 4227           clrb -(a7)
2F46: 4E4F           trap #15
2F48: A164           sysTrapFntSetFont
2F4A: 544F           addqw #2, a7
2F4C: 3003           movew d3, d0
2F4E: 5240           addqw #1, d0
2F50: 48C0           extl d0
2F52: 2F00           movel d0, -(a7)
2F54: 486E FFF0      pea.ex -16(a6)
2F58: 4E4F           trap #15
2F5A: A0C9           sysTrapStrIToA
2F5C: 41FA 015A      lea -23360(pc), d0
2F60: 4850           pea (a0)
2F62: 486E FFF0      pea.ex -16(a6)
2F66: 4E4F           trap #15
2F68: A0C6           sysTrapStrCat
2F6A: 3F04           movew d4, -(a7)
2F6C: 486E FFF0      pea.ex -16(a6)
2F70: 4E4F           trap #15
2F72: A0C7           sysTrapStrLen
2F74: 584F           addqw #4, a7
2F76: 3F00           movew d0, -(a7)
2F78: 486E FFF0      pea.ex -16(a6)
2F7C: 4E4F           trap #15
2F7E: A16B           sysTrapFntCharsWidth
2F80: 7211           moveq #17, d1
2F82: 9240           subrw d0, d1
2F84: 5C4F           addqw #6, a7
2F86: 3F01           movew d1, -(a7)
2F88: 486E FFF0      pea.ex -16(a6)
2F8C: 4E4F           trap #15
2F8E: A0C7           sysTrapStrLen
2F90: 584F           addqw #4, a7
2F92: 3F00           movew d0, -(a7)
2F94: 486E FFF0      pea.ex -16(a6)
2F98: 4E4F           trap #15
2F9A: A220           sysTrapWinDrawChars
2F9C: 3F04           movew d4, -(a7)
2F9E: 3F3C 0011      movew.ex #17, -(a7)
2FA2: 7016           moveq #22, d0
2FA4: C1C3           muls d3, d0
2FA6: 41ED F556      lea -2730(a5), d0
2FAA: 4870 0800      pea.ex 0(a0,d0.l)
2FAE: 4E4F           trap #15
2FB0: A0C7           sysTrapStrLen
2FB2: 584F           addqw #4, a7
2FB4: 3F00           movew d0, -(a7)
2FB6: 7016           moveq #22, d0
2FB8: C1C3           muls d3, d0
2FBA: 41ED F556      lea -2730(a5), d0
2FBE: 4870 0800      pea.ex 0(a0,d0.l)
2FC2: 4E4F           trap #15
2FC4: A220           sysTrapWinDrawChars
2FC6: 7016           moveq #22, d0
2FC8: C1C3           muls d3, d0
2FCA: 41ED F556      lea -2730(a5), d0
2FCE: 2F30 0810      movel.ex 16(a0,d0.l), -(a7)
2FD2: 486E FFF0      pea.ex -16(a6)
2FD6: 4E4F           trap #15
2FD8: A0C9           sysTrapStrIToA
2FDA: 3F04           movew d4, -(a7)
2FDC: 486E FFF0      pea.ex -16(a6)
2FE0: 4E4F           trap #15
2FE2: A0C7           sysTrapStrLen
2FE4: 584F           addqw #4, a7
2FE6: 3F00           movew d0, -(a7)
2FE8: 486E FFF0      pea.ex -16(a6)
2FEC: 4E4F           trap #15
2FEE: A16B           sysTrapFntCharsWidth
2FF0: 7277           moveq #119, d1
2FF2: 9240           subrw d0, d1
2FF4: 5C4F           addqw #6, a7
2FF6: 3F01           movew d1, -(a7)
2FF8: 486E FFF0      pea.ex -16(a6)
2FFC: 4E4F           trap #15
2FFE: A0C7           sysTrapStrLen
3000: 584F           addqw #4, a7
3002: 3F00           movew d0, -(a7)
3004: 486E FFF0      pea.ex -16(a6)
3008: 4E4F           trap #15
300A: A220           sysTrapWinDrawChars
300C: 7016           moveq #22, d0
300E: C1C3           muls d3, d0
3010: 41ED F556      lea -2730(a5), d0
3014: 3270 0814      movew.ex 20(a0,d0.l), a1
3018: 2F09           movel a1, -(a7)
301A: 486E FFF0      pea.ex -16(a6)
301E: 4E4F           trap #15
3020: A0C9           sysTrapStrIToA
3022: 3F04           movew d4, -(a7)
3024: 486E FFF0      pea.ex -16(a6)
3028: 4E4F           trap #15
302A: A0C7           sysTrapStrLen
302C: 584F           addqw #4, a7
302E: 3F00           movew d0, -(a7)
3030: 486E FFF0      pea.ex -16(a6)
3034: 4E4F           trap #15
3036: A16B           sysTrapFntCharsWidth
3038: 323C 0095      movew.ex #149, d1
303C: 9240           subrw d0, d1
303E: 5C4F           addqw #6, a7
3040: 3F01           movew d1, -(a7)
3042: 486E FFF0      pea.ex -16(a6)
3046: 4E4F           trap #15
3048: A0C7           sysTrapStrLen
304A: 584F           addqw #4, a7
304C: 3F00           movew d0, -(a7)
304E: 486E FFF0      pea.ex -16(a6)
3052: 4E4F           trap #15
3054: A220           sysTrapWinDrawChars
3056: 5243           addqw #1, d3
3058: 4FEF 0048      lea 72(a7), d7
305C: 0C43 0009      cmpiw #9, d3
3060: 6C14           bge 20
3062: 7016           moveq #22, d0
3064: C1C3           muls d3, d0
3066: 41ED F556      lea -2730(a5), d0
306A: 5088           addql #0, a0
306C: 5088           addql #0, a0
306E: 4AB0 0800      tstl.ex 0(a0,d0.l)
3072: 6E00 FEB2      bgt 65202
3076: 1F06           moveb d6, -(a7)
3078: 4E4F           trap #15
307A: A164           sysTrapFntSetFont
307C: 487A FD46      pea.ex -24116(pc)
3080: 2F0B           movel a3, -(a7)
3082: 4E4F           trap #15
3084: A19F           sysTrapFrmSetEventHandler
3086: 2F0B           movel a3, -(a7)
3088: 4E4F           trap #15
308A: A193           sysTrapFrmDoDialog
308C: 2F0B           movel a3, -(a7)
308E: 4E4F           trap #15
3090: A170           sysTrapFrmDeleteForm
3092: 2F05           movel d5, -(a7)
3094: 4E4F           trap #15
3096: A174           sysTrapFrmSetActiveForm
3098: 4FEF 0016      lea 22(a7), d7
309C: 4CDF 1C78      movem (a7)+, <1c78>
30A0: 4E5E           unlk a6
30A2: 4E75           rts
30A4: 9148           subxmw -(a0), -(a0)
30A6: 6967           bvs 103
30A8: 6853           bvc 83
30AA: 636F           bls 111
30AC: 7265           moveq #101, d1
30AE: 7344           dc.w #29508
30B0: 6973           bvs 115
30B2: 706C           moveq #108, d0
30B4: 6179           bsr 121
30B6: 0004           dc.w #4
30B8: 2E20           movel -(a0), d7
30BA: 0000           dc.w #0
30BC: 4E56 0000      link a6, #0
30C0: 48E7 1E20      movem <1e20>, -(a7)
30C4: 246E 0008      movel.ex 8(a6), a2
30C8: 282E 000C      movel.ex 12(a6), d4
30CC: 3A2E 0010      movew.ex 16(a6), d5
30D0: 1C2E 0012      moveb.ex 18(a6), d6
30D4: 7609           moveq #9, d3
30D6: 6002           bra 2
30D8: 5343           subqw #1, d3
30DA: 4A43           tstw d3
30DC: 6F16           ble 22
30DE: 3003           movew d3, d0
30E0: 5340           subqw #1, d0
30E2: C1FC 0016      muls.ex #22, d0
30E6: 41ED F556      lea -2730(a5), d0
30EA: 5088           addql #0, a0
30EC: 5088           addql #0, a0
30EE: B8B0 0800      cmpl.ex 0(a0,d0.l), d4
30F2: 6EE4           bgt -28
30F4: 0C43 0009      cmpiw #9, d3
30F8: 6C00 00B6      bge 182
30FC: 4A06           tstb d6
30FE: 674C           beq 76
3100: 4A43           tstw d3
3102: 6F48           ble 72
3104: 3003           movew d3, d0
3106: 5340           subqw #1, d0
3108: C1FC 0016      muls.ex #22, d0
310C: 41ED F556      lea -2730(a5), d0
3110: 4870 0800      pea.ex 0(a0,d0.l)
3114: 2F0A           movel a2, -(a7)
3116: 4E4F           trap #15
3118: A0C8           sysTrapStrCompare
311A: 4A40           tstw d0
311C: 504F           addqw #0, a7
311E: 662C           bne 44
3120: 3003           movew d3, d0
3122: 5340           subqw #1, d0
3124: C1FC 0016      muls.ex #22, d0
3128: 41ED F556      lea -2730(a5), d0
312C: 5088           addql #0, a0
312E: 5088           addql #0, a0
3130: B8B0 0800      cmpl.ex 0(a0,d0.l), d4
3134: 6616           bne 22
3136: 3003           movew d3, d0
3138: 5340           subqw #1, d0
313A: C1FC 0016      muls.ex #22, d0
313E: 41ED F556      lea -2730(a5), d0
3142: 41E8 0014      lea 20(a0), d0
3146: BA70 0800      cmpw.ex 0(a0,d0.l), d5
314A: 6764           beq 100
314C: 7008           moveq #8, d0
314E: 9043           subrw d3, d0
3150: C0FC 0016      mulu.ex #22, d0
3154: 2F00           movel d0, -(a7)
3156: 7016           moveq #22, d0
3158: C1C3           muls d3, d0
315A: 41ED F556      lea -2730(a5), d0
315E: D1C0           addal d0, a0
3160: 4850           pea (a0)
3162: 3003           movew d3, d0
3164: 5240           addqw #1, d0
3166: C1FC 0016      muls.ex #22, d0
316A: 41ED F556      lea -2730(a5), d0
316E: D1C0           addal d0, a0
3170: 4850           pea (a0)
3172: 4E4F           trap #15
3174: A026           sysTrapMemMove
3176: 7016           moveq #22, d0
3178: C1C3           muls d3, d0
317A: 41ED F556      lea -2730(a5), d0
317E: 5088           addql #0, a0
3180: 5088           addql #0, a0
3182: 2184 0800      movel.mx d4, 0(a0,d0.l)
3186: 7016           moveq #22, d0
3188: C1C3           muls d3, d0
318A: 41ED F556      lea -2730(a5), d0
318E: 41E8 0014      lea 20(a0), d0
3192: 3185 0800      movew.mx d5, 0(a0,d0.l)
3196: 2F0A           movel a2, -(a7)
3198: 7016           moveq #22, d0
319A: C1C3           muls d3, d0
319C: 41ED F556      lea -2730(a5), d0
31A0: 4870 0800      pea.ex 0(a0,d0.l)
31A4: 4E4F           trap #15
31A6: A0C5           sysTrapStrCopy
31A8: 1B43 F61C      moveb.mx d3, -2532(a5)
31AC: 4FEF 0014      lea 20(a7), d7
31B0: 4CDF 0478      movem (a7)+, <0478>
31B4: 4E5E           unlk a6
31B6: 4E75           rts
31B8: 9248           subrw a0, d1
31BA: 6967           bvs 103
31BC: 6853           bvc 83
31BE: 636F           bls 111
31C0: 7265           moveq #101, d1
31C2: 7341           dc.w #29505
31C4: 6464           bcc 100
31C6: 5363           subqw #1, -(a3)
31C8: 6F72           ble 114
31CA: 6500 0000      bcs 0
31CE: 4E56 0000      link a6, #0
31D2: 48E7 1F38      movem <1f38>, -(a7)
31D6: 7609           moveq #9, d3
31D8: 6002           bra 2
31DA: 5343           subqw #1, d3
31DC: 4A43           tstw d3
31DE: 6F1A           ble 26
31E0: 3003           movew d3, d0
31E2: 5340           subqw #1, d0
31E4: C1FC 0016      muls.ex #22, d0
31E8: 41ED F556      lea -2730(a5), d0
31EC: 5088           addql #0, a0
31EE: 5088           addql #0, a0
31F0: 222D FB70      movel.ex -1168(a5), d1
31F4: B2B0 0800      cmpl.ex 0(a0,d0.l), d1
31F8: 6EE0           bgt -32
31FA: 0C43 0009      cmpiw #9, d3
31FE: 6C00 011E      bge 286
3202: 4878 0029      pea.ex (0029).w
3206: 4E4F           trap #15
3208: A01E           sysTrapMemHandleNew
320A: 2848           movel a0, a4
320C: 2F0C           movel a4, -(a7)
320E: 4E4F           trap #15
3210: A021           sysTrapMemHandleLock
3212: 2448           movel a0, a2
3214: 0C2D 0009 F61C cmpib.ex #9, -2532(a5)
321A: 504F           addqw #0, a7
321C: 671C           beq 28
321E: 7000           moveq #0, d0
3220: 102D F61C      moveb.ex -2532(a5), d0
3224: C1FC 0016      muls.ex #22, d0
3228: 41ED F556      lea -2730(a5), d0
322C: 4870 0800      pea.ex 0(a0,d0.l)
3230: 2F0A           movel a2, -(a7)
3232: 4E4F           trap #15
3234: A0C5           sysTrapStrCopy
3236: 504F           addqw #0, a7
3238: 6038           bra 56
323A: 41FA 0102      lea -22714(pc), d0
323E: 4850           pea (a0)
3240: 2F0A           movel a2, -(a7)
3242: 4E4F           trap #15
3244: A0C5           sysTrapStrCopy
3246: 42A7           clrl -(a7)
3248: 42A7           clrl -(a7)
324A: 2F0A           movel a2, -(a7)
324C: 42A7           clrl -(a7)
324E: 42A7           clrl -(a7)
3250: 42A7           clrl -(a7)
3252: 4E4F           trap #15
3254: A2A9           sysTrapDlkGetSyncInfo
3256: 3F3C 0020      movew.ex #32, -(a7)
325A: 2F0A           movel a2, -(a7)
325C: 4E4F           trap #15
325E: A0CC           sysTrapStrChr
3260: 2A08           movel a0, d5
3262: 4A85           tstl d5
3264: 4FEF 0026      lea 38(a7), d7
3268: 6704           beq 4
326A: 2045           movel d5, a0
326C: 4210           clrb (a0)
326E: 422A 000F      clrb.ex 15(a2)
3272: 2F0A           movel a2, -(a7)
3274: 4E4F           trap #15
3276: A035           sysTrapMemPtrUnlock
3278: 1B43 F61C      moveb.mx d3, -2532(a5)
327C: 4E4F           trap #15
327E: A173           sysTrapFrmGetActiveForm
3280: 2E08           movel a0, d7
3282: 3F3C 04B0      movew.ex #1200, -(a7)
3286: 4E4F           trap #15
3288: A16F           sysTrapFrmInitForm
328A: 2648           movel a0, a3
328C: 3F3C 04B3      movew.ex #1203, -(a7)
3290: 2F0B           movel a3, -(a7)
3292: 4E4F           trap #15
3294: A180           sysTrapFrmGetObjectIndex
3296: 3800           movew d0, d4
3298: 2F0C           movel a4, -(a7)
329A: 3F04           movew d4, -(a7)
329C: 2F0B           movel a3, -(a7)
329E: 4E4F           trap #15
32A0: A183           sysTrapFrmGetObjectPtr
32A2: 5C4F           addqw #6, a7
32A4: 2F08           movel a0, -(a7)
32A6: 4E4F           trap #15
32A8: A158           sysTrapFldSetTextHandle
32AA: 3F04           movew d4, -(a7)
32AC: 2F0B           movel a3, -(a7)
32AE: 4E4F           trap #15
32B0: A179           sysTrapFrmSetFocus
32B2: 1F3C 0001      moveb.ex #1, -(a7)
32B6: 4227           clrb -(a7)
32B8: 4227           clrb -(a7)
32BA: 4E4F           trap #15
32BC: A281           sysTrapGrfSetState
32BE: 2F0B           movel a3, -(a7)
32C0: 4E4F           trap #15
32C2: A193           sysTrapFrmDoDialog
32C4: 3C00           movew d0, d6
32C6: 42A7           clrl -(a7)
32C8: 3F04           movew d4, -(a7)
32CA: 2F0B           movel a3, -(a7)
32CC: 4E4F           trap #15
32CE: A183           sysTrapFrmGetObjectPtr
32D0: 5C4F           addqw #6, a7
32D2: 2F08           movel a0, -(a7)
32D4: 4E4F           trap #15
32D6: A158           sysTrapFldSetTextHandle
32D8: 2F0B           movel a3, -(a7)
32DA: 4E4F           trap #15
32DC: A170           sysTrapFrmDeleteForm
32DE: 2F07           movel d7, -(a7)
32E0: 4E4F           trap #15
32E2: A174           sysTrapFrmSetActiveForm
32E4: 0C46 04B4      cmpiw #1204, d6
32E8: 4FEF 0034      lea 52(a7), d7
32EC: 661E           bne 30
32EE: 4A12           tstb (a2)
32F0: 671A           beq 26
32F2: 4227           clrb -(a7)
32F4: 7000           moveq #0, d0
32F6: 102D F72E      moveb.ex -2258(a5), d0
32FA: 5240           addqw #1, d0
32FC: 3F00           movew d0, -(a7)
32FE: 2F2D FB70      movel.ex -1168(a5), -(a7)
3302: 2F0A           movel a2, -(a7)
3304: 4EBA FDB6      jsr.ex -23356(pc)
3308: 4FEF 000C      lea 12(a7), d7
330C: 2F0C           movel a4, -(a7)
330E: 4E4F           trap #15
3310: A02B           sysTrapMemHandleFree
3312: 0C46 04B4      cmpiw #1204, d6
3316: 584F           addqw #4, a7
3318: 6604           bne 4
331A: 4EBA FAF8      jsr.ex -24036(pc)
331E: 4CDF 1CF8      movem (a7)+, <1cf8>
3322: 4E5E           unlk a6
3324: 4E75           rts
3326: 9448           subrw a0, d2
3328: 6967           bvs 103
332A: 6853           bvc 83
332C: 636F           bls 111
332E: 7265           moveq #101, d1
3330: 7343           dc.w #29507
3332: 6865           bvc 101
3334: 636B           bls 107
3336: 5363           subqw #1, -(a3)
3338: 6F72           ble 114
333A: 6500 0002      bcs 2
333E: 0000           dc.w #0
3340: 4E56 0000      link a6, #0
3344: 48E7 1030      movem <1030>, -(a7)
3348: 266E 0008      movel.ex 8(a6), a3
334C: 1B6D F61D F623 moveb.emx -2531(a5), -2525(a5)
3352: 3F3C 057B      movew.ex #1403, -(a7)
3356: 2F0B           movel a3, -(a7)
3358: 4E4F           trap #15
335A: A180           sysTrapFrmGetObjectIndex
335C: 5C4F           addqw #6, a7
335E: 3F00           movew d0, -(a7)
3360: 2F0B           movel a3, -(a7)
3362: 4E4F           trap #15
3364: A183           sysTrapFrmGetObjectPtr
3366: 2448           movel a0, a2
3368: 2F3C 0003 0000 movel.exl #196608, -(a7)
336E: 1F2D F61D      moveb.ex -2531(a5), -(a7)
3372: 486D FFFC      pea.ex -4(a5)
3376: 4EBA D392      jsr.ex 31506(pc)
337A: 3600           movew d0, d3
337C: 3F03           movew d3, -(a7)
337E: 2F0A           movel a2, -(a7)
3380: 4E4F           trap #15
3382: A1B7           sysTrapLstSetSelection
3384: 4FEF 0016      lea 22(a7), d7
3388: 4CDF 0C08      movem (a7)+, <0c08>
338C: 4E5E           unlk a6
338E: 4E75           rts
3390: 9550           submw d2, (a0)
3392: 7265           moveq #101, d1
3394: 6665           bne 101
3396: 7265           moveq #101, d1
3398: 6E63           bgt 99
339A: 6573           bcs 115
339C: 4469 616C      negw.ex 24940(a1)
33A0: 6F67           ble 103
33A2: 496E           dc.w #18798
33A4: 6974           bvs 116
33A6: 0000           dc.w #0
33A8: 4E56 0000      link a6, #0
33AC: 48E7 1030      movem <1030>, -(a7)
33B0: 246E 0008      movel.ex 8(a6), a2
33B4: 7600           moveq #0, d3
33B6: 0C52 000C      cmpiw #12, (a2)
33BA: 6620           bne 32
33BC: 302A 0008      movew.ex 8(a2), d0
33C0: 0440 057B      subiw #1403, d0
33C4: 6702           beq 2
33C6: 6046           bra 70
33C8: 7000           moveq #0, d0
33CA: 302A 000E      movew.ex 14(a2), d0
33CE: 41ED FFFC      lea -4(a5), d0
33D2: 1B70 0800 F623 moveb.emx 0(a0,d0.l), -2525(a5)
33D8: 7601           moveq #1, d3
33DA: 6032           bra 50
33DC: 0C52 0009      cmpiw #9, (a2)
33E0: 662C           bne 44
33E2: 302A 0008      movew.ex 8(a2), d0
33E6: 0440 057C      subiw #1404, d0
33EA: 6706           beq 6
33EC: 5340           subqw #1, d0
33EE: 6714           beq 20
33F0: 601C           bra 28
33F2: 1B6D F623 F61D moveb.emx -2525(a5), -2531(a5)
33F8: 4267           clrw -(a7)
33FA: 4E4F           trap #15
33FC: A19E           sysTrapFrmReturnToForm
33FE: 7601           moveq #1, d3
3400: 544F           addqw #2, a7
3402: 600A           bra 10
3404: 4267           clrw -(a7)
3406: 4E4F           trap #15
3408: A19E           sysTrapFrmReturnToForm
340A: 7601           moveq #1, d3
340C: 544F           addqw #2, a7
340E: 0C52 0018      cmpiw #24, (a2)
3412: 6616           bne 22
3414: 4E4F           trap #15
3416: A173           sysTrapFrmGetActiveForm
3418: 2648           movel a0, a3
341A: 2F0B           movel a3, -(a7)
341C: 4EBA FF22      jsr.ex -22712(pc)
3420: 2F0B           movel a3, -(a7)
3422: 4E4F           trap #15
3424: A171           sysTrapFrmDrawForm
3426: 7601           moveq #1, d3
3428: 504F           addqw #0, a7
342A: 1003           moveb d3, d0
342C: 4CDF 0C08      movem (a7)+, <0c08>
3430: 4E5E           unlk a6
3432: 4E75           rts
3434: 9C50           subrw (a0), d6
3436: 7265           moveq #101, d1
3438: 6665           bne 101
343A: 7265           moveq #101, d1
343C: 6E63           bgt 99
343E: 6573           bcs 115
3440: 4469 616C      negw.ex 24940(a1)
3444: 6F67           ble 103
3446: 4861           dc.w #18529
3448: 6E64           bgt 100
344A: 6C65           bge 101
344C: 4576           dc.w #17782
344E: 656E           bcs 110
3450: 7400           moveq #0, d2
3452: 0000           dc.w #0
3454: 4E56 0000      link a6, #0
3458: 302E 0008      movew.ex 8(a6), d0
345C: 0440 0064      subiw #100, d0
3460: 0C40 0005      cmpiw #5, d0
3464: 624A           bhi 74
3466: D040           addrw d0, d0
3468: 303B 0006      movew.ex 6(pc,d0.w), d0
346C: 4EFB 0002      jmp.ex 2(pc,d0.w)
3470: 000C           dc.w #12
3472: 0024 0030      orib #48, -(a4)
3476: 0036 0040 0016 orib.ex #64, 22(a6,d0.w)
347C: 4EBA EA54      jsr.ex -27942(pc)
3480: 4EBA E25C      jsr.ex -29978(pc)
3484: 602A           bra 42
3486: 2F3C 7368 646B movel.exl #1936221291, -(a7)
348C: 4E4F           trap #15
348E: A2AF           sysTrapAbtShowAbout
3490: 584F           addqw #4, a7
3492: 601C           bra 28
3494: 3F3C 03E8      movew.ex #1000, -(a7)
3498: 4E4F           trap #15
349A: A195           sysTrapFrmHelp
349C: 544F           addqw #2, a7
349E: 6010           bra 16
34A0: 4EBA F972      jsr.ex -24036(pc)
34A4: 600A           bra 10
34A6: 3F3C 0578      movew.ex #1400, -(a7)
34AA: 4E4F           trap #15
34AC: A19C           sysTrapFrmPopupForm
34AE: 544F           addqw #2, a7
34B0: 7001           moveq #1, d0
34B2: 4E5E           unlk a6
34B4: 4E75           rts
34B6: 9242           subrw d2, d1
34B8: 6F61           ble 97
34BA: 7264           moveq #100, d1
34BC: 5669 6577      addqw.ex #3, 25975(a1)
34C0: 446F 436F      negw.ex 17263(a7)
34C4: 6D6D           blt 109
34C6: 616E           bsr 110
34C8: 6400 0000      bcc 0
34CC: 4E56 FFE4      link a6, #-28
34D0: 48E7 1830      movem <1830>, -(a7)
34D4: 246E 0008      movel.ex 8(a6), a2
34D8: 7600           moveq #0, d3
34DA: 4A52           tstw (a2)
34DC: 6700 0124      beq 292
34E0: 0C52 0004      cmpiw #4, (a2)
34E4: 6600 00D6      bne 214
34E8: 0C6A 0073 0008 cmpiw.ex #115, 8(a2)
34EE: 660A           bne 10
34F0: 1B7C 0004 F714 moveb.emx #4, -2284(a5)
34F6: 6000 00C0      bra 192
34FA: 0C6A 0074 0008 cmpiw.ex #116, 8(a2)
3500: 6600 009E      bne 158
3504: 4E4F           trap #15
3506: A0F7           sysTrapTimGetTicks
3508: 90AD F720      subrl.ex -2272(a5), d0
350C: D0AD F61E      addrl.ex -2530(a5), d0
3510: 7264           moveq #100, d1
3512: 4EBA 03CC      jsr.ex -21272(pc)
3516: 2800           movel d0, d4
3518: 486E FFE4      pea.ex -28(a6)
351C: 2F04           movel d4, -(a7)
351E: 4E4F           trap #15
3520: A0FC           sysTrapTimSecondsToDateTime
3522: 486E FFF2      pea.ex -14(a6)
3526: 1F3C 0002      moveb.ex #2, -(a7)
352A: 1F2E FFE7      moveb.ex -25(a6), -(a7)
352E: 1F2E FFE9      moveb.ex -23(a6), -(a7)
3532: 4E4F           trap #15
3534: A268           sysTrapTimeToAscii
3536: 41FA 00EC      lea -21972(pc), d0
353A: 4850           pea (a0)
353C: 486E FFF2      pea.ex -14(a6)
3540: 4E4F           trap #15
3542: A0C6           sysTrapStrCat
3544: 0C6E 000A FFE4 cmpiw.ex #10, -28(a6)
354A: 4FEF 001A      lea 26(a7), d7
354E: 6C10           bge 16
3550: 41FA 00D4      lea -21970(pc), d0
3554: 4850           pea (a0)
3556: 486E FFF2      pea.ex -14(a6)
355A: 4E4F           trap #15
355C: A0C6           sysTrapStrCat
355E: 504F           addqw #0, a7
3560: 306E FFE4      movew.ex -28(a6), a0
3564: 2F08           movel a0, -(a7)
3566: 486E FFF2      pea.ex -14(a6)
356A: 4E4F           trap #15
356C: A0C7           sysTrapStrLen
356E: 7200           moveq #0, d1
3570: 3200           movew d0, d1
3572: 41EE FFF2      lea -14(a6), d0
3576: 584F           addqw #4, a7
3578: 4870 1800      pea.ex 0(a0,d1.l)
357C: 4E4F           trap #15
357E: A0C9           sysTrapStrIToA
3580: 2F3C 0044 0082 movel.exl #4456578, -(a7)
3586: 486E FFF2      pea.ex -14(a6)
358A: 4E4F           trap #15
358C: A0C7           sysTrapStrLen
358E: 584F           addqw #4, a7
3590: 3F00           movew d0, -(a7)
3592: 486E FFF2      pea.ex -14(a6)
3596: 4E4F           trap #15
3598: A220           sysTrapWinDrawChars
359A: 4FEF 0012      lea 18(a7), d7
359E: 6018           bra 24
35A0: 0C6A 000C 0008 cmpiw.ex #12, 8(a2)
35A6: 6610           bne 16
35A8: 0C2D 0006 F714 cmpib.ex #6, -2284(a5)
35AE: 6608           bne 8
35B0: 4EBA E920      jsr.ex -27942(pc)
35B4: 4EBA E128      jsr.ex -29978(pc)
35B8: 7001           moveq #1, d0
35BA: 6048           bra 72
35BC: 0C52 0015      cmpiw #21, (a2)
35C0: 660E           bne 14
35C2: 3F2A 0008      movew.ex 8(a2), -(a7)
35C6: 4EBA FE8C      jsr.ex -22436(pc)
35CA: 7001           moveq #1, d0
35CC: 544F           addqw #2, a7
35CE: 6034           bra 52
35D0: 0C52 001C      cmpiw #28, (a2)
35D4: 672C           beq 44
35D6: 0C52 0018      cmpiw #24, (a2)
35DA: 661A           bne 26
35DC: 4E4F           trap #15
35DE: A173           sysTrapFrmGetActiveForm
35E0: 2648           movel a0, a3
35E2: 2F0B           movel a3, -(a7)
35E4: 4E4F           trap #15
35E6: A171           sysTrapFrmDrawForm
35E8: 4EBA E8E8      jsr.ex -27942(pc)
35EC: 4EBA E0F0      jsr.ex -29978(pc)
35F0: 7601           moveq #1, d3
35F2: 584F           addqw #4, a7
35F4: 600C           bra 12
35F6: 0C52 001A      cmpiw #26, (a2)
35FA: 6606           bne 6
35FC: 4EBA E0E0      jsr.ex -29978(pc)
3600: 7601           moveq #1, d3
3602: 1003           moveb d3, d0
3604: 4CDF 0C18      movem (a7)+, <0c18>
3608: 4E5E           unlk a6
360A: 4E75           rts
360C: 9442           subrw d2, d2
360E: 6F61           ble 97
3610: 7264           moveq #100, d1
3612: 5669 6577      addqw.ex #3, 25975(a1)
3616: 4861           dc.w #18529
3618: 6E64           bgt 100
361A: 6C65           bge 101
361C: 4576           dc.w #17782
361E: 656E           bcs 110
3620: 7400           moveq #0, d2
3622: 0004           dc.w #4
3624: 3A00           movew d0, d5
3626: 3000           movew d0, d0
3628: 4E56 0000      link a6, #0
362C: 48E7 1030      movem <1030>, -(a7)
3630: 266E 0008      movel.ex 8(a6), a3
3634: 0C53 0017      cmpiw #23, (a3)
3638: 6640           bne 64
363A: 362B 0008      movew.ex 8(a3), d3
363E: 3F03           movew d3, -(a7)
3640: 4E4F           trap #15
3642: A16F           sysTrapFrmInitForm
3644: 2448           movel a0, a2
3646: 2F0A           movel a2, -(a7)
3648: 4E4F           trap #15
364A: A174           sysTrapFrmSetActiveForm
364C: 5C4F           addqw #6, a7
364E: 0C43 03E8      cmpiw #1000, d3
3652: 6708           beq 8
3654: 0C43 0578      cmpiw #1400, d3
3658: 6710           beq 16
365A: 601A           bra 26
365C: 487A FE6E      pea.ex -22316(pc)
3660: 2F0A           movel a2, -(a7)
3662: 4E4F           trap #15
3664: A19F           sysTrapFrmSetEventHandler
3666: 504F           addqw #0, a7
3668: 600C           bra 12
366A: 487A FD3C      pea.ex -22608(pc)
366E: 2F0A           movel a2, -(a7)
3670: 4E4F           trap #15
3672: A19F           sysTrapFrmSetEventHandler
3674: 504F           addqw #0, a7
3676: 7001           moveq #1, d0
3678: 6002           bra 2
367A: 7000           moveq #0, d0
367C: 4CDF 0C08      movem (a7)+, <0c08>
3680: 4E5E           unlk a6
3682: 4E75           rts
3684: 9641           subrw d1, d3
3686: 7070           moveq #112, d0
3688: 6C69           bge 105
368A: 6361           bls 97
368C: 7469           moveq #105, d2
368E: 6F6E           ble 110
3690: 4861           dc.w #18529
3692: 6E64           bgt 100
3694: 6C65           bge 101
3696: 4576           dc.w #17782
3698: 656E           bcs 110
369A: 7400           moveq #0, d2
369C: 0000           dc.w #0
369E: 4E56 FFE6      link a6, #-26
36A2: 4EBA CB56      jsr.ex 30210(pc)
36A6: 2F00           movel d0, -(a7)
36A8: 486E FFE8      pea.ex -24(a6)
36AC: 4E4F           trap #15
36AE: A11D           sysTrapEvtGetEvent
36B0: 0C6E 0006 FFE8 cmpiw.ex #6, -24(a6)
36B6: 504F           addqw #0, a7
36B8: 6622           bne 34
36BA: 3F3C 03E8      movew.ex #1000, -(a7)
36BE: 4E4F           trap #15
36C0: A17E           sysTrapFrmGetFormPtr
36C2: B1EE FFF4      cmpal.ex -12(a6), a0
36C6: 544F           addqw #2, a7
36C8: 6600 0082      bne 130
36CC: 1B7C 0001 F71A moveb.emx #1, -2278(a5)
36D2: 4E4F           trap #15
36D4: A0F7           sysTrapTimGetTicks
36D6: 2B40 F71C      movel.mx d0, -2276(a5)
36DA: 6070           bra 112
36DC: 0C6E 0005 FFE8 cmpiw.ex #5, -24(a6)
36E2: 6650           bne 80
36E4: 3F3C 03E8      movew.ex #1000, -(a7)
36E8: 4E4F           trap #15
36EA: A17E           sysTrapFrmGetFormPtr
36EC: B1EE FFF0      cmpal.ex -16(a6), a0
36F0: 544F           addqw #2, a7
36F2: 6658           bne 88
36F4: 4E4F           trap #15
36F6: A197           sysTrapFrmGetFirstForm
36F8: B1EE FFF0      cmpal.ex -16(a6), a0
36FC: 664E           bne 78
36FE: 4A2D F71A      tstb.ex -2278(a5)
3702: 660C           bne 12
3704: 4E4F           trap #15
3706: A0F7           sysTrapTimGetTicks
3708: 5880           addql #4, d0
370A: 2B40 F716      movel.mx d0, -2282(a5)
370E: 603C           bra 60
3710: 422D F71A      clrb.ex -2278(a5)
3714: 4E4F           trap #15
3716: A0F7           sysTrapTimGetTicks
3718: 90AD F71C      subrl.ex -2276(a5), d0
371C: 0680 0000 00C8 addil #200, d0
3722: D1AD F716      addml.ex d0, -2282(a5)
3726: 4E4F           trap #15
3728: A0F7           sysTrapTimGetTicks
372A: 90AD F71C      subrl.ex -2276(a5), d0
372E: D1AD F720      addml.ex d0, -2272(a5)
3732: 6018           bra 24
3734: 4EBA CAC4      jsr.ex 30210(pc)
3738: 4A80           tstl d0
373A: 6610           bne 16
373C: 4EBA E172      jsr.ex -29512(pc)
3740: 4EBA E74A      jsr.ex -28012(pc)
3744: 4EBA F340      jsr.ex -24946(pc)
3748: 4EBA F590      jsr.ex -24350(pc)
374C: 0C6E 0004 FFE8 cmpiw.ex #4, -24(a6)
3752: 6622           bne 34
3754: 0C6E 0204 FFF0 cmpiw.ex #516, -16(a6)
375A: 651A           bcs 26
375C: 0C6E 0207 FFF0 cmpiw.ex #519, -16(a6)
3762: 6212           bhi 18
3764: 0C2D 0006 F714 cmpib.ex #6, -2284(a5)
376A: 670A           beq 10
376C: 302E FFF4      movew.ex -12(a6), d0
3770: 0240 0100      andiw #256, d0
3774: 673C           beq 60
3776: 486E FFE8      pea.ex -24(a6)
377A: 4E4F           trap #15
377C: A0A9           sysTrapSysHandleEvent
377E: 4A00           tstb d0
3780: 584F           addqw #4, a7
3782: 662E           bne 46
3784: 486E FFE6      pea.ex -26(a6)
3788: 486E FFE8      pea.ex -24(a6)
378C: 42A7           clrl -(a7)
378E: 4E4F           trap #15
3790: A1BF           sysTrapMenuHandleEvent
3792: 4A00           tstb d0
3794: 4FEF 000C      lea 12(a7), d7
3798: 6618           bne 24
379A: 486E FFE8      pea.ex -24(a6)
379E: 4EBA FE88      jsr.ex -21968(pc)
37A2: 4A00           tstb d0
37A4: 584F           addqw #4, a7
37A6: 660A           bne 10
37A8: 486E FFE8      pea.ex -24(a6)
37AC: 4E4F           trap #15
37AE: A1A0           sysTrapFrmDispatchEvent
37B0: 584F           addqw #4, a7
37B2: 0C6E 0016 FFE8 cmpiw.ex #22, -24(a6)
37B8: 6600 FEE8      bne 65256
37BC: 4E5E           unlk a6
37BE: 4E75           rts
37C0: 8945           dc.w #35141
37C2: 7665           moveq #101, d3
37C4: 6E74           bgt 116
37C6: 4C6F           dc.w #19567
37C8: 6F70           ble 112
37CA: 0000           dc.w #0
37CC: 4E56 0000      link a6, #0
37D0: 48E7 1C00      movem <1c00>, -(a7)
37D4: 3A2E 0008      movew.ex 8(a6), d5
37D8: 382E 000E      movew.ex 14(a6), d4
37DC: 3F04           movew d4, -(a7)
37DE: 2F3C 0300 0000 movel.exl #50331648, -(a7)
37E4: 4EBA CDEE      jsr.ex 31196(pc)
37E8: 3600           movew d0, d3
37EA: 4A43           tstw d3
37EC: 5C4F           addqw #6, a7
37EE: 6706           beq 6
37F0: 7000           moveq #0, d0
37F2: 3003           movew d3, d0
37F4: 6022           bra 34
37F6: 4A45           tstw d5
37F8: 661C           bne 28
37FA: 4EBA CA56      jsr.ex 30298(pc)
37FE: 3600           movew d0, d3
3800: 3F3C 03E8      movew.ex #1000, -(a7)
3804: 4E4F           trap #15
3806: A19B           sysTrapFrmGotoForm
3808: 4A43           tstw d3
380A: 544F           addqw #2, a7
380C: 6604           bne 4
380E: 4EBA FE8E      jsr.ex -21850(pc)
3812: 4EBA CCFC      jsr.ex 31000(pc)
3816: 7000           moveq #0, d0
3818: 4CDF 0038      movem (a7)+, <0038>
381C: 4E5E           unlk a6
381E: 4E75           rts
3820: 8950           ormw d4, (a0)
3822: 696C           bvs 108
3824: 6F74           ble 116
3826: 4D61           dc.w #19809
3828: 696E           bvs 110
382A: 0000           dc.w #0
382C: 4E56 FFF4      link a6, #-12
3830: 48E7 1800      movem <1800>, -(a7)
3834: 486E FFF4      pea.ex -12(a6)
3838: 486E FFF8      pea.ex -8(a6)
383C: 486E FFFC      pea.ex -4(a6)
3840: 4E4F           trap #15
3842: A08F           sysTrapSysAppStartup
3844: 3800           movew d0, d4
3846: 4A44           tstw d4
3848: 4FEF 000C      lea 12(a7), d7
384C: 671C           beq 28
384E: 41FA 0066      lea -21314(pc), d0
3852: 4850           pea (a0)
3854: 3F3C 005C      movew.ex #92, -(a7)
3858: 41FA 0078      lea -21286(pc), d0
385C: 4850           pea (a0)
385E: 4E4F           trap #15
3860: A084           sysTrapErrDisplayFileLineMsg
3862: 7000           moveq #0, d0
3864: 4FEF 000A      lea 10(a7), d7
3868: 6036           bra 54
386A: 206E FFFC      movel.ex -4(a6), a0
386E: 3F28 0006      movew.ex 6(a0), -(a7)
3872: 2F28 0002      movel.ex 2(a0), -(a7)
3876: 3F10           movew (a0), -(a7)
3878: 487A 000E      pea.ex -21360(pc)
387C: 487A 0004      pea.ex -21366(pc)
3880: 0697 FFFF FF4A addil #-182, (a7)
3886: 4E75           rts
3888: 2600           movel d0, d3
388A: 2F2E FFF4      movel.ex -12(a6), -(a7)
388E: 2F2E FFF8      movel.ex -8(a6), -(a7)
3892: 2F2E FFFC      movel.ex -4(a6), -(a7)
3896: 4E4F           trap #15
3898: A090           sysTrapSysAppExit
389A: 2003           movel d3, d0
389C: 4FEF 0014      lea 20(a7), d7
38A0: 4CDF 0018      movem (a7)+, <0018>
38A4: 4E5E           unlk a6
38A6: 4E75           rts
38A8: 8B5F           ormw d5, (a7)+
38AA: 5F53           subqw #7, (a3)
38AC: 7461           moveq #97, d2
38AE: 7274           moveq #116, d1
38B0: 7570           dc.w #30064
38B2: 5F5F           subqw #7, (a7)+
38B4: 002A           dc.w #42
38B6: 4572           dc.w #17778
38B8: 726F           moveq #111, d1
38BA: 7220           moveq #32, d1
38BC: 6C61           bge 97
38BE: 756E           dc.w #30062
38C0: 6368           bls 104
38C2: 696E           bvs 110
38C4: 6720           beq 32
38C6: 6170           bsr 112
38C8: 706C           moveq #108, d0
38CA: 6963           bvs 99
38CC: 6174           bsr 116
38CE: 696F           bvs 111
38D0: 6E00 5374      bgt 21364
38D4: 6172           bsr 114
38D6: 7475           moveq #117, d2
38D8: 7043           moveq #67, d0
38DA: 6F64           ble 100
38DC: 652E           bcs 46
38DE: 6300 48E7      bls 18663
38E2: 3000           movew d0, d0
38E4: 7402           moveq #2, d2
38E6: 4EFB 2200      jmp.ex 0(pc,d2.w)
38EA: 600A           bra 10
38EC: 4C41           dc.w #19521
38EE: 0000           dc.w #0
38F0: 4CDF 000C      movem (a7)+, <000c>
38F4: 4E75           rts
38F6: 2401           movel d1, d2
38F8: 4242           clrw d2
38FA: 4842           swap d2
38FC: 661C           bne 28
38FE: 3600           movew d0, d3
3900: 4240           clrw d0
3902: 4840           swap d0
3904: 6706           beq 6
3906: 80C1           divu d1, d0
3908: 3400           movew d0, d2
390A: 4842           swap d2
390C: 3003           movew d3, d0
390E: 80C1           divu d1, d0
3910: 3400           movew d0, d2
3912: 2002           movel d2, d0
3914: 4CDF 000C      movem (a7)+, <000c>
3918: 4E75           rts
391A: 3400           movew d0, d2
391C: 4240           clrw d0
391E: 4840           swap d0
3920: 4842           swap d2
3922: 2601           movel d1, d3
3924: 720F           moveq #15, d1
3926: D482           addrl d2, d2
3928: D180           addxrl d0, d0
392A: B083           cmpl d3, d0
392C: 6504           bcs 4
392E: 9083           subrl d3, d0
3930: 5202           addqb #1, d2
3932: 51C9 FFF2      dbf d1, 65522
3936: 2002           movel d2, d0
3938: 4CDF 000C      movem (a7)+, <000c>
393C: 4E75           rts
393E: 895F           ormw d4, (a7)+
3940: 5F6C 6469      subqw.ex #7, 25705(a4)
3944: 7675           moveq #117, d3
3946: 5F5F           subqw #7, (a7)+
3948: 0000           dc.w #0

