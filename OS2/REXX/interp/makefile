#
# A Makefile for kal.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = kal
DESC     = Kernel Abstracion Layer API
OBJS     = ..\shared\stub.$(O) file.$(O)
ADD_COPT = -i=..\include
LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
  -@del $(EXEDIR)\rexx.exe
  -@del $(EXEDIR)\regina.exe
  -@del $(EXEDIR)\execiser.exe
  -@del $(DLLDIR)\regina.dll
  @%create wrapper.cmd
  @%append wrapper.cmd @set REGINA_SRCDIRN=.
  @%append wrapper.cmd @$(MAKE) -f makefile.os2.wcc
  @wrapper.cmd
  @del wrapper.cmd
  @move *.exe $(EXEDIR)
  @move regina.dll $(DLLDIR)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
