
#############################################################################
#
# Makefile for TWIN Library
#
# The Willows TWIN Software consists of the Willows TWIN Libraries, and
# the tools necessary to build the Libraries and Applications.
# The components are:
#
# TOOLS		Tools to build the libraries and applications
# PARTS		Subsystems of the library, DOS, multimedia
# DRIVER	The platform dependent graphical interface 
# TWIN		The actual core TWIN library
# LIBS		Support DLL's for TWIN, Keyboard, WinSock, commdlg...
#
# DEMO		Source samples for the TWIN library
#
#############################################################################
#
# These are defaults, and can/will be overwritten by lower level Makefiles
#
#############################################################################

TWINDIR=.
include ./Makefile.config
INSTALLDIR=$(TWINDIR)/$(TWINOPT)

#####################################################################
#
# Tools necessary to build the library
#
#####################################################################

TOOLS = bin/rc

#####################################################################
#
# Components built separately
#
# Debugger vs. Interpreter	binary emulator code
# xdos				dos emulation library 
# multimedia			mmedia.dll code
# dll				support for .dll's
# memory			support for win32 memory functions
#
#####################################################################

PARTS_ALL  = xdos multimedia/mmsystem dll memory

#####################################################################
#
# Optional components
#
# PARTS_CPU  only used on linux, with direct execution ie. CPUTYPE=CPU
# PARTS_INT  only used on risc, with interpreter ie. CPUTYPE=INT
#
######################################################################

PARTS_CPU = debugger readline
PARTS_INT = intp32

PARTS =  $(PARTS_$(CPUTYPE)) $(PARTS_ALL)

#####################################################################
#
# Platform Abstraction Layer
#
# x11 graphics interface 	for support for x11 based systems
#
#####################################################################

DRIVER  = pal/x11

#####################################################################
#
# The twin library itself
#
# libtwin.so			the basic library, built-in user, kernel, gdi
#
#####################################################################

TWIN     = win

#####################################################################
#
# Standard Well-Known Windows DLL's
#
#####################################################################

DLLS      = keyboard shell commdlg winsock sound ddeml lzexpand mmsystem win32
DLLS32    = $(DLLS) commctrl 

LIBS      = $(DLLS$(TWIN_APISIZE))

#####################################################################
#
# Sample code using the library
# 
DEMO    = samples

#####################################################################

include ./makeinclude/Makefile.version

#####################################################################
#
#	default target will build rc and the library
#	world wil build rc, library and samples
#

all:	target
	@echo "Build: Willows Library"
	@echo "Build: $(TWIN)"
	@date
	@echo "========== Tools =========="
	@for i in $(TOOLS); do cd $(TWINDIR)/$$i && $(MAKE) TWINDIR=$(TWINDIR); done
	@echo "========== Library ========"
	@for i in $(PARTS); do cd $(TWINDIR)/$$i && $(MAKE)  TWINDIR=$(TWINDIR); done
	@for i in $(DRIVER); do cd $(TWINDIR)/$$i && $(MAKE)  TWINDIR=$(TWINDIR); done
	@for i in $(TWIN); do cd $(TWINDIR)/$$i && $(MAKE)  -j8 TWINDIR=$(TWINDIR); done
	@for i in $(LIBS); do cd $(TWINDIR)/$$i && $(MAKE)  TWINDIR=$(TWINDIR); done
	@for i in win; do cd $(TWINDIR)/$$i && $(MAKE) xwin$(TWIN_APISIZE) TWINDIR=$(TWINDIR); done
	@date
	@echo "Build: Willows Library complete."

#
#
#

demo:
	@echo "========== Samples ========"
	@for i in $(DEMO); do cd $(TWINDIR)/$$i && $(MAKE) TWINDIR=$(TWINDIR); done


world:	all demo

###########################################################################
#
# Support targets
#
# target:	make sure all dependent directories are built
# clean:	how to clean up the mess we made
# clobber:	how to really clean up the mess	
# depend:	how to make sure we can build things
# install:	how to install the mess we made
#

