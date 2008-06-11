# $Id: makefile,v 1.2 2004/08/16 04:51:14 prokushev Exp $


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

PROJ = helpmsg
DESC = Help message shower
32_BITS  = 1

!include $(%ROOT)/mk/all.mk

all: $(PROJ).exe .symbolic
 @$(MAKE) $(MAKEOPT) install

# Clean all

clean: .symbolic
 -@del *.oo2 $(LOG)
 -@del *.exe $(LOG)
 -@del *.ao2 $(LOG)
 -@del *.ppo $(LOG)
 -@del *.o   $(LOG)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)os2
