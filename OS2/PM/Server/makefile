#  Makefile variables for FreePM
#
#  Viking

PROJ = FreePM
DESC = FreePM Server
DIRS = drivers
srcfiles = $(p)F_session$(e) $(p)F_utils$(e) $(p)Fs_config$(e) $(p)Fs_globals$(e) &
           $(p)Fs_hab$(e) $(p)Fs_main$(e) $(p)Fs_pipe$(e) &
           $(p)Fs_queue$(e) $(p)Fs_wnd$(e) $(p)F_debug$(e) $(p)F_DeskTop$(e) &
           $(p)F_errors$(e) $(p)F_GPI$(e) $(p)F_hab$(e) $(p)WindowClass$(e)

#  option stack=3m
# Strange, the server crashes with debug build! No, it crashes for some else reason.
# Found it! To little heapsize! Was only 287 KiB while it assumed 2.6 MiB!
# It must be set with stack= not with heapsize= !
# Changing the stack size has no effect at all.
# Increased the stack for threads
# 65 KiB from 32 KB. Hope it works or else I have to change the stack manually.
# Fixed! With help from Valerius. Added the switch "-sg" which grows the stack.

ADD_COPT    = -bm -od -mf -sg -d2 -mf
ADD_LINKOPT = lib gdlib.lib

!include $(%ROOT)/mk/appsos2.mk

TARGETS = subdirs $(PATH)$(PROJ).exe
TARGET = all

qwe: .SYMBOLIC
 $(SAY) $(LINKOPT)