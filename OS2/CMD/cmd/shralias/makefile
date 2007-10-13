# $Id: makefile,v 1.3 2004/08/16 04:51:13 prokushev Exp $

#ROOT=..\..\..\..

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk
!include $(%ROOT)/mk/all.mk

DIR = $(ROOT)$(SEP)bin$(SEP)os2
PROJ = shralias
DESC = SHRALIAS.EXE
OBJS = shralias.obj
ADD_COPT = -i=..$(SEP)include -ox
#ADD_LIB = MATH387S,%watcom%$(SEP)lib286$(SEP)os2$(SEP)os2
LINKOPT = lib math387s, os2$(SEP)os2, os2386 LIBPATH $(LIBDIR)

#!include $(ROOT)\makefile.inc

all: $(PROJ).exe # $(PROJ).sym

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk FORMAT OS2 LX PMCOMPATIBLE
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)

#@%append $^&.lnk LIBRARY $(LINKLIB)

# Clean target
clean : .symbolic
    @if exist *.obj @del /q *.obj $(LOG)
    @if exist *.map @del /q *.map $(LOG)
    @if exist *.wmp @del /q *.wmp $(LOG)
    @if exist *.lnk @del /q *.lnk $(LOG)
