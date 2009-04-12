/* $Id: fsckmsgc.h,v 1.1.1.1 2003/05/21 13:39:50 pasha Exp $ */

/* static char *SCCSID = "@(#)1.26.1.2  12/2/99 11:13:46 src/jfs/utils/chkdsk/fsckmsgc.h, jfschk, w45.fs32, fixbld";*/
/* 
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 *   MODULE_NAME:		fsckmsgc.h
 *
 *   COMPONENT_NAME: 	jfschk
 *
 *
 */
#ifndef H_FSCKMSGC
#define H_FSCKMSGC

/*
 * The following defines are equivalent to the offsets in the
 * message definition arrays.
 *
 */

/* ***** IMPORTANT ***** IMPORTANT ***** IMPORTANT ***** IMPORTANT *****
 *
 * xfsck.h contains       #define fsck_highest_msgid_defined
 * That constant MUST be maintained in synch with the message id
 * constants defined here since the message text array and the
 * message attributes array (declared in fsckmsgp.h) are
 * both dimensioned using it.
 *
 * VERY IMPORTANT:	While each message from 0 through 399 may be issued 
 *			to the customer (and is in JFS.TXT if this is the case),
 *			messages above 399 must NEVER be translated.  These
 *			MUST be strictly messages for debug mode and to be
 *			written to the log.
 *			This is because, in JFS.TXT, only 400 message numbers
 *			are reserved for chkdsk.
 *
 * ***** IMPORTANT ***** IMPORTANT ***** IMPORTANT ***** IMPORTANT ***** */
#define avail_285              285
#define avail_286              286
#define avail_287              287
#define avail_288              288
#define avail_289              289
#define avail_298              298
#define avail_299              299
#define avail_300              300
#define avail_304              304
#define avail_305              305
#define avail_306              306
#define avail_307              307
#define avail_308              308
#define avail_311              311
#define avail_370              370
#define avail_371              371
#define avail_372              372
#define avail_373              373
#define avail_374              374
#define avail_375              375
#define avail_376              376
#define avail_377              377
#define avail_378              378
#define avail_379              379
#define avail_380              380
#define avail_381              381
#define avail_382              382
#define avail_383              383
#define avail_384              384

                                                                      /* 1 line deleted          @F1 */
#define avail_for_debug_only_589              589
#define avail_for_debug_only_590              590
#define avail_for_debug_only_591              591
#define avail_for_debug_only_592              592
#define avail_for_debug_only_593              593
#define avail_for_debug_only_594              594
#define avail_for_debug_only_595              595
#define avail_for_debug_only_596              596
#define avail_for_debug_only_597              597
#define avail_for_debug_only_598              598
#define avail_for_debug_only_599              599

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   *
   * symbolic constants for text message inserts 
   *
   */

#define fsck_ACL                  385
#define fsck_aggr_inode         385
#define fsck_aggregate          385
#define fsck_directory            386
#define fsck_dirpfx                386
#define fsck_dmap                 387
#define fsck_dotext               399
#define fsck_EA                    388
#define fsck_file                    389
#define fsck_fileset                389
#define fsck_fset_inode          389
#define fsck_inoext                390
#define fsck_inopfx                390
#define fsck_L0                     391
#define fsck_L1                     392
#define fsck_L2                     393
#define fsck_metadata           394
#define fsck_metaIAG             390
#define fsck_objcontents        395
#define fsck_primary              396
#define fsck_secondary          397
#define fsck_symbolic_link      398

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   *
   * symbolic constants for message text 
   *
   */

#define fsck_MSGOK                      0

#define fsck_AGGCLN                   186
#define fsck_AGGCLNNOTDRTY            187
#define fsck_AGGDRTY                  188
#define fsck_AGGDRTYNOTCLN            189
#define fsck_AGGMRKDCLN               190
#define fsck_AGGMRKDDRTY              191
#define fsck_ALLFXD                     1
#define fsck_ALLOCHIGHMEM                     422
#define fsck_ALLOCHIGHMEMRSLT              423

