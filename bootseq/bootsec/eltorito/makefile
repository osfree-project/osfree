#
# Makefile for OS/3 loader MBR sector.
# 06/10/03 valerius
#

PROJ       = eltorito
PROJ0      = eltorito
TRGT       = $(PROJ).bin
DEFINES    =
ADD_ASMOPT = $(DEFINES)

STARTUP    =
ALL        = $(p)$(PROJ0)$(e)
srcfiles   = $(STARTUP) $(ALL)

!include $(%ROOT)/mk/bootsec.mk
