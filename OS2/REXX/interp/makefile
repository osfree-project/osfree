#
# A Makefile for kal.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(FILESDIR)$(SEP)os2$(SEP)dll
PROJ     = kal
DESC     = Kernel Abstracion Layer API
OBJS     = ..$(SEP)shared$(SEP)stub.$(O) file.$(O)
ADD_COPT = -i=..$(SEP)include
LINKOPT  = lib os2386.lib,clib3r.lib


!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
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
  $(CP) regina.dll $(DIR)