#define fsck_BADAGFELIST               69
#define fsck_BADAGFELIST1             196
#define fsck_BADAGFILIST              197
#define fsck_BADAGFILIST1             198
#define fsck_BADBLKALLOC              229
#define fsck_BADBLKALLOCCTL           230
#define fsck_BADBLKCTTTL                3
#define fsck_BADBLKNO                   4
#define fsck_BADBMAPCAGFCL            256
#define fsck_BADBMAPCOTH              257
#define fsck_BADBMAPSLFV              225
#define fsck_BADBMAPSLNV              226
#define fsck_BADBMAPSOTHER            227
#define fsck_BADBSBLCHN                 5
#define fsck_BADDINOFREELIST1         161
#define fsck_BADDINOFREELIST2         204
#define fsck_BADDINOFREELIST3         205
#define fsck_BADDINOFREELIST4         203
#define fsck_BADDINONODESIZ           201
#define fsck_BADDINOODDNODESIZ        202
#define fsck_BADDIRENTRY              200
#define fsck_BADDMAPPMAPS             228
#define fsck_BADFSBLCHN                 6
#define fsck_BADIAG                   123
#define fsck_BADIAGAG                 199
#define fsck_BADIAGAGCRCTD            223
#define fsck_BADIAGAGSTRT             206
#define fsck_BADIAGAGSTRTCRCTD        224
#define fsck_BADIAGFIES               207
#define fsck_BADIAGFIS                208
#define fsck_BADIAGFLIST              209
#define fsck_BADIAGFL1                210
#define fsck_BADIAGIAGNUM             211
#define fsck_BADIAGNFEXT              212
#define fsck_BADIAGNFINO              213
#define fsck_BADIAGPMAP               214
#define fsck_BADIAM                   215
#define fsck_BADIAMA                   295
#define fsck_BADIAMAGNBI               96
#define fsck_BADIAMAGNFI               97
#define fsck_BADIAMBPIE               216
#define fsck_BADIAMCTL                217
#define fsck_BADIAMCTLA              294
#define fsck_BADIAMIAGPXDL            112
#define fsck_BADIAMIAGPXDU            124
#define fsck_BADIAML2BPIE             218
#define fsck_BADIAMNBI                219
#define fsck_BADIAMNFI                220
#define fsck_BADIAMNXTIAG             221
#define fsck_BADINOCLAIMSDUPSD         271
#define fsck_BADINOCLAIMSDUPSF         141
#define fsck_BADINOCLAIMSDUPSO         272
#define fsck_BADINODATAFORMAT         142
#define fsck_BADINODATAFORMATD       278
#define fsck_BADINODATAFORMATO       280
#define fsck_BADINODXDFLDD               8
#define fsck_BADINODXDFLDL               368
#define fsck_BADINODXDFLDO               369
#define fsck_BADINOFORMAT             143
#define fsck_BADINOFORMATD             279
#define fsck_BADINOFORMATO             281
#define fsck_BADINOFRONTGAP           194
#define fsck_BADINOINTERNGAP          195
#define fsck_BADINOLKCT                 9
#define fsck_BADINOMTNODE             192
#define fsck_BADINONODESELF             357
#define fsck_BADINOODDINTRNEXT        193
#define fsck_BADINOOTHR                54
#define fsck_BADINOREF                 10
#define fsck_BADINOSTAMP              179
#define fsck_BADINOTYP                  7
#define fsck_BADKEYS                   13
#define fsck_BADLINKCTS                95
#define fsck_BADMETAINOF              49
#define fsck_BADMETAINOP              290
#define fsck_BADMETAINOS              291
#define fsck_BADSBAGSIZ                15
#define fsck_BADSBBLSIZ                16
#define fsck_BADSBFJLA                260
#define fsck_BADSBFJLL                261
#define fsck_BADSBFSSIZ                17
#define fsck_BADSBFWSL                 81
#define fsck_BADSBFWSL1               259
#define fsck_BADSBFWSA                 82
#define fsck_BADSBMGC                  21
#define fsck_BADSBOTHR                 14
#define fsck_BADSBVRSN                 22
#define fsck_BDSBBTHCRRPT              18
#define fsck_BDSBNWRTACC               19
#define fsck_BLSIZLTLVBLSIZ            25
#define fsck_BMAPBADHT                231
#define fsck_BMAPBADL2NLF             232
#define fsck_BMAPBADLFI               233
#define fsck_BMAPBADLFV               234
#define fsck_BMAPBADLNV               235
#define fsck_BMAPBADBMN               236
#define fsck_BMAPBADNLF               237
#define fsck_BMAPCAGNF                238
#define fsck_BMAPCASB                 239
#define fsck_BMAPCBMXB                 23
#define fsck_BMAPCBPAG                240
#define fsck_BMAPCDMCLAG              241
#define fsck_BMAPCDMCLH               242
#define fsck_BMAPCDMCLW               243
#define fsck_BMAPCDMCSTI              258
#define fsck_BMAPCL2BPAG              244
#define fsck_BMAPCL2BPP               245
#define fsck_BMAPCMAAG                246
#define fsck_BMAPCMXLVL               247
#define fsck_BMAPCNAG                 248
#define fsck_BMAPCNF                   24
#define fsck_BMAPCPAG                 249
#define fsck_BOOTSECFXD                12

