# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = som
TRGT = $(PROJ).dll
DLL = 1
DESC = SOM Kernel
srcfiles = $(p)somkern$(e) &
	 $(p)rhbsomid$(e) &
	 $(p)somalloc$(e) &
	 $(p)somobjva$(e) &
	 $(p)somapi$(e) &
	 $(p)sommutex$(e) &
	 $(p)somkpath$(e) &
	 $(p)somshlb$(e)

!ifeq %OS OS2
ADD_OPTS = 
defs = $(%WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
ADD_OPTS = -DSTRICT
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS WIN64
ADD_OPTS = -i=$(SOM_DIR)som$(SEP)win32 -D_WIN32 -DSTRICT
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS DOS
ADD_OPTS = 
defs = $(%WATCOM)$(SEP)h$(SEP)dos
!endif
!ifeq %OS LINUX
ADD_OPTS = 
defs = $(%WATCOM)$(SEP)lh
!endif

SOM_DIR=$(MYDIR)..$(SEP)contrib$(SEP)
SOM_INCL= -i=$(SOM_DIR)include &
         -i=$(SOM_DIR)som$(SEP)$(%OS) &
         -i=$(SOM_DIR)som$(SEP)include &
		 -i=$(SOM_DIR)som$(SEP)src &
		 -i=$(SOM_DIR)somcdr$(SEP)include &
		 -i=$(SOM_DIR)rhbmtut$(SEP)include &
		 -i=$(SOM_DIR)somcdr$(SEP)src 	&
		 -i=$(ROOT)build$(SEP)include$(SEP)somidl$(SEP)$(%OS) &
		 -i=$(SOM_DIR)somkpub$(SEP)include &
		 -i=$(SOM_DIR)somtk$(SEP)include
#-i=$(INTDIR) &

ADD_COPT = -dNDEBUG -i=$(defs)  -bc -bm -br -dWIN32_LEAN_AND_MEAN -dHAVE_CONFIG_H &
           $(SOM_INCL) -DBUILD_SOM $(ADD_OPTS)

ADD_LINKOPT = lib clbrdll
#STACKSIZE = 0x20000
#UNI2H = 1
C = r

!include $(%ROOT)/mk/tools.mk

.c: $(MYDIR)..$(SEP)contrib$(SEP)som$(SEP)src;$(SOM_DIR)somid$(SEP)src

.h: $(MYDIR)..$(SEP)contrib$(SEP)include

#.$(O): $(PATH)






----------------------------------


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
