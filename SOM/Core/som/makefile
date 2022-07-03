# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = som
TRGT = $(PROJ).dll
DLL = 1
DESC = SOM Kernel
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata
srcfiles = $(p)somkern$(e) &
	 $(p)rhbsomid$(e) &
	 $(p)somalloc$(e) &
	 $(p)somobjva$(e) &
	 $(p)somapi$(e) &
	 $(p)sommutex$(e) &
	 $(p)somkpath$(e) &
	 $(p)rhbseh2$(e) &
!ifeq %OS LINUX
	 $(p)rhbseh$(e) &
!endif
	 $(p)somshlb$(e)

DEST    = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)common$(SEP)dll
#INSTALL_ADD = 1
!if 0
EXPORTS = somMainProgram, &
		DllMain, & 
		SOMInitModule, & 
			SOMObjectClassData , &
			SOMObjectCClassData , &
			SOM_WarnLevel                       , &
			SOMClassMgrObject                   , &
			SOMRequestMutexSem                  , &
			SOMReleaseMutexSem                  , &
			SOMMalloc                           , &
			SOMCalloc                           , &
			SOMRealloc                          , &
			SOMFree                             , &
			SOMClassCClassData                  , &
			SOMClassClassData                   , &
			SOM_TraceLevel                      , &
			SOM_AssertLevel                     , &
			SOMError                            , &
			SOMGetThreadId                      , &
			SOM_MaxThreads                      , &
			SOMClassMgrClassData                , &
			SOMCreateMutexSem                   , &
			SOMClassMgrCClassData               , &
			SOMDeleteModule                     , &
			SOMLoadModule                       , &
			SOMClassInitFuncName                , &
			SOM_MajorVersion                    , &
			SOM_MinorVersion                    , &
			SOMDestroyMutexSem                  , &
			SOMObjectNewClass                   , &
			SOMClassNewClass                    , &
			SOMClassMgrNewClass                 , &
			somRegisterClassLibrary             , &
			somEnvironmentNew                   , &
			somDataResolve                      , &
			somDataResolveChk                   , &
			somResolve                          , &
			somParentNumResolve                 , &
			somClassResolve                     , &
			somAncestorResolve                  , &
			somResolveByName                    , &
			somParentResolve                    , &
			somGetClassFromMToken               , &
			somBuildClass                       , &
			somConstructClass                   , &
			somApply                            , &
			somEnvironmentEnd                   , &
			somCheckId                          , &
			somRegisterId                       , &
			somIdFromString                     , &
			somStringFromId                     , &
			somCompareIds                       , &
			somUniqueKey                        , &
			somTotalRegIds                      , &
			somSetExpectedIds                   , &
			somBeginPersistentIds               , &
			somEndPersistentIds                 , &
			somAbnormalEnd                      , &
			somLPrintf                          , &
			somPrintf                           , &
			somVprintf                          , &
			somPrefixLevel                      , &
			somIsObj                            , &
			somTestCls                          , &
			somTest                             , &
			somAssert                           , &
			somSetOutChar                       , &
			somExceptionId                      , &
			somExceptionValue                   , &
			somExceptionFree                    , &
			somSetException                     , &
			somGetGlobalEnvironment             , &
			somIdMarshal                        , &
			somStartCriticalSection             , &
			somEndCriticalSection               , &
			SOMOutCharRoutine                   , &
			somCheckArgs                        , &
			somFreeThreadData                   , &
			somva_SOMObject_somDispatchV        , &
			somva_SOMObject_somDispatchL        , &
			somva_SOMObject_somDispatchA        , &
			somva_SOMObject_somDispatchD        , &
			somva_SOMObject_somDispatch         , &
			somva_SOMObject_somClassDispatch    , &
			va_SOMObject_somDispatchV           , &
			va_SOMObject_somDispatchL           , &
			va_SOMObject_somDispatchA           , &
			va_SOMObject_somDispatchD           , &
			va_SOMObject_somDispatch            , &
			va_SOMObject_somClassDispatch

!endif

SOM_DIR=$(SRC)contrib$(SEP)
SOM_INCL= -i=$(SOM_DIR)include &
         -i=$(SOM_DIR)som$(SEP)include &
		 -i=$(SOM_DIR)som$(SEP)src &
		 -i=$(SOM_DIR)somcdr$(SEP)include &
		 -i=$(SOM_DIR)rhbmtut$(SEP)include &
		 -i=$(SOM_DIR)somcdr$(SEP)src 	&
		 -i=$(%OS2TK)$(SEP)som$(SEP)include &
		 -i=$(SOM_DIR)somkpub$(SEP)include &
		 -i=$(SOM_DIR)somtk$(SEP)include

!ifdef HOSTBLD