#define fsck_CANTCHKEA                 84
#define fsck_CANTCONTINUE             326
#define fsck_CANTINITSVCLOG           327
#define fsck_CANTREADAITP              324
#define fsck_CANTREADAITS              293
#define fsck_CANTREADAITEXT1          163
#define fsck_CANTREADEAITEXT1         162
#define fsck_CANTRECONINSUFSTG       359
#define fsck_CANTRECOVERINOS          136
#define fsck_CANTREPAIRAIS             37
#define fsck_CANTREPAIRINO            144
#define fsck_CHKDSKFSSYNCHRC          341
#define fsck_CHKDSKSYSALLOC           336
#define fsck_CHKDSKSYSCALLRC          338
#define fsck_CHKDSKSYSOPENRC          337
#define fsck_CHKLOGINVALRCD            41
#define fsck_CHKLOGNEW                 31
#define fsck_CHKLOGNOVOL              328
#define fsck_CHKLOGOLD                 35
#define fsck_CHKLOGUSAGE               20
#define fsck_CHKLOGSBOK               321
#define fsck_CLRBBACTIVITY                  348
#define fsck_CLRBBLKSRC                      353
#define fsck_CLRBBLVMLISTDATA            349
#define fsck_CLRBBLVMNUMLISTS             350
#define fsck_CLRBBOPENFAILED               356
#define fsck_CLRBBRANGE                       351
#define fsck_CLRBBRESULTS                    352
#define fsck_CNTRDDEVCHAR             121
#define fsck_CNTRESUPB                  312
#define fsck_CNTRESUPP                  40
#define fsck_CNTRESUPS                  297
#define fsck_CNTWRTBS                  93
#define fsck_CNTWRTBSMBR               28
#define fsck_CNTWRTSUPP                      44
#define fsck_CNTWRTSUPS                      156

#define fsck_DASDLIMITSPRIMED                586                /* @F1 */
#define fsck_DASDUSEDPRIMED                  587                /* @F1 */
#define fsck_DEFAULTVOL                      27
#define fsck_DEVBEGINFMTRC                339
#define fsck_DEVCLOSERC                     132
#define fsck_DEVGETCHARRC                  52
#define fsck_DEVLOCKRC                        77
#define fsck_DEVOPENRDRC                     83
#define fsck_DEVOPENRDWRRC               134
#define fsck_DEVOPENRDWRSRC              355
#define fsck_DEVREDETERMRC             90
#define fsck_DEVUNLOCKRC              129
#define fsck_DIRWHDLKS                 46
#define fsck_DMAPBADNBLK              251
#define fsck_DMAPBADNFREE             252
#define fsck_DMAPBADSTRT              253
#define fsck_DOSEXECPGMRC             354
#define fsck_DRIVEID                  145
#define fsck_DRIVETYPE                146
#define fsck_DUPBLKMDREF              160
#define fsck_DUPBLKMDREFS             159
#define fsck_DUPBLKREF                 48
#define fsck_DUPBLKREFS                62