target: makedepend
	@echo "Building/verifying installation directories"
	@-if [ ! -d $(INSTALLDIR) ]; then mkdir $(INSTALLDIR); fi
	@-if [ ! -d $(INSTALLDIR)/bin ]; then mkdir $(INSTALLDIR)/bin; fi
	@-if [ ! -d $(INSTALLDIR)/lib ]; then mkdir $(INSTALLDIR)/lib; fi 
	@-if [ ! -d $(INSTALLDIR)/include ]; then mkdir $(INSTALLDIR)/include; fi 

makedepend: 
	@for i in $(TOOLS); do touch $(TWINDIR)/$$i/Makedepend.$(TARGET) ; done
	@for i in $(PARTS); do touch $(TWINDIR)/$$i/Makedepend.$(TARGET) ; done
	@for i in $(DRIVER); do touch $(TWINDIR)/$$i/Makedepend.$(TARGET) ; done
	@for i in $(TWIN); do touch $(TWINDIR)/$$i/Makedepend.$(TARGET) ; done
	@for i in $(LIBS); do touch $(TWINDIR)/$$i/Makedepend.$(TARGET) ; done
	@for i in $(DEMO); do touch $(TWINDIR)/$$i/Makedepend.$(TARGET) ; done

#
#
#

clean:	makedepend
	@for i in $(TOOLS); do cd $(TWINDIR)/$$i && $(MAKE) clean TWINDIR=$(TWINDIR) ; done
	@for i in $(PARTS); do cd $(TWINDIR)/$$i && $(MAKE) clean TWINDIR=$(TWINDIR) ; done
	@for i in $(DRIVER); do cd $(TWINDIR)/$$i && $(MAKE) clean TWINDIR=$(TWINDIR) ; done
	@for i in $(TWIN); do cd $(TWINDIR)/$$i && $(MAKE) clean TWINDIR=$(TWINDIR) ; done
	@for i in $(LIBS); do cd $(TWINDIR)/$$i && $(MAKE) clean TWINDIR=$(TWINDIR) ; done
	@for i in samples; do cd $(TWINDIR)/$$i && $(MAKE) clean TWINDIR=$(TWINDIR) ; done
	rm -f ./*/lib/*.o
	rm -f config.cache config.log config.status 
	rm -f Makedepend.$(TARGET)
	rm -f $(TWINDIR)/*/Makedepend.$(TARGET)
	rm -f $(TWINDIR)/*/*/Makedepend.$(TARGET)

#
#
#

clobber: clean
	for i in $(LIBS); do cd $(TWINDIR)/$$i $(MAKE) clobber TWINDIR=$(TWINDIR) ; done


#
#
#
depend: makedepend
	@for i in $(TOOLS); do cd $(TWINDIR)/$$i && $(MAKE) depend TWINDIR=$(TWINDIR) ; done
	@for i in $(PARTS); do cd $(TWINDIR)/$$i && $(MAKE) depend TWINDIR=$(TWINDIR) ; done
	@for i in $(DRIVER); do cd $(TWINDIR)/$$i && $(MAKE) depend TWINDIR=$(TWINDIR) ; done
	@for i in $(TWIN); do cd $(TWINDIR)/$$i && $(MAKE) depend TWINDIR=$(TWINDIR) ; done
	@for i in $(LIBS); do cd $(TWINDIR)/$$i && $(MAKE) depend TWINDIR=$(TWINDIR) ; done
	@for i in $(DEMO); do cd $(TWINDIR)/$$i && $(MAKE) depend TWINDIR=$(TWINDIR) ; done

#
#
#

install: target 
	@echo Copy components in...
	@cp $(TWINDIR)/win/xwin$(TWIN_APISIZE) $(INSTALLDIR)/bin
	@cp $(TWINDIR)/win/lib*.so*  $(INSTALLDIR)/lib
	@cp $(TWINDIR)/include/*.h $(INSTALLDIR)/include
	@cp $(TWINDIR)/bin/rc/rc $(INSTALLDIR)/bin


