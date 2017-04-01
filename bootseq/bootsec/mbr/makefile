#
# Makefile for OS/3 loader MBR sector.
# 06/10/03 valerius
#

PROJ       = mbr
PROJ0      = mbr
TRGT       = $(PROJ).bin
DEFINES    =
ADD_ASMOPT = $(DEFINES) -i=$(%ROOT)include

STARTUP    =
ALL        = $(p)$(PROJ0)$(e)
srcfiles   = $(STARTUP) $(ALL)

!include $(%ROOT)/mk/bootsec.mk