#define fsck_EAFORMATBAD              113
#define fsck_ERRONAGG                  45
#define fsck_ERRONAITRD               110
#define fsck_ERRONLOG                  11
#define fsck_ERRONWSP                  50
#define fsck_ERRORSDETECTED           171
#define fsck_ERRORSINAITP              325
#define fsck_ERRORSINAITS              296
#define fsck_EXHDYNSTG                 51
#define fsck_EXHFILSYSSTG            364

#define fsck_FSMNTD                    53
#define fsck_FSSMMRY1                  55
#define fsck_FSSMMRY2                  56
#define fsck_FSSMMRY3                  57
#define fsck_FSSMMRY4                  58
#define fsck_FSSMMRY5                  59
#define fsck_FSSMMRY6                  60
#define fsck_FSSMMRY7                  63
#define fsck_FSSMMRY8                  64
#define fsck_FSSMMRY9                  61
       
#define fsck_HEARTBEAT0               262
#define fsck_HEARTBEAT1               263
#define fsck_HEARTBEAT2               264
#define fsck_HEARTBEAT3               265
#define fsck_HEARTBEAT4               266
#define fsck_HEARTBEAT5               267
#define fsck_HEARTBEAT6               268
#define fsck_HEARTBEAT7               269
#define fsck_HEARTBEAT8               270

#define fsck_ILLINOREF                     65
#define fsck_INOCANTNAME               33
#define fsck_INCDASDUSEDCRCT        274
#define fsck_INCINOREF                     71
#define fsck_INCINOREFCRCT             72
#define fsck_INCONSIST2NDRY            29
#define fsck_INCONSIST2NDRY1           30
#define fsck_INOACL                        360
#define fsck_INOACLCLRD                 362
#define fsck_INOBADREF                 34
#define fsck_INOCLRD                   73
#define fsck_INOCNTGETPATH             67
#define fsck_INOEA                     68
#define fsck_INOEACLRD                 74
#define fsck_INOINLINECONFLICT        158
#define fsck_INOINLSFND                 309
#define fsck_INOINLSFNF                 75
#define fsck_INOLKCTFXD                76
#define fsck_INOMINOR                   365
#define fsck_INOMINORFXD              366
#define fsck_INONOPATHS                78
#define fsck_INOPATHBAD                358
#define fsck_INOPATHCRCT              167
#define fsck_INOPATHOK                   79
#define fsck_INOREFRMV                 80
#define fsck_INOSINLSFND               173
#define fsck_INOSINLSFNDS             275
#define fsck_INOSINLSFNF               276
#define fsck_INOSINLSFNFS             277
#define fsck_INTERNALERROR             347

#define fsck_LOGFORMATFAIL            119
#define fsck_LOGFORMATRC              120
#define fsck_LOGREDOFAIL               85
#define fsck_LOGREDORC                 86
#define fsck_LOGSPECINVALID            47
#define fsck_LSFNCNTCRE                32
#define fsck_LSFNNOTDIR                87
#define fsck_LSFNNOTFOUND              94
#define fsck_LVMFOUNDBDBLKS              342
#define fsck_LVMFSNOWAVAIL                345
#define fsck_LVMGETBBLKINFORC           343
#define fsck_LVMGETTBLSIZERC             344
#define fsck_LVMTRNSBBLKSTOJFS         346

