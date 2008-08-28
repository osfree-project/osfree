#  Makefile variables for FreePM
#
#  Viking

PROJ = pmclient
DESC = PM client library
srcfiles = $(p)Fc_main$(e) $(p)F_globals$(e) $(p)F_utils$(e) $(p)Fc_config$(e) &
           $(p)F_errors$(e) $(p)F_debug$(e) # $(p)F_Client$(e) $(p)F_messages$(e)
DEST    = os2$(SEP)dll

#  option stack=3m
# Strange, the server crashes with debug build! No, it crashes for some else reason.
# Found it! To little heapsize! Was only 287 KiB while it assumed 2.6 MiB!
# It must be set with stack= not with heapsize= !
# Changing the stack size has no effect at all.
# Increased the stack for threads
# 65 KiB from 32 KB. Hope it works or else I have to change the stack manually.
# Fixed! With help from Valerius. Added the switch "-sg" which grows the stack.

ADD_COPT =-bm -od -mf -sg -d2 -mf -bd
DLL      = 1
EXPORTS  = InitServerConnection.1,      &
           CloseServerConnection.2,     &
           _F_SendCmdToServer.3,        &
           _F_SendDataToServer.4,       &
           _F_RecvDataFromServer.5,     &
           _F_SendGenCmdToServer.6,     &
           _F_SendGenCmdDataToServer.7, &
           fatal.8

!include $(%ROOT)/mk/appsos2.mk
