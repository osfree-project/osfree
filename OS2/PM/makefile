#
#  Handle makefiles in subdirs
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

!include $(%ROOT)/mk/all.mk

DIRS=Client Server API TestApps

all: .SYMBOLIC
   $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