#define fsck_MNCNTRCNCTINOD             310
#define fsck_MNCNTRCNCTINOF             70
#define fsck_MNCNTRCNCTINOSD           174
#define fsck_MNCNTRCNCTINOSDS         301
#define fsck_MNCNTRCNCTINOSF           302
#define fsck_MNCNTRCNCTINOSFS         303
#define fsck_MNTFSYS2                  88
#define fsck_MODIFIED                  89
#define fsck_MRKSBDONE                 91
#define fsck_MSSNGBLKS                 92

#define fsck_PARMAUTOCHK              333
#define fsck_PARMCLRBDBLKLST     340
#define fsck_PARMDEBUG                335
#define fsck_PARMFIXLVL               330
#define fsck_PARMIFDRTY               331
#define fsck_PARMOMITLOGREDO      284
#define fsck_PARMPMCHK                334
#define fsck_PARMVERBOSE              332
#define fsck_PHASE0                    99
#define fsck_PHASE1                   100
#define fsck_PHASE2                   101
#define fsck_PHASE3                   102
#define fsck_PHASE4                   103
#define fsck_PHASE5                   104
#define fsck_PHASE6                   105
#define fsck_PHASE7R                 106
#define fsck_PHASE7V                 282
#define fsck_PHASE8R                 107
#define fsck_PHASE8V                 283
#define fsck_PHASE9                   108
#define fsck_PMAPSBOFF                254
#define fsck_PMAPSBON                 255
#define fsck_PRMMUTEXCLLEVELS         115
#define fsck_PRMUSAGE                 116
#define fsck_PRMUNRECOPTION           117
#define fsck_PRMUNSUPPENUM            118

#define fsck_REPAIRSINPROGRESS        114
#define fsck_RIBADDATFMT                2
#define fsck_RIBADFMT                  26
#define fsck_RIBADTREE                 38
#define fsck_RICRETREE                 39
#define fsck_RICRRCTDREF              166
#define fsck_RIINCINOREF              172
#define fsck_RINOTDIR                 125
#define fsck_RIUNALLOC                126
#define fsck_RODIRSWHLKS              153
#define fsck_RODIRWHLKS               168
#define fsck_ROINCINOREF              169
#define fsck_ROINCINOREFS             154
#define fsck_ROOTALLOC                127
#define fsck_ROOTNOWDIR               128
#define fsck_ROUALINOREF              170
#define fsck_ROUALINOREFS             155
#define fsck_ROUNCONNIO               175
#define fsck_ROUNCONNIOS              180

#define fsck_SBBADP                    122
#define fsck_SBBADS                    292
#define fsck_SBOKP                     130
#define fsck_SBOKS                     148
#define fsck_SEPARATOR                 43
#define fsck_SESSEND                  131
#define fsck_SESSPRMDFLT              133
#define fsck_SESSSTART                 42 
#define fsck_SPARSEFILSYS              273
#define fsck_STDSUMMARY1              313
#define fsck_STDSUMMARY2              314
#define fsck_STDSUMMARY3              315
#define fsck_STDSUMMARY4              316
#define fsck_STDSUMMARY4A            363
#define fsck_STDSUMMARY5              317
#define fsck_STDSUMMARY6              318

#define fsck_UALINOREF                135
#define fsck_URCVCLOSE                165
#define fsck_URCVREAD                  98
#define fsck_URCVUNLCK                164
#define fsck_URCVWRT                  137

#define fsck_WILLCLEARACL            361
#define fsck_WILLCLEAREA              111
#define fsck_WILLFIXDIRWHDLKS         150
#define fsck_WILLFIXINCREF            151
#define fsck_WILLFIXINOMINOR        367
#define fsck_WILLFIXLINKCTS           157
#define fsck_WILLFIXRIBADDATFMT       138
#define fsck_WILLFIXRIBADFMT          139
#define fsck_WILLFIXRODIRSWHLKS       183
#define fsck_WILLFIXROINCINOREFS      184
#define fsck_WILLFIXROUALINOREFS      181
#define fsck_WILLFIXROUNCONNIOS       182
#define fsck_WILLRELEASEINO           147
#define fsck_WILLRELEASEINOS          140
#define fsck_WILLRMVBADENTRY          109
#define fsck_WILLRMVBADREF             36
#define fsck_WRSUP                    149

