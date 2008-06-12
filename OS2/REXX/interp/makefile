#
# A Makefile for kal.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)$(SEP)files$(SEP)os2$(SEP)dll
PROJ     = kal
DESC     = Kernel Abstracion Layer API
OBJS     = ..$(SEP)shared$(SEP)stub.$(O) file.$(O)
ADD_COPT = -i=..$(SEP)include
LINKOPT  = lib os2386.lib,clib3r.lib

EXEDIR = $(DIR)$(SEP)..
DLLDIR = $(DIR)

!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
  -@del $(EXEDIR)$(SEP)rexx.exe
  -@del $(EXEDIR)$(SEP)regina.exe
  -@del $(EXEDIR)$(SEP)execiser.exe
  -@del $(DLLDIR)$(SEP)regina.dll
  @%create wrapper.cmd
  @%append wrapper.cmd @set REGINA_SRCDIRN=.
  @%append wrapper.cmd @wmake -f makefile.os2.wcc all
  @wrapper.cmd
  @$(RM) wrapper.cmd
  @$(MAKE) $(MAKEOPT) install

.IGNORE
clean: .SYMBOLIC
  $(SAY) Making clean... $(LOG)
  $(CLEAN_CMD)

install: .SYMBOLIC
  $(SAY) Making install... $(LOG)
  @move *.exe $(EXEDIR)
  @move regina.dll $(DLLDIR)
