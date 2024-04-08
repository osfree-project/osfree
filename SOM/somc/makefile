
PROJ = somc
TRGT = $(PROJ).dll
srcfiles = $(p)somc$(e) &
	$(p)symtab$(e) &
	$(p)emitlib$(e) &
	$(p)modifiers$(e) &
	$(p)smlib$(e)

DEST = os2tk45$(SEP)som$(SEP)common$(SEP)dll

DLL=1

#	TCCalloc.1, &
#     TCFree.                                       2, &
#     TCMalloc.                                     3, &
#     TCRealloc.                                    4, &
#     _DLL_InitTerm.                                5, &
#     get_double_alignment.                         6, &
#     somVaBuf_add.                                 7, &
#     somVaBuf_create.                              8, &
#     somVaBuf_destroy.                             9, &
#     somVaBuf_get_valist.                         10, &

EXPORTS = somtAncestorClass.                           11, &
     somtCPP.                                     12, &
#     somtDllStem.                                 13, &
     somtGetReintroducedMethods.                  14, &
     somtInitialiseCreatetc.                      15, &
     somtInitialiseEmitlib.                       16, &
     somtInitialiseSmmeta.                        17, &
     somtInitialiseSmsmall.                       18, &
     somtInitialiseSmtypes.                       19, &
     somtInitialiseSomc.                          20, &
#     somtParentNumFromName.                       21, &
     somtaddEntry.                                22, &
     somtaddEntryBuf.                             23, &
     somtaddExt.                                  24, &
     somtaddGAtt.                                 25, &
     somtaddHeader.                               26, &
     somtaddstar.                                 27, &
     somtallocBuf.                                28, &
     somtallocDataList.                           29, &
     somtallocMethodList.                         30, &
     somtallocMlist.                              31, &
     somtargFlag.                                 32, &
     somtarrayToPtr.                              33, &
#     somtattMap.                                  34, &
     somtattNormalise.                            35, &
#     somtattalloc.                                36, &
     somtattjoin.                                 37, &
     somtbasename.                                38, &
     somtcalcFileName.                            39, &
     somtcalloc.                                  40, &
     somtclassIdlCall.                            41, &
     somtclassList.                               42, &
     somtclassName.                               43, &
     somtclassPrefix.                             44, &
     somtcleanFiles.                              45, &
     somtcleanFilesFatal.                         46, &
     somtcleanup_f.                               47, &
     somtclsfilename.                             48, &
     somtclsname.                                 49, &
     somtcreateMemBuf.                            50, &
     somtcreateStab.                              51, &
     somtcreateTypeCodes.                         52, &
     somtctos.                                    53, &
     somtdbcsLastChar.                            54, &
     somtdbcsPostincr.                            55, &
     somtdbcsPreincr.                             56, &
     somtdbcsScan.                                57, &
     somtdbcsStrchr.                              58, &
     somtdbcsStrrchr.                             59, &
     somtdbcsStrstr.                              60, &
     somtdbcslowercase.                           61, &
     somtdbcsuppercase.                           62, &
     somtdeclareIdlVarargs.                       63, &
     somtdiskFull.                                64, &
     somtdupMlist.                                65, &
     somtdymain.                                  66, &
#     somtemitClassDataTokens.                     67, &
     somtemitModule.                              68, &
     somtemitModuleTypes.                         69, &
     somtemitPassthru.                            70, &
     somtemitPredefinedTcConstants.               71, &
#     somtemitPredefinedTypeCode.                  72, &
     somtemitReturns.                             73, &
     somtemitTcConstant.                          74, &
     somtemitTypes.                               75, &
     somtepname.                                  76, &
     somteptotype.                                77, &
     somterror.                                   78, &
     somterrorSL.                                 79, &
     somtexit.                                    80, &
     somtext.                                     81, &
     somtfatal.                                   82, &
     somtfatalSL.                                 83, &
     somtfclose.                                  84, &
     somtfcloseSL.                                85, &
     somtfexists.                                 86, &
     somtfilePath.                                87, &
     somtfileStem.                                88, &
     somtfindBaseEp.                              89, &
     somtfindBaseEpNonPtr.                        90, &
     somtfindMethodName.                          91, &
     somtfree.                                    92, &
     somtfreeDataList.                            93, &
     somtfreeMethodList.                          94, &
     somtfreeMlist.                               95, &
     somtfreeStab.                                96, &
     somtfreeWorld.                               97, &
     somtfullComment.                             98, &
     somtfullPrototype.                           99, &
     somtfullTypedef.                            100, &
     somtg_buf.                                  101, &
     somtg_f.                                    102, &
     somtg_membuf.                               103, &
     somtg_sccsid.                               104, &
     somtg_somlink.                              105, &
     somtgenAttStubs.                            106, &
     somtgenSeqName.                             107, &
     somtgetAbistyle.                            108, &
     somtgetAtt.                                 109, &
     somtgetDesc.                                110, &
     somtgetEntry.                               111, &
     somtgetNonRepeatedParent.                   112, &
#     somtgetOidlAtt.                             113, &
     somtgetType.                                114, &
     somtgetVersion.                             115, &
#     somtgetap.                                  116, &
     somtgetatt.                                 117, &
     somtgetdatt.                                118, &
     somtgetgatt.                                119, &
     somtglbdefs.                                120, &
     somthandleDiskFull.                         121, &
     somticstrcmp.                               122, &
     somtimplicit.                               123, &
     somtimplicitArgs.                           124, &
     somtimplicitMeta.                           125, &
     somtincludeOnce.                            126, &
