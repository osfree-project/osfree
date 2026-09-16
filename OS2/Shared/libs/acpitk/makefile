#
# A Makefile for libmmap
# (c) osFree project,
# valerius, 2006/10/30
#

PROJ      = acpi32
TRGT      = $(PROJ).lib

DISABLE_LIB = 1

!include $(%ROOT)tools/mk/libs.mk

TARGETS   = $(PATH)$(TRGT)

$(PATH)$(TRGT): $(MYDIR)lib$(SEP)$(TRGT)
 @$(CP) $< $^@ $(BLACKHOLE)
