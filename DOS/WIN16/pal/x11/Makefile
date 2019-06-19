#########################################################################
#
# @(#)Makefile	1.12
#
# Copyright (c) 1995-1996, Willows Software Inc. All rights reserved.
#
#########################################################################

include $(TWINDIR)/Makefile.config

#########################################################################

TWIN_AUXINC =	-I$(TWINDIR)/win
  
#########################################################################

DRVOBJ  = 	DrvGraphics.o DrvImages.o DrvDC.o DrvFloodFill.o \
		DrvRegions.o DrvText.o DrvUtils.o DrvIPC.o DrvSystem.o \
		DrvMemory.o DrvInit.o RopCodes.o DrvWindows.o DrvCursor.o \
		DrvWinSock.o DrvColors.o DrvEvents.o XICCCM.o DrvKeyboard.o \
		XEvents.o DrvConfig.o DrvTime.o DrvPrinting.o\
		DrvFiles.o $(TARGOBJ) $(TARGTHROBJ)

DRVSRC  =	$(DRVOBJ:.o=.c) $(TARGSRC) $(TARGTHRSRC)

DRVHDR  = 	DrvImages.h DrvDC.h DrvROPCodes.h \
		DrvGraphics.h DrvText.h DrvIPC.h DrvDP.h 


#########################################################################

all:	x11.o $(TWINLIB)/x11.o

nolink: $(DRVOBJ)

x11.o:	$(DRVOBJ) $(TWINLIB)/x11.o
	${LD} -r -o x11.o $(DRVOBJ)
	cp x11.o $(TWINLIB)/x11.o

$(TWINLIB)/x11.o:

DrvSystem_$(TARGET).o: DrvSystem_$(TARGET).s
	$(CC) $(GFLAG) -c $*.s

#########################################################################

ctags:
	ctags *.c *.h >/dev/null 2>&1

depend:
	@-makedepend -f Makedepend.$(TARGET) $(TWIN_SYSINC) $(DRVSRC) 2>/dev/null

clean:	clobber

clobber: 	
	rm -rf $(DRVOBJ) core x11.o

##########################################################################
#
# Source file dependencies are located in Makedepend.$(TARGET)
#
##########################################################################

include Makedepend.$(TARGET)

##########################################################################
# DO NOT DELETE THIS LINE -- make depend depends on it.