#define fsck_XCHKDMPBADFNAME          323
#define fsck_XCHKDMPBADFORMAT         320
#define fsck_XCHKDMPMTORBADREAD       319
#define fsck_XCHKDMPOPNFAIL           185
#define fsck_XCHKDMPUSAGE             222
#define fsck_XCHKLOGBADFNAME          322
#define fsck_XCHKLOGNEW               176
#define fsck_XCHKLOGNOVOL             329
#define fsck_XCHKLOGOLD               177
#define fsck_XCHKLOGOPNFAIL           178
#define fsck_XCHKLOGSBOK              250
#define fsck_XCHKLOGUSAGE              66
#define fsck_XTRABLKS                 152


#define lrdo_ALLOC4BMAP                         424
#define lrdo_ALLOC4BMAPFAIL                    431
#define lrdo_ALLOC4DOBLK                        426
#define lrdo_ALLOC4DOBLKFAIL                   433
#define lrdo_ALLOC4EXTDTPG                    430
#define lrdo_ALLOC4EXTDTPGFAIL                437
#define lrdo_ALLOC4IMAP                           425
#define lrdo_ALLOC4IMAPFAIL                     432
#define lrdo_ALLOC4NODOFL                      429
#define lrdo_ALLOC4NODOFLFAIL                  436
#define lrdo_ALLOC4NOREDOFL                   427
#define lrdo_ALLOC4NOREDOFLFAIL              434
#define lrdo_ALLOC4REDOPG                      428
#define lrdo_ALLOC4REDOPGFAIL                  435
#define lrdo_ALREADYREDONE                   400

#define lrdo_BADCOMMIT                          450
#define lrdo_BADDISKBLKNUM               484
#define lrdo_BADINODENUM               485
#define lrdo_BADNOREDOPAGE               454
#define lrdo_BADNOREDOINOEXT               455
#define lrdo_BADLOGSER               489
#define lrdo_BADMOUNT                           451
#define lrdo_BADREDOPAGE               453
#define lrdo_BADUPDATEMAP               456
#define lrdo_BADUPDMAPREC               488
#define lrdo_BRDBADBLOCK              545
#define lrdo_BRDREADBLKFAIL              546
#define lrdo_BUFFLUSHFAIL               475

#define lrdo_CANTINITMAPS               483
#define lrdo_CANTREADBLK               486
#define lrdo_CANTREADFSSUPER               463
#define lrdo_CANTREADLOGSUP                444
#define lrdo_CANTUPDLOGSUP                  445
#define lrdo_CANTWRITELOGSUPER               462

#define lrdo_DAFTMRKBMPFAILED                   549
#define lrdo_DAFTUPDPGFAILED                   548
#define lrdo_DEDPBREADFAILED                   550
#define lrdo_DEVOPNREADERROR               452
#define lrdo_DNRIFNDNOREDORECFAIL                   556
#define lrdo_DNRPFNDDTPGPGREDOFAIL                   552
#define lrdo_DNRPFNDDTRTPGREDOFAIL                   551
#define lrdo_DNRPFNDXTPGPGREDOFAIL                   554
#define lrdo_DNRPFNDXTRTPGREDOFAIL                   553
#define lrdo_DNRPUNKNOWNTYPE                   555
#define lrdo_DPRFBADSLOTNXTIDX                   559
#define lrdo_DPRFBADSTBLENTRY                   558
#define lrdo_DRRFBADSLOTNXTIDX                   561
#define lrdo_DRRFBADSTBLENTRY                   560
#define lrdo_DUMPUNKNOWNTYPE                   557

