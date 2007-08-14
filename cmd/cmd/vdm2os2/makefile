# $Id: makefile,v 1.3 2004/08/16 04:51:13 prokushev Exp $

ROOT=..\..\..\..

DIR = $(ROOT)\bin\os2
PROJ = vdm2os2
DESC = vdm2os2
OBJS = vdm2os2.obj
ADD_COPT = -i=..\include -ox
#ADD_LIB = MATH387S,%watcom%\lib286\os2\os2

!include $(ROOT)\makefile.inc

#LINKOPT=-zq
#-Lr
LINKLIB=%watcom%\lib286\dos\clibs

all: $(DIR)\$(PROJ).exe $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
# %append $^&.lnk $(LINKOPT)
 @%append $^&.lnk FORMAT DOS
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)


# Clean target
clean : .symbolic
    @if exist *.obj @del *.obj $(LOG)
    @if exist *.map @del *.map $(LOG)
    @if exist *.wmp @del *.wmp $(LOG)
    @if exist *.lnk @del *.lnk $(LOG)
