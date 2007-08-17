# $Id: makefile,v 1.3 2004/08/16 04:51:10 prokushev Exp $

ROOT=..\..\..\..

DIR = $(ROOT)\bin\os2
PROJ = cmd
DESC = Command processor
OBJS = main.obj parser.obj os2init.obj cmds.obj expand.obj eval.obj &
           ea.obj dircmds.obj filecmds.obj syscmds.obj os2cmds.obj      &
           os2calls.obj misc.obj env.obj batch.obj screenio.obj msg.obj &
           error.obj window.obj listall.obj lines.obj kparse.obj        &
           iofmt32.obj select.obj os2tty.obj iparse.obj     &
           bcd32.obj os32util.obj ..\shared\istring.obj
ADD_COPT = -i=..\include -ox
ADD_LIB = MATH387S,OS2

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).exe $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).exe: $(OBJS) 4os2.rc
 @%create $^&.lnk
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk FORMAT OS2 LX PMCOMPATIBLE
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)
 @$(RC) 4os2.rc $@ $(LOG)

# Clean target
clean : .symbolic
    @if exist *.obj @del /q *.obj $(LOG)
    @if exist *.res @del /q *.res $(LOG)
    @if exist *.msg @del /q *.msg $(LOG)
    @if exist *.map @del /q *.map $(LOG)
    @if exist *.wmp @del /q *.wmp $(LOG)
    @if exist *.lnk @del /q *.lnk $(LOG)
