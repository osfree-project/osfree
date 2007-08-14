# $Id: makefile,v 1.3 2004/08/16 04:51:21 prokushev Exp $

ROOT=..\..\..

DIR = $(ROOT)\bin\os2\dll
PROJ = msg
DESC = Message files API
OBJS = ..\shared\stub.obj
ADD_COPT = -i=..\include

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).dll $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(DIR)\$(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk option    map
 @%append $^&.lnk option    quiet
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
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
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)

# Clean all

clean: .symbolic
 -@del *.obj $(LOG)
 -@del *.sbr $(LOG)
 -@del *.map $(LOG)
 -@del *.err $(LOG)
 -@del *.wmp $(LOG)
 -@del *.lnk $(LOG)
 -@del *.sym $(LOG)
