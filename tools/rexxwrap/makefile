# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

32_BITS  = 1

DIR = $(ROOT)$(SEP)tools$(SEP)bin
PROJ = rexxwrapper
DESC = rexx to executable wrapper

!include $(%ROOT)/mk/all.mk

TARGETS = $(DIR)$(SEP)$(PROJ).exe $(DIR)$(SEP)$(PROJ).sym

$(DIR)$(SEP)$(PROJ).exe: rexxwrap.cmd
  $(REXX) rexxwrap.cmd -program=$(PROJ) -rexxfiles=rexxwrap.cmd -srcdir=$(%ROOT)$(SEP)tools$(SEP)rexxwrap -compiler=wcc -interpreter=os2rexx -intlib=rexx.lib -intincdir=$(%WATCOM)$(SEP)h$(SEP)os2 -compress
!ifeq ENV Windows
  $(REXX) rexxwrap.cmd -program=$(PROJ) -rexxfiles=rexxwrap.cmd -srcdir=$(%ROOT)$(SEP)tools$(SEP)rexxwrap -compiler=wcc -interpreter=regina -intlib=$(REXX_PATH)$(SEP)lib$(SEP)regina.lib -intincdir=$(REXX_PATH)$(SEP)include -compress
!endif
  $(CP) $(PROJ).exe $(DIR)
