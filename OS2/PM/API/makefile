#
# A Makefile for PM API DLLs
# (c) osFree project,
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

DIRS = shared PMMERGE PMDRAG PMWIN PMMLE PMSHAPI PMGRE
#       PMPIC PMSHLTKT PMSPL PMPRE &
#       PMVIOP PMWINX REGISTRY
#PMATM PMUNIF PMBIDI PMCLIP PMCTLS PMDDE PMDDEML PMGPI &

!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
