#
#  Handle makefiles in subdirs
#

!include ../build.conf
!include ../mk/site.mk

32_BITS  = 1

!include ../mk/all.mk

DIRS=shared src_c src_s

all: .SYMBOLIC
   $(MAKE) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(MAKE) TARGET=$^@ subdirs
