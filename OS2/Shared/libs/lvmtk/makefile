#
# A Makefile for libmmap
# (c) osFree project,
# valerius, 2006/10/30
#

PROJ      = lvm
TRGT      = $(PROJ).lib

!include $(%ROOT)/mk/libs.mk

TARGETS   = $(PATH)$(TRGT)
#DEST = none

$(PATH)$(TRGT): $(MYDIR)lib$(SEP)$(TRGT)
 @$(CP) $< $^@ $(BLACKHOLE)