#define lrdo_ERRORCANTCONTIN               459
#define lrdo_ERRORCANTUPDMAPS               460
#define lrdo_ERRORCANTUPDFSSUPER               461
#define lrdo_ERRORNEEDREFORMAT               458
#define lrdo_ERRORONVOL               481
#define lrdo_EXTFSREADBLKFAIL1       497
#define lrdo_EXTFSREADBLKFAIL2       498
#define lrdo_EXTFSREADBLKFAIL3       502
#define lrdo_EXTFSREADBLKFAIL4       503
#define lrdo_EXTFSREADBLKFAIL5       505
#define lrdo_EXTFSREADBLKMAPINOFAIL       496
#define lrdo_EXTFSREADFSSUPERFAIL       495
#define lrdo_EXTFSREADLOGSUPFAIL       508
#define lrdo_EXTFSINITLOGREDOFAIL       504
#define lrdo_EXTFSWRITEBLKFAIL1       499
#define lrdo_EXTFSWRITEBLKFAIL2       500
#define lrdo_EXTFSWRITEBLKFAIL3       501
#define lrdo_EXTFSWRITEBLKFAIL4       506
#define lrdo_EXTFSWRITEFSSUPERFAIL       507
#define lrdo_EXTFSWRITELOGSUPFAIL       509

#define lrdo_FEOLPGV1FAIL                        574
#define lrdo_FEOLPGV2FAIL                        575
#define lrdo_FEOLPGV3FAIL                        576
#define lrdo_FEOLPGV4FAIL                        577
#define lrdo_FEOLPGV4AFAIL                      578
#define lrdo_FINDLOGENDFAIL                 490
#define lrdo_FSSUPERBADLOGLOC               466
#define lrdo_FSSUPERBADLOGSER               467
#define lrdo_FSSUPERBADMAGIC               464
#define lrdo_FSSUPERBADVERS               465

#define lrdo_INITFAILED                           410
#define lrdo_INITMAPSFAIL               468
#define lrdo_IOERRONLOG                 493
#define lrdo_IOERRREADINGBLK               487

#define lrdo_LOGEND                                401
#define lrdo_LOGENDBAD1                        409
#define lrdo_LOGENDBAD2                        414
#define lrdo_LOGNOTINLINE1               477
#define lrdo_LOGNOTINLINE2               479
#define lrdo_LOGOPEN               480
#define lrdo_LOGREADFAIL                 491
#define lrdo_LOGSUPBADBLKSZ                 411
#define lrdo_LOGSUPBADL2BLKSZ              412
#define lrdo_LOGSUPBADLOGSZ                 413
#define lrdo_LOGSUPBADMGC                     407
#define lrdo_LOGSUPBADVER                     408
#define lrdo_LOGWRAP                     494
#define lrdo_LOGWRAPPED                       449
#define lrdo_LRLOGWRAP                           581
#define lrdo_LRREADFAIL                           582
#define lrdo_LRMWFAIL1                            583
#define lrdo_LRMWFAIL2                            584
#define lrdo_LRMWFAIL3                            588		/* @D1 */

#define lrdo_MBMPBLKOUTRANGE                   562
#define lrdo_MOUNTRECORD                        421
#define lrdo_MWREADFAIL                          585

#define lrdo_NEXTADDRINVALID                 446
#define lrdo_NEXTADDROUTRANGE             447
#define lrdo_NOTAFSDEV               478
#define lrdo_NOTAFSDEVNUM               476
#define lrdo_NEXTADDRSAME                    448

#define lrdo_OPENFAILED               482

#define lrdo_PVGETPGFAIL                         579

