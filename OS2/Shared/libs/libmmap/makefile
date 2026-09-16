#
# A Makefile for libmmap
# (c) osFree project,
# valerius, 2006/10/30
#

PROJ      = libmmap
TRGT      = $(PROJ).lib

DISABLE_LIB = 1

!include $(%ROOT)tools/mk/libs.mk

TARGETS   = $(PATH)$(TRGT)

$(PATH)$(TRGT): $(MYDIR)$(TRGT)
 @$(CP) $< $^@ $(BLACKHOLE)
