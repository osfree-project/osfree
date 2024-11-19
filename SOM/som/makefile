PROJ = som
TRGT = $(PROJ).dll
srcfiles = 	$(p)somalloc$(e) &
		$(p)somapi$(e) &
		$(p)somkern$(e) &
		$(p)somkpath$(e) &
		$(p)sommutex$(e) &
		$(p)somobjva$(e) &
		$(p)somshlb$(e) &
		$(p)somunk$(e) &
		$(p)rhbseh2$(e)
!ifeq %OS LINUX
!ifdef HOSTBLD
		$(p)rhbseh$(e)
!endif
!endif


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

EXPORTS = &
	SOMCalloc 		              1, &
	SOMClassCClassData 		      2, &
	SOMClassClassData 		      3, &
	SOMClassInitFuncName 		  4, &
	SOMClassMgrCClassData 		  5, &
	SOMClassMgrClassData 		  6, &
	SOMClassMgrObject 		      7, &
	SOMDeleteModule 		      8, &
	SOMError 		              9, &
 	SOMFree 		              10, &
 	SOMLoadModule 		          11, &
 	SOMMalloc 		              12, &
 	SOMObjectCClassData 		  13, &
 	SOMObjectClassData 		      14, &
 	SOMOutCharRoutine 		      15, &
 	SOMRealloc 		              16, &
 	SOM_AssertLevel 		      17, &
 	SOM_TraceLevel 		          18, &
 	SOM_WarnLevel 		          19, &
 	SOMClassMgrNewClass 		  20, &
 	SOMClassNewClass 		      21, &
 	SOMObjectNewClass 		      22, &
 	somAssert 		              23, &
 	somBeginPersistentIds 		  24, &
 	somCheckArgs 		          25, &
 	somCheckId 		              26, &
 	somCompareIds 		          27, &
 	somConstructClass 		      28, &
 	somEndPersistentIds 		  29, &
 	somEnvironmentNew 		      30, &
 	somIdFromString 		      31, &
 	somLPrintf 		              32, &
 	somParentResolve 		      33, &
 	somPrefixLevel 		          34, &
 	somPrintf 		              35, &
 	somRegisterId 		          36, &
 	somResolve 		              37, &
	somSaveMetrics 		          38, &
 	somSetExpectedIds 		      39, &
 	somStringFromId 		      40, &
 	somTest 		              41, &
 	somTestCls 		              42, &
 	somTotalRegIds 		          43, &
 	somUniqueKey 		          44, &
 	somVprintf 		              45, &
	somWriteMetrics 		      46, &
 	somDataResolve                47


!include $(%ROOT)SOM/common.mk

!ifdef HOSTBLD
OS2TK = $(%OS2TK)$(SEP)
!else
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45$(SEP)
!endif

TARGETS= $(PATH)rhbsomuk.h $(PATH)somobj.h $(PATH)somcls.h $(PATH)somcm.h $(PATH)somobj.ih $(PATH)somcls.ih $(PATH)somcm.ih $(PATH)somkernp.kih $(OS2TK)som$(SEP)lib$(SEP)somtk.lib $(FILESDIR)os2$(SEP)dll$(SEP)som.dll

ADD_COPT = $(ADD_COPT) -i=$(OS2TK)$(SEP)som$(SEP)include &
		 -i=$(PORT_BASE)som$(SEP)include &
		 -i=$(PORT_BASE)somcdr$(SEP)include &
		 -i=$(PORT_BASE)somcdr$(SEP)src &
		 -i=$(PORT_BASE)rhbmtut$(SEP)include &
		 -i=$(PORT_BASE)somir$(SEP)include &
		 -i=$(PORT_BASE)som$(SEP)src &
                 -D_PLATFORM_OS2_  -DBUILD_SOM -DSOMKERN_DLL_TERM -DBUILD_RHBSEH2  # -DSTRICT 

.c: $(PORT_BASE)som$(SEP)src;$(PORT_BASE)rhbseh2$(SEP)src

.h: $(PORT_BASE)som$(SEP)include


$(OS2TK)som$(SEP)lib$(SEP)somtk.lib: $(PATH)som.dll
	$(verbose)$(SAY) MDHIER   $(OS2TK)som$(SEP)lib $(LOG)
	$(verbose)$(MDHIER) $(OS2TK)som$(SEP)lib
	$(verbose)$(SAY) LIB      $^. $(LOG)
	$(verbose)$(LIB) -b $^@ +$< $(BLACKHOLE)

