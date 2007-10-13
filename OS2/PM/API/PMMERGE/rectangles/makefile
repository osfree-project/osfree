#
# A Makefile for pmmerge.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
OBJS     = rectangles.$(O)
ADD_COPT = -i=..$(SEP)include -bd 

!include $(%ROOT)/mk/all.mk

all: $(OBJS) .SYMBOLIC

install: .SYMBOLIC
 $(CP) $(PROJ).exe ..$(SEP)bin

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
