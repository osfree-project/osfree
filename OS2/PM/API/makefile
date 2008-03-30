#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

DIRS = shared PMMERGE PMDRAG PMWIN
#       PMPIC PMPRE PMREXXIO PMSHLTKT PMSPL &
#       PMUNIF PMVIOP PMWINX REGISTRY PMGRE PMMLE PMSHAPI
#PMATM PMBIDI PMCLIP PMCTLS PMDDE PMDDEML PMGPI &

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