SCOPT =     -I$(PORT_BASE)somkpub$(SEP)som &
            -I$(PORT_BASE)somd &
            -I$(PORT_BASE)somestrm &
            -I$(PORT_BASE)somnmf &
            -I$(PORT_BASE)somabs1 &
            -I$(PORT_BASE)somu &
            -I$(PORT_BASE)somcorba &
            -I$(PORT_BASE)somref &
            -I$(PORT_BASE)somos &
            -I$(PORT_BASE)somcslib &
            -I$(PORT_BASE)somir

$(PATH)somobj.h: $(PORT_BASE)somkpub$(SEP)som$(SEP)somobj.idl
 $(verbose)$(SAY) SC $<
 $(verbose)$(SC) -D__PRIVATE__ $(SCOPT) -sh -I $(DEST) -v $< -o $(PATH) $(BLACKHOLE)

$(PATH)somcls.h: $(PORT_BASE)somkpub$(SEP)som$(SEP)somcls.idl
 $(verbose)$(SAY) SC $<
 $(verbose)$(SC) -D__PRIVATE__ $(SCOPT) -sh -I $(DEST) -v $< -o $(PATH) $(BLACKHOLE)

$(PATH)somcm.h: $(PORT_BASE)somkpub$(SEP)som$(SEP)somcm.idl
 $(verbose)$(SAY) SC $<
 $(verbose)$(SC) -D__PRIVATE__ $(SCOPT) -sh -I $(DEST) -v $< -o $(PATH) $(BLACKHOLE)

$(PATH)rhbsomuk.h: $(PORT_BASE)somcdr$(SEP)rhbsomuk.idl
 $(verbose)$(SAY) SC $<
 $(verbose)$(SC) -D__PRIVATE__ $(SCOPT) -sh -I $(DEST) -v $< -o $(PATH) $(BLACKHOLE)

$(PATH)somobj.ih: $(PORT_BASE)somkpub$(SEP)som$(SEP)somobj.idl
 $(verbose)$(SAY) SC $<
 $(verbose)$(SC) -D__PRIVATE__ $(SCOPT) -sih -I $(DEST) -v $< -o $(PATH) $(BLACKHOLE)

$(PATH)somcls.ih: $(PORT_BASE)somkpub$(SEP)som$(SEP)somcls.idl
 $(verbose)$(SAY) SC $<
 $(verbose)$(SC) -D__PRIVATE__ $(SCOPT) -sih -I $(DEST) -v $< -o $(PATH) $(BLACKHOLE)

$(PATH)somcm.ih: $(PORT_BASE)somkpub$(SEP)som$(SEP)somcm.idl
 $(verbose)$(SAY) SC $<
 $(verbose)$(SC) -D__PRIVATE__ $(SCOPT) -sih -I $(DEST) -v $< -o $(PATH) $(BLACKHOLE)


$(PATH)somdtype.h: $(PORT_BASE)somd$(SEP)somdtype.idl
 $(verbose)$(SAY) SC $<
 $(verbose)$(SC) -D__PRIVATE__ $(SCOPT) -sh -I $(DEST) -v $< -o $(PATH) $(BLACKHOLE)

$(PATH)containd.h: $(PORT_BASE)somir$(SEP)containd.idl
 $(verbose)$(SAY) SC $<
 $(verbose)$(SC) -D__PRIVATE__ $(SCOPT) -sh -I $(DEST) -v $< -o $(PATH) $(BLACKHOLE)

$(PATH)somkernp.kih: $(PORT_BASE)som$(SEP)somkernp.idl
 $(verbose)$(SAY) SC $<
 $(verbose)$(SC) -D__PRIVATE__ -D__GENERATE_SOM__ $(SCOPT) -skih -I $(DEST) -v $< -o $(PATH) $(BLACKHOLE) 

$(FILESDIR)os2$(SEP)dll$(SEP)som.dll: $(PATH)som.dll
 $(verbose)$(SAY) INST     $^. $(LOG)
 $(verbose)$(CP) $< $^@ $(BLACKHOLE)
