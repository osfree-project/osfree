#  Makefile variables for FreePM
#
#  Viking

PROJ = pm0

srcfiles = $(p)pm0$(e)

#  option stack=3m
# Strange, the server crashes with debug build! No, it crashes for some else reason.
# Found it! To little heapsize! Was only 287 KiB while it assumed 2.6 MiB!
# It must be set with stack= not with heapsize= !
# Changing the stack size has no effect at all.
# Increased the stack for threads
# 65 KiB from 32 KB. Hope it works or else I have to change the stack manually.
# Fixed! With help from Valerius. Added the switch "-sg" which grows the stack.

ADD_COPT    = -bm -od -mf -sg -d2 -mf
INSTALL_PRE = 1
DEST        = os2$(SEP)test

!include $(%ROOT)/mk/appsos2.mk

install_pre: $(PATH)$(PROJ).exe .SYMBOLIC
! ifeq ENV OS/2
 renmodul.exe /i $[@ pmwin fmwin
! else
 @rem
! endif