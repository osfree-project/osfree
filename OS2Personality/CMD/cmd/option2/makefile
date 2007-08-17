# $Id: makefile,v 1.3 2004/08/16 04:51:12 prokushev Exp $

ROOT=..\..\..\..

DIR = $(ROOT)\bin\os2
PROJ = option2
DESC = OPTION2.EXE
OBJS = option2.obj os2cfg.obj os2nb.obj ..\shared\istring.obj
ADD_COPT = -i=..\include -ox
ADD_LIB = MATH387S,%watcom%\lib286\os2\os2

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).exe $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk FORMAT OS2 LX PM
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)
 @$(RC) -i ..\include option2.rc $@ $(LOG)


# Clean target
clean : .symbolic
    @if exist *.obj @del /q *.obj $(LOG)
    @if exist *.res @del /q *.res $(LOG)
    @if exist *.map @del /q *.map $(LOG)
    @if exist *.wmp @del /q *.wmp $(LOG)
    @if exist *.lnk @del /q *.lnk $(LOG)
