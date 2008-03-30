# $Id: makefile,v 1.3 2004/08/16 04:51:13 prokushev Exp $


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

DIR = $(ROOT)\bin\os2
PROJ = vdm2os2
DESC = vdm2os2
OBJS = vdm2os2.obj
ADD_COPT = -i=..$(SEP)include

!include $(%ROOT)/mk/all.mk

LINKLIB=%watcom%\lib286\dos\clibs

all: $(PROJ).exe #$(PROJ).sym

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk FORMAT DOS
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)


# Clean target
clean : .symbolic
    @if exist *.obj @del *.obj $(LOG)
    @if exist *.map @del *.map $(LOG)
    @if exist *.wmp @del *.wmp $(LOG)
    @if exist *.lnk @del *.lnk $(LOG)
