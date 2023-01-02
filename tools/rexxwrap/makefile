# $Id: makefile,v 1.1 2004/08/16 0627:05 prokushev Exp $

PROJ = rexxwrapper
TRGT = $(PROJ).exe
DESC = rexx to executable wrapper

!include $(%ROOT)tools/mk/tools.mk

#TARGETS = $(PATH)$(PROJ).exe

$(PATH)$(PROJ).exe: $(MYDIR)rexxwrap.cmd
!ifeq %OS OS2
  $(REXX) rexxwrap.cmd -program=$^* -rexxfiles=$]@ -srcdir=$(MYDIR) -compiler=wcc -interpreter=os2rexx -intlib=rexx.lib -intincdir=$(%WATCOM)$(SEP)h$(SEP)os2 -compress
!endif
!ifeq %OS Windows
  $(REXX) rexxwrap.cmd -program=$^* -rexxfiles=$]@ -srcdir=$(MYDIR) -compiler=wcc -interpreter=regina  -intlib=$(BLD)lib$(SEP)regina.lib -intincdir=$(BLD)include -compress
!endif
!ifeq %OS LINUX
  $(REXX) ./rexxwrap.cmd -program=$^* -rexxfiles=$]@ -srcdir=$(MYDIR) -compiler=wcc -interpreter=regina  -intlib=$(SEP)usr$(SEP)lib$(SEP)regina.lib -intincdir=$(SEP)usr$(SEP)include -compress
!endif
  $(CP) $(PATH)$(PROJ).exe $(DEST)
