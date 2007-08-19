#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = msg
DESC     = Message files API
OBJS     = ..\shared\stub.obj
ADD_COPT = -i=..\include
LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk option    quiet
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.map
 @%append $^&.lnk import
 @%append $^&.lnk   DOSPUTMESSAGE DOSCALLS.383,
 @%append $^&.lnk   DOSTRUEGETMESSAGE DOSCALLS.384,
 @%append $^&.lnk   DOSINSMESSAGE DOSCALLS.385,
 @%append $^&.lnk   DosInsertMessage DOSCALLS.386,
 @%append $^&.lnk   DosPutMessage2 DOSCALLS.387,
 @%append $^&.lnk   DosTrueGetMessage2 DOSCALLS.388,
 @%append $^&.lnk   DOSIQUERYMESSAGECP DOSCALLS.389,
 @%append $^&.lnk   DosIQueryMessageCP2 DOSCALLS.390,
 @%append $^&.lnk   DOSCLOSEMESSAGEFILE DOSCALLS.649
 @%append $^&.lnk export
 @%append $^&.lnk   DOSPUTMESSAGE.1,
 @%append $^&.lnk   DOSTRUEGETMESSAGE.2,
 @%append $^&.lnk   DOSINSMESSAGE.3,
 @%append $^&.lnk   DosInsertMessage.4,
 @%append $^&.lnk   DosPutMessage2.5,
 @%append $^&.lnk   DosTrueGetMessage2.6,
 @%append $^&.lnk   DOSIQUERYMESSAGECP.7,
 @%append $^&.lnk   DosIQueryMessageCP2.8,
 @%append $^&.lnk   DOSCLOSEMESSAGEFILE.9
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

install: .SYMBOLIC
 $(CP) $(PROJ).exe ..$(SEP)bin

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
