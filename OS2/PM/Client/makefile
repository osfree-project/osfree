#  Makefile variables for FreePM
#
#  Viking


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
PROJ=pmclient

OBJS = Fc_main.$(O) F_globals.$(O) F_utils.$(O) Fc_config.$(O) &
       F_errors.$(O) F_debug.$(O) # F_Client.$(O) F_messages.$(O)


#  option stack=3m
# Strange, the server crashes with debug build! No, it crashes for some else reason.
# Found it! To little heapsize! Was only 287 KiB while it assumed 2.6 MiB!
# It must be set with stack= not with heapsize= !
# Changing the stack size has no effect at all.
# Increased the stack for threads
# 65 KiB from 32 KB. Hope it works or else I have to change the stack manually.
# Fixed! With help from Valerius. Added the switch "-sg" which grows the stack.

ADD_COPT=-bm -od -mf -sg -d2 -mf -bd
#LINKOPT=lib pm_client.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2 dll
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.map
 @%append $^&.lnk export
 @%append $^&.lnk   InitServerConnection.1,
 @%append $^&.lnk   CloseServerConnection.2,
 @%append $^&.lnk   _F_SendCmdToServer.3,
 @%append $^&.lnk   _F_SendDataToServer.4,
 @%append $^&.lnk   _F_RecvDataFromServer.5,
 @%append $^&.lnk   _F_SendGenCmdToServer.6,
 @%append $^&.lnk   _F_SendGenCmdDataToServer.7,
 @%append $^&.lnk   fatal.8
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).dll $(FILESDIR)$(SEP)os2$(SEP)dll
 $(CP) freepm.ini $(FILESDIR)$(SEP)os2$(SEP)
