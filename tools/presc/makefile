# $Id: makefile,v 1.4 2004/08/16 04:51:09 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

DIR = $(ROOT)$(SEP)tools$(SEP)bin
PROJ = presc
DESC = Build level display facility

!include $(%ROOT)/mk/all.mk

all: presc.exe install

presc.exe: presc.rx
  rexxwrapper -program=$(PROJ) -rexxfiles=presc.rx -srcdir=$(%ROOT)$(SEP)tools$(SEP)rexxwrap -compiler=wcc -interpreter=os2rexx -intlib=rexx.lib -intincdir=$(%WATCOM)$(SEP)h$(SEP)os2 -compress
!ifeq ENV Windows
  rexxwrapper -program=$(PROJ) -rexxfiles=presx.rx -srcdir=$(%ROOT)$(SEP)tools$(SEP)rexxwrap -compiler=wcc -interpreter=regina -intlib=$(REXX_PATH)$(SEP)lib$(SEP)regina.lib -intincdir=$(REXX_PATH)$(SEP)include -compress
!endif
  $(CP) $(PROJ).exe $(DIR)
#  rexxwrapper -program=presc.exe -rexxfiles=presc.rx -srcdir=$(%ROOT)$(SEP)tools$(SEP)rexxwrap -compiler=wcc -interpreter=regina -intlib=$(%REXX_PATH)$(SEP)lib$(SEP)regina.lib -intincdir=$(%REXX_PATH)$(SEP)include -compress

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)os2
