# $Id: makefile,v 1.4 2004/08/16 04:51:09 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

PROJ = fdisk
DESC = Disk volumes manager

!include $(%ROOT)/mk/all.mk

all: $(PROJ).exe install

clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)os2
