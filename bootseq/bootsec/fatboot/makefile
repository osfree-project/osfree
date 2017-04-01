#
# Makefile for OS/3 loader MBR sector.
# 06/10/03 valerius
#

PROJ       = fatboot
PROJ0      = fatboot
TRGT       = $(PROJ).bin
#DEFINES    = -dOS2LDR
DEFINES    =
ADD_ASMOPT = $(DEFINES)

STARTUP    =
ALL        = $(p)$(PROJ0)$(e)
srcfiles   = $(STARTUP) $(ALL)

!include $(%ROOT)/mk/bootsec.mk