#     somtinitMalloc.                             127, &
     somtinitialiseMeta.                         128, &
     somtinternal.                               129, &
     somtinternalSL.                             130, &
     somtinternal_f.                             131, &
     somtipcbuf.                                 132, &
     somtisDbcs.                                 133, &
#     somtisInheritedModifierSet.                 134, &
     somtisMutRef.                               135, &
#     somtisSpecialMethod.                        136, &
     somtisparent.                               137, &
     somtisvoid.                                 138, &
     somtkeyword.                                139, &
     somtlistAttribute.                          140, &
#     somtload.                                   141, &
     somtlowercase.                              142, &
     somtmajorVersion.                           143, &
     somtmalloc.                                 144, &
     somtmclose.                                 145, &
     somtmeta.                                   146, &
     somtmget.                                   147, &
     somtminorVersion.                           148, &
     somtmlistend.                               149, &
     somtmodule.                                 150, &
     somtmopen.                                  151, &
     somtmp.                                     152, &
     somtmprintf.                                153, &
     somtmrierror.                               154, &
     somtmrifatal.                               155, &
     somtmriinternal.                            156, &
     somtmrimsg.                                 157, &
     somtmriwarn.                                158, &
     somtmsg.                                    159, &
     somtmsgSL.                                  160, &
     somtnewMethodsCount.                        161, &
     somtnextword.                               162, &
#     somtnotc.                                   163, &
     somtnthArg.                                 164, &
     somtobseleteHeaderFile.                     165, &
     somtoidlComment.                            166, &
     somtokfopen.                                167, &
     somtokremove.                               168, &
     somtopenEmitFile.                           169, &
     somtopenEmitFileCalled.                     170, &
     somtopenEmitFileSL.                         171, &
     somtoutPath.                                172, &
     somtpclsfilename.                           173, &
     somtpclsname.                               174, &
     somtprefixedPrototype.                      175, &
     somtprivateMethodsCount.                    176, &
     somtprocessTraps.                           177, &
     somtqualifyNames.                           178, &
     somtreadDescFile.                           179, &
     somtrealExit.                               180, &
     somtrealloc.                                181, &
     somtremoveExt.                              182, &
     somtreplaceDataName.                        183, &
     somtresetEmitSignals.                       184, &
     somtresetEmitSignalsSL.                     185, &
     somtreturnsPtr.                             186, &
     somtreturnsStruct.                          187, &
     somtrmSelf.                                 188, &
     somtrmstar.                                 189, &
     somtsatos.                                  190, &
     somtsaved.                                  191, &
     somtscbuf.                                  192, &
     somtscmsg.                                  193, &
     somtsearchFile.                             194, &
     somtsetDefaultDesc.                         195, &
     somtsetEmitSignals.                         196, &
     somtsetInternalMessages.                    197, &
     somtsetTypeDefn.                            198, &
#     somtsetext.                                 199, &
     somtshortArgList.                           200, &
     somtshortDefine.                            201, &
     somtshowVersion.                            202, &
     somtsimpleName.                             203, &
     somtsizeofEntry.                            204, &
     somtskipws.                                 205, &
     somtsmalloc.                                206, &
     somtsmfree.                                 207, &
     somtsomcls.                                 208, &
     somtstab.                                   209, &
     somtstabFirst.                              210, &
     somtstabFirstName.                          211, &
     somtstabNext.                               212, &
     somtstabNextName.                           213, &
     somtstrictidl.                              214, &
     somtstringFmt.                              215, &
     somttcAlignment.                            216, &
     somttcKind.                                 217, &
#     somttcPrint.                                218, &
     somttcSeqFromListString.                    219, &
     somttcSize.                                 220, &
     somttraverseParents.                        221, &
     somttype.                                   222, &
     somtuninitialiseMeta.                       223, &
     somtuniqFmt.                                224, &
     somtuniqString.                             225, &
     somtunload.                                 226, &
     somtunsetEmitSignals.                       227, &
     somtunsetEmitSignalsSL.                     228, &
     somtuppercase.                              229, &
     somtwarn.                                   230, &
     somtwarnSL.                                 231, &
     somtwidenType.                              232, &
     somtwriteaccess.                            233, &
     somtsearchFileSL
#     somvalistGetTarget.                         234, &
#     somvalistSetTarget.                         235, &
#     tciPrintf.                                  236, &
#     tciSetException.                            237


!include $(%ROOT)SOM/common.mk

!ifdef HOSTBLD
OS2TK = $(%OS2TK)$(SEP)
!else
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45$(SEP)
!endif

TARGETS= $(OS2TK)som$(SEP)lib$(SEP)somtk.lib

ADD_COPT = $(ADD_COPT) -i=$(OS2TK)$(SEP)som$(SEP)include -i=$(PORT_BASE)somc$(SEP)include

.c: $(PORT_BASE)somc$(SEP)src

.h: $(PORT_BASE)somc$(SEP)include

$(OS2TK)som$(SEP)lib$(SEP)somtk.lib: $(PATH)somc.dll
	$(verbose)$(SAY) MDHIER   $(OS2TK)som$(SEP)lib $(LOG)
	$(verbose)$(MDHIER) $(OS2TK)som$(SEP)lib
	$(verbose)$(SAY) LIB      $^. $(LOG)
	$(verbose)$(LIB) -b $^@ +$< $(BLACKHOLE)
