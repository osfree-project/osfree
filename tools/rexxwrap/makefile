# $Id: makefile,v 1.1 2004/08/16 0627:05 prokushev Exp $

PROJ = rexxwrapper
DESC = rexx to executable wrapper

!include $(%ROOT)/mk/tools.mk

TARGETS = $(PATH)$(PROJ).exe

$(PATH)$(PROJ).exe: $(MYDIR)rexxwrap.cmd
  $(REXX) rexxwrap.cmd -program=$^* -rexxfiles=$]@ -srcdir=$(MYDIR) -compiler=wcc -interpreter=os2rexx -intlib=rexx.lib -intincdir=$(%WATCOM)$(SEP)h$(SEP)os2 -compress
!ifeq ENV Windows
  $(REXX) rexxwrap.cmd -program=$^* -rexxfiles=$]@ -srcdir=$(MYDIR) -compiler=wcc -interpreter=regina  -intlib=$(REXX_PATH)$(SEP)lib$(SEP)regina.lib -intincdir=$(REXX_PATH)$(SEP)include -compress
!endif
  $(CP) $(PATH)$(PROJ).exe $(DEST)
