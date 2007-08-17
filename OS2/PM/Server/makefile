#  Makefile variables for FreePM
#
#  Viking


ROOT=..\..

!include ../../build.conf
!include ../../mk/site.mk

32_BITS  = 1
PROJ=FreePM
OBJS = F_session.$(O) F_utils.$(O) Fs_config.$(O) Fs_globals.$(O) &
       Fs_hab.$(O) Fs_main.$(O) Fs_pipe.$(O) Fs_PMdev.$(O) &
       Fs_queue.$(O) Fs_wnd.$(O) F_debug.$(O) F_DeskTop.$(O) &
       F_errors.$(O) &

#  option stack=3m
# Strange, the server crashes with debug build! No, it crashes for some else reason.
# Found it! To little heapsize! Was only 287 KiB while it assumed 2.6 MiB!
# It must be set with stack= not with heapsize= !
# Changing the stack size has no effect at all.
# Increased the stack for threads
# 65 KiB from 32 KB. Hope it works or else I have to change the stack manually.
# Fixed! With help from Valerius. Added the switch "-sg" which grows the stack.

ADD_COPT=-bm -od -mf -sg -d2 -mf
LINKOPT=lib pm_server.lib lib gdlib.lib

!include ../../mk/all.mk


#defines files mask for clean target
CLEANMASK = *.lnk *.map *.obj *.err *.log *.bak *.sym

all: $(PROJ).exe .SYMBOLIC

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

.IGNORE
