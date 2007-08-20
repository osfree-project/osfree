#
#  Handle makefiles in subdirs
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

!include $(%ROOT)/mk/all.mk

DIRS=win1

all: .SYMBOLIC
   $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