SRC = $(MYDIR)..$(SEP)..$(SEP)
!ifeq %OS OS2
ADD_OPTS = -i=$(SOM_DIR)som$(SEP)win32 -D_PLATFORM_OS2_ -DSTRICT -DBUILD_RHBSEH2
ADD_LINKOPT = lib clib3r
defs = $(%WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
ADD_OPTS = -DSTRICT -bc -bm -br -dWIN32_LEAN_AND_MEAN
ADD_LINKOPT = lib clbrdll
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS WIN64
ADD_LINKOPT = lib clbrdll
ADD_OPTS = -i=$(SOM_DIR)som$(SEP)win32 -D_WIN32 -DSTRICT -DBUILD_RHBSEH2 -bc -bm -br -dWIN32_LEAN_AND_MEAN
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS DOS
ADD_OPTS = 
defs = $(%WATCOM)$(SEP)h$(SEP)dos
!endif
!ifeq %OS LINUX
ADD_OPTS = -i=$(SOM_DIR)som$(SEP)win32 -D_PLATFORM_UNIX_ -D_PLATFORM_X11_ -DSTRICT -DBUILD_RHBSEH2 &
    -DHAVE_UNISTD_H -DHAVE_SIGNAL_H
ADD_LINKOPT = lib clib3r
defs = $(%WATCOM)$(SEP)lh
!endif

!else

SRC = $(MYDIR)..$(SEP)
ADD_OPTS = -i=$(SOM_DIR)som$(SEP)win32 -D_PLATFORM_OS2_ -DSTRICT -DBUILD_RHBSEH2
ADD_LINKOPT = lib clib3r
defs = $(%WATCOM)$(SEP)h$(SEP)os2

!endif

ADD_COPT = -dNDEBUG -i=$(defs) -dHAVE_CONFIG_H &
           $(SOM_INCL) -DBUILD_SOM -i=$(BLD)host$(SEP)$(%HOST)$(SEP)$(RELDIR) $(ADD_OPTS)
#STACKSIZE = 0x20000
#UNI2H = 1
C = r

!ifdef HOSTBLD
!include $(%ROOT)/mk/tools.mk
!else
!include $(%ROOT)/mk/appsos2.mk
#!include $(%ROOT)/mk/libsos2.mk
!endif


.c: $(SRC)contrib$(SEP)som$(SEP)src;$(SOM_DIR)somid$(SEP)src;$(SOM_DIR)rhbseh2$(SEP)src;$(SOM_DIR)rhbseh$(SEP)src

.h: $(SRC)contrib$(SEP)include

#install_add: .SYMBOLIC $(PATH)som.dll
# @$(MDHIER) $(%OS2TK)$(SEP)som$(SEP)common$(SEP)dll
# $(verbose)$(CP) $(PATH)som.dll $(%OS2TK)$(SEP)som$(SEP)common$(SEP)dll $(BLACKHOLE)

#.$(O): $(PATH)

#----------------------------------


#CC_OPT=$(STDOPT) $(SOM_INCL) 



#$(TARGET): $(OBJS)
#	$(LINKDLL) $(LINKDLL_HEAD) \
#		-o $@\
#		$(OBJS) \
#		`$(SHLB_EXP) som som` \
#		`$(SHLB_ENT) somEnvironmentNew` \
#		`$(ARLB_REF) rhbmtuta rhbmtuta` \
#		`$(SHLB_MAP) som som` \
#		$(STDLIB) \
#		$(LINKDLL_TAIL)

#$(INTDIR)/rhbsomid.o: ../../somid/src/rhbsomid.c 
#	$(CC_DLL) $(CC_OPT) -c ../../somid/src/rhbsomid.c -o $@

#$(INTDIR)/somalloc.o: ../src/somalloc.c 
#	$(CC_DLL) $(CC_OPT) -c ../src/somalloc.c -o $@ 

#$(INTDIR)/sommutex.o: ../src/sommutex.c 
#	$(CC_DLL) $(CC_OPT) -c ../src/sommutex.c -o $@ 

#$(INTDIR)/somapi.o: ../src/somapi.c 
#	$(CC_DLL) $(CC_OPT) -c ../src/somapi.c -o $@ 

#$(INTDIR)/somkpath.o: ../src/somkpath.c 
#	$(CC_DLL) $(CC_OPT) -c ../src/somkpath.c -o $@ 

#$(INTDIR)/somobjva.o: ../src/somobjva.c 
#	$(CC_DLL) $(CC_OPT) -c ../src/somobjva.c -o $@ 

#$(INTDIR)/somshlb.o: ../src/somshlb.c ../../shlbtest/$(PLATFORM)/$(BUILDTYPE)/shlbtest.h
#	$(CC_DLL) $(CC_OPT) -I../../shlbtest/$(PLATFORM)/$(BUILDTYPE) -c ../src/somshlb.c -o $@ 

#$(INTDIR)/somkern.o: ../src/somkern.c $(INTDIR)/somkernp.kih $(INTDIR)/somirdll.h ../src/somobj.c ../src/somcls.c ../src/somcm.c
#	$(CC_DLL) $(CC_OPT) -c ../src/somkern.c -o $@ 

#$(INTDIR)/somirdll.h: ../unix/som.mak 
#	$(SHLB_DLO) -s somir >$@
#	cat $@
