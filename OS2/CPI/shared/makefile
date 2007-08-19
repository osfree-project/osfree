#  Makefile for shared objects
#
#  osFree project, 2007, prokushev


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

OBJS = stub.$(O)

ADD_COPT=-bm -od -mf -sg -d2 -mf

!include $(%ROOT)/mk/all.mk

all: $(OBJS)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
