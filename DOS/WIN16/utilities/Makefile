#########################################################################
#
#	@(#)Makefile	1.13
#
#	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
#
#	Makefile for WUtilities Library, a support library for common controls
#
#########################################################################

include $(TWINDIR)/Makefile.config
TWIN_AUXINC     = -I$(TWINDIR)/win

.SUFFIXES:	.cpp
.cpp.o:
	$(CXX) $(CFLAGS) -c $<

#########################################################################

WUTILITIESOBJ = \
	WArray.o \
	WBlock.o \
	WChunk.o \
	WElement.o \
	WGlobalHeap.o \
	WHandle.o \
	WHeap.o \
	WHeapFuncs.o \
	WSharedLib.o \
	WString.o \
	WUtilities.o \
	Utilities.res.o \
	Utilities_Init.o

WUTILITIESSRC = \
	WArray.cpp \
	WBlock.cpp \
	WChunk.cpp \
	WElement.cpp \
	WGlobalHeap.cpp \
	WHandle.cpp \
	WHeap.cpp \
	WHeapFuncs.cpp \
	WSharedLib.cpp \
	WString.cpp \
	WUtilities.cpp \
	Utilities.res.c \
	Utilities_Init.cpp

WUTILITIESRESOURCE = \
	Utilities.rc

WUTILITIESHDR = 

WUTILITIESLIB = libutilities32$(BLD_LIBEXT)

#########################################################################

all:	$(WUTILITIESLIB)

Utilities.res.c: Utilities.rc Utilities.def
	$(RC) -I$(TWINDIR)/win -I$(TWINDIR)/include $(TWIN_SYSINC) -C Utilities_Init.cpp -EUtilities_LibMain -DWIN32 Utilities.rc

$(WUTILITIESLIB): $(WUTILITIESOBJ)
	rm -f $(WUTILITIESLIB)
	$(CCSHARE) $(LDSHARED) -o $(WUTILITIESLIB) $(WUTILITIESOBJ) $(SHARELIB)
	cp $(WUTILITIESLIB) $(TWINDIR)/win

#########################################################################

ctags:
	ctags *.c *.h >/dev/null 2>&1

depend:
	makedepend -f Makedepend.$(TARGET) -I. $(TWIN_SYSINC) $(TWIN_WININC) $(WUTILITIESSRC)

clean:	clobber

clobber: 	
	rm -rf $(WUTILITIESOBJ) $(WUTILITIESLIB)

##########################################################################
#
# Source file dependencies are located in Makedepend.$(TARGET)
#
##########################################################################

#include $(TWINDIR)/Makedepend.$(TARGET)

##########################################################################
# DO NOT DELETE THIS LINE -- make depend depends on it.
