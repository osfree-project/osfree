#
#  Handle makefiles in subdirs
#

!include ../../build.conf
!include ../../mk/site.mk

32_BITS  = 1

!include ../../mk/all.mk

DIRS=shared Client Server

all: .SYMBOLIC
   $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