#define lrdo_RBLDGDMAPERROR              538
#define lrdo_RBLDGIMAPERROR1              535
#define lrdo_RBLDGIMAPERROR2              547
#define lrdo_RBMPREADDATFAIL              515
#define lrdo_RBMPREADNXTLFFAIL              516
#define lrdo_RBMPREADXTFAIL              514
#define lrdo_READBMAPFAIL              511
#define lrdo_READBMAPINOFAIL              510
#define lrdo_READFS2NDSBFAIL               472
#define lrdo_READFSPRIMSBFAIL               471
#define lrdo_READFSSUPERFAIL               469
#define lrdo_READIMAPFAIL              513
#define lrdo_READIMAPINOFAIL              512
#define lrdo_REXTNDBEGIN                        402
#define lrdo_REXTNDDONE                         403
#define lrdo_REXTNDFAIL                          406
#define lrdo_REXTNDTOPOST                     405
#define lrdo_REXTNDTOPRE                       404
#define lrdo_RIMPREADCTLFAIL              518
#define lrdo_RIMPREADDATFAIL              520
#define lrdo_RIMPREADNXTLFFAIL              519
#define lrdo_RIMPREADXTFAIL              517
#define lrdo_RPTNUMDOBLK                        418
#define lrdo_RPTNUMLOGREC                      417
#define lrdo_RPTNUMNODOBLK                    419
#define lrdo_RPTSYNCADDR                       416
#define lrdo_RPTSYNCNUM                         415
#define lrdo_RXTREADLFFAIL              544

#define lrdo_SLPWRITEFAIL                        580
#define lrdo_SYNCRECORD                          420

#define lrdo_UMPREADBMAPINOFAIL              523
#define lrdo_UMPREADIMAPINOFAIL              521
#define lrdo_UMPWRITEIMAPCTLFAIL              522
#define lrdo_UMPWRITEBMAPCTLFAIL              524
#define lrdo_UNKNOWNTYPE               457
#define lrdo_UNRECOGTYPE                 492
#define lrdo_UPDMPBADLFIDX              543
#define lrdo_UPPGBADINODESEGOFFSET             564
#define lrdo_UPPGBREADFAIL1                         565
#define lrdo_UPPGBREADFAIL2                         566
#define lrdo_UPPGBREADFAIL3                         567
#define lrdo_UPPGBREADFAIL4                         568
#define lrdo_UPPGDTRTRFLFAIL                         571
#define lrdo_UPPGDTPGRFLFAIL                         572
#define lrdo_UPPGFNDPGREDOFAIL                   563
#define lrdo_UPPGMBMPFAIL                         570
#define lrdo_UPPGMIMPFAIL                         569
#define lrdo_UPPGSEDPFAIL                         573
#define lrdo_USINGBMAPALLOC4DOBLK              440
#define lrdo_USINGBMAPALLOC4EDPG                443
#define lrdo_USINGBMAPALLOC4IMAP                 438
#define lrdo_USINGBMAPALLOC4NDFL                 442
#define lrdo_USINGBMAPALLOC4NRFL                 439
#define lrdo_USINGBMAPALLOC4RDPG                 441

#define lrdo_WRBMPBADLFIDX0              537
#define lrdo_WRBMPBADMAPSIZE              536
#define lrdo_WRBMPBADTOTPG              542
#define lrdo_WRBMPBLKWRITEFAIL              540
#define lrdo_WRBMPDONE              532
#define lrdo_WRBMPNOTRBLDGBMAP              534
#define lrdo_WRBMPREADLFFAIL              541
#define lrdo_WRBMPRXTFAIL              539
#define lrdo_WRBMPSTART              533
#define lrdo_WRIMPBADNPAGES                529
#define lrdo_WRIMPBLKWRITEFAIL              527
#define lrdo_WRIMPDONE              530
#define lrdo_WRIMPNOTRBLDGIMAP              525
#define lrdo_WRIMPREADLFFAIL                528
#define lrdo_WRIMPRXTFAIL              526
#define lrdo_WRIMPSTART              531
#define lrdo_WRITEFS2NDSBFAIL               474
#define lrdo_WRITEFSPRIMSBFAIL               473
#define lrdo_WRITEFSSUPERFAIL               470

#endif














