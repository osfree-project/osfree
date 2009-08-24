#
# Makefile for OS/3 loader bootsector.
# 06/10/03 valerius
#

#
# There are three variants of the bootsector.
# 1st variant (bootsect_1.bin) supports only
# contiguous muFSD's, e.g., in a partition
# boot block. The 2nd variant (bootsect.bin) is
# suitable for loading muFSD from a fragmented
# file from partition data area, like from FAT
# or foreign (like ext2fs etc.) filesystem
# without a special boot block. (but the size of
# file is limited to 64 Kb.) The 3rd variant is
# suitable for loading fragmented files of
# arbitrary size.
#

PROJ       = bootsect_2
PROJ0      = bootsect
DEFINES    = -dCODE_2
ADD_ASMOPT = $(DEFINES)

STARTUP    =
ALL        = $(p)$(PROJ0)$(e)

srcfiles   = $(STARTUP) $(ALL)

!include $(%ROOT)/mk/bootsec.mk

.asm: $(MYDIR)..

$(PATH)$(PROJ0).$(O): $(MYDIR)..$(SEP)$(PROJ0).asm
